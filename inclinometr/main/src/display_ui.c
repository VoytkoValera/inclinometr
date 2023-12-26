/*
 * pin_config.h
 *
 *  Created on: 26 дек. 2023 г.
 *      Author: Valera
 */

#include <stdio.h>
#include "display.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "display_helper.h"

static void slider_event_cb(lv_event_t *e);
static lv_obj_t *slider_label;

void display_slider()
{
    lv_disp_t *dispp = lv_disp_get_default();
    /*Create a slider in the center of the display*/
    lv_obj_t *slider = lv_slider_create(lv_scr_act());
    lv_obj_set_x(slider, 0);
    lv_obj_set_y(slider, -40);
    lv_obj_set_align(slider, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /*Create a label below the slider*/
    slider_label = lv_label_create(lv_scr_act());
    lv_label_set_text(slider_label, "0%");
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

static void slider_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
    lv_label_set_text(slider_label, buf);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

void display_window()
{
    lv_obj_t *win = lv_win_create(lv_scr_act(), 40);
    assert(win);
    lv_win_add_title(win, "test123!");
}
