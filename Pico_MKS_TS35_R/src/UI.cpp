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
    lv_style_set_align(&status_style, LV_ALIGN_TOP_MID);

    status_label = lv_label_create(base_obj);
    lv_obj_add_style(status_label, &status_style, 0);
    lv_label_set_text(status_label, stateString.c_str());
}

lv_span_t *axis_value_span[3];
lv_obj_t *file_progress;
lv_style_t axis_style;
lv_style_t axis_group_style;

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
    lv_style_set_width(&axis_style, LV_PCT(100));

    lv_style_init(&axis_group_style);
    lv_style_set_pad_row(&axis_group_style, 0);
    lv_style_set_pad_column(&axis_group_style, 0);
    lv_style_set_pad_all(&axis_group_style, 0);

    auto axis_group = lv_obj_create(base_obj);
    lv_obj_add_flag(axis_group, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_set_width(axis_group, LV_PCT(50));
    lv_obj_set_align(axis_group, LV_ALIGN_LEFT_MID);
    lv_obj_set_flex_flow(axis_group, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(axis_group, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_add_style(axis_group, &axis_group_style, LV_PART_MAIN);

    for (int i = 0; i < 3; ++i) {
        auto span_grp = lv_spangroup_create(axis_group);

        lv_obj_add_style(span_grp, &axis_style, LV_PART_MAIN);

        auto axis_label_span = lv_spangroup_new_span(span_grp);
        lv_span_set_text(axis_label_span, axisNumToString(i).c_str());
        axis_value_span[i] = lv_spangroup_new_span(span_grp);
        lv_span_set_text(axis_value_span[i], floatToString(myAxes[i], 2).c_str());
    }

    file_progress = lv_bar_create(axis_group);
    lv_obj_set_width(file_progress, LV_PCT(100));
    lv_bar_set_value(file_progress, myPercent, LV_ANIM_OFF);
}

lv_style_t base_style;

void init_base() {
    base_obj = lv_screen_active();

//    lv_style_init(&base_style);
//    lv_style_set_pad_row(&base_style, 0);
//    lv_style_set_pad_column(&base_style, 0);
//    lv_obj_set_flex_flow(base_obj, LV_FLEX_FLOW_ROW);
//    lv_obj_add_style(base_obj, &base_style, LV_PART_MAIN);
}

lv_style_t jogging_style;
lv_style_t jogging_button_style;
lv_obj_t *jogging_grid;

static int32_t col_dsc[] = {LV_PCT(33), LV_PCT(33), LV_PCT(33), LV_GRID_TEMPLATE_LAST};
static int32_t row_dsc[] = {LV_PCT(33), LV_PCT(33), LV_PCT(33), LV_GRID_TEMPLATE_LAST};

void jogging_button_cb(lv_event_t *e) {
    auto *row_col_data = static_cast<std::pair<u_int8_t, u_int8_t> *>(e->user_data);
    Serial.print(row_col_data->first);
    Serial.print(", ");
    Serial.println(row_col_data->second);
}

void create_jogging_button(const char *symbol, u_int8_t row, u_int8_t col) {
    lv_obj_t *obj = lv_button_create(jogging_grid);
    lv_obj_add_style(obj, &jogging_button_style, LV_PART_MAIN);
    static auto row_col_data = std::pair<u_int8_t, u_int8_t>(row, col);
    lv_obj_add_event_cb(obj, &jogging_button_cb, LV_EVENT_CLICKED, &row_col_data);
    /*Stretch the cell horizontally and vertically too
     *Set span to 1 to make the cell 1 column/row sized*/
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, col, 1,
                         LV_GRID_ALIGN_STRETCH, row, 1);

    lv_obj_t *label = lv_label_create(obj);
    lv_label_set_text(label, symbol);
    lv_obj_center(label);
}

void init_jogging_ui() {
    lv_style_init(&jogging_style);
    lv_style_set_align(&jogging_style, LV_ALIGN_RIGHT_MID);
    lv_style_set_width(&jogging_style, LV_PCT(50));
    lv_style_set_pad_all(&jogging_style, 0);

    lv_style_init(&jogging_button_style);
    lv_style_set_width(&jogging_button_style, LV_PCT(33));
    lv_style_set_pad_all(&jogging_button_style, 0);
    lv_style_set_text_font(&jogging_button_style, &lv_font_montserrat_24);

    jogging_grid = lv_obj_create(base_obj);
    lv_obj_set_style_grid_column_dsc_array(jogging_grid, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(jogging_grid, row_dsc, 0);
    lv_obj_align(jogging_grid, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_layout(jogging_grid, LV_LAYOUT_GRID);
    lv_obj_add_style(jogging_grid, &jogging_style, 0);

    // up-button
    create_jogging_button(LV_SYMBOL_UP, 0, 1);
    create_jogging_button(LV_SYMBOL_LEFT, 1, 0);
    create_jogging_button(LV_SYMBOL_HOME, 1, 1);
    create_jogging_button(LV_SYMBOL_RIGHT, 1, 2);
    create_jogging_button(LV_SYMBOL_DOWN, 2, 1);
}

void init_ui() {
    init_base();
    init_state_ui();
    init_axis_ui();
    init_jogging_ui();
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
    lv_bar_set_value(file_progress, myPercent, LV_ANIM_ON);
}

void redraw() {
    redraw_state();
    redraw_axes();
}