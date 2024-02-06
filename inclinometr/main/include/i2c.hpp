/*
 * i2c.hpp
 *
 *  Created on: 24 янв. 2024 г.
 *      Author: Valera
 */

#ifndef MAIN_INCLUDE_I2C_HPP_
#define MAIN_INCLUDE_I2C_HPP_
#ifdef __cplusplus
extern "C" {
#endif

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include  "sdkconfig.h"

#define IIC_SDA_PIN CONFIG_IIC_SDA_PIN
#define IIC_SCL_PIN CONFIG_IIC_SCL_PIN
#define IIC_CLK_SPEED CONFIG_IIC_CLK_SPEED

#define DEFAULT_IIC_ADDR  0x2B

/*Register Rddr*/
/***************************************************************************/
#define CONVERTION_RESULT_REG_START             0X00
#define SET_CONVERSION_TIME_REG_START           0X08
#define SET_CONVERSION_OFFSET_REG_START         0X0C
#define SET_LC_STABILIZE_REG_START              0X10
#define SET_FREQ_REG_START                      0X14

#define SENSOR_STATUS_REG                       0X18
#define ERROR_CONFIG_REG                        0X19
#define SENSOR_CONFIG_REG                       0X1A
#define MUL_CONFIG_REG                          0X1B
#define SENSOR_RESET_REG                        0X1C
#define SET_DRIVER_CURRENT_REG                  0X1E

#define READ_MANUFACTURER_ID                    0X7E
#define READ_DEVICE_ID                          0X7F

/******************************************************************************/

/*ERROR bit*/
#define UR_ERR2OUT                              ((uint16_t)1<<15)
#define OR_ERR2OUT                              ((uint16_t)1<<14)
#define WD_ERR2OUT                              ((uint16_t)1<<13)
#define AH_ERR2OUT                              ((uint16_t)1<<12)
#define AL_ERR2OUT                              ((uint16_t)1<<11)
#define UR_ERR2INT                              ((uint16_t)1<<7)
#define OR_ERR2INT                              ((uint16_t)1<<6)
#define WD_ERR2INT                              ((uint16_t)1<<5)
#define AH_ERR2INT                              ((uint16_t)1<<4)
#define AL_ERR2INT                              ((uint16_t)1<<3)
#define ZC_ERR2INT                              ((uint16_t)1<<2)
#define DRDY_2INT                               ((uint16_t)1<<0)

/******************************************************************************/


/*SENSOR CONFIG bit*/
#define ACTIVE_CHANNEL                         ( ((uint16_t)1<<15) | ((uint16_t)1<<14) )
#define SLEEP_MODE_EN                           ((uint16_t)1<<13)
#define RP_OVERRIDE_EN                          ((uint16_t)1<<12)
#define SENSOR_ACTIVATE_SEL                     ((uint16_t)1<<11)
#define AUTO_AMP_DIS                            ((uint16_t)1<<10)
#define REF_CLK_SRC                             ((uint16_t)1<<9)
#define INTB_DIS                                ((uint16_t)1<<7)
#define HIGH_CURRENT_DRV                        ((uint16_t)1<<6)
/*! !!!
    The low bit 0~5 are reserved,must set to 000001.
*/


#define CHANNEL_0  0
#define CHANNEL_1  1


#define CHANNEL_NUM  2

class LDC1612_IIC_OPRTS {
public:
	void IIC_begin(void);
	esp_err_t IIC_write_byte(uint8_t reg, uint8_t byte);
	void IIC_read_byte(uint8_t reg, uint8_t* byte);
	void set_iic_addr(uint8_t IIC_ADDR);
	void IIC_read_16bit(uint8_t start_reg, uint16_t* value);
	esp_err_t IIC_write_16bit(uint8_t reg, uint16_t value);
private:
	uint8_t _IIC_ADDR;
	i2c_port_t _IIC_NUM;
};

class LDC1612: public LDC1612_IIC_OPRTS {
  public:
	LDC1612(uint8_t IIC_ADDR = DEFAULT_IIC_ADDR);
    ~LDC1612() {}
    int32_t init();
    void read_sensor_infomation();
    int32_t get_channel_result(uint8_t channel, uint32_t* result);
    int32_t set_conversion_time(uint8_t channel, uint16_t value);
    int32_t set_LC_stabilize_time(uint8_t channel);
    int32_t set_conversion_offset(uint8_t channel, uint16_t value);
    uint32_t get_sensor_status();
    int32_t set_ERROR_CONFIG(uint16_t value);
    int32_t set_sensor_config(uint16_t value);
    int32_t set_mux_config(uint16_t value);
    int32_t reset_sensor();
    int32_t set_driver_current(uint8_t channel, uint16_t value);
    int32_t set_FIN_FREF_DIV(uint8_t channel);
    void select_channel_to_convert(uint8_t channel, uint16_t* value);
    int32_t single_channel_config(uint8_t channel);
    /*Total two channels*/
    int32_t LDC1612_mutiple_channel_config();

    void set_Rp(uint8_t channel, float n_kom);
    void set_L(uint8_t channel, float n_uh);
    void set_C(uint8_t channel, float n_pf);
    void set_Q_factor(uint8_t channel, float q);
  private:
    int32_t parse_result_data(uint8_t channel, uint32_t raw_result, uint32_t* result);
    void sensor_status_parse(uint16_t value);
    float resistance[CHANNEL_NUM];
    float inductance[CHANNEL_NUM];
    float capacitance[CHANNEL_NUM];
    float Fref[CHANNEL_NUM];
    float Fsensor[CHANNEL_NUM];
    float Q_factor[CHANNEL_NUM];
};

#ifdef __cplusplus
}
#endif
#endif /* MAIN_INCLUDE_I2C_HPP_ */
