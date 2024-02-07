/*
 * wifi.hpp
 *
 *  Created on: 4 февр. 2024 г.
 *      Author: Valera
 */

#ifndef MAIN_INCLUDE_WIFI_HPP_
#define MAIN_INCLUDE_WIFI_HPP_

#include "nvs_flash.h"
#include "esp_wifi.h"
#include <string>

#define DEFAULT_WIFI_MODE 1
#define DEFAULT_WIFI_SSID "esp32"
#define DEFAULT_WIFI_PASSWD "hohhohohohh"

class WIFI{
private:
	void get_nvs();
	nvs_handle_t* _nvs_descriptor;
	bool check_mode();
	bool check_ssid();
	bool check_passwd();
public:
	void init_wifi();
	wifi_interface_t _mode;
	std::string _passwd, _ssid;
	wifi_interface_t get_mode();
	std::string get_ssid();
	std::string get_passwd();

	void set_mode(wifi_interface_t mode);
	void set_ssid(std::string ssid);
	void set_passwd(std::string passwd);
};



#endif /* MAIN_INCLUDE_WIFI_HPP_ */
