#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "esp_log.h"
#include "ws_server.h"


void app_main(void)
{
	start_ws();
    for(;;) {
        sleep(1);
    }
}
