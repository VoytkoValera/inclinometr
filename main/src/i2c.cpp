/*
 * i2c.cpp
 *
 *  Created on: 24 янв. 2024 г.
 *      Author: Valera
 */
#include "i2c.hpp"
#include "driver/i2c.h"
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "math.h"
#include "esp_log.h"
#define  SERIAL 0


esp_err_t LDC1612_IIC_OPRTS::IIC_write_byte(uint8_t reg, uint8_t byte){
	esp_err_t err = ESP_OK;
	const uint8_t tx_buf[2] = {reg, byte};
	err = i2c_master_write_to_device(_IIC_NUM, _IIC_ADDR, tx_buf, 2, 1000/ portTICK_PERIOD_MS);
	return err;
}

esp_err_t LDC1612_IIC_OPRTS::IIC_write_16bit(uint8_t reg, uint16_t byte){
	esp_err_t err = ESP_OK;
	uint8_t tx_buf[3] = {reg};
	memcpy(tx_buf+1, &byte, 2);
	err = i2c_master_write_to_device(_IIC_NUM, _IIC_ADDR, tx_buf, 2, 1000/ portTICK_PERIOD_MS);
	return err;
}

void LDC1612_IIC_OPRTS::IIC_read_byte(uint8_t reg, uint8_t* byte){
	i2c_master_write_read_device(_IIC_NUM, _IIC_ADDR, &reg, 1, byte, 1, pdMS_TO_TICKS(1000));
}


void LDC1612_IIC_OPRTS::IIC_read_16bit(uint8_t reg, uint16_t* byte){
	i2c_master_write_read_device(_IIC_NUM, _IIC_ADDR, &reg, 1, (uint8_t*)byte, 2, pdMS_TO_TICKS(1000));
}

void LDC1612_IIC_OPRTS::set_iic_addr(uint8_t IIC_ADDR) {
    _IIC_ADDR = IIC_ADDR;
}

void LDC1612_IIC_OPRTS::IIC_begin(){

	esp_err_t status = ESP_OK;
	i2c_config_t _config{};
	//_mode = I2C_MODE_MASTER;

	_config.mode = I2C_MODE_MASTER;
	_config.sda_io_num = IIC_SDA_PIN;
	_config.scl_io_num = IIC_SCL_PIN;
	_config.master.clk_speed = IIC_CLK_SPEED;
	_config.sda_pullup_en = false;
	_config.scl_pullup_en = false;
	_config.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

	status |= i2c_param_config(_IIC_NUM, &_config);
	status |= i2c_driver_install(_IIC_NUM, I2C_MODE_MASTER, 0, 0, 0);
}

//------------------------------------------
const char* TAG = "LDC1612";
void LDC1612::read_sensor_infomation() {
    uint16_t value = 0;
    IIC_read_16bit(READ_MANUFACTURER_ID, &value);
#if SERIAL
    //Serial.print("manufacturer id =0x");
    //Serial.println(value, HEX);
#endif
    ESP_LOGI(TAG, "manufacturer id =%X", value);
    IIC_read_16bit(READ_DEVICE_ID, &value);
#if SERIAL
    //Serial.print("DEVICE id =0x");
    //Serial.println(value, HEX);
#endif
    ESP_LOGI(TAG, "DEVICE id =%X", value);
    return ;
}


/** @brief constructor,support set IIC addr,default iic addr is 0x2b.

 * */
LDC1612::LDC1612(uint8_t IIC_ADDR) {
    set_iic_addr(IIC_ADDR);
}


/** @brief config sensor

 * */
int32_t LDC1612::init() {
    /*Start IIC communication!*/
    IIC_begin();

    /*reset sensor*/
    return 0;
}

