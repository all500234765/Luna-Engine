#include "Gamepad.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>

void Gamepad::Update() {
    // Update state
    GetFullState();

    // Update buttons
    for( int i = 0; i < ButtonCount; i++ ) {
        CurrState[i] = IsButtonDown(GamepadButtonState(i));
        DownState[i] = (!PrevState[i] && CurrState[i]);
    }
}

// Usage:
//  Update();
//  if( ButtonPress(ButtonState::Start) ) {}
//  ...
//  Refresh();
void Gamepad::Refresh() {
    memcpy(PrevState, CurrState, sizeof(PrevState));
}

XINPUT_STATE Gamepad::GetFullState() {
    ZeroMemory(&mState, sizeof(XINPUT_STATE));
    XInputGetState(Index, &mState);
    return mState;
}

int Gamepad::GetIndex() {
    return Index;
}

bool Gamepad::IsConnected() {
    ZeroMemory(&mState, sizeof(XINPUT_STATE));
    return (XInputGetState(Index, &mState) == ERROR_SUCCESS);
}

bool Gamepad::IsDeadZoneL() {
    short sx = mState.Gamepad.sThumbLX;
    short sy = mState.Gamepad.sThumbLY;

    if( sx > +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || sx < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ) return false;
    if( sy > +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || sy < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ) return false;
    return true;
}

bool Gamepad::IsDeadZoneR() {
    short sx = mState.Gamepad.sThumbRX;
    short sy = mState.Gamepad.sThumbRY;

    if( sx > +XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || sx < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ) return false;
    if( sy > +XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || sy < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ) return false;
    return true;
}

float Gamepad::LeftX() {
    return (static_cast<float>(mState.Gamepad.sThumbLX) / 32768.f);
}

float Gamepad::LeftY() {
    return (static_cast<float>(mState.Gamepad.sThumbLY) / 32768.f);
}

float Gamepad::RightX() {
    return (static_cast<float>(mState.Gamepad.sThumbRX) / 32768.f);
}

float Gamepad::RightY() {
    return (static_cast<float>(mState.Gamepad.sThumbRY) / 32768.f);
}

float Gamepad::TriggerL() {
    BYTE Trig = mState.Gamepad.bLeftTrigger;

    if( Trig > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ) return (static_cast<float>(Trig) / 255.f);
    return 0.f;
}

float Gamepad::TriggerR() {
    BYTE Trig = mState.Gamepad.bRightTrigger;

    if( Trig > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ) return (static_cast<float>(Trig) / 255.f);
    return 0.f;
}

void Gamepad::Vibrate(float left, float right) {
    XINPUT_VIBRATION mVibState;
    ZeroMemory(&mVibState, sizeof(XINPUT_VIBRATION));

    mVibState.wLeftMotorSpeed = static_cast<int>(left * 65535.f);
    mVibState.wRightMotorSpeed = static_cast<int>(right * 65535.f);
    XInputSetState(Index, &mVibState);
}

void Gamepad::Vibrate(float value, bool isRight) {
    XINPUT_VIBRATION mVibState;
    ZeroMemory(&mVibState, sizeof(XINPUT_VIBRATION));

    if( !isRight ) mVibState.wLeftMotorSpeed = static_cast<int>(value * 65535.f); // Left
    else           mVibState.wRightMotorSpeed = static_cast<int>(value * 65535.f); // Right
    XInputSetState(Index, &mVibState);
}

bool Gamepad::IsButtonPressed(GamepadButtonState button) {
    return DownState[button];
}

bool Gamepad::IsButtonDown(GamepadButtonState button) {
    return ((mState.Gamepad.wButtons & XINPUT_GamepadButtons[button]) == XINPUT_GamepadButtons[button]);
}
