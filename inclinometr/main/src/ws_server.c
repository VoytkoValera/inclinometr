#include "../include/ws_server.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "mdns.h"
#include "lwip/dns.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "websocket_server.h"

#include "wifiStack.h"

#include <cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif

static int client_queue_size = 10;
static QueueHandle_t client_queue;

typedef struct{
	char* ssid;
	char* passwd;
}wifiLoginData_t;

static void (*WSCallBack)(char* msg, uint64_t len) = NULL;

void setWSRxCallBack(void *pCallBack)
{
	WSCallBack = pCallBack;
}

void websocket_callback(uint8_t num,WEBSOCKET_TYPE_t type,char* msg,uint64_t len){
	const static char* TAG = "websocket_callback";
	switch(type) {
		case WEBSOCKET_CONNECT:
	      ESP_LOGI(TAG,"client %i connected!",num);
	      break;
	    case WEBSOCKET_DISCONNECT_EXTERNAL:
	      ESP_LOGI(TAG,"client %i sent a disconnect message",num);
	      break;
	    case WEBSOCKET_DISCONNECT_INTERNAL:
	      ESP_LOGI(TAG,"client %i was disconnected",num);
	      break;
	    case WEBSOCKET_DISCONNECT_ERROR:
	      ESP_LOGI(TAG,"client %i was disconnected due to an error",num);
	      break;
	    case WEBSOCKET_TEXT:
			if(len){
			  if (WSCallBack != NULL){
				  WSCallBack(msg, len);
			  }
			  else{
				  ESP_LOGI(TAG, "%s", msg);
			  }
		  }
	      break;
	    case WEBSOCKET_BIN:
	      ESP_LOGI(TAG,"client %hu sent binary message of size %lu:\n%s",num,(uint32_t)len,msg);
	      break;
	    case WEBSOCKET_PING:
	      ESP_LOGI(TAG,"client %hu pinged us with message of size %lu:\n%s",num,(uint32_t)len,msg);
	      break;
	    case WEBSOCKET_PONG:
	      ESP_LOGI(TAG,"client %i responded to the ping",num);
	      break;
	  }
}

static void http_serve(struct netconn* conn){
	const static char* TAG = "http_server";
	static err_t err;
	struct netbuf* inbuf;
	static char* buf;
	static uint16_t buflen;

	netconn_set_recvtimeout(conn, 1000);
	ESP_LOGI(TAG,"reading from client...");
	err = netconn_recv(conn, &inbuf);
	ESP_LOGI(TAG,"read from client");
	if (err == ESP_OK){
		netbuf_data(inbuf, (void**)&buf, &buflen);
		if (buf){
			if (strstr(buf,"GET /")&& strstr(buf,"Upgrade: websocket")){
				ESP_LOGI(TAG,"Requesting websocket on /");
				ws_server_add_client(conn,buf,buflen,"/",websocket_callback);
				netbuf_delete(inbuf);
			}
		}
	}
}

static void server_task(void* pvParameters) {
  const static char* TAG = "server_task";
  struct netconn *conn, *newconn;
  static err_t err;
  client_queue = xQueueCreate(client_queue_size,sizeof(struct netconn*));

  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn,NULL,80);
  netconn_listen(conn);
  ESP_LOGI(TAG,"server listening");
  do {
    err = netconn_accept(conn, &newconn);
    ESP_LOGI(TAG,"new client");
    if(err == ERR_OK) {
      xQueueSendToBack(client_queue,&newconn,portMAX_DELAY);
      //http_serve(newconn);
    }
  } while(err == ERR_OK);
  netconn_close(conn);
  netconn_delete(conn);
  ESP_LOGE(TAG,"task ending, rebooting board");
  esp_restart();
}

// receives clients from queue, handles them
static void server_handle_task(void* pvParameters) {
  const static char* TAG = "server_handle_task";
  struct netconn* conn;
  ESP_LOGI(TAG,"task starting");
  for(;;) {
    xQueueReceive(client_queue,&conn,portMAX_DELAY);
    if(!conn) continue;
    http_serve(conn);
  }
  vTaskDelete(NULL);
}

static void count_task(void* pvParameters) {
  const static char* TAG = "count_task";
  int clients;
  uint8_t n = 0;
  const int DELAY = 1000 / portTICK_PERIOD_MS; // 1 second

  ESP_LOGI(TAG,"starting task");
  for(;;) {
    //len = sprintf(out,word,n);
    //clients = ws_server_send_text_all(out,len);
    if(clients > 0) {
      //ESP_LOGI(TAG,"sent: \"%s\" to %i clients",out,clients);
    }
    n++;
    vTaskDelay(DELAY);
  }
}

