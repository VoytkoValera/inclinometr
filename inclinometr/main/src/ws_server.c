#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "websocket_server.h"

#include "ws_server.h"
#include "wifiStack.h"

#include <cJSON.h>

static int client_queue_size = 10;
static QueueHandle_t client_queue;

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
	      if(len) { // if the message length was greater than zero
	    	  cJSON* json_data = cJSON_Parse(msg);
	    	  if (json_data == NULL) {
	    	          const char *error_ptr = cJSON_GetErrorPtr();
	    	          if (error_ptr != NULL) {
	    	              ESP_LOGW(TAG,"%s", error_ptr);
	    	          }
	    	          cJSON_Delete(json_data);
	    	          break;
			  }
	    	  cJSON *name = cJSON_GetObjectItemCaseSensitive(json_data, "name");
	    	  if (cJSON_IsString(name) && (name->valuestring != NULL)) {
	    	          printf("Name: %s\n", name->valuestring);
			  }
	    	  cJSON_Delete(json_data);
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
  char out[20];
  int len;
  int clients;
  const static char* word = "%i";
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

void start_ws(void){
	ESP_ERROR_CHECK(initWifi());
	ESP_ERROR_CHECK(startWifiAccessPoint(NULL, NULL));
	ws_server_start();
	xTaskCreate(&server_task,"server_task",3000,NULL,9,NULL);
	xTaskCreate(&server_handle_task,"server_handle_task",4000,NULL,6,NULL);
	xTaskCreate(&count_task,"count_task",6000,NULL,2,NULL);
}