int32_t LDC1612::single_channel_config(uint8_t channel) {
    /*Set coil inductor parameter first.*/
    /*20 TURNS*/
    set_Rp(CHANNEL_0, 15.727);
    set_L(CHANNEL_0, 18.147);
    set_C(CHANNEL_0, 100);
    set_Q_factor(CHANNEL_0, 35.97);

    // /*25 TURNS*/
    // set_Rp(CHANNEL_0,24.9);
    // set_L(CHANNEL_0,53.95);
    // set_C(CHANNEL_0,100);
    // set_Q_factor(CHANNEL_0,32.57);

    /*36 TURNS,single layer*/

    // set_Rp(CHANNEL_0,28.18);
    // set_L(CHANNEL_0,18.56);
    // set_C(CHANNEL_0,100);
    // set_Q_factor(CHANNEL_0,43.25);

    /*40 TURNS*/
    // set_Rp(CHANNEL_0,57.46);
    // set_L(CHANNEL_0,85.44);
    // set_C(CHANNEL_0,100);
    // set_Q_factor(CHANNEL_0,40.7);


    if (set_FIN_FREF_DIV(CHANNEL_0)) {
        return -1;
    }

    set_LC_stabilize_time(CHANNEL_0);

    /*Set conversion interval time*/
    set_conversion_time(CHANNEL_0, 0x0546);

    /*Set driver current!*/
    set_driver_current(CHANNEL_0, 0xa000);

    /*single conversion*/
    set_mux_config(0x20c);
    /*start channel 0*/
    uint16_t config = 0x1601;
    select_channel_to_convert(CHANNEL_0, &config);
    set_sensor_config(config);
    return 0;
}

int32_t LDC1612::LDC1612_mutiple_channel_config() {
    /*Set coil inductor parameter first.*/
    /*20 TURNS*/
    set_Rp(CHANNEL_0, 15.727);
    set_L(CHANNEL_0, 18.147);
    set_C(CHANNEL_0, 100);
    set_Q_factor(CHANNEL_0, 35.97);

    /*25 TURNS*/
    set_Rp(CHANNEL_1, 15.727);
    set_L(CHANNEL_1, 18.147);
    set_C(CHANNEL_1, 100);
    set_Q_factor(CHANNEL_1, 35.97);

    // /*36 TURNS single layer*/
    // set_Rp(CHANNEL_0,28.18);
    // set_L(CHANNEL_0,18.56);
    // set_C(CHANNEL_0,100);
    // set_Q_factor(CHANNEL_0,43.25);

    // /*40 TURNS*/
    // set_Rp(CHANNEL_1,57.46);
    // set_L(CHANNEL_1,85.44);
    // set_C(CHANNEL_1,100);
    // set_Q_factor(CHANNEL_0,40.7);


    if (set_FIN_FREF_DIV(CHANNEL_0)) {
        return -1;
    }
    set_FIN_FREF_DIV(CHANNEL_1);

    /* 16*38/Fref=30us ,wait 30us for LC sensor stabilize before initiation of a conversion.*/
    set_LC_stabilize_time(CHANNEL_0);
    set_LC_stabilize_time(CHANNEL_1);

    /*Set conversion interval time*/
    set_conversion_time(CHANNEL_0, 0x0546);
    set_conversion_time(CHANNEL_1, 0x0546);

    /*Set driver current!*/
    set_driver_current(CHANNEL_0, 0xa000);
    set_driver_current(CHANNEL_1, 0xa000);


    /*mutiple conversion*/
    set_mux_config(0x820c);
    //set_mux_config(0x20c);
    /*start channel 0*/
    set_sensor_config(0x1601);
    //uint16_t config=0x1601;
    //select_channel_to_convert(0,&config);
    return 0;
}




/** @brief parse the data which read from data register.
    @param channel LDC1612 has total two channels.
    @param raw_result the raw data which read from data register,it contains error codes and sensor value;
 * */
