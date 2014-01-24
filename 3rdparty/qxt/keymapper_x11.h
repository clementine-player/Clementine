#ifndef KEYMAPPER_X11_H
#define KEYMAPPER_X11_H

// (davidsansome) Nicked from qkeymapper_x11.cpp

#include <Qt>

#define XK_MISCELLANY
#define XK_LATIN1
#define XK_KOREAN
#define XK_XKB_KEYS
#include <X11/keysymdef.h>

//
// Keyboard event translation
//

#ifndef XK_ISO_Left_Tab
#define XK_ISO_Left_Tab         0xFE20
#endif

#ifndef XK_dead_hook
#define XK_dead_hook            0xFE61
#endif

#ifndef XK_dead_horn
#define XK_dead_horn            0xFE62
#endif

#ifndef XK_Codeinput
#define XK_Codeinput            0xFF37
#endif

#ifndef XK_Kanji_Bangou
#define XK_Kanji_Bangou         0xFF37 /* same as codeinput */
#endif

// Fix old X libraries
#ifndef XK_KP_Home
#define XK_KP_Home              0xFF95
#endif
#ifndef XK_KP_Left
#define XK_KP_Left              0xFF96
#endif
#ifndef XK_KP_Up
#define XK_KP_Up                0xFF97
#endif
#ifndef XK_KP_Right
#define XK_KP_Right             0xFF98
#endif
#ifndef XK_KP_Down
#define XK_KP_Down              0xFF99
#endif
#ifndef XK_KP_Prior
#define XK_KP_Prior             0xFF9A
#endif
#ifndef XK_KP_Next
#define XK_KP_Next              0xFF9B
#endif
#ifndef XK_KP_End
#define XK_KP_End               0xFF9C
#endif
#ifndef XK_KP_Insert
#define XK_KP_Insert            0xFF9E
#endif
#ifndef XK_KP_Delete
#define XK_KP_Delete            0xFF9F
#endif

// the next lines are taken from XFree > 4.0 (X11/XF86keysyms.h), defining some special
// multimedia keys. They are included here as not every system has them.
#define XF86XK_Standby          0x1008FF10
#define XF86XK_AudioLowerVolume 0x1008FF11
#define XF86XK_AudioMute        0x1008FF12
#define XF86XK_AudioRaiseVolume 0x1008FF13
#define XF86XK_AudioPlay        0x1008FF14
#define XF86XK_AudioStop        0x1008FF15
#define XF86XK_AudioPrev        0x1008FF16
#define XF86XK_AudioNext        0x1008FF17
#define XF86XK_HomePage         0x1008FF18
#define XF86XK_Calculator       0x1008FF1D
#define XF86XK_Mail             0x1008FF19
#define XF86XK_Start            0x1008FF1A
#define XF86XK_Search           0x1008FF1B
#define XF86XK_AudioRecord      0x1008FF1C
#define XF86XK_Back             0x1008FF26
#define XF86XK_Forward          0x1008FF27
#define XF86XK_Stop             0x1008FF28
#define XF86XK_Refresh          0x1008FF29
#define XF86XK_Favorites        0x1008FF30
#define XF86XK_AudioPause       0x1008FF31
#define XF86XK_AudioMedia       0x1008FF32
#define XF86XK_MyComputer       0x1008FF33
#define XF86XK_OpenURL          0x1008FF38
#define XF86XK_Launch0          0x1008FF40
#define XF86XK_Launch1          0x1008FF41
#define XF86XK_Launch2          0x1008FF42
#define XF86XK_Launch3          0x1008FF43
#define XF86XK_Launch4          0x1008FF44
#define XF86XK_Launch5          0x1008FF45
#define XF86XK_Launch6          0x1008FF46
#define XF86XK_Launch7          0x1008FF47
#define XF86XK_Launch8          0x1008FF48
#define XF86XK_Launch9          0x1008FF49
#define XF86XK_LaunchA          0x1008FF4A
#define XF86XK_LaunchB          0x1008FF4B
#define XF86XK_LaunchC          0x1008FF4C
#define XF86XK_LaunchD          0x1008FF4D
#define XF86XK_LaunchE          0x1008FF4E
#define XF86XK_LaunchF          0x1008FF4F
// end of XF86keysyms.h

// Special keys used by Qtopia, mapped into the X11 private keypad range.
#define QTOPIAXK_Select         0x11000601
#define QTOPIAXK_Yes            0x11000602
#define QTOPIAXK_No             0x11000603
#define QTOPIAXK_Cancel         0x11000604
#define QTOPIAXK_Printer        0x11000605
#define QTOPIAXK_Execute        0x11000606
#define QTOPIAXK_Sleep          0x11000607
#define QTOPIAXK_Play           0x11000608
#define QTOPIAXK_Zoom           0x11000609
#define QTOPIAXK_Context1       0x1100060A
#define QTOPIAXK_Context2       0x1100060B
#define QTOPIAXK_Context3       0x1100060C
#define QTOPIAXK_Context4       0x1100060D
#define QTOPIAXK_Call           0x1100060E
#define QTOPIAXK_Hangup         0x1100060F
#define QTOPIAXK_Flip           0x11000610

