//
// Created by Thomas Peuß on 21.02.2024.
//

#include "UI.h"
#include <lvgl.h>
#include <FluidNCModel.h>

lv_obj_t *status_label;
lv_style_t status_style;

void init_ui() {
    lv_style_init(&status_style);
    lv_style_set_radius(&status_style, 5);
    lv_style_set_border_width(&status_style, 2);
    lv_style_set_pad_all(&status_style, 10);
    lv_style_set_text_font(&status_style, &lv_font_montserrat_20);

    status_label = lv_label_create(lv_screen_active());
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_style(status_label, &status_style, 0);
    lv_label_set_text(status_label, stateString.c_str());
}

void redraw() {
    lv_label_set_text(status_label, stateString.c_str());
}