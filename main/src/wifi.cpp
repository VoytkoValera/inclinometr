/*
 * wifi.cpp
 *
 *  Created on: 4 февр. 2024 г.
 *      Author: Valera
 */

#include "wifiStack.h"
#include "wifi.hpp"

#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"

#include <string>

static const char* TAG = "WIFI";

void WIFI::get_nvs(){
	static esp_err_t err;
	err = nvs_open("storage", NVS_READWRITE, WIFI::_nvs_descriptor);
	if (err!= ESP_OK){
		err = nvs_flash_init();
		if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
		{
			ESP_ERROR_CHECK(nvs_flash_erase());
			nvs_flash_init();
		}
		nvs_open("storage", NVS_READWRITE, WIFI::_nvs_descriptor);
	}
}

bool WIFI::check_mode(){
	static esp_err_t err;
	err = nvs_get_u8(*_nvs_descriptor, "wifi_mode", NULL);
	if (err == ESP_ERR_NVS_NOT_FOUND){
		return false;
	}
	return true;
}

bool WIFI::check_ssid(){
	static esp_err_t err;
	err = nvs_get_str(*_nvs_descriptor, "wifi_ssid", NULL, NULL);
	if (err == ESP_ERR_NVS_NOT_FOUND){
			return false;
		}
		return true;
}

bool WIFI::check_passwd(){
	static esp_err_t err;
	err = nvs_get_str(*_nvs_descriptor, "wifi_passwd", NULL, NULL);
	if (err == ESP_ERR_NVS_NOT_FOUND){
			return false;
		}
		return true;
}

wifi_interface_t WIFI::get_mode(){
	wifi_interface_t mode = WIFI_IF_AP;
	static esp_err_t err;
	err = nvs_get_u8(*_nvs_descriptor, "wifi_mode", (uint8_t*)&mode);
	switch(err){
		case ESP_OK:
			//ESP_LOGI(TAG,"mode was found");
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			// add log
			break;
		default:
			//ESP_LOGW(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
			break;
	}
		return mode;
}

std::string WIFI::get_ssid(){
	char* ssid = "";
	size_t nvs_required_size;
	static esp_err_t err;
	err = nvs_get_str(*_nvs_descriptor, "wifi_ssid", NULL, &nvs_required_size);
	if (err == ESP_OK){
		ssid = (char*)malloc(nvs_required_size);
		err = nvs_get_str(*_nvs_descriptor, "wifi_ssid", ssid, &nvs_required_size );
	}
	std::string password(ssid);
	return password;
}

std::string WIFI::get_passwd(){
	char* passwd = "";
	size_t nvs_required_size;
	static esp_err_t err;
	err = nvs_get_str(*_nvs_descriptor, "wifi_passwd", NULL, &nvs_required_size);
	if (err == ESP_OK){
		passwd = (char*)malloc(nvs_required_size);
		err = nvs_get_str(*_nvs_descriptor, "wifi_passwd", passwd, &nvs_required_size );
	}
	std::string password(passwd);
	return password;
}

void WIFI::set_mode(wifi_interface_t mode){
	nvs_set_u8(*_nvs_descriptor, "wifi_mode", (uint8_t)mode);
	_mode = mode;
}

void WIFI::set_ssid(std::string ssid){
	nvs_set_str(*_nvs_descriptor, "wifi_mode", ssid.c_str());
	_ssid = ssid;
}

void WIFI::set_passwd(std::string passwd){
	nvs_set_str(*_nvs_descriptor, "wifi_mode", passwd.c_str());
	_passwd = passwd;
}

void WIFI::init_wifi(){
	WIFI::get_nvs();
	if (WIFI::check_mode()){
		_mode = get_mode();
	}
	else {WIFI::set_mode((wifi_interface_t)DEFAULT_WIFI_MODE);}
	if (WIFI::check_ssid()){
		_ssid = get_ssid();
	}
	else WIFI::set_ssid(DEFAULT_WIFI_SSID);
	if (WIFI::check_passwd()){
		_ssid = get_passwd();
	}else WIFI::set_passwd(DEFAULT_WIFI_PASSWD);

	if(_mode == WIFI_IF_AP ){
		ESP_ERROR_CHECK(startWifiAccessPoint(_ssid.c_str(), _passwd.c_str()));
	}
	else if (_mode == WIFI_IF_STA){
		ESP_ERROR_CHECK(startWifiStation(_ssid.c_str(), _passwd.c_str()));
	}

}
