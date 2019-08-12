#include "Keyboard.h"

#include <iostream>

Keyboard::Keyboard() {
    memset(&mState, 0, sizeof(State));
    memset(&mLastState, 0, sizeof(State));
    memset(&_mStates.__Member, 0, sizeof(_mStates.__Member));

    // Bug fix...
    /*SetState(VK_W, true);
    SetState(VK_S, true);*/
}

#ifndef UPDATE_BUTTON_STATE
#define UPDATE_BUTTON_STATE(field) _mStates.__Member.field = static_cast<ButtonState>( ( !!mState.field ) | ( ( !!mState.field ^ !!mLastState.field ) << 1 ) );
#endif

void Keyboard::Refresh() {
    UPDATE_BUTTON_STATE(Reserved00);
    UPDATE_BUTTON_STATE(Reserved01);
    UPDATE_BUTTON_STATE(Reserved02);
    UPDATE_BUTTON_STATE(Reserved03);
    UPDATE_BUTTON_STATE(Reserved04);
    UPDATE_BUTTON_STATE(Reserved05);
    UPDATE_BUTTON_STATE(Reserved06);
    UPDATE_BUTTON_STATE(Reserved07);
    UPDATE_BUTTON_STATE(Back);
    UPDATE_BUTTON_STATE(Tab);
    UPDATE_BUTTON_STATE(Reserved10);
    UPDATE_BUTTON_STATE(Reserved11);
    UPDATE_BUTTON_STATE(Reserved12);
    UPDATE_BUTTON_STATE(Enter);
    UPDATE_BUTTON_STATE(Reserved20);
    UPDATE_BUTTON_STATE(Reserved21);
    UPDATE_BUTTON_STATE(Reserved30);
    UPDATE_BUTTON_STATE(Reserved31);
    UPDATE_BUTTON_STATE(Reserved32);
    UPDATE_BUTTON_STATE(Pause);
    UPDATE_BUTTON_STATE(CapsLock);
    UPDATE_BUTTON_STATE(Kana);
    UPDATE_BUTTON_STATE(Reserved40);
    UPDATE_BUTTON_STATE(Reserved41);
    UPDATE_BUTTON_STATE(Reserved50);
    UPDATE_BUTTON_STATE(Kanji);
    UPDATE_BUTTON_STATE(Reserved60);
    UPDATE_BUTTON_STATE(Escape);
    UPDATE_BUTTON_STATE(ImeConvert);
    UPDATE_BUTTON_STATE(ImeNoConvert);
    UPDATE_BUTTON_STATE(Reserved70);
    UPDATE_BUTTON_STATE(Reserved71);
    UPDATE_BUTTON_STATE(Space);
    UPDATE_BUTTON_STATE(PageUp);
    UPDATE_BUTTON_STATE(PageDown);
    UPDATE_BUTTON_STATE(End);
    UPDATE_BUTTON_STATE(Home);
    UPDATE_BUTTON_STATE(Left);
    UPDATE_BUTTON_STATE(Up);
    UPDATE_BUTTON_STATE(Right);
    UPDATE_BUTTON_STATE(Down);
    UPDATE_BUTTON_STATE(Select);
    UPDATE_BUTTON_STATE(Print);
    UPDATE_BUTTON_STATE(Execute);
    UPDATE_BUTTON_STATE(PrintScreen);
    UPDATE_BUTTON_STATE(Insert);
    UPDATE_BUTTON_STATE(Delete);
    UPDATE_BUTTON_STATE(Help);
    UPDATE_BUTTON_STATE(D0);
    UPDATE_BUTTON_STATE(D1);
    UPDATE_BUTTON_STATE(D2);
    UPDATE_BUTTON_STATE(D3);
    UPDATE_BUTTON_STATE(D4);
    UPDATE_BUTTON_STATE(D5);
    UPDATE_BUTTON_STATE(D6);
    UPDATE_BUTTON_STATE(D7);
    UPDATE_BUTTON_STATE(D8);
    UPDATE_BUTTON_STATE(D9);
    UPDATE_BUTTON_STATE(Reserved80);
    UPDATE_BUTTON_STATE(Reserved81);
    UPDATE_BUTTON_STATE(Reserved82);
    UPDATE_BUTTON_STATE(Reserved83);
    UPDATE_BUTTON_STATE(Reserved84);
    UPDATE_BUTTON_STATE(Reserved85);
    UPDATE_BUTTON_STATE(Reserved90);
    UPDATE_BUTTON_STATE(A);
    UPDATE_BUTTON_STATE(B);
    UPDATE_BUTTON_STATE(C);
    UPDATE_BUTTON_STATE(D);
    UPDATE_BUTTON_STATE(E);
    UPDATE_BUTTON_STATE(F);
    UPDATE_BUTTON_STATE(G);
    UPDATE_BUTTON_STATE(H);
    UPDATE_BUTTON_STATE(I);
    UPDATE_BUTTON_STATE(J);
    UPDATE_BUTTON_STATE(K);
    UPDATE_BUTTON_STATE(L);
    UPDATE_BUTTON_STATE(M);
    UPDATE_BUTTON_STATE(N);
    UPDATE_BUTTON_STATE(O);
    UPDATE_BUTTON_STATE(P);
    UPDATE_BUTTON_STATE(Q);
    UPDATE_BUTTON_STATE(R);
    UPDATE_BUTTON_STATE(S);
    UPDATE_BUTTON_STATE(T);
    UPDATE_BUTTON_STATE(U);
    UPDATE_BUTTON_STATE(V);
    UPDATE_BUTTON_STATE(W);
    UPDATE_BUTTON_STATE(X);
    UPDATE_BUTTON_STATE(Y);
    UPDATE_BUTTON_STATE(Z);
    UPDATE_BUTTON_STATE(LeftWindows);
    UPDATE_BUTTON_STATE(RightWindows);
    UPDATE_BUTTON_STATE(Apps);
    UPDATE_BUTTON_STATE(Reserved100);
    UPDATE_BUTTON_STATE(Sleep);
    UPDATE_BUTTON_STATE(NumPad0);
    UPDATE_BUTTON_STATE(NumPad1);
    UPDATE_BUTTON_STATE(NumPad2);
    UPDATE_BUTTON_STATE(NumPad3);
    UPDATE_BUTTON_STATE(NumPad4);
    UPDATE_BUTTON_STATE(NumPad5);
    UPDATE_BUTTON_STATE(NumPad6);
    UPDATE_BUTTON_STATE(NumPad7);
    UPDATE_BUTTON_STATE(NumPad8);
    UPDATE_BUTTON_STATE(NumPad9);
    UPDATE_BUTTON_STATE(Multiply);
    UPDATE_BUTTON_STATE(Add);
    UPDATE_BUTTON_STATE(Separator);
    UPDATE_BUTTON_STATE(Subtract);
    UPDATE_BUTTON_STATE(Decimal);
    UPDATE_BUTTON_STATE(Divide);
    UPDATE_BUTTON_STATE(F1);
    UPDATE_BUTTON_STATE(F2);
    UPDATE_BUTTON_STATE(F3);
    UPDATE_BUTTON_STATE(F4);
    UPDATE_BUTTON_STATE(F5);
    UPDATE_BUTTON_STATE(F6);
    UPDATE_BUTTON_STATE(F7);
    UPDATE_BUTTON_STATE(F8);
    UPDATE_BUTTON_STATE(F9);
    UPDATE_BUTTON_STATE(F10);
    UPDATE_BUTTON_STATE(F11);
    UPDATE_BUTTON_STATE(F12);
    UPDATE_BUTTON_STATE(F13);
    UPDATE_BUTTON_STATE(F14);
    UPDATE_BUTTON_STATE(F15);
    UPDATE_BUTTON_STATE(F16);
    UPDATE_BUTTON_STATE(F17);
    UPDATE_BUTTON_STATE(F18);
    UPDATE_BUTTON_STATE(F19);
    UPDATE_BUTTON_STATE(F20);
    UPDATE_BUTTON_STATE(F21);
    UPDATE_BUTTON_STATE(F22);
    UPDATE_BUTTON_STATE(F23);
    UPDATE_BUTTON_STATE(F24);
    UPDATE_BUTTON_STATE(Reserved110);
    UPDATE_BUTTON_STATE(Reserved111);
    UPDATE_BUTTON_STATE(Reserved112);
    UPDATE_BUTTON_STATE(Reserved113);
    UPDATE_BUTTON_STATE(Reserved114);
    UPDATE_BUTTON_STATE(Reserved115);
    UPDATE_BUTTON_STATE(Reserved116);
    UPDATE_BUTTON_STATE(Reserved117);
    UPDATE_BUTTON_STATE(NumLock);
    UPDATE_BUTTON_STATE(Scroll);
    UPDATE_BUTTON_STATE(Reserved120);
    UPDATE_BUTTON_STATE(Reserved121);
    UPDATE_BUTTON_STATE(Reserved122);
    UPDATE_BUTTON_STATE(Reserved123);
    UPDATE_BUTTON_STATE(Reserved124);
    UPDATE_BUTTON_STATE(Reserved125);
    UPDATE_BUTTON_STATE(Reserved130);
    UPDATE_BUTTON_STATE(Reserved131);
    UPDATE_BUTTON_STATE(Reserved132);
    UPDATE_BUTTON_STATE(Reserved133);
    UPDATE_BUTTON_STATE(Reserved134);
    UPDATE_BUTTON_STATE(Reserved135);
    UPDATE_BUTTON_STATE(Reserved136);
    UPDATE_BUTTON_STATE(Reserved137);
    UPDATE_BUTTON_STATE(LeftShift);
    UPDATE_BUTTON_STATE(RightShift);
    UPDATE_BUTTON_STATE(LeftControl);
    UPDATE_BUTTON_STATE(RightControl);
    UPDATE_BUTTON_STATE(LeftAlt);
    UPDATE_BUTTON_STATE(RightAlt);
    UPDATE_BUTTON_STATE(BrowserBack);
    UPDATE_BUTTON_STATE(BrowserForward);
    UPDATE_BUTTON_STATE(BrowserRefresh);
    UPDATE_BUTTON_STATE(BrowserStop);
    UPDATE_BUTTON_STATE(BrowserSearch);
    UPDATE_BUTTON_STATE(BrowserFavorites);
    UPDATE_BUTTON_STATE(BrowserHome);
    UPDATE_BUTTON_STATE(VolumeMute);
    UPDATE_BUTTON_STATE(VolumeDown);
    UPDATE_BUTTON_STATE(VolumeUp);
    UPDATE_BUTTON_STATE(MediaNextTrack);
    UPDATE_BUTTON_STATE(MediaPreviousTrack);
    UPDATE_BUTTON_STATE(MediaStop);
    UPDATE_BUTTON_STATE(MediaPlayPause);
    UPDATE_BUTTON_STATE(LaunchMail);
    UPDATE_BUTTON_STATE(SelectMedia);
    UPDATE_BUTTON_STATE(LaunchApplication1);
    UPDATE_BUTTON_STATE(LaunchApplication2);
    UPDATE_BUTTON_STATE(Reserved140);
    UPDATE_BUTTON_STATE(Reserved141);
    UPDATE_BUTTON_STATE(OemSemicolon);
    UPDATE_BUTTON_STATE(OemPlus);
    UPDATE_BUTTON_STATE(OemComma);
    UPDATE_BUTTON_STATE(OemMinus);
    UPDATE_BUTTON_STATE(OemPeriod);
    UPDATE_BUTTON_STATE(OemQuestion);
    UPDATE_BUTTON_STATE(OemTilde);
    UPDATE_BUTTON_STATE(Reserved150);
    UPDATE_BUTTON_STATE(Reserved151);
    UPDATE_BUTTON_STATE(Reserved152);
    UPDATE_BUTTON_STATE(Reserved153);
    UPDATE_BUTTON_STATE(Reserved154);
    UPDATE_BUTTON_STATE(Reserved155);
    UPDATE_BUTTON_STATE(Reserved156);
    UPDATE_BUTTON_STATE(Reserved160);
    UPDATE_BUTTON_STATE(Reserved161);
    UPDATE_BUTTON_STATE(Reserved162);
    UPDATE_BUTTON_STATE(Reserved163);
    UPDATE_BUTTON_STATE(Reserved164);
    UPDATE_BUTTON_STATE(Reserved165);
    UPDATE_BUTTON_STATE(Reserved166);
    UPDATE_BUTTON_STATE(Reserved167);
    UPDATE_BUTTON_STATE(Reserved170);
    UPDATE_BUTTON_STATE(Reserved171);
    UPDATE_BUTTON_STATE(Reserved172);
    UPDATE_BUTTON_STATE(Reserved173);
    UPDATE_BUTTON_STATE(Reserved174);
    UPDATE_BUTTON_STATE(Reserved175);
    UPDATE_BUTTON_STATE(Reserved176);
    UPDATE_BUTTON_STATE(Reserved177);
    UPDATE_BUTTON_STATE(Reserved180);
    UPDATE_BUTTON_STATE(Reserved181);
    UPDATE_BUTTON_STATE(Reserved182);
    UPDATE_BUTTON_STATE(OemOpenBrackets);
    UPDATE_BUTTON_STATE(OemPipe);
    UPDATE_BUTTON_STATE(OemCloseBrackets);
    UPDATE_BUTTON_STATE(OemQuotes);
    UPDATE_BUTTON_STATE(Oem8);
    UPDATE_BUTTON_STATE(Reserved190);
    UPDATE_BUTTON_STATE(Reserved191);
    UPDATE_BUTTON_STATE(OemBackslash);
    UPDATE_BUTTON_STATE(Reserved200);
    UPDATE_BUTTON_STATE(Reserved201);
    UPDATE_BUTTON_STATE(ProcessKey);
    UPDATE_BUTTON_STATE(Reserved210);
    UPDATE_BUTTON_STATE(Reserved211);
    UPDATE_BUTTON_STATE(Reserved220);
    UPDATE_BUTTON_STATE(Reserved221);
    UPDATE_BUTTON_STATE(Reserved222);
    UPDATE_BUTTON_STATE(Reserved223);
    UPDATE_BUTTON_STATE(Reserved224);
    UPDATE_BUTTON_STATE(Reserved225);
    UPDATE_BUTTON_STATE(Reserved226);
    UPDATE_BUTTON_STATE(Reserved227);
    UPDATE_BUTTON_STATE(Reserved230);
    UPDATE_BUTTON_STATE(Reserved231);
    UPDATE_BUTTON_STATE(Reserved232);
    UPDATE_BUTTON_STATE(OemCopy);
    UPDATE_BUTTON_STATE(OemAuto);
    UPDATE_BUTTON_STATE(OemEnlW);
    UPDATE_BUTTON_STATE(Reserved240);
    UPDATE_BUTTON_STATE(Attn);
    UPDATE_BUTTON_STATE(Crsel);
    UPDATE_BUTTON_STATE(Exsel);
    UPDATE_BUTTON_STATE(EraseEof);
    UPDATE_BUTTON_STATE(Play);
    UPDATE_BUTTON_STATE(Zoom);
    UPDATE_BUTTON_STATE(Reserved250);
    UPDATE_BUTTON_STATE(Pa1);
    UPDATE_BUTTON_STATE(OemClear);
    UPDATE_BUTTON_STATE(Reserved260);

    mLastState = mState;
}

