set (CMAKE_CXX_STANDARD 20)
set (COMPONENT_DIRS ${PROJECT_DIR}/components)
idf_component_register(SRCS "main.cpp" "src/i2c.cpp" "src/ble.cpp" "src/wifi.cpp" "src/ws_server"
                    INCLUDE_DIRS "include")

