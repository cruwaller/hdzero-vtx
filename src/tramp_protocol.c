#include "tramp_protocol.h"
#include "dm6300.h"
#include "global.h"
#include "hardware.h"
#include "msp_displayport.h"
#include "print.h"

uint8_t tr_tx_busy = 0;
uint8_t tramp_lock = 0;

#ifdef USE_TRAMP
static trampReceiveState_e trampReceiveState = S_WAIT_LEN;
static uint8_t tbuf[16];
static uint8_t rbuf[16];
static uint8_t r_ptr;

// Calculate tramp protocol checksum of provided buffer
static uint8_t tramp_checksum(uint8_t *buf) {
    uint8_t crc = 0;
    uint8_t i;

    for (i = 1; i < 14; i++) {
        crc += buf[i];
    }

    return crc;
}

void tramp_reset_receive(void) {
    trampReceiveState = S_WAIT_LEN;
    r_ptr = 0;
}

void trampResponse(void) {
    uint8_t i;

    tbuf[14] = tramp_checksum(tbuf);
    tbuf[15] = 0x00;

    tr_tx_busy = 1;

    for (i = 0; i < 16; i++) {
        tr_tx(tbuf[i]);
    }

    WAIT(1);
    tr_tx_busy = 0;
    tr_read(); // fix me!
}

static uint16_t get_freq(void) {
    uint16_t freq = 0;
    switch (RF_FREQ) {
    case 0:
        freq = 5658;
        break;
    case 1:
        freq = 5695;
        break;
    case 2:
        freq = 5732;
        break;
    case 3:
        freq = 5769;
        break;
    case 4:
        freq = 5806;
        break;
    case 5:
        freq = 5843;
        break;
    case 6:
        freq = 5880;
        break;
    case 7:
        freq = 5917;
        break;
    case 8:
        freq = 5760;
        break;
    case 9:
        freq = 5800;
        break;
    default:
        break;
    }
    return freq;
}

static void set_freq(uint16_t freq) {
    uint8_t ch = 0xff;
    switch (freq) {
    case 5658:
        ch = 0;
        break;
    case 5695:
        ch = 1;
        break;
    case 5732:
        ch = 2;
        break;
    case 5769:
        ch = 3;
        break;
    case 5806:
        ch = 4;
        break;
    case 5843:
        ch = 5;
        break;
    case 5880:
        ch = 6;
        break;
    case 5917:
        ch = 7;
        break;
    case 5760:
        ch = 8;
        break;
    case 5800:
        ch = 9;
        break;
    default:
        break;
    }

    if (ch != 0xff) {
        RF_FREQ = ch;
        if (dm6300_init_done) {
            DM6300_SetChannel(RF_FREQ);
        }
#ifdef _DEBUG_TRAMP
        _outchar('0' + RF_FREQ);
#endif
    }
}

static uint16_t get_power(void) {
    uint16_t power = 0;

    switch (RF_POWER) {
    case 0:
        power = 25;
        break;
    case 1:
        power = 200;
        break;
    case 2:
        power = 0;
        break;
    default:
        power = 0;
        break;
    }

    return power;
}

static void set_power(uint16_t power) {
    switch (power) {
    case 0:
        RF_POWER = POWER_0mW;
        break;
    case 1:
        RF_POWER = POWER_PIT;
        break;

    case 14: // dBm
    case 25: // mw
        RF_POWER = POWER_25mW;
        break;

    case 23: // dBm
    case 100:
    case 200:
    case 350:
    case 1000:
        RF_POWER = POWER_200mW;
        break;

    default:
        break;
    }
    if (RF_POWER == POWER_0mW) {
        // enter 0mw
        cur_pwr = RF_POWER;
        vtx_pit = PIT_0MW;

        DM6300_powerOff();
        temp_err = 1;
    } else {
        SetPower_6300RF(RF_POWER, RF_FREQ, pwr_offset);
        cur_pwr = RF_POWER;
        if (RF_POWER == POWER_PIT)
            vtx_pit = PIT_P1MW;
    }
}

