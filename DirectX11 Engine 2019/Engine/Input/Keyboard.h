#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma region Keys
#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A
#pragma endregion


#include "ButtonStateEnum.h"

namespace KeyboardUtils {
    enum Keys
    {
        None = 0,

        Back = 0x8,
        Tab = 0x9,

        Enter = 0xd,

        Pause = 0x13,
        CapsLock = 0x14,
        Kana = 0x15,

        Kanji = 0x19,

        Escape = 0x1b,
        ImeConvert = 0x1c,
        ImeNoConvert = 0x1d,

        Space = 0x20,
        PageUp = 0x21,
        PageDown = 0x22,
        End = 0x23,
        Home = 0x24,
        Left = 0x25,
        Up = 0x26,
        Right = 0x27,
        Down = 0x28,
        Select = 0x29,
        Print = 0x2a,
        Execute = 0x2b,
        PrintScreen = 0x2c,
        Insert = 0x2d,
        Delete = 0x2e,
        Help = 0x2f,
        D0 = 0x30,
        D1 = 0x31,
        D2 = 0x32,
        D3 = 0x33,
        D4 = 0x34,
        D5 = 0x35,
        D6 = 0x36,
        D7 = 0x37,
        D8 = 0x38,
        D9 = 0x39,

        A = 0x41,
        B = 0x42,
        C = 0x43,
        D = 0x44,
        E = 0x45,
        F = 0x46,
        G = 0x47,
        H = 0x48,
        I = 0x49,
        J = 0x4a,
        K = 0x4b,
        L = 0x4c,
        M = 0x4d,
        N = 0x4e,
        O = 0x4f,
        P = 0x50,
        Q = 0x51,
        R = 0x52,
        S = 0x53,
        T = 0x54,
        U = 0x55,
        V = 0x56,
        W = 0x57,
        X = 0x58,
        Y = 0x59,
        Z = 0x5a,
        LeftWindows = 0x5b,
        RightWindows = 0x5c,
        Apps = 0x5d,

        Sleep = 0x5f,
        NumPad0 = 0x60,
        NumPad1 = 0x61,
        NumPad2 = 0x62,
        NumPad3 = 0x63,
        NumPad4 = 0x64,
        NumPad5 = 0x65,
        NumPad6 = 0x66,
        NumPad7 = 0x67,
        NumPad8 = 0x68,
        NumPad9 = 0x69,
        Multiply = 0x6a,
        Add = 0x6b,
        Separator = 0x6c,
        Subtract = 0x6d,

        Decimal = 0x6e,
        Divide = 0x6f,
        F1 = 0x70,
        F2 = 0x71,
        F3 = 0x72,
        F4 = 0x73,
        F5 = 0x74,
        F6 = 0x75,
        F7 = 0x76,
        F8 = 0x77,
        F9 = 0x78,
        F10 = 0x79,
        F11 = 0x7a,
        F12 = 0x7b,
        F13 = 0x7c,
        F14 = 0x7d,
        F15 = 0x7e,
        F16 = 0x7f,
        F17 = 0x80,
        F18 = 0x81,
        F19 = 0x82,
        F20 = 0x83,
        F21 = 0x84,
        F22 = 0x85,
        F23 = 0x86,
        F24 = 0x87,

        NumLock = 0x90,
        Scroll = 0x91,

        LeftShift = 0xa0,
        RightShift = 0xa1,
        LeftControl = 0xa2,
        RightControl = 0xa3,
        LeftAlt = 0xa4,
        RightAlt = 0xa5,
        BrowserBack = 0xa6,
        BrowserForward = 0xa7,
        BrowserRefresh = 0xa8,
        BrowserStop = 0xa9,
        BrowserSearch = 0xaa,
        BrowserFavorites = 0xab,
        BrowserHome = 0xac,
        VolumeMute = 0xad,
        VolumeDown = 0xae,
        VolumeUp = 0xaf,
        MediaNextTrack = 0xb0,
        MediaPreviousTrack = 0xb1,
        MediaStop = 0xb2,
        MediaPlayPause = 0xb3,
        LaunchMail = 0xb4,
        SelectMedia = 0xb5,
        LaunchApplication1 = 0xb6,
        LaunchApplication2 = 0xb7,