// keyboard mapping table
static const unsigned int KeyTbl[] = {

    // misc keys

    XK_Escape,                  Qt::Key_Escape,
    XK_Tab,                     Qt::Key_Tab,
    XK_ISO_Left_Tab,            Qt::Key_Backtab,
    XK_BackSpace,               Qt::Key_Backspace,
    XK_Return,                  Qt::Key_Return,
    XK_Insert,                  Qt::Key_Insert,
    XK_Delete,                  Qt::Key_Delete,
    XK_Clear,                   Qt::Key_Delete,
    XK_Pause,                   Qt::Key_Pause,
    XK_Print,                   Qt::Key_Print,
    0x1005FF60,                 Qt::Key_SysReq,         // hardcoded Sun SysReq
    0x1007ff00,                 Qt::Key_SysReq,         // hardcoded X386 SysReq

    // cursor movement

    XK_Home,                    Qt::Key_Home,
    XK_End,                     Qt::Key_End,
    XK_Left,                    Qt::Key_Left,
    XK_Up,                      Qt::Key_Up,
    XK_Right,                   Qt::Key_Right,
    XK_Down,                    Qt::Key_Down,
    XK_Prior,                   Qt::Key_PageUp,
    XK_Next,                    Qt::Key_PageDown,

    // modifiers

    XK_Shift_L,                 Qt::Key_Shift,
    XK_Shift_R,                 Qt::Key_Shift,
    XK_Shift_Lock,              Qt::Key_Shift,
    XK_Control_L,               Qt::Key_Control,
    XK_Control_R,               Qt::Key_Control,
    XK_Meta_L,                  Qt::Key_Meta,
    XK_Meta_R,                  Qt::Key_Meta,
    XK_Alt_L,                   Qt::Key_Alt,
    XK_Alt_R,                   Qt::Key_Alt,
    XK_Caps_Lock,               Qt::Key_CapsLock,
    XK_Num_Lock,                Qt::Key_NumLock,
    XK_Scroll_Lock,             Qt::Key_ScrollLock,
    XK_Super_L,                 Qt::Key_Super_L,
    XK_Super_R,                 Qt::Key_Super_R,
    XK_Menu,                    Qt::Key_Menu,
    XK_Hyper_L,                 Qt::Key_Hyper_L,
    XK_Hyper_R,                 Qt::Key_Hyper_R,
    XK_Help,                    Qt::Key_Help,
    0x1000FF74,                 Qt::Key_Backtab,        // hardcoded HP backtab
    0x1005FF10,                 Qt::Key_F11,            // hardcoded Sun F36 (labeled F11)
    0x1005FF11,                 Qt::Key_F12,            // hardcoded Sun F37 (labeled F12)

    // numeric and function keypad keys

    XK_KP_Enter,                Qt::Key_Enter,

    // special and additional keys

    XK_Clear,                   Qt::Key_Clear,
    XK_Delete,                  Qt::Key_Delete,
    XK_space,                   Qt::Key_Space,
    XK_exclam,                  Qt::Key_Exclam,
    XK_quotedbl,                Qt::Key_QuoteDbl,
    XK_numbersign,              Qt::Key_NumberSign,
    XK_dollar,                  Qt::Key_Dollar,
    XK_percent,                 Qt::Key_Percent,
    XK_ampersand,               Qt::Key_Ampersand,
    XK_apostrophe,              Qt::Key_Apostrophe,
    XK_parenleft,               Qt::Key_ParenLeft,
    XK_parenright,              Qt::Key_ParenRight,
    XK_asterisk,                Qt::Key_Asterisk,
    XK_plus,                    Qt::Key_Plus,
    XK_comma,                   Qt::Key_Comma,
    XK_minus,                   Qt::Key_Minus,
    XK_period,                  Qt::Key_Period,
    XK_slash,                   Qt::Key_Slash,
    XK_colon,                   Qt::Key_Colon,
    XK_semicolon,               Qt::Key_Semicolon,
    XK_less,                    Qt::Key_Less,
    XK_equal,                   Qt::Key_Equal,
    XK_greater,                 Qt::Key_Greater,
    XK_question,                Qt::Key_Question,
    XK_bracketleft,             Qt::Key_BracketLeft,
    XK_backslash,               Qt::Key_Backslash,
    XK_bracketright,            Qt::Key_BracketRight,
    XK_asciicircum,             Qt::Key_AsciiCircum,
    XK_underscore,              Qt::Key_Underscore,

    // International input method support keys

    // International & multi-key character composition
    XK_ISO_Level3_Shift,        Qt::Key_AltGr,
    XK_Multi_key,		Qt::Key_Multi_key,
    XK_Codeinput,		Qt::Key_Codeinput,
    XK_SingleCandidate,		Qt::Key_SingleCandidate,
    XK_MultipleCandidate,	Qt::Key_MultipleCandidate,
    XK_PreviousCandidate,	Qt::Key_PreviousCandidate,

    // Misc Functions
    XK_Mode_switch,		Qt::Key_Mode_switch,
    XK_script_switch,		Qt::Key_Mode_switch,

    // Japanese keyboard support
    XK_Kanji,			Qt::Key_Kanji,
    XK_Muhenkan,		Qt::Key_Muhenkan,
    //XK_Henkan_Mode,		Qt::Key_Henkan_Mode,
    XK_Henkan_Mode,		Qt::Key_Henkan,
    XK_Henkan,			Qt::Key_Henkan,
    XK_Romaji,			Qt::Key_Romaji,
    XK_Hiragana,		Qt::Key_Hiragana,
    XK_Katakana,		Qt::Key_Katakana,
    XK_Hiragana_Katakana,	Qt::Key_Hiragana_Katakana,
    XK_Zenkaku,			Qt::Key_Zenkaku,
    XK_Hankaku,			Qt::Key_Hankaku,
    XK_Zenkaku_Hankaku,		Qt::Key_Zenkaku_Hankaku,
    XK_Touroku,			Qt::Key_Touroku,
    XK_Massyo,			Qt::Key_Massyo,
    XK_Kana_Lock,		Qt::Key_Kana_Lock,
    XK_Kana_Shift,		Qt::Key_Kana_Shift,
    XK_Eisu_Shift,		Qt::Key_Eisu_Shift,
    XK_Eisu_toggle,		Qt::Key_Eisu_toggle,
    //XK_Kanji_Bangou,		Qt::Key_Kanji_Bangou,
    //XK_Zen_Koho,		Qt::Key_Zen_Koho,
    //XK_Mae_Koho,		Qt::Key_Mae_Koho,
    XK_Kanji_Bangou,		Qt::Key_Codeinput,
    XK_Zen_Koho,		Qt::Key_MultipleCandidate,
    XK_Mae_Koho,		Qt::Key_PreviousCandidate,

#ifdef XK_KOREAN
    // Korean keyboard support
    XK_Hangul,			Qt::Key_Hangul,
    XK_Hangul_Start,		Qt::Key_Hangul_Start,
    XK_Hangul_End,		Qt::Key_Hangul_End,
    XK_Hangul_Hanja,		Qt::Key_Hangul_Hanja,
    XK_Hangul_Jamo,		Qt::Key_Hangul_Jamo,
    XK_Hangul_Romaja,		Qt::Key_Hangul_Romaja,
    //XK_Hangul_Codeinput,	Qt::Key_Hangul_Codeinput,
    XK_Hangul_Codeinput,	Qt::Key_Codeinput,
    XK_Hangul_Jeonja,		Qt::Key_Hangul_Jeonja,
    XK_Hangul_Banja,		Qt::Key_Hangul_Banja,
    XK_Hangul_PreHanja,		Qt::Key_Hangul_PreHanja,
    XK_Hangul_PostHanja,	Qt::Key_Hangul_PostHanja,
    //XK_Hangul_SingleCandidate,Qt::Key_Hangul_SingleCandidate,
    //XK_Hangul_MultipleCandidate,Qt::Key_Hangul_MultipleCandidate,
    //XK_Hangul_PreviousCandidate,Qt::Key_Hangul_PreviousCandidate,
    XK_Hangul_SingleCandidate,	Qt::Key_SingleCandidate,
    XK_Hangul_MultipleCandidate,Qt::Key_MultipleCandidate,
    XK_Hangul_PreviousCandidate,Qt::Key_PreviousCandidate,
    XK_Hangul_Special,		Qt::Key_Hangul_Special,
    //XK_Hangul_switch,		Qt::Key_Hangul_switch,
    XK_Hangul_switch,		Qt::Key_Mode_switch,
#endif  // XK_KOREAN

    // dead keys
    XK_dead_grave,              Qt::Key_Dead_Grave,
    XK_dead_acute,              Qt::Key_Dead_Acute,
    XK_dead_circumflex,         Qt::Key_Dead_Circumflex,
    XK_dead_tilde,              Qt::Key_Dead_Tilde,
    XK_dead_macron,             Qt::Key_Dead_Macron,
    XK_dead_breve,              Qt::Key_Dead_Breve,
    XK_dead_abovedot,           Qt::Key_Dead_Abovedot,
    XK_dead_diaeresis,          Qt::Key_Dead_Diaeresis,
    XK_dead_abovering,          Qt::Key_Dead_Abovering,
    XK_dead_doubleacute,        Qt::Key_Dead_Doubleacute,
    XK_dead_caron,              Qt::Key_Dead_Caron,
    XK_dead_cedilla,            Qt::Key_Dead_Cedilla,
    XK_dead_ogonek,             Qt::Key_Dead_Ogonek,
    XK_dead_iota,               Qt::Key_Dead_Iota,
    XK_dead_voiced_sound,       Qt::Key_Dead_Voiced_Sound,
    XK_dead_semivoiced_sound,   Qt::Key_Dead_Semivoiced_Sound,
    XK_dead_belowdot,           Qt::Key_Dead_Belowdot,
    XK_dead_hook,               Qt::Key_Dead_Hook,
    XK_dead_horn,               Qt::Key_Dead_Horn,

    // Special multimedia keys
    // currently only tested with MS internet keyboard

    // browsing keys
    XF86XK_Back,                Qt::Key_Back,
    XF86XK_Forward,             Qt::Key_Forward,
    XF86XK_Stop,                Qt::Key_Stop,
    XF86XK_Refresh,             Qt::Key_Refresh,
    XF86XK_Favorites,           Qt::Key_Favorites,
    XF86XK_AudioMedia,          Qt::Key_LaunchMedia,
    XF86XK_OpenURL,             Qt::Key_OpenUrl,
    XF86XK_HomePage,            Qt::Key_HomePage,
    XF86XK_Search,              Qt::Key_Search,

    // media keys
    XF86XK_AudioLowerVolume,    Qt::Key_VolumeDown,
    XF86XK_AudioMute,           Qt::Key_VolumeMute,
    XF86XK_AudioRaiseVolume,    Qt::Key_VolumeUp,
    XF86XK_AudioPlay,           Qt::Key_MediaPlay,
    XF86XK_AudioStop,           Qt::Key_MediaStop,
    XF86XK_AudioPrev,           Qt::Key_MediaPrevious,
    XF86XK_AudioNext,           Qt::Key_MediaNext,
    XF86XK_AudioRecord,         Qt::Key_MediaRecord,

    // launch keys
    XF86XK_Mail,                Qt::Key_LaunchMail,
    XF86XK_MyComputer,          Qt::Key_Launch0,
    XF86XK_Calculator,          Qt::Key_Launch1,
    XF86XK_Standby,             Qt::Key_Standby,

    XF86XK_Launch0,             Qt::Key_Launch2,
    XF86XK_Launch1,             Qt::Key_Launch3,
    XF86XK_Launch2,             Qt::Key_Launch4,
    XF86XK_Launch3,             Qt::Key_Launch5,
    XF86XK_Launch4,             Qt::Key_Launch6,
    XF86XK_Launch5,             Qt::Key_Launch7,
    XF86XK_Launch6,             Qt::Key_Launch8,
    XF86XK_Launch7,             Qt::Key_Launch9,
    XF86XK_Launch8,             Qt::Key_LaunchA,
    XF86XK_Launch9,             Qt::Key_LaunchB,
    XF86XK_LaunchA,             Qt::Key_LaunchC,
    XF86XK_LaunchB,             Qt::Key_LaunchD,
    XF86XK_LaunchC,             Qt::Key_LaunchE,
    XF86XK_LaunchD,             Qt::Key_LaunchF,

    // Qtopia keys
    QTOPIAXK_Select,            Qt::Key_Select,
    QTOPIAXK_Yes,               Qt::Key_Yes,
    QTOPIAXK_No,                Qt::Key_No,
    QTOPIAXK_Cancel,            Qt::Key_Cancel,
    QTOPIAXK_Printer,           Qt::Key_Printer,
    QTOPIAXK_Execute,           Qt::Key_Execute,
    QTOPIAXK_Sleep,             Qt::Key_Sleep,
    QTOPIAXK_Play,              Qt::Key_Play,
    QTOPIAXK_Zoom,              Qt::Key_Zoom,
    QTOPIAXK_Context1,          Qt::Key_Context1,
    QTOPIAXK_Context2,          Qt::Key_Context2,
    QTOPIAXK_Context3,          Qt::Key_Context3,
    QTOPIAXK_Context4,          Qt::Key_Context4,
    QTOPIAXK_Call,              Qt::Key_Call,
    QTOPIAXK_Hangup,            Qt::Key_Hangup,
    QTOPIAXK_Flip,              Qt::Key_Flip,

    0,                          0
};

#endif // KEYMAPPER_X11_H