Keyboard::Keyboard(HWND q) {
    Keyboard();

    RAWINPUTDEVICE rid[1];
    rid[0].dwFlags = 0;
    rid[0].hwndTarget = q;
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x06;

    // Register RID
    if( !RegisterRawInputDevices(rid, 1, sizeof(rid[0])) ) {
        std::cout << "Can't register Keyboard RID: " << GetLastError() << std::endl;
    }
}

void Keyboard::SetState(WPARAM w, bool Down) {
    if( w < 0 || w > 0xfe ) return;
    auto ptr = reinterpret_cast<uint32_t*>(&mState);
    unsigned int bf = 1u << (w & 0x1f);

    if( Down ) {
        ptr[(w >> 5)] |= bf;
    } else {
        ptr[(w >> 5)] &= ~bf;
    }
}

bool Keyboard::IsDown(WPARAM key) {
    return _mStates.KeyState((KeyboardUtils::Keys)key) == ButtonState::HELD; // DownState[key];
}

bool Keyboard::IsPressed(WPARAM key) {
    // TODO: Release compilation: Fix somewhy true value by default or press issues
    return _mStates.KeyState((KeyboardUtils::Keys)key) == ButtonState::PRESSED; // ((reinterpret_cast<uint32_t>(&mState) & key) == key);
}

bool Keyboard::IsReleased(WPARAM key) {
    return _mStates.KeyState((KeyboardUtils::Keys)key) == ButtonState::RELEASED; // mState.IsKeyUp((KeyboardUtils::Keys)key); // UpState[key];
}
