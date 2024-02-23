//
// Created by Thomas Peuß on 21.02.2024.
//

#include "UI.h"
#include <lvgl.h>
#include <FluidNCModel.h>
#include <map>

std::map<state_t, lv_color_t> stateColors = {
        {Idle,         lv_color_white()},
        {Alarm,        lv_palette_lighten(LV_PALETTE_RED, 2)},
        {CheckMode,    lv_color_white()},
        {Homing,       lv_palette_lighten(LV_PALETTE_CYAN, 2)},
        {Cycle,        lv_palette_lighten(LV_PALETTE_GREEN, 2)},
        {Hold,         lv_palette_lighten(LV_PALETTE_YELLOW, 2)},
        {Jog,          lv_palette_lighten(LV_PALETTE_CYAN, 2)},
        {SafetyDoor,   lv_color_white()},
        {Sleep,        lv_color_white()},
        {ConfigAlarm,  lv_color_white()},
        {Critical,     lv_color_white()},
        {Disconnected, lv_palette_lighten(LV_PALETTE_RED, 2)},
};

lv_obj_t *base_obj;

lv_obj_t *status_label;
lv_style_t status_style;

void init_state_ui() {
    lv_style_init(&status_style);
    lv_style_set_radius(&status_style, 5);
    lv_style_set_border_width(&status_style, 2);
    lv_style_set_border_color(&status_style, stateColors[state]);
    lv_style_set_pad_ver(&status_style, 10);
    lv_style_set_pad_hor(&status_style, 40);
    lv_style_set_text_font(&status_style, &lv_font_montserrat_24);
    lv_style_set_text_align(&status_style, LV_TEXT_ALIGN_CENTER);
    lv_style_set_bg_opa(&status_style, LV_OPA_50);
    lv_style_set_bg_color(&status_style, stateColors[state]);
    lv_style_set_width(&status_style, LV_PCT(100));

    status_label = lv_label_create(base_obj);
    lv_obj_add_style(status_label, &status_style, 0);
    lv_label_set_text(status_label, stateString.c_str());
}

lv_span_t *axis_value_span[3];
lv_style_t axis_style;

void init_axis_ui() {
    lv_style_init(&axis_style);
    lv_style_set_radius(&axis_style, 5);
    lv_style_set_border_width(&axis_style, 2);
    lv_style_set_border_color(&axis_style, lv_color_white());
    lv_style_set_pad_hor(&axis_style, 3);
    lv_style_set_text_font(&axis_style, &lv_font_montserrat_24);
    lv_style_set_text_color(&axis_style, lv_color_white());
    lv_style_set_text_align(&axis_style, LV_TEXT_ALIGN_RIGHT);
    lv_style_set_bg_opa(&axis_style, LV_OPA_50);
    lv_style_set_bg_color(&axis_style, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_width(&axis_style, LV_PCT(50));

    for (int i = 0; i < 3; ++i) {
        auto span_grp = lv_spangroup_create(base_obj);
        lv_obj_add_style(span_grp, &axis_style, LV_PART_MAIN);

        auto axis_label_span = lv_spangroup_new_span(span_grp);
        lv_span_set_text(axis_label_span, axisNumToString(i).c_str());
        axis_value_span[i] = lv_spangroup_new_span(span_grp);
        lv_span_set_text(axis_value_span[i], floatToString(myAxes[i], 2).c_str());
    }
}

lv_style_t base_style;

void init_base() {
    base_obj = lv_screen_active();

    lv_style_init(&base_style);
    lv_style_set_pad_row(&base_style, 0);
    lv_style_set_pad_column(&base_style, 0);
    lv_obj_set_flex_flow(base_obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_style(base_obj, &base_style, LV_PART_MAIN);
}

void init_ui() {
    init_base();
    init_state_ui();
    init_axis_ui();
}

void redraw_state() {
    lv_style_set_border_color(&status_style, stateColors[state]);
    // lv_style_set_text_color(&status_style, stateColors[state]);
    lv_style_set_bg_color(&status_style, stateColors[state]);
    lv_obj_refresh_style(status_label, LV_PART_ANY, LV_STYLE_PROP_ANY);
    lv_label_set_text(status_label, stateString.c_str());
}

void redraw_axes() {
    for (int i = 0; i < 3; ++i) {
        lv_span_set_text(axis_value_span[i], floatToString(myAxes[i], 2).c_str());
    }
}

void redraw() {
    redraw_state();
    redraw_axes();
}