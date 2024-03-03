#include <Arduino.h>
#include "FluidNCModel.h"
#include <TFT_eSPI.h>
#include <lvgl.h>  // Hardware-specific library
#include <UI.h>

typedef struct {
    TFT_eSPI *tft;
} lv_tft_espi_t;

#define SCREEN_ROTATION 1
#define DRAW_BUF_SIZE (TFT_HEIGHT * TFT_WIDTH / 10 * (LV_COLOR_DEPTH / 8))

uint32_t draw_buf[DRAW_BUF_SIZE / 4];

TFT_eSPI *tft;  // Invoke custom library

#if LV_USE_LOG != 0

void my_print(lv_log_level_t level, const char *buf) {
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}

#endif

static void event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

void initBeep();

void beep() {
    digitalWrite(PIN_BEEPER, HIGH);
    delay(10);
    digitalWrite(PIN_BEEPER, LOW);
}

void blinkLED(bool fast = false) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(fast ? 50 : 1000);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(fast ? 50 : 1000);
    Serial.println("Blinky");
}

void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
    uint16_t x, y;
    bool touched = tft->getTouch(&x, &y, 600);

    if (!touched) {
        data->state = LV_INDEV_STATE_RELEASED;
    } else {
        // beep();
        data->state = LV_INDEV_STATE_PRESSED;
#if (SCREEN_ROTATION == 1) || (SCREEN_ROTATION == 3) || (SCREEN_ROTATION == 5) || (SCREEN_ROTATION == 7)
        data->point.x = y;
        data->point.y = x;
#else
        data->point.x = x;
        data->point.y = y;
#endif
    }
}

void touch_calibrate() {
    uint16_t calData[5];
    uint8_t calDataOK = 0;

    // Calibrate
    tft->fillScreen(TFT_BLACK);
    tft->setCursor(20, 0);
    tft->setTextFont(2);
    tft->setTextSize(1);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);

    tft->println("Touch corners as indicated");

    tft->setTextFont(1);
    tft->println();

    tft->calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    Serial.println();
    Serial.println();
    Serial.println("// Use this calibration code in setup():");
    Serial.print("  uint16_t calData[5] = ");
    Serial.print("{ ");

    for (uint8_t i = 0; i < 5; i++) {
        Serial.print(calData[i]);
        if (i < 4) Serial.print(", ");
    }

    Serial.println(" };");
    Serial.print("  tft->setTouch(calData);");
    Serial.println();
    Serial.println();

    tft->fillScreen(TFT_BLACK);

    tft->setTextColor(TFT_GREEN, TFT_BLACK);
    tft->println("Calibration complete!");
    tft->println("Calibration code sent to Serial port.");

    delay(4000);
}

UART SerialFluidNC(FLUIDNC_TX_PIN, FLUIDNC_RX_PIN);

void fnc_putchar(uint8_t c) {
    SerialFluidNC.write(c);
}

int fnc_getchar() {
    if (SerialFluidNC.available()) {
        update_rx_time();
        return SerialFluidNC.read();
    }
    return -1;
}

int milliseconds() {
    return millis();
}

void setup(void) {
    Serial.begin(9600);
    SerialFluidNC.begin(115200, SERIAL_8N1);
    fnc_wait_ready();
    fnc_send_line("$G", 1000);  // Initial modes report
    fnc_send_line("$I", 1000);

    initBeep();

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print);
#endif
    lv_display_t *disp;
    /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
    disp = lv_tft_espi_create(TFT_WIDTH, TFT_HEIGHT, draw_buf, sizeof(draw_buf));
    const auto driver_data = (lv_tft_espi_t *) lv_display_get_driver_data(disp);
    tft = driver_data->tft;
    tft->setRotation(SCREEN_ROTATION);
    // touch_calibrate();
    uint16_t calData[5] = {365, 3262, 443, 2633, 3};
    tft->setTouch(calData);

    /*Initialize the (dummy) input device driver*/
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
    lv_indev_set_read_cb(indev, my_touchpad_read);

    /* Create a simple label
     * ---------------------
     lv_obj_t *label = lv_label_create( lv_scr_act() );
     lv_label_set_text( label, "Hello Arduino, I'm LVGL!" );
     lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

     * Try an example. See all the examples
     *  - Online: https://docs.lvgl.io/master/examples.html
     *  - Source codes: https://github.com/lvgl/lvgl/tree/master/examples
     * ----------------------------------------------------------------

     lv_example_btn_1();

     * Or try out a demo. Don't forget to enable the demos in lv_conf.h. E.g. LV_USE_DEMOS_WIDGETS
     * -------------------------------------------------------------------------------------------

     lv_demo_widgets();
     */

    //lv_example_button_1();

    init_ui();

    Serial.println("Setup done");
}

void initBeep() {
    pinMode(PIN_BEEPER, OUTPUT);
    digitalWrite(PIN_BEEPER, LOW);
}

void loop() {
    lv_task_handler(); /* let the GUI do its work */

    long start = millis();
    while (SerialFluidNC.available()) {
        fnc_poll();
    }
    long end = millis();

    if (end - start == 0) {
        delay(1);
        lv_tick_inc(1);
    } else {
        lv_tick_inc(end - start); /* tell LVGL how much time has passed */
    }

    if (!fnc_is_connected()) {
        if (state != Disconnected) {
            set_disconnected_state();
            redraw();
        }
    }

    // Serial.println("loop");
//    // Binary inversion of colours
    //tft->invertDisplay( true ); // Where i is true or false
//
//    tft.fillScreen(TFT_BLACK);
//    tft.drawRect(0, 0, tft.width(), tft.height(), TFT_GREEN);
//
//    tft.setCursor(0, 4, 4);
//
//    tft.setTextColor(TFT_WHITE);
//    tft.println(" Invert ON\n");
//
//    tft.println(" White text");
//
//    tft.setTextColor(TFT_RED);
//    tft.println(" Red text");
//
//    tft.setTextColor(TFT_GREEN);
//    tft.println(" Green text");
//
//    tft.setTextColor(TFT_BLUE);
//    tft.println(" Blue text");

    //  blinkLED();

}