int32_t LDC1612::parse_result_data(uint8_t channel, uint32_t raw_result, uint32_t* result) {
    uint8_t value = 0;
    *result = raw_result & 0x0fffffff;
    if (0xfffffff == *result) {
        ESP_LOGW(TAG, "can't detect coil Coil Inductance!!!");
        *result = 0;
        return -1;
    }
    // else if(0==*result)
    // {
    //     Serial.println("result is none!!!");
    // }
    value = raw_result >> 24;
    if (value & 0x80) {
#if SERIAL
        Serial.print("channel ");
        Serial.print(channel);
        Serial.println(": ERR_UR-Under range error!!!");
#endif
        ESP_LOGE(TAG, "channel %d: ERR_UR-Under range error!!!", channel);
    }
    if (value & 0x40) {
#if SERIAL
        Serial.print("channel ");
        Serial.print(channel);
        Serial.println(": ERR_OR-Over range error!!!");
#endif
        ESP_LOGE(TAG, "channel %d: ERR_OR-Over range error!!!", channel);
    }
    if (value & 0x20) {
#if SERIAL
        Serial.print("channel ");
        Serial.print(channel);
        Serial.println(": ERR_WD-Watch dog timeout error!!!");
#endif
        ESP_LOGE(TAG, "channel %d: ERR_WD-Watch dog timeout error!!!", channel);
    }
    if (value & 0x10) {
#if SERIAL
        Serial.print("channel ");
        Serial.print(channel);
        Serial.println(": ERR_AE-error!!!");
#endif
        ESP_LOGE(TAG, "channel %d: ERR_AE-error!!!", channel);
    }
    return 0;
}


/** @brief read the raw channel result from register.
    @param channel LDC1612 has total two channels.
    @param result raw data
 * */
int32_t LDC1612::get_channel_result(uint8_t channel, uint32_t* result) {
    uint32_t raw_value = 0;
    if (NULL == result) {
        return -1;
    }
    uint16_t value = 0;
    IIC_read_16bit(CONVERTION_RESULT_REG_START + channel * 2, &value);
    raw_value |= (uint32_t)value << 16;
    IIC_read_16bit(CONVERTION_RESULT_REG_START + channel * 2 + 1, &value);
    raw_value |= (uint32_t)value;
    parse_result_data(channel, raw_value, result);
    return 0;
}

/** @brief set conversion interval time.
    @param channel LDC1612 has total two channels.
    @param result The value to be set.
 * */
int32_t LDC1612::set_conversion_time(uint8_t channel, uint16_t value) {
    return IIC_write_16bit(SET_CONVERSION_TIME_REG_START + channel, value);
}


/** @brief set conversion offset.
    @param channel LDC1612 has total two channels.
    @param result The value to be set.
 * */
int32_t LDC1612::set_conversion_offset(uint8_t channel, uint16_t value) {
    return IIC_write_16bit(SET_CONVERSION_OFFSET_REG_START + channel, value);
}

/** @brief Before conversion,wait LC sensor stabilize for a short time.
    @param channel LDC1612 has total two channels.
    @param result The value to be set.
 * */
int32_t LDC1612::set_LC_stabilize_time(uint8_t channel) {
    uint16_t value = 0;
    value = 30;
    return IIC_write_16bit(SET_LC_STABILIZE_REG_START + channel, value);
}


/** @brief set input frequency divide and fref divide.
    @param channel LDC1612 has total two channels.
    @param FIN_DIV FIN input divide
    @param FREF_DIV fref,reference frequency of sensor.
 * */
int32_t LDC1612::set_FIN_FREF_DIV(uint8_t channel) {
    uint16_t value;
    uint16_t FIN_DIV, FREF_DIV;

    Fsensor[channel] = 1 / (2 * 3.14 * sqrt(inductance[channel] * capacitance[channel] * pow(10, -18))) * pow(10, -6);
#if SERIAL
    Serial.print("fsensor =");
    Serial.println(Fsensor[channel]);
#endif
    ESP_LOGI(TAG, "fsensor = %f", Fsensor[channel]);


    FIN_DIV = (uint16_t)(Fsensor[channel] / 8.75 + 1);


    if (Fsensor[channel] * 4 < 40) {
        FREF_DIV = 2;
        Fref[channel] = 40 / 2;
    } else {
        FREF_DIV = 4;
        Fref[channel] = 40 / 4;
    }

    value = FIN_DIV << 12;
    value |= FREF_DIV;
    return IIC_write_16bit(SET_FREQ_REG_START + channel, value);
}


