#ifndef __HARDWARE_H_
#define __HARDWARE_H_

#include "common.h"
#include "i2c.h"
#include "i2c_device.h"
#include "stdint.h"

typedef enum {
    ON,
    OFF,
} ledType_e;

typedef enum {
    BW_27M,
    BW_17M
} BWType_e;

#define PWR_DEFAULT 2
#define SPARKLE_T   20

// eeprom parameter
#define EEP_ADDR_CAM_TYPE    0x40
#define EEP_ADDR_CAM_PROFILE 0x41
// profile 1: 0x42~0x51
// profile 2: 0x52~0x61
// profile 2: 0x62~0x71
#define EEP_ADDR_CAM_SETTING 0x42

#define EEP_ADDR_RF_FREQ      0x80
#define EEP_ADDR_RF_POWER     0x81
#define EEP_ADDR_LPMODE       0x82
#define EEP_ADDR_PITMODE      0x83
#define EEP_ADDR_25MW         0x84
#define EEP_ADDR_TEAM_RACE    0x85
#define EEP_ADDR_SA_LOCK      0x88
#define EEP_ADDR_POWER_LOCK   0x89
#define EEP_ADDR_VTX_CONFIG   0x8A
#define EEP_ADDR_BAUDRATE     0x8B
#define EEP_ADDR_LOWBAND_LOCK 0x8C
#define EEP_ADDR_DCOC_EN      0xC0
#define EEP_ADDR_DCOC_IH      0xC1
#define EEP_ADDR_DCOC_IL      0xC2
#define EEP_ADDR_DCOC_QH      0xC3
#define EEP_ADDR_DCOC_QL      0xC4
#define EEP_ADDR_LIFETIME_0   0xF0
#define EEP_ADDR_LIFETIME_1   0xF1
#define EEP_ADDR_LIFETIME_2   0xF2
#define EEP_ADDR_LIFETIME_3   0xF3
#define EEP_ADDR_EEP_VLD      0xFF

#define FREQ_NUM_INTERNAL 8
#define FREQ_NUM_EXTERNAL 18
extern uint8_t lowband_lock;
#define FREQ_NUM (lowband_lock ? 10 : 18)

enum {
    POWER_25mW = 0,
    POWER_200mW,
#if defined HDZERO_FREESTYLE || HDZERO_FREESTYLE_V2
    POWER_500mW,
    POWER_1000mW,
#endif
    POWER_PIT,
    POWER_0mW, // power off
};

enum {
    PIT_OFF,
    PIT_P1MW,
    PIT_0MW,
};

enum {
    LP_MODE_OFF,
    LP_MODE_ALWAYS,
    LP_MODE_UNTIL_FIRST_ARM,
};

enum {
    TEAM_RACE_OFF,
    TEAM_RACE_MODE1,
    TEAM_RACE_MODE2,
};

#ifdef USE_TC3587_LED
#define LED_BLUE_ON  I2C_Write16(ADDR_TC3587, 0x0014, 0x0000)
#define LED_BLUE_OFF I2C_Write16(ADDR_TC3587, 0x0014, 0x8000)
#else
#define LED_BLUE_ON  LED_1 = ON
#define LED_BLUE_OFF LED_1 = OFF
#endif

void Init_HW();
void OnButton1();
void video_detect();
void PwrLMT();
void TempDetect();

void Init_6300RF(uint8_t const freq, uint8_t const pwr);
void InitAndCalibrate_6300RF(uint8_t const freq, uint8_t const pwr);
void SetPower_6300RF(uint8_t const freq, uint8_t const pwr, uint8_t const offset);

void GetVtxParameter();
void Setting_Save();
void Imp_RF_Param();
void CFG_Back();

void Set_720P50(uint8_t page);
void Set_720P60(uint8_t page);
void Set_540P90(uint8_t page);
void Set_540P90_crop(uint8_t page);
void Set_960x720P60(uint8_t page);
void Set_720P30(uint8_t page, uint8_t is_43);
void Set_540P60(uint8_t page);
void Set_1080P30(uint8_t page);

void Flicker_LED(uint8_t n);
void LED_Flip();
void LED_Task();

uint8_t RF_BW_check(void);
void uart_baudrate_detect(void);
uint8_t temperature_level(void);
void vtx_paralized(void);

void timer_task();

#if defined HDZERO_FREESTYLE || HDZERO_FREESTYLE_V2
extern uint8_t powerLock;
#endif
extern uint8_t RF_FREQ;
extern uint8_t RF_POWER;
extern uint8_t LP_MODE;
extern uint8_t PIT_MODE;
extern uint8_t OFFSET_25MW;
extern uint8_t TEAM_RACE;
extern uint8_t KEYBOARD_ON;
extern uint8_t EE_VALID;
extern uint8_t RF_BW;
extern uint8_t RF_BW_last;
extern uint8_t BAUDRATE;

extern uint8_t pwr_offset;
extern uint8_t heat_protect;

extern uint8_t fc_lock;
extern uint8_t vtx_pit;
extern uint8_t vtx_pit_save;
extern uint8_t SA_lock;
extern uint8_t pwr_lmt_sec;
extern uint8_t pwr_lmt_done;

extern uint8_t cur_pwr;

extern uint8_t last_SA_lock;
extern uint8_t pwr_init;
extern uint8_t ch_init;

extern uint8_t led_status;

extern uint8_t dispF_cnt;
extern uint8_t dispL_cnt;
extern uint8_t temp_err;
extern uint8_t rf_delay_init_done;

extern int16_t temperature;
#endif /* __HARDWARE_H_ */