void get_nvs_descriptor(nvs_handle_t* nvs_descriptor){
	static  err;
	err = nvs_open("storage", NVS_READWRITE, nvs_descriptor);
	if (err!= ESP_OK){
		err = nvs_flash_init();
		if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
		{
			ESP_ERROR_CHECK(nvs_flash_erase());
			nvs_flash_init();
		}
		nvs_open("storage", NVS_READWRITE, nvs_descriptor);
	}
}
wifi_mode_t check_mode(nvs_handle_t* nvs_descriptor){
	const char* TAG = "CHECK MODE";
	uint8_t mode;
	static esp_err_t err;
	err = nvs_get_u8(*nvs_descriptor, "wifi_mode", &mode);
	switch(err){
		case ESP_OK:
			ESP_LOGI(TAG,"mode was found");
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			ESP_LOGI(TAG,"mode was not found");
			mode = WIFI_MODE_AP;
			nvs_set_u8(*nvs_descriptor, "wifi_mode", mode);
			break;
		default:
			ESP_LOGW(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
	}
//	if (err == ESP_ERR_NOT_FOUND){
//		mode  = WIFI_MODE_AP;
//		ESP_LOGI(TAG,"mode setup was not found");
//	}
	return (wifi_mode_t)mode;

}
wifiLoginData_t login_data;
wifiLoginData_t* check_ssid_pass(nvs_handle_t* nvs_descriptor, wifi_mode_t mode){
	const char* TAG = "check login data";
	size_t nvs_required_size;
	static esp_err_t err;
	err = nvs_get_str(*nvs_descriptor, "ssid", NULL, &nvs_required_size);
	switch (err)
	{
		case ESP_OK:
			ESP_LOGI(TAG,"ssid was found");
			login_data.ssid = malloc(nvs_required_size);
			err = nvs_get_str(*nvs_descriptor, "ssid", login_data.ssid, &nvs_required_size );
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			ESP_LOGI(TAG, "ssid was not found");
			login_data.ssid = (mode == WIFI_MODE_AP) ? CONFIG_AP_SSID : CONFIG_STA_SSID;
			nvs_set_str(*nvs_descriptor, "ssid", login_data.ssid);
			break;
		default :
			ESP_LOGW(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
	}
	err = nvs_get_str(*nvs_descriptor, "passwd", NULL, &nvs_required_size);
		switch (err)
		{
			case ESP_OK:
				ESP_LOGI(TAG,"passwd was found");
				login_data.passwd = malloc(nvs_required_size);
				err = nvs_get_str(*nvs_descriptor, "passwd", login_data.passwd, &nvs_required_size );
				break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(TAG, "passwd was not found");
				login_data.passwd = (mode == WIFI_MODE_AP) ? CONFIG_AP_PASSWORD : CONFIG_STA_PASSWORD;
				nvs_set_str(*nvs_descriptor, "passwd", login_data.passwd);
				break;
			default :
				ESP_LOGW(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
		}
	return &login_data;
}

void wifi_start(wifi_mode_t mode, char* ssid, char* passwd){
	const char* TAG = "wifi start";
	if(mode == WIFI_MODE_AP){
		ESP_ERROR_CHECK(startWifiAccessPoint(ssid, passwd));
		ESP_LOGI(TAG,"start AP mode");
	}
	if(mode == WIFI_MODE_STA){
		ESP_ERROR_CHECK(startWifiStation(ssid, passwd));
		ESP_LOGI(TAG,"start STA mode");
	}
	ESP_LOGI(TAG,"%d", (int)mode);
}


static void init_mdns(void){
	const char* TAG = "mdns config";
	//initialize mDNS
	ESP_ERROR_CHECK(mdns_init());
	//set mDNS hostname (required if you want to advertise services)
	ESP_ERROR_CHECK( mdns_hostname_set("esp32inclinometr") );
	ESP_LOGI(TAG, "mdns hostname set to: [%s]", "esp32inclinometr");
}

void start_ws(void){
	nvs_handle_t nvs_descriptor;
	ESP_ERROR_CHECK(initWifi());
	get_nvs_descriptor(&nvs_descriptor);
	wifi_mode_t mode = check_mode(&nvs_descriptor);
	wifiLoginData_t* login_data = check_ssid_pass(&nvs_descriptor, mode);
	wifi_start(mode, login_data->ssid, login_data->passwd);
	init_mdns();
	ws_server_start();
	xTaskCreate(&server_task,"server_task",3000,NULL,9,NULL);
	xTaskCreate(&server_handle_task,"server_handle_task",4000,NULL,6,NULL);
	xTaskCreate(&count_task,"count_task",6000,NULL,2,NULL);
}

#ifdef __cplusplus
}
#endif