        OemSemicolon = 0xba,
        OemPlus = 0xbb,
        OemComma = 0xbc,
        OemMinus = 0xbd,
        OemPeriod = 0xbe,
        OemQuestion = 0xbf,
        OemTilde = 0xc0,

        OemOpenBrackets = 0xdb,
        OemPipe = 0xdc,
        OemCloseBrackets = 0xdd,
        OemQuotes = 0xde,
        Oem8 = 0xdf,

        OemBackslash = 0xe2,

        ProcessKey = 0xe5,

        OemCopy = 0xf2,
        OemAuto = 0xf3,
        OemEnlW = 0xf4,

        Attn = 0xf6,
        Crsel = 0xf7,
        Exsel = 0xf8,
        EraseEof = 0xf9,
        Play = 0xfa,
        Zoom = 0xfb,

        Pa1 = 0xfd,
        OemClear = 0xfe,
    };
}

typedef unsigned int uint32_t;

class Keyboard {
private:
    struct State
    {
        bool Reserved00 : 1;
        bool Reserved01 : 1;
        bool Reserved02 : 1;
        bool Reserved03 : 1;
        bool Reserved04 : 1;
        bool Reserved05 : 1;
        bool Reserved06 : 1;
        bool Reserved07 : 1;
        bool Back : 1;              // VK_BACK, 0x8
        bool Tab : 1;               // VK_TAB, 0x9
        bool Reserved10 : 1;
        bool Reserved11 : 1;
        bool Reserved12 : 1;
        bool Enter : 1;             // VK_RETURN, 0xD
        bool Reserved20 : 1;
        bool Reserved21 : 1;
        bool Reserved30 : 1;
        bool Reserved31 : 1;
        bool Reserved32 : 1;
        bool Pause : 1;             // VK_PAUSE, 0x13
        bool CapsLock : 1;          // VK_CAPITAL, 0x14
        bool Kana : 1;              // VK_KANA, 0x15
        bool Reserved40 : 1;
        bool Reserved41 : 1;
        bool Reserved50 : 1;
        bool Kanji : 1;             // VK_KANJI, 0x19
        bool Reserved60 : 1;
        bool Escape : 1;            // VK_ESCAPE, 0x1B
        bool ImeConvert : 1;        // VK_CONVERT, 0x1C
        bool ImeNoConvert : 1;      // VK_NONCONVERT, 0x1D
        bool Reserved70 : 1;
        bool Reserved71 : 1;
        bool Space : 1;             // VK_SPACE, 0x20
        bool PageUp : 1;            // VK_PRIOR, 0x21
        bool PageDown : 1;          // VK_NEXT, 0x22
        bool End : 1;               // VK_END, 0x23
        bool Home : 1;              // VK_HOME, 0x24
        bool Left : 1;              // VK_LEFT, 0x25
        bool Up : 1;                // VK_UP, 0x26
        bool Right : 1;             // VK_RIGHT, 0x27
        bool Down : 1;              // VK_DOWN, 0x28
        bool Select : 1;            // VK_SELECT, 0x29
        bool Print : 1;             // VK_PRINT, 0x2A
        bool Execute : 1;           // VK_EXECUTE, 0x2B
        bool PrintScreen : 1;       // VK_SNAPSHOT, 0x2C
        bool Insert : 1;            // VK_INSERT, 0x2D
        bool Delete : 1;            // VK_DELETE, 0x2E
        bool Help : 1;              // VK_HELP, 0x2F
        bool D0 : 1;                // 0x30
        bool D1 : 1;                // 0x31
        bool D2 : 1;                // 0x32
        bool D3 : 1;                // 0x33
        bool D4 : 1;                // 0x34
        bool D5 : 1;                // 0x35
        bool D6 : 1;                // 0x36
        bool D7 : 1;                // 0x37
        bool D8 : 1;                // 0x38
        bool D9 : 1;                // 0x39
        bool Reserved80 : 1;
        bool Reserved81 : 1;
        bool Reserved82 : 1;
        bool Reserved83 : 1;
        bool Reserved84 : 1;
        bool Reserved85 : 1;
        bool Reserved90 : 1;
        bool A : 1;                 // 0x41
        bool B : 1;                 // 0x42
        bool C : 1;                 // 0x43
        bool D : 1;                 // 0x44
        bool E : 1;                 // 0x45
        bool F : 1;                 // 0x46
        bool G : 1;                 // 0x47
        bool H : 1;                 // 0x48
        bool I : 1;                 // 0x49
        bool J : 1;                 // 0x4A
        bool K : 1;                 // 0x4B
        bool L : 1;                 // 0x4C
        bool M : 1;                 // 0x4D
        bool N : 1;                 // 0x4E
        bool O : 1;                 // 0x4F
        bool P : 1;                 // 0x50
        bool Q : 1;                 // 0x51
        bool R : 1;                 // 0x52
        bool S : 1;                 // 0x53
        bool T : 1;                 // 0x54
        bool U : 1;                 // 0x55
        bool V : 1;                 // 0x56
        bool W : 1;                 // 0x57
        bool X : 1;                 // 0x58
        bool Y : 1;                 // 0x59
        bool Z : 1;                 // 0x5A
        bool LeftWindows : 1;       // VK_LWIN, 0x5B
        bool RightWindows : 1;      // VK_RWIN, 0x5C
        bool Apps : 1;              // VK_APPS, 0x5D
        bool Reserved100 : 1;
        bool Sleep : 1;             // VK_SLEEP, 0x5F
        bool NumPad0 : 1;           // VK_NUMPAD0, 0x60
        bool NumPad1 : 1;           // VK_NUMPAD1, 0x61
        bool NumPad2 : 1;           // VK_NUMPAD2, 0x62
        bool NumPad3 : 1;           // VK_NUMPAD3, 0x63
        bool NumPad4 : 1;           // VK_NUMPAD4, 0x64
        bool NumPad5 : 1;           // VK_NUMPAD5, 0x65
        bool NumPad6 : 1;           // VK_NUMPAD6, 0x66
        bool NumPad7 : 1;           // VK_NUMPAD7, 0x67
        bool NumPad8 : 1;           // VK_NUMPAD8, 0x68
        bool NumPad9 : 1;           // VK_NUMPAD9, 0x69
        bool Multiply : 1;          // VK_MULTIPLY, 0x6A
        bool Add : 1;               // VK_ADD, 0x6B
        bool Separator : 1;         // VK_SEPARATOR, 0x6C
        bool Subtract : 1;          // VK_SUBTRACT, 0x6D
        bool Decimal : 1;           // VK_DECIMANL, 0x6E
        bool Divide : 1;            // VK_DIVIDE, 0x6F
        bool F1 : 1;                // VK_F1, 0x70
        bool F2 : 1;                // VK_F2, 0x71
        bool F3 : 1;                // VK_F3, 0x72
        bool F4 : 1;                // VK_F4, 0x73
        bool F5 : 1;                // VK_F5, 0x74
        bool F6 : 1;                // VK_F6, 0x75
        bool F7 : 1;                // VK_F7, 0x76
        bool F8 : 1;                // VK_F8, 0x77
        bool F9 : 1;                // VK_F9, 0x78
        bool F10 : 1;               // VK_F10, 0x79
        bool F11 : 1;               // VK_F11, 0x7A
        bool F12 : 1;               // VK_F12, 0x7B
        bool F13 : 1;               // VK_F13, 0x7C
        bool F14 : 1;               // VK_F14, 0x7D
        bool F15 : 1;               // VK_F15, 0x7E
        bool F16 : 1;               // VK_F16, 0x7F
        bool F17 : 1;               // VK_F17, 0x80
        bool F18 : 1;               // VK_F18, 0x81
        bool F19 : 1;               // VK_F19, 0x82
        bool F20 : 1;               // VK_F20, 0x83
        bool F21 : 1;               // VK_F21, 0x84
        bool F22 : 1;               // VK_F22, 0x85
        bool F23 : 1;               // VK_F23, 0x86
        bool F24 : 1;               // VK_F24, 0x87
        bool Reserved110 : 1;
        bool Reserved111 : 1;
        bool Reserved112 : 1;
        bool Reserved113 : 1;
        bool Reserved114 : 1;
        bool Reserved115 : 1;
        bool Reserved116 : 1;
        bool Reserved117 : 1;
        bool NumLock : 1;           // VK_NUMLOCK, 0x90
        bool Scroll : 1;            // VK_SCROLL, 0x91
        bool Reserved120 : 1;
        bool Reserved121 : 1;
        bool Reserved122 : 1;
        bool Reserved123 : 1;
        bool Reserved124 : 1;
        bool Reserved125 : 1;
        bool Reserved130 : 1;
        bool Reserved131 : 1;
        bool Reserved132 : 1;
        bool Reserved133 : 1;
        bool Reserved134 : 1;
        bool Reserved135 : 1;
        bool Reserved136 : 1;
        bool Reserved137 : 1;
        bool LeftShift : 1;         // VK_LSHIFT, 0xA0
        bool RightShift : 1;        // VK_RSHIFT, 0xA1
        bool LeftControl : 1;       // VK_LCONTROL, 0xA2
        bool RightControl : 1;      // VK_RCONTROL, 0xA3
        bool LeftAlt : 1;           // VK_LMENU, 0xA4
        bool RightAlt : 1;          // VK_RMENU, 0xA5
        bool BrowserBack : 1;       // VK_BROWSER_BACK, 0xA6
        bool BrowserForward : 1;    // VK_BROWSER_FORWARD, 0xA7
        bool BrowserRefresh : 1;    // VK_BROWSER_REFRESH, 0xA8
        bool BrowserStop : 1;       // VK_BROWSER_STOP, 0xA9
        bool BrowserSearch : 1;     // VK_BROWSER_SEARCH, 0xAA
        bool BrowserFavorites : 1;  // VK_BROWSER_FAVORITES, 0xAB
        bool BrowserHome : 1;       // VK_BROWSER_HOME, 0xAC
        bool VolumeMute : 1;        // VK_VOLUME_MUTE, 0xAD
        bool VolumeDown : 1;        // VK_VOLUME_DOWN, 0xAE
        bool VolumeUp : 1;          // VK_VOLUME_UP, 0xAF
        bool MediaNextTrack : 1;    // VK_MEDIA_NEXT_TRACK, 0xB0
        bool MediaPreviousTrack : 1;// VK_MEDIA_PREV_TRACK, 0xB1
        bool MediaStop : 1;         // VK_MEDIA_STOP, 0xB2
        bool MediaPlayPause : 1;    // VK_MEDIA_PLAY_PAUSE, 0xB3
        bool LaunchMail : 1;        // VK_LAUNCH_MAIL, 0xB4
        bool SelectMedia : 1;       // VK_LAUNCH_MEDIA_SELECT, 0xB5
        bool LaunchApplication1 : 1;// VK_LAUNCH_APP1, 0xB6
        bool LaunchApplication2 : 1;// VK_LAUNCH_APP2, 0xB7
        bool Reserved140 : 1;
        bool Reserved141 : 1;
        bool OemSemicolon : 1;      // VK_OEM_1, 0xBA
        bool OemPlus : 1;           // VK_OEM_PLUS, 0xBB
        bool OemComma : 1;          // VK_OEM_COMMA, 0xBC
        bool OemMinus : 1;          // VK_OEM_MINUS, 0xBD
        bool OemPeriod : 1;         // VK_OEM_PERIOD, 0xBE
        bool OemQuestion : 1;       // VK_OEM_2, 0xBF
        bool OemTilde : 1;          // VK_OEM_3, 0xC0
        bool Reserved150 : 1;
        bool Reserved151 : 1;
        bool Reserved152 : 1;
        bool Reserved153 : 1;
        bool Reserved154 : 1;
        bool Reserved155 : 1;
        bool Reserved156 : 1;
        bool Reserved160 : 1;
        bool Reserved161 : 1;
        bool Reserved162 : 1;
        bool Reserved163 : 1;
        bool Reserved164 : 1;
        bool Reserved165 : 1;
        bool Reserved166 : 1;
        bool Reserved167 : 1;
        bool Reserved170 : 1;
        bool Reserved171 : 1;
        bool Reserved172 : 1;
        bool Reserved173 : 1;
        bool Reserved174 : 1;
        bool Reserved175 : 1;
        bool Reserved176 : 1;
        bool Reserved177 : 1;
        bool Reserved180 : 1;
        bool Reserved181 : 1;
        bool Reserved182 : 1;
        bool OemOpenBrackets : 1;   // VK_OEM_4, 0xDB
        bool OemPipe : 1;           // VK_OEM_5, 0xDC
        bool OemCloseBrackets : 1;  // VK_OEM_6, 0xDD
        bool OemQuotes : 1;         // VK_OEM_7, 0xDE
        bool Oem8 : 1;              // VK_OEM_8, 0xDF
        bool Reserved190 : 1;
        bool Reserved191 : 1;
        bool OemBackslash : 1;      // VK_OEM_102, 0xE2
        bool Reserved200 : 1;
        bool Reserved201 : 1;
        bool ProcessKey : 1;        // VK_PROCESSKEY, 0xE5
        bool Reserved210 : 1;
        bool Reserved211 : 1;
        bool Reserved220 : 1;
        bool Reserved221 : 1;
        bool Reserved222 : 1;
        bool Reserved223 : 1;
        bool Reserved224 : 1;
        bool Reserved225 : 1;
        bool Reserved226 : 1;
        bool Reserved227 : 1;
        bool Reserved230 : 1;
        bool Reserved231 : 1;
        bool Reserved232 : 1;
        bool OemCopy : 1;           // 0XF2
        bool OemAuto : 1;           // 0xF3
        bool OemEnlW : 1;           // 0xF4
        bool Reserved240 : 1;
        bool Attn : 1;              // VK_ATTN, 0xF6
        bool Crsel : 1;             // VK_CRSEL, 0xF7
        bool Exsel : 1;             // VK_EXSEL, 0xF8
        bool EraseEof : 1;          // VK_EREOF, 0xF9
        bool Play : 1;              // VK_PLAY, 0xFA
        bool Zoom : 1;              // VK_ZOOM, 0xFB
        bool Reserved250 : 1;
        bool Pa1 : 1;               // VK_PA1, 0xFD
        bool OemClear : 1;          // VK_OEM_CLEAR, 0xFE
        bool Reserved260 : 1;
    };

