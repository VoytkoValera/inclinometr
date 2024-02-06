#include <stdio.h>
#include "esp_log.h"

extern "C" { void app_main(void); }

void app_main(void)
{
    ESP_LOGI("HI", "%s", __FILE__);
}