/** @brief Error output config.
    @param result The value to be set.
 * */
int32_t LDC1612::set_ERROR_CONFIG(uint16_t value) {
    return IIC_write_16bit(ERROR_CONFIG_REG, value);
}



/** @brief mux  config.
    @param result The value to be set.
 * */
int32_t LDC1612::set_mux_config(uint16_t value) {
    return IIC_write_16bit(MUL_CONFIG_REG, value);
}

/** @brief reset sensor.

 * */
int32_t LDC1612::reset_sensor() {
    return IIC_write_16bit(SENSOR_RESET_REG, 0x8000);
}

/** @brief set drive current of sensor.
    @param result The value to be set.
 * */
int32_t LDC1612::set_driver_current(uint8_t channel, uint16_t value) {
    return IIC_write_16bit(SET_DRIVER_CURRENT_REG + channel, value);
}


/** @brief Main config part of sensor.Contains select channel、start conversion、sleep mode、sensor activation mode、INT pin disable ..
    @param result The value to be set.
 * */
int32_t LDC1612::set_sensor_config(uint16_t value) {
    return IIC_write_16bit(SENSOR_CONFIG_REG, value);
}


/** @brief select channel to convert

 * */
void LDC1612::select_channel_to_convert(uint8_t channel, uint16_t* value) {
    switch (channel) {
        case 0: *value &= 0x3fff;
            break;
        case 1: *value &= 0x7fff;
            *value |= 0x4000;
            break;
        case 2: *value &= 0xbfff;
            *value |= 0x8000;
            break;
        case 3: *value |= 0xc000;
            break;
    }
}


void LDC1612::set_Rp(uint8_t channel, float n_kom) {
    resistance[channel] = n_kom;
}

void LDC1612::set_L(uint8_t channel, float n_uh) {
    inductance[channel] = n_uh;
}

void LDC1612::set_C(uint8_t channel, float n_pf) {
    capacitance[channel] = n_pf;
}


void LDC1612::set_Q_factor(uint8_t channel, float q) {
    Q_factor[channel] = q;
}


const char* status_str[] = {"conversion under range error", "conversion over range error",
                            "watch dog timeout error", "Amplitude High Error",
                            "Amplitude Low Error", "Zero Count Error",
                            "Data Ready", "unread conversion is present for channel 0",
                            " unread conversion is present for Channel 1.",
                            "unread conversion ispresent for Channel 2.",
                            "unread conversion is present for Channel 3."
                           };


/** @brief parse sensor statu data.

 * */
void LDC1612::sensor_status_parse(uint16_t value) {
    uint16_t section = 0;
    section = value >> 14;
    switch (section) {
        case 0: ESP_LOGI(TAG, "Channel 0 is source of flag or error.");
            break;
        case 1: ESP_LOGI(TAG, "Channel 1 is source of flag or error.");
            break;
        /*Only support LDC1614*/
        case 2: ESP_LOGI(TAG, "Channel 2 is source of flag or error.");
            break;
        case 3: ESP_LOGI(TAG, "Channel 3 is source of flag or error.");
            break;
        default:
            break;
    }
    for (uint32_t i = 0; i < 6; i++) {
        if (value & (uint16_t)1 << (8 + i)) {
        	ESP_LOGI(TAG, "%s", status_str[6 - i]);
        }
    }
    if (value & (1 << 6)) {
    	ESP_LOGI(TAG, "%s", status_str[6]);
    }
    for (uint32_t i = 0; i < 4; i++) {
        if (value & (1 << i)) {
        	ESP_LOGI(TAG, "%s", status_str[10 - i]);
        }
    }
}

/** @brief get sensor status

 * */
uint32_t LDC1612::get_sensor_status() {
    uint16_t value = 0;
    IIC_read_16bit(SENSOR_STATUS_REG, &value);

    // Serial.print("status =");
    // Serial.println(value,HEX);

    sensor_status_parse(value);
    return value;
}

