#include <Arduino.h>
#include "GrblParser.h"
#include <io_controller.h>

// https://github.com/stm32duino/Arduino_Core_STM32/wiki/

// STM UART Reference
// HardwareSerial Serial1(); PA10, PA9
// HardwareSerial Serial2(PA3, PA2);
// HardwareSerial Serial3(PB11, PB10);

HardwareSerial Serial_FNC(PA10, PA9);    // connects from STM32 to ESP32 and FNC
HardwareSerial Serial_Pendant(PA3, PA2); // connects from STM32 to Display

class Displayer : public GrblParser
{

    void debug_message(String message)
    {
        Serial_Pendant.print(message);
    }

    void parse_message(String message)
    {
        String level; // message level
        String body;

        level = message.substring(5); // // trim [MSG:

        int pos = level.indexOf(" ");
        if (pos == -1)
        {
            return;
        }

        level = level.substring(0, pos-1);  // remove the colon

        body = message.substring(6 + level.length()+1);
        body.remove(body.length() - 1);

        // if this is an IO op then get the pin number.
        // [MSG:INI io.1=inp,low,pu]
        if (level == "INI" || level == "GET" || level == "SET")
        {
            int pin_num;
            String param_list;

            if (!body.startsWith("io."))
            {
                return;
            }
            pos = body.indexOf(".");
            int nextpos = body.indexOf("=");
            if (pos == -1 or nextpos == -1)
            {
                return;
            }

            pin_num = body.substring(pos + 1, nextpos).toInt();
            param_list = body.substring(nextpos + 1);

            if (level == "INI")
            {
                if (pins[pin_num].init(param_list) != STM32_Pin::FailCodes::None)
                {
                    debug_message("IN Error");
                    return;

                }
            }

            if (level == "SET")
            {
                float val = param_list.toFloat();
                if (pins[pin_num].set_output(val) != STM32_Pin::FailCodes::None)
                {
                    debug_message("Set Error");

                }
                return;
            }
            return;
        }

        Serial_Pendant.println(message); // prints unhandled messages
    }

    void show_state(const String &state)
    {
        Serial_Pendant.print(state);
    }

    void show_gcode_modes(const gcode_modes &modes)
    {
        Serial_Pendant.print("Got modes");
    }

    void show_info_message(String message)
    {
        Serial_Pendant.println("Got message");
        Serial_Pendant.println(message);
    }

    void process_set_message(String message)
    {
    }

} displayer;

void setup()
{

    io_init();

    Serial_FNC.begin(115200);     // PA10, PA9
    Serial_Pendant.begin(115200); // PA3, PA2

    pinMode(PC13, OUTPUT); // for rx/tx activity LED
    Serial_Pendant.println("\r\n[MSG:INFO: Hello pendant]");
    //Serial_FNC.println("Hello FNC");
}

void loop()
{
    while (Serial_FNC.available()) // From Terminal
    {
        char c = Serial_FNC.read();
        Serial_Pendant.write(c);
        displayer.write(c);  // for production
    }

    while (Serial_Pendant.available()) // From FNC
    {
        char c = Serial_Pendant.read();
        Serial_FNC.write(c);
        // displayer.write(c); // for testing from pendant terminal
    }

    //delay(5);

    read_all_pins();

}


void debug_message(String message)
{
    Serial_Pendant.print("DEBUG:");
    Serial_Pendant.println(message);
}

extern "C" void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        while (1)
            ;
    }
    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        while (1)
            ;
    }
}