    State mState;
    State mLastState;

    struct {
        struct {
            ButtonState Reserved00;
            ButtonState Reserved01;
            ButtonState Reserved02;
            ButtonState Reserved03;
            ButtonState Reserved04;
            ButtonState Reserved05;
            ButtonState Reserved06;
            ButtonState Reserved07;
            ButtonState Back;               // VK_BACK, 0x8
            ButtonState Tab;                // VK_TAB, 0x9
            ButtonState Reserved10;
            ButtonState Reserved11;
            ButtonState Reserved12;
            ButtonState Enter;              // VK_RETURN, 0xD
            ButtonState Reserved20;
            ButtonState Reserved21;
            ButtonState Reserved30;
            ButtonState Reserved31;
            ButtonState Reserved32;
            ButtonState Pause;              // VK_PAUSE, 0x13
            ButtonState CapsLock;           // VK_CAPITAL, 0x14
            ButtonState Kana;               // VK_KANA, 0x15
            ButtonState Reserved40;
            ButtonState Reserved41;
            ButtonState Reserved50;
            ButtonState Kanji;              // VK_KANJI, 0x19
            ButtonState Reserved60;
            ButtonState Escape;             // VK_ESCAPE, 0x1B
            ButtonState ImeConvert;         // VK_CONVERT, 0x1C
            ButtonState ImeNoConvert;       // VK_NONCONVERT, 0x1D
            ButtonState Reserved70;
            ButtonState Reserved71;
            ButtonState Space;              // VK_SPACE, 0x20
            ButtonState PageUp;             // VK_PRIOR, 0x21
            ButtonState PageDown;           // VK_NEXT, 0x22
            ButtonState End;                // VK_END, 0x23
            ButtonState Home;               // VK_HOME, 0x24
            ButtonState Left;               // VK_LEFT, 0x25
            ButtonState Up;                 // VK_UP, 0x26
            ButtonState Right;              // VK_RIGHT, 0x27
            ButtonState Down;               // VK_DOWN, 0x28
            ButtonState Select;             // VK_SELECT, 0x29
            ButtonState Print;              // VK_PRINT, 0x2A
            ButtonState Execute;            // VK_EXECUTE, 0x2B
            ButtonState PrintScreen;        // VK_SNAPSHOT, 0x2C
            ButtonState Insert;             // VK_INSERT, 0x2D
            ButtonState Delete;             // VK_DELETE, 0x2E
            ButtonState Help;               // VK_HELP, 0x2F
            ButtonState D0;                 // 0x30
            ButtonState D1;                 // 0x31
            ButtonState D2;                 // 0x32
            ButtonState D3;                 // 0x33
            ButtonState D4;                 // 0x34
            ButtonState D5;                 // 0x35
            ButtonState D6;                 // 0x36
            ButtonState D7;                 // 0x37
            ButtonState D8;                 // 0x38
            ButtonState D9;                 // 0x39
            ButtonState Reserved80;
            ButtonState Reserved81;
            ButtonState Reserved82;
            ButtonState Reserved83;
            ButtonState Reserved84;
            ButtonState Reserved85;
            ButtonState Reserved90;
            ButtonState A;                  // 0x41
            ButtonState B;                  // 0x42
            ButtonState C;                  // 0x43
            ButtonState D;                  // 0x44
            ButtonState E;                  // 0x45
            ButtonState F;                  // 0x46
            ButtonState G;                  // 0x47
            ButtonState H;                  // 0x48
            ButtonState I;                  // 0x49
            ButtonState J;                  // 0x4A
            ButtonState K;                  // 0x4B
            ButtonState L;                  // 0x4C
            ButtonState M;                  // 0x4D
            ButtonState N;                  // 0x4E
            ButtonState O;                  // 0x4F
            ButtonState P;                  // 0x50
            ButtonState Q;                  // 0x51
            ButtonState R;                  // 0x52
            ButtonState S;                  // 0x53
            ButtonState T;                  // 0x54
            ButtonState U;                  // 0x55
            ButtonState V;                  // 0x56
            ButtonState W;                  // 0x57
            ButtonState X;                  // 0x58
            ButtonState Y;                  // 0x59
            ButtonState Z;                  // 0x5A
            ButtonState LeftWindows;        // VK_LWIN, 0x5B
            ButtonState RightWindows;       // VK_RWIN, 0x5C
            ButtonState Apps;               // VK_APPS, 0x5D
            ButtonState Reserved100;
            ButtonState Sleep;              // VK_SLEEP, 0x5F
            ButtonState NumPad0;            // VK_NUMPAD0, 0x60
            ButtonState NumPad1;            // VK_NUMPAD1, 0x61
            ButtonState NumPad2;            // VK_NUMPAD2, 0x62
            ButtonState NumPad3;            // VK_NUMPAD3, 0x63
            ButtonState NumPad4;            // VK_NUMPAD4, 0x64
            ButtonState NumPad5;            // VK_NUMPAD5, 0x65
            ButtonState NumPad6;            // VK_NUMPAD6, 0x66
            ButtonState NumPad7;            // VK_NUMPAD7, 0x67
            ButtonState NumPad8;            // VK_NUMPAD8, 0x68
            ButtonState NumPad9;            // VK_NUMPAD9, 0x69
            ButtonState Multiply;           // VK_MULTIPLY, 0x6A
            ButtonState Add;                // VK_ADD, 0x6B
            ButtonState Separator;          // VK_SEPARATOR, 0x6C
            ButtonState Subtract;           // VK_SUBTRACT, 0x6D
            ButtonState Decimal;            // VK_DECIMANL, 0x6E
            ButtonState Divide;             // VK_DIVIDE, 0x6F
            ButtonState F1;                 // VK_F1, 0x70
            ButtonState F2;                 // VK_F2, 0x71
            ButtonState F3;                 // VK_F3, 0x72
            ButtonState F4;                 // VK_F4, 0x73
            ButtonState F5;                 // VK_F5, 0x74
            ButtonState F6;                 // VK_F6, 0x75
            ButtonState F7;                 // VK_F7, 0x76
            ButtonState F8;                 // VK_F8, 0x77
            ButtonState F9;                 // VK_F9, 0x78
            ButtonState F10;                // VK_F10, 0x79
            ButtonState F11;                // VK_F11, 0x7A
            ButtonState F12;                // VK_F12, 0x7B
            ButtonState F13;                // VK_F13, 0x7C
            ButtonState F14;                // VK_F14, 0x7D
            ButtonState F15;                // VK_F15, 0x7E
            ButtonState F16;                // VK_F16, 0x7F
            ButtonState F17;                // VK_F17, 0x80
            ButtonState F18;                // VK_F18, 0x81
            ButtonState F19;                // VK_F19, 0x82
            ButtonState F20;                // VK_F20, 0x83
            ButtonState F21;                // VK_F21, 0x84
            ButtonState F22;                // VK_F22, 0x85
            ButtonState F23;                // VK_F23, 0x86
            ButtonState F24;                // VK_F24, 0x87
            ButtonState Reserved110;
            ButtonState Reserved111;
            ButtonState Reserved112;
            ButtonState Reserved113;
            ButtonState Reserved114;
            ButtonState Reserved115;
            ButtonState Reserved116;
            ButtonState Reserved117;
            ButtonState NumLock;            // VK_NUMLOCK, 0x90
            ButtonState Scroll;             // VK_SCROLL, 0x91
            ButtonState Reserved120;
            ButtonState Reserved121;
            ButtonState Reserved122;
            ButtonState Reserved123;
            ButtonState Reserved124;
            ButtonState Reserved125;
            ButtonState Reserved130;
            ButtonState Reserved131;
            ButtonState Reserved132;
            ButtonState Reserved133;
            ButtonState Reserved134;
            ButtonState Reserved135;
            ButtonState Reserved136;
            ButtonState Reserved137;
            ButtonState LeftShift;          // VK_LSHIFT, 0xA0
            ButtonState RightShift;         // VK_RSHIFT, 0xA1
            ButtonState LeftControl;        // VK_LCONTROL, 0xA2
            ButtonState RightControl;       // VK_RCONTROL, 0xA3
            ButtonState LeftAlt;            // VK_LMENU, 0xA4
            ButtonState RightAlt;           // VK_RMENU, 0xA5
            ButtonState BrowserBack;        // VK_BROWSER_BACK, 0xA6
            ButtonState BrowserForward;     // VK_BROWSER_FORWARD, 0xA7
            ButtonState BrowserRefresh;     // VK_BROWSER_REFRESH, 0xA8
            ButtonState BrowserStop;        // VK_BROWSER_STOP, 0xA9
            ButtonState BrowserSearch;      // VK_BROWSER_SEARCH, 0xAA
            ButtonState BrowserFavorites;   // VK_BROWSER_FAVORITES, 0xAB
            ButtonState BrowserHome;        // VK_BROWSER_HOME, 0xAC
            ButtonState VolumeMute;         // VK_VOLUME_MUTE, 0xAD
            ButtonState VolumeDown;         // VK_VOLUME_DOWN, 0xAE
            ButtonState VolumeUp;           // VK_VOLUME_UP, 0xAF
            ButtonState MediaNextTrack;     // VK_MEDIA_NEXT_TRACK, 0xB0
            ButtonState MediaPreviousTrack; // VK_MEDIA_PREV_TRACK, 0xB1
            ButtonState MediaStop;          // VK_MEDIA_STOP, 0xB2
            ButtonState MediaPlayPause;     // VK_MEDIA_PLAY_PAUSE, 0xB3
            ButtonState LaunchMail;         // VK_LAUNCH_MAIL, 0xB4
            ButtonState SelectMedia;        // VK_LAUNCH_MEDIA_SELECT, 0xB5
            ButtonState LaunchApplication1; // VK_LAUNCH_APP1, 0xB6
            ButtonState LaunchApplication2; // VK_LAUNCH_APP2, 0xB7
            ButtonState Reserved140;
            ButtonState Reserved141;
            ButtonState OemSemicolon;       // VK_OEM_1, 0xBA
            ButtonState OemPlus;            // VK_OEM_PLUS, 0xBB
            ButtonState OemComma;           // VK_OEM_COMMA, 0xBC
            ButtonState OemMinus;           // VK_OEM_MINUS, 0xBD
            ButtonState OemPeriod;          // VK_OEM_PERIOD, 0xBE
            ButtonState OemQuestion;        // VK_OEM_2, 0xBF
            ButtonState OemTilde;           // VK_OEM_3, 0xC0
            ButtonState Reserved150;
            ButtonState Reserved151;
            ButtonState Reserved152;
            ButtonState Reserved153;
            ButtonState Reserved154;
            ButtonState Reserved155;
            ButtonState Reserved156;
            ButtonState Reserved160;
            ButtonState Reserved161;
            ButtonState Reserved162;
            ButtonState Reserved163;
            ButtonState Reserved164;
            ButtonState Reserved165;
            ButtonState Reserved166;
            ButtonState Reserved167;
            ButtonState Reserved170;
            ButtonState Reserved171;
            ButtonState Reserved172;
            ButtonState Reserved173;
            ButtonState Reserved174;
            ButtonState Reserved175;
            ButtonState Reserved176;
            ButtonState Reserved177;
            ButtonState Reserved180;
            ButtonState Reserved181;
            ButtonState Reserved182;
            ButtonState OemOpenBrackets;    // VK_OEM_4, 0xDB
            ButtonState OemPipe;            // VK_OEM_5, 0xDC
            ButtonState OemCloseBrackets;   // VK_OEM_6, 0xDD
            ButtonState OemQuotes;          // VK_OEM_7, 0xDE
            ButtonState Oem8;               // VK_OEM_8, 0xDF
            ButtonState Reserved190;
            ButtonState Reserved191;
            ButtonState OemBackslash;       // VK_OEM_102, 0xE2
            ButtonState Reserved200;
            ButtonState Reserved201;
            ButtonState ProcessKey;         // VK_PROCESSKEY, 0xE5
            ButtonState Reserved210;
            ButtonState Reserved211;
            ButtonState Reserved220;
            ButtonState Reserved221;
            ButtonState Reserved222;
            ButtonState Reserved223;
            ButtonState Reserved224;
            ButtonState Reserved225;
            ButtonState Reserved226;
            ButtonState Reserved227;
            ButtonState Reserved230;
            ButtonState Reserved231;
            ButtonState Reserved232;
            ButtonState OemCopy;            // 0XF2
            ButtonState OemAuto;            // 0xF3
            ButtonState OemEnlW;            // 0xF4
            ButtonState Reserved240;
            ButtonState Attn;               // VK_ATTN, 0xF6
            ButtonState Crsel;              // VK_CRSEL, 0xF7
            ButtonState Exsel;              // VK_EXSEL, 0xF8
            ButtonState EraseEof;           // VK_EREOF, 0xF9
            ButtonState Play;               // VK_PLAY, 0xFA
            ButtonState Zoom;               // VK_ZOOM, 0xFB
            ButtonState Reserved250;
            ButtonState Pa1;                // VK_PA1, 0xFD
            ButtonState OemClear;           // VK_OEM_CLEAR, 0xFE
            ButtonState Reserved260;
        } __Member;

        ButtonState KeyState(KeyboardUtils::Keys key) const {
            const ButtonState* ptr = reinterpret_cast<const ButtonState*>(&__Member);
            if( key >= 0x8 && key <= 0xFE ) {
                return ptr[key];
            }

            return ButtonState::UP;
        }
    } _mStates;
public:
    Keyboard();

    void Refresh();

    Keyboard(HWND q);

    void SetState(WPARAM w, bool Down);
    bool IsDown(WPARAM key);
    bool IsPressed(WPARAM key);
    bool IsReleased(WPARAM key);
};
