// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include <Arduino.h>
#include "GrblParserC.h"

// Same states as FluidNC except for the last one
enum state_t {
    Idle = 0,      // Must be zero.
    Alarm,         // In alarm state. Locks out all g-code processes. Allows settings access.
    CheckMode,     // G-code check mode. Locks out planner and motion only.
    Homing,        // Performing homing cycle
    Cycle,         // Cycle is running or motions are being executed.
    Hold,          // Active feed hold
    Jog,           // Jogging mode.
    SafetyDoor,    // Safety door is ajar. Feed holds and de-energizes system.
    Sleep,         // Sleep state.
    ConfigAlarm,   // You can't do anything but fix your config file.
    Critical,      // You can't do anything but reset with CTRL-x or the reset button
    Disconnected,  // We can't talk to FluidNC
};

// Variables and functions to model the state of the FluidNC controller

extern state_t state;
extern String stateString;

extern pos_t myAxes[6];
extern bool myLimitSwitches[6];
extern bool myProbeSwitch;
extern String myFile;
extern file_percent_t myPercent;
extern override_percent_t myFro;
extern int lastAlarm;
extern int lastError;
extern uint32_t errorExpire;

void send_line(const String &s, int timeout = 2000);

void send_line(const char *s, int timeout = 2000);

String floatToString(float val, int afterDecimal);

String axisNumToString(int axis);

state_t decode_state_string(const char *stateString);

String decode_error_number(int error_num);

String modeString();

bool fnc_is_connected();

void set_disconnected_state();

void update_rx_time();
