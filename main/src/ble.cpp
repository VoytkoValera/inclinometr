/*
 * ble.cpp
 *
 *  Created on: 24 янв. 2024 г.
 *      Author: Valera
 */
#include <NimBLEDevice.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <cstdint>
#include <cstdio>
#include "ble.hpp"

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer, BLEConnInfo& connInfo) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer, BLEConnInfo& connInfo, int reason) {
      deviceConnected = false;
    }
  /***************** New - Security handled here ********************
  ****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest(){
      std::printf("Server PassKeyRequest\n");
      return 123456;
    }

    bool onConfirmPIN(uint32_t pass_key){
      printf("The passkey YES/NO number: %" PRIu32"\n", pass_key);
      return true;
    }

    void onAuthenticationComplete(BLEConnInfo& connInfo){
      printf("Starting BLE work!\n");
    }
  /*******************************************************************/
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic, BLEConnInfo& connInfo) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        printf("*********\n");
        printf("%s\n", rxValue.c_str());
        printf("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          printf("%d", rxValue[i]);

        printf("\n*********\n");
      }
    }
};

void connectedTask (void * parameter){
    for(;;) {
        if (deviceConnected) {
            pTxCharacteristic->setValue(&txValue, 1);
            pTxCharacteristic->notify();
            txValue++;
        }

        // disconnecting
        if (!deviceConnected && oldDeviceConnected) {
            pServer->startAdvertising(); // restart advertising
            printf("start advertising\n");
            oldDeviceConnected = deviceConnected;
        }
        // connecting
        if (deviceConnected && !oldDeviceConnected) {
            // do stuff here on connecting
            oldDeviceConnected = deviceConnected;
        }

        vTaskDelay(10/portTICK_PERIOD_MS); // Delay between loops to reset watchdog timer
    }

    vTaskDelete(NULL);
}

void start_ble_uart(void){
	BLEDevice::init("UART Service");

	// Create the BLE Server
	pServer = BLEDevice::createServer();
	NimBLEAdvertising();
	pServer->setCallbacks(new MyServerCallbacks());

	// Create the BLE Service
	BLEService *pService = pServer->createService(SERVICE_UUID);

	// Create a BLE Characteristic
	pTxCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_UUID_TX,
									/******* Enum Type NIMBLE_PROPERTY now *******
										BLECharacteristic::PROPERTY_NOTIFY
										);
									**********************************************/
										NIMBLE_PROPERTY::NOTIFY
									   );

	/***************************************************
	NOTE: DO NOT create a 2902 descriptor
	it will be created automatically if notifications
	or indications are enabled on a characteristic.

	pCharacteristic->addDescriptor(new BLE2902());
	****************************************************/

	BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
											CHARACTERISTIC_UUID_RX,
									/******* Enum Type NIMBLE_PROPERTY now *******
											BLECharacteristic::PROPERTY_WRITE
											);
									*********************************************/
											NIMBLE_PROPERTY::WRITE
											);

	pRxCharacteristic->setCallbacks(new MyCallbacks());

	// Start the service
	pService->start();

	xTaskCreate(connectedTask, "connectedTask", 5000, NULL, 1, NULL);

	// Start advertising
	pServer->getAdvertising()->start();
	printf("Waiting a client connection to notify...\n");
}