// Process response and return code if valid else 0
static uint8_t tramp_reply(void) {
    const uint8_t respCode = rbuf[1];
    uint16_t freq = get_freq();
    uint16_t power = get_power();
    const uint8_t locked = 0;
    uint8_t pitmode = 0;

#ifdef _DEBUG_TRAMP
    _outchar('\r');
    _outchar('\n');
    _outchar('<');
    _outchar(respCode);
#endif
    memset(tbuf, 0x00, 16);
    switch (respCode) {
    case 'r': {
        tbuf[0] = 0x0f;
        tbuf[1] = 'r';
        tbuf[2] = 5658 & 0xff;
        tbuf[3] = 5658 >> 8;
        tbuf[4] = 5917 & 0xff;
        tbuf[5] = 5917 >> 8;
        tbuf[6] = 200 & 0xff;
        tbuf[7] = 200 >> 8;

        trampResponse();
#ifdef _DEBUG_TRAMP
        _outchar('>');
#endif
        return respCode;
    }

    case 'v': {
        tbuf[0] = 0x0f;
        tbuf[1] = 'v';
        tbuf[2] = freq & 0xff;
        tbuf[3] = freq >> 8;
        tbuf[4] = power & 0xff;
        tbuf[5] = power >> 8;
        tbuf[6] = locked;
        tbuf[7] = pitmode;
        tbuf[8] = power & 0xff;
        tbuf[9] = power >> 8;

        trampResponse();
#ifdef _DEBUG_TRAMP
        _outchar('>');
#endif
        return respCode;
    }

    case 'F': {
        freq = ((uint16_t)rbuf[2] & 0xff) | ((uint16_t)rbuf[3] << 8);
        set_freq(freq);
#ifdef _DEBUG_TRAMP
        _outchar('>');
#endif
        return respCode;
    }

    case 'P': {
        power = ((uint16_t)rbuf[2] & 0xff) | ((uint16_t)rbuf[3] << 8);
        set_power(power);
#ifdef _DEBUG_TRAMP
        _outchar('0' + RF_POWER);
        _outchar('>');
#endif
        return respCode;
    }

    case 's': {
        tbuf[0] = 0x0f;
        tbuf[1] = 's';
        tbuf[6] = 0x01;
        tbuf[7] = 0x00;

        trampResponse();
#ifdef _DEBUG_TRAMP
        _outchar('>');
#endif
        return respCode;
    }

    default:
        break;
    }
    return 0;
}
// returns completed response code or 0

void tramp_receive(void) {
    uint8_t rdata = 0;
    while (tr_ready()) {
        rdata = tr_read();
        rbuf[r_ptr++] = rdata;

        switch (trampReceiveState) {
        case S_WAIT_LEN: {
            if (rdata == 0x0F) {
                // Found header byte, advance to wait for code
                trampReceiveState = S_WAIT_CODE;
            } else {
                // Unexpected header, reset state machine
                tramp_reset_receive();
            }
            break;
        }
        case S_WAIT_CODE: {

            trampReceiveState = S_DATA;
            break;
        }
        case S_DATA: {
            if (r_ptr == 16) {
                // Buffer is full, calculate checksum
                if ((rbuf[14] == tramp_checksum(rbuf)) && (rbuf[15] == 0)) {
                    tramp_lock = 1;
                    tramp_reply();
                }
#ifdef _DEBUG_TRAMP
                else {
                    _outchar('?');
                    _outchar(rbuf[1]);
                }
#endif

                // Reset state machine ready for next response
                tramp_reset_receive();
            }
            break;
        }
        default: {
            // Invalid state, reset state machine
            tramp_reset_receive();
            break;
        }
        }
    }
}

void tramp_init(void) {
    uint16_t time_ms = 3000;
    RF_POWER = POWER_0mW;
    while (time_ms--) {
        timer_task();
        tramp_receive();
    }
    if (!tramp_lock)
        RF_POWER = I2C_Read8(ADDR_EEPROM, EEP_ADDR_RF_POWER);
}
#endif