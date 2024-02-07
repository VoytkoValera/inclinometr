#include "esp_log.h"
#include "connection.hpp"
#include "wifi.hpp"
#include "ws_server.hpp"
#include "ble.hpp"

CONNECTION::CONNECTION(connection_mode_t mode){
    _mode = mode
    if (_mode == WS_MODE){
        CONNECTION::start_ws();
    }
    if (_mode == BLE_MODE){
        CONNECTION::start_ble();
    }
}

void CONNECTION::start_ws(){
    WIFI wifi()
}