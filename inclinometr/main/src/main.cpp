#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <ws_server.h>

#include "esp_log.h"
#include "display.h"
#include "display_ui.h"


extern "C" { void app_main(void); }

void app_main(void){
	start_ws();
	lvgl_setup();
	bsp_display_lock(0);
	display_window();
	bsp_display_unlock();
}
