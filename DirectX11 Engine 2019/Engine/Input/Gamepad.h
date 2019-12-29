#pragma once

#include "pc.h"

// XInput Button values
static const WORD XINPUT_GamepadButtons[] = {
      XINPUT_GAMEPAD_A,
      XINPUT_GAMEPAD_B,
      XINPUT_GAMEPAD_X,
      XINPUT_GAMEPAD_Y,
      XINPUT_GAMEPAD_DPAD_UP,
      XINPUT_GAMEPAD_DPAD_DOWN,
      XINPUT_GAMEPAD_DPAD_LEFT,
      XINPUT_GAMEPAD_DPAD_RIGHT,
      XINPUT_GAMEPAD_LEFT_SHOULDER,
      XINPUT_GAMEPAD_RIGHT_SHOULDER,
      XINPUT_GAMEPAD_LEFT_THUMB,
      XINPUT_GAMEPAD_RIGHT_THUMB,
      XINPUT_GAMEPAD_START,
      XINPUT_GAMEPAD_BACK
};

typedef enum {
    A, B, X, Y,
    
    DPad_Up, DPad_Down, DPad_Left, DPad_Right,
    
    ShoulderL, ShoulderR,
    ThumbstickL, ThumbstickR,

    Start, Back, 

    _StickL, _StickR
} GamepadButtonState;

class Gamepad {
private:
    // Total gamepad buttons
    static const int ButtonCount = 14;

    // Gamepad index
    int Index = 0;

    // Gamepad state
    XINPUT_BATTERY_INFORMATION mBatteryInfo = {};
    XINPUT_STATE mState = {};

    bool PrevState[ButtonCount];
    bool CurrState[ButtonCount];
    bool DownState[ButtonCount];

public:
    Gamepad() {
        for( int i = 0; i < ButtonCount; i++ ) {
            PrevState[i] = false;
            CurrState[i] = false;
            DownState[i] = false;
        }
    };

    Gamepad(int index): Index(index) {
        for( int i = 0; i < ButtonCount; i++ ) {
            PrevState[i] = false;
            CurrState[i] = false;
            DownState[i] = false;
        }
    };

    void Update();
    void Refresh();
    
    XINPUT_STATE GetFullState();
    int GetIndex();
    bool IsConnected();

    bool IsDeadZoneL();
    bool IsDeadZoneR();

    float LeftX();
    float LeftY();

    float RightX();
    float RightY();

    float TriggerL();
    float TriggerR();

    void Vibrate(float value, bool isRight);
    void Vibrate(float left, float right);
    bool IsButtonPressed(GamepadButtonState button);
    bool IsButtonDown(GamepadButtonState button);

    bool HasBattery();
    BYTE BatteryLevel();
};
