/*
 * connection.hpp
 *
 *  Created on: 24 янв. 2024 г.
 *      Author: Valera
 */

#ifndef MAIN_INCLUDE_BLE_HPP_
#define MAIN_INCLUDE_BLE_HPP_

#include <cJSON.h>

typedef enum{
    WS_MODE = 1,
    BLE_MODE = 2
}connection_mode_t;

class CONNECTION
{
private:
    connection_mode_t _mode
    void start_ws();
    void start_ble();
    void get_data_cb(cJSON msg);
    void send(cJSON msg);
public:
    CONNECTION(connection_mode_t mode);
    ~CONNECTION();
};



#endif