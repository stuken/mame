//============================================================
//
//  input_retro.cpp - retro input implementation
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"
#include "assignmenthelper.h"

// emu
#include "inpttype.h"

// MAME headers
#include "emu.h"
#include "uiinput.h"
#include "corestr.h"

#include "input_common.h"
#include "input_retro.h"

#include "libretro/osdretro.h"
#include "libretro/window.h"
#include "libretro/libretro-internal/libretro.h"
#include "libretro/libretro-internal/libretro_shared.h"


extern bool libretro_supports_bitmasks;

unsigned short retro_key_state[RETROK_LAST] = {0};
unsigned short retro_key_event_state[RETROK_LAST] = {0};

int mouseLX[8];
int mouseLY[8];
int lightgunX[8];
int lightgunY[8];

Joystate joystate[8];
Mousestate mousestate[8];
Lightgunstate lightgunstate[8];

unsigned mouse_count = 0;
unsigned joy_count = 0;
unsigned lightgun_count = 0;

#ifndef RETROK_TILDE
#define RETROK_TILDE 178
#endif

enum
{
	SWITCH_B,           // button bits
	SWITCH_A,
	SWITCH_Y,
	SWITCH_X,
	SWITCH_L1,
	SWITCH_R1,
	SWITCH_L3,
	SWITCH_R3,
	SWITCH_START,
	SWITCH_SELECT,

	SWITCH_DPAD_UP,     // D-pad bits
	SWITCH_DPAD_DOWN,
	SWITCH_DPAD_LEFT,
	SWITCH_DPAD_RIGHT,

	SWITCH_L2,          // for arcade stick/pad with LT/RT buttons
	SWITCH_R2,

	SWITCH_TOTAL
};

enum
{
	AXIS_L2,            // half-axes for triggers
	AXIS_R2,

	AXIS_LSX,           // full-precision axes
	AXIS_LSY,
	AXIS_RSX,
	AXIS_RSY,

	AXIS_TOTAL
};

kt_table const ktable[] =
{
   {"A",         RETROK_a,            ITEM_ID_A},
   {"B",         RETROK_b,            ITEM_ID_B},
   {"C",         RETROK_c,            ITEM_ID_C},
   {"D",         RETROK_d,            ITEM_ID_D},
   {"E",         RETROK_e,            ITEM_ID_E},
   {"F",         RETROK_f,            ITEM_ID_F},
   {"G",         RETROK_g,            ITEM_ID_G},
   {"H",         RETROK_h,            ITEM_ID_H},
   {"I",         RETROK_i,            ITEM_ID_I},
   {"J",         RETROK_j,            ITEM_ID_J},
   {"K",         RETROK_k,            ITEM_ID_K},
   {"L",         RETROK_l,            ITEM_ID_L},
   {"M",         RETROK_m,            ITEM_ID_M},
   {"N",         RETROK_n,            ITEM_ID_N},
   {"O",         RETROK_o,            ITEM_ID_O},
   {"P",         RETROK_p,            ITEM_ID_P},
   {"Q",         RETROK_q,            ITEM_ID_Q},
   {"R",         RETROK_r,            ITEM_ID_R},
   {"S",         RETROK_s,            ITEM_ID_S},
   {"T",         RETROK_t,            ITEM_ID_T},
   {"U",         RETROK_u,            ITEM_ID_U},
   {"V",         RETROK_v,            ITEM_ID_V},
   {"W",         RETROK_w,            ITEM_ID_W},
   {"X",         RETROK_x,            ITEM_ID_X},
   {"Y",         RETROK_y,            ITEM_ID_Y},
   {"Z",         RETROK_z,            ITEM_ID_Z},
   {"0",         RETROK_0,            ITEM_ID_0},
   {"1",         RETROK_1,            ITEM_ID_1},
   {"2",         RETROK_2,            ITEM_ID_2},
   {"3",         RETROK_3,            ITEM_ID_3},
   {"4",         RETROK_4,            ITEM_ID_4},
   {"5",         RETROK_5,            ITEM_ID_5},
   {"6",         RETROK_6,            ITEM_ID_6},
   {"7",         RETROK_7,            ITEM_ID_7},
   {"8",         RETROK_8,            ITEM_ID_8},
   {"9",         RETROK_9,            ITEM_ID_9},
   {"F1",        RETROK_F1,           ITEM_ID_F1},
   {"F2",        RETROK_F2,           ITEM_ID_F2},
   {"F3",        RETROK_F3,           ITEM_ID_F3},
   {"F4",        RETROK_F4,           ITEM_ID_F4},
   {"F5",        RETROK_F5,           ITEM_ID_F5},
   {"F6",        RETROK_F6,           ITEM_ID_F6},
   {"F7",        RETROK_F7,           ITEM_ID_F7},
   {"F8",        RETROK_F8,           ITEM_ID_F8},
   {"F9",        RETROK_F9,           ITEM_ID_F9},
   {"F10",       RETROK_F10,          ITEM_ID_F10},
   {"F11",       RETROK_F11,          ITEM_ID_F11},
   {"F12",       RETROK_F12,          ITEM_ID_F12},
   {"F13",       RETROK_F13,          ITEM_ID_F13},
   {"F14",       RETROK_F14,          ITEM_ID_F14},
   {"F15",       RETROK_F15,          ITEM_ID_F15},
   {"ESC",       RETROK_ESCAPE,       ITEM_ID_ESC},
   {"TILDE",     RETROK_BACKQUOTE,    ITEM_ID_TILDE},
   {"MINUS",     RETROK_MINUS,        ITEM_ID_MINUS},
   {"EQUALS",    RETROK_EQUALS,       ITEM_ID_EQUALS},
   {"BACKSPACE", RETROK_BACKSPACE,    ITEM_ID_BACKSPACE},
   {"TAB",       RETROK_TAB,          ITEM_ID_TAB},
   {"[",         RETROK_LEFTBRACKET,  ITEM_ID_OPENBRACE},
   {"]",         RETROK_RIGHTBRACKET, ITEM_ID_CLOSEBRACE},
   {"ENTER",     RETROK_RETURN,       ITEM_ID_ENTER},
   {";",         RETROK_SEMICOLON,    ITEM_ID_COLON},
   {"\'",        RETROK_QUOTE,        ITEM_ID_QUOTE},
   {"\\",        RETROK_BACKSLASH,    ITEM_ID_BACKSLASH},
   {"|",         RETROK_OEM_102,      ITEM_ID_BACKSLASH2},
   {",",         RETROK_COMMA,        ITEM_ID_COMMA},
   {".",         RETROK_PERIOD,       ITEM_ID_STOP},
   {"/",         RETROK_SLASH,        ITEM_ID_SLASH},
   {"SPACE",     RETROK_SPACE,        ITEM_ID_SPACE},
   {"INS",       RETROK_INSERT,       ITEM_ID_INSERT},
   {"DEL",       RETROK_DELETE,       ITEM_ID_DEL},
   {"HOME",      RETROK_HOME,         ITEM_ID_HOME},
   {"END",       RETROK_END,          ITEM_ID_END},
   {"PGUP",      RETROK_PAGEUP,       ITEM_ID_PGUP},
   {"PGDW",      RETROK_PAGEDOWN,     ITEM_ID_PGDN},
   {"LEFT",      RETROK_LEFT,         ITEM_ID_LEFT},
   {"RIGHT",     RETROK_RIGHT,        ITEM_ID_RIGHT},
   {"UP",        RETROK_UP,           ITEM_ID_UP},
   {"DOWN",      RETROK_DOWN,         ITEM_ID_DOWN},
   {"KPO",       RETROK_KP0,          ITEM_ID_0_PAD},
   {"KP1",       RETROK_KP1,          ITEM_ID_1_PAD},
   {"KP2",       RETROK_KP2,          ITEM_ID_2_PAD},
   {"KP3",       RETROK_KP3,          ITEM_ID_3_PAD},
   {"KP4",       RETROK_KP4,          ITEM_ID_4_PAD},
   {"KP5",       RETROK_KP5,          ITEM_ID_5_PAD},
   {"KP6",       RETROK_KP6,          ITEM_ID_6_PAD},
   {"KP7",       RETROK_KP7,          ITEM_ID_7_PAD},
   {"KP8",       RETROK_KP8,          ITEM_ID_8_PAD},
   {"KP9",       RETROK_KP9,          ITEM_ID_9_PAD},
   {"KP/",       RETROK_KP_DIVIDE,    ITEM_ID_SLASH_PAD},
   {"KP*",       RETROK_KP_MULTIPLY,  ITEM_ID_ASTERISK},
   {"KP-",       RETROK_KP_MINUS,     ITEM_ID_MINUS_PAD},
   {"KP+",       RETROK_KP_PLUS,      ITEM_ID_PLUS_PAD},
   {"KP.",       RETROK_KP_PERIOD,    ITEM_ID_DEL_PAD},
   {"KPENTER",   RETROK_KP_ENTER,     ITEM_ID_ENTER_PAD},
   {"PRTSCR",    RETROK_PRINT,        ITEM_ID_PRTSCR},
   {"PAUSE",     RETROK_PAUSE,        ITEM_ID_PAUSE},
   {"LSHIFT",    RETROK_LSHIFT,       ITEM_ID_LSHIFT},
   {"RSHIFT",    RETROK_RSHIFT,       ITEM_ID_RSHIFT},
   {"LCTRL",     RETROK_LCTRL,        ITEM_ID_LCONTROL},
   {"RCTRL",     RETROK_RCTRL,        ITEM_ID_RCONTROL},
   {"LALT",      RETROK_LALT,         ITEM_ID_LALT},
   {"RALT",      RETROK_RALT,         ITEM_ID_RALT},
   {"SCRLOCK",   RETROK_SCROLLOCK,    ITEM_ID_SCRLOCK},
   {"NUMLOCK",   RETROK_NUMLOCK,      ITEM_ID_NUMLOCK},
   {"CPSLOCK",   RETROK_CAPSLOCK,     ITEM_ID_CAPSLOCK},
   {"LSUPER",    RETROK_LSUPER,       ITEM_ID_LWIN},
   {"RSUPER",    RETROK_RSUPER,       ITEM_ID_RWIN},
   {"MENU",      RETROK_MENU,         ITEM_ID_MENU},
   {"BREAK",     RETROK_BREAK,        ITEM_ID_CANCEL},
   {"-1",        -1,                  ITEM_ID_INVALID},
};

const char *Buttons_Name[RETRO_MAX_BUTTONS] =
{
	"B",           //0
	"Y",           //1
	"SELECT",      //2
	"START",       //3
	"D-Pad Up",    //4
	"D-Pad Down",  //5
	"D-Pad Left",  //6
	"D-Pad Right", //7
	"A",           //8
	"X",           //9
	"L1",          //10
	"R1",          //11
	"L2",          //12
	"R2",          //13
	"L3",          //14
	"R3",          //15
};

//    Default : B ->B1 | A ->B2 | Y ->B3 | X ->B4 | L ->B5 | R ->B6
int Buttons_mapping[] =
{
   RETROPAD_B,
   RETROPAD_A,
   RETROPAD_Y,
   RETROPAD_X,
   RETROPAD_L,
   RETROPAD_R
};

void Input_Binding(running_machine &machine)
{
   log_cb(RETRO_LOG_INFO, "SOURCE FILE: %s\n", machine.system().type.source());
   log_cb(RETRO_LOG_INFO, "PARENT: %s\n", machine.system().parent);
   log_cb(RETRO_LOG_INFO, "NAME: %s\n", machine.system().name);
   log_cb(RETRO_LOG_INFO, "DESCRIPTION: %s\n", machine.system().type.fullname());
   log_cb(RETRO_LOG_INFO, "YEAR: %s\n", machine.system().year);
   log_cb(RETRO_LOG_INFO, "MANUFACTURER: %s\n", machine.system().manufacturer);

   Buttons_mapping[0]=RETROPAD_B;
   Buttons_mapping[1]=RETROPAD_A;
   Buttons_mapping[2]=RETROPAD_Y;
   Buttons_mapping[3]=RETROPAD_X;
   Buttons_mapping[4]=RETROPAD_L;
   Buttons_mapping[5]=RETROPAD_R;

   if (
         !core_stricmp(machine.system().name, "avengrgs")    ||
         !core_stricmp(machine.system().parent, "avengrgs")  ||
         !core_stricmp(machine.system().name, "bloodwar")    ||
         !core_stricmp(machine.system().parent, "bloodwar")  ||
         !core_stricmp(machine.system().name, "daraku")    ||
         !core_stricmp(machine.system().parent, "daraku")  ||
         !core_stricmp(machine.system().name, "drgnmst")   ||
         !core_stricmp(machine.system().parent, "drgnmst")   ||
         !core_stricmp(machine.system().name, "primrage")   ||
         !core_stricmp(machine.system().parent, "primrage")   ||
         !core_stricmp(machine.system().name, "rabbit")    ||
         !core_stricmp(machine.system().parent, "rabbit")  ||
         !core_stricmp(machine.system().name, "shogwarr")   ||
         !core_stricmp(machine.system().parent, "shogwarr")   ||
         !core_stricmp(machine.system().name, "tekken")    ||
         !core_stricmp(machine.system().parent, "tekken")  ||
         !core_stricmp(machine.system().name, "tekken2")   ||
         !core_stricmp(machine.system().parent, "tekken2")   ||
         !core_stricmp(machine.system().name, "tkdensho")   ||
         !core_stricmp(machine.system().parent, "tkdensho")   ||
         !core_stricmp(machine.system().name, "vf")   ||
         !core_stricmp(machine.system().parent, "vf")
      )
   {
      /* Tekken 1/2/Virtua Fighter/Etc.*/

      Buttons_mapping[0]=RETROPAD_Y;
      Buttons_mapping[1]=RETROPAD_X;
      Buttons_mapping[2]=RETROPAD_B;
      Buttons_mapping[3]=RETROPAD_A;
      Buttons_mapping[4]=RETROPAD_L;
      Buttons_mapping[5]=RETROPAD_R;

   }
   else if (
              !core_stricmp(machine.system().name, "jojo")    ||
              !core_stricmp(machine.system().parent, "jojo")  ||
              !core_stricmp(machine.system().name, "jojoba")    ||
              !core_stricmp(machine.system().parent, "jojoba")  ||
              !core_stricmp(machine.system().name, "souledge")    ||
              !core_stricmp(machine.system().parent, "souledge")  ||
              !core_stricmp(machine.system().name, "soulclbr")    ||
              !core_stricmp(machine.system().parent, "soulclbr")    ||
              !core_stricmp(machine.system().name, "svg")    ||
              !core_stricmp(machine.system().parent, "svg")
           )
   {
      /* Soul Edge/Soul Calibur/JoJo/SVG */

      Buttons_mapping[0]=RETROPAD_Y;
      Buttons_mapping[1]=RETROPAD_X;
      Buttons_mapping[2]=RETROPAD_A;
      Buttons_mapping[3]=RETROPAD_B;
      Buttons_mapping[4]=RETROPAD_L;
      Buttons_mapping[5]=RETROPAD_R;

   }
   else if (
              !core_stricmp(machine.system().name, "doapp")
           )
   {
      /* Dead or Alive++ */

      Buttons_mapping[0]=RETROPAD_B;
      Buttons_mapping[1]=RETROPAD_Y;
      Buttons_mapping[2]=RETROPAD_X;
      Buttons_mapping[3]=RETROPAD_A;
      Buttons_mapping[4]=RETROPAD_L;
      Buttons_mapping[5]=RETROPAD_R;

   }
   else if (
              !core_stricmp(machine.system().name, "ehrgeiz") ||
              !core_stricmp(machine.system().parent, "ehrgeiz")
           )
   {
      /* Ehrgeiz */

      Buttons_mapping[0]=RETROPAD_Y;
      Buttons_mapping[1]=RETROPAD_B;
      Buttons_mapping[2]=RETROPAD_A;
      Buttons_mapping[3]=RETROPAD_X;
      Buttons_mapping[4]=RETROPAD_L;
      Buttons_mapping[5]=RETROPAD_R;

   }
   else if (
              !core_stricmp(machine.system().name, "ts2") ||
              !core_stricmp(machine.system().parent, "ts2")
           )
   {
      /* Toshinden 2 */

      Buttons_mapping[0]=RETROPAD_L;
      Buttons_mapping[1]=RETROPAD_Y;
      Buttons_mapping[2]=RETROPAD_X;
      Buttons_mapping[3]=RETROPAD_R;
      Buttons_mapping[4]=RETROPAD_B;
      Buttons_mapping[5]=RETROPAD_A;

   }
   else if (
              !core_stricmp(machine.system().name, "dstlk") ||
              !core_stricmp(machine.system().parent, "dstlk") ||
              !core_stricmp(machine.system().name, "hsf2") ||
              !core_stricmp(machine.system().parent, "hsf2") ||
              !core_stricmp(machine.system().name, "msh") ||
              !core_stricmp(machine.system().parent, "msh") ||
              !core_stricmp(machine.system().name, "mshvsf") ||
              !core_stricmp(machine.system().parent, "mshvsf") ||
              !core_stricmp(machine.system().name, "mvsc") ||
              !core_stricmp(machine.system().parent, "mvsc") ||
              !core_stricmp(machine.system().name, "nwarr") ||
              !core_stricmp(machine.system().parent, "nwarr") ||
              !core_stricmp(machine.system().name, "redearth") ||
              !core_stricmp(machine.system().parent, "redearth") ||
              !core_stricmp(machine.system().name, "rvschool") ||
              !core_stricmp(machine.system().parent, "rvschool") ||
              !core_stricmp(machine.system().name, "sf2") ||
              !core_stricmp(machine.system().parent, "sf2") ||
              !core_stricmp(machine.system().name, "sf2ce") ||
              !core_stricmp(machine.system().parent, "sf2ce") ||
              !core_stricmp(machine.system().name, "sf2hf") ||
              !core_stricmp(machine.system().parent, "sf2hf") ||
              !core_stricmp(machine.system().name, "sfa") ||
              !core_stricmp(machine.system().parent, "sfa") ||
              !core_stricmp(machine.system().name, "sfa2") ||
              !core_stricmp(machine.system().parent, "sfa2") ||
              !core_stricmp(machine.system().name, "sfa3") ||
              !core_stricmp(machine.system().parent, "sfa3") ||
              !core_stricmp(machine.system().name, "sfex") ||
              !core_stricmp(machine.system().parent, "sfex") ||
              !core_stricmp(machine.system().name, "sfex2") ||
              !core_stricmp(machine.system().parent, "sfex2") ||
              !core_stricmp(machine.system().name, "sfex2p") ||
              !core_stricmp(machine.system().parent, "sfex2p") ||
              !core_stricmp(machine.system().name, "sfexp") ||
              !core_stricmp(machine.system().parent, "sfexp") ||
              !core_stricmp(machine.system().name, "sfiii") ||
              !core_stricmp(machine.system().parent, "sfiii") ||
              !core_stricmp(machine.system().name, "sfiii2") ||
              !core_stricmp(machine.system().parent, "sfiii2") ||
              !core_stricmp(machine.system().name, "sfiii3") ||
              !core_stricmp(machine.system().parent, "sfiii3") ||
              !core_stricmp(machine.system().name, "sftm") ||
              !core_stricmp(machine.system().parent, "sftm") ||
              !core_stricmp(machine.system().name, "sfz2al") ||
              !core_stricmp(machine.system().parent, "sfz2al") ||
              !core_stricmp(machine.system().name, "sfzch") ||
              !core_stricmp(machine.system().parent, "sfzch") ||
              !core_stricmp(machine.system().name, "ssf2") ||
              !core_stricmp(machine.system().parent, "ssf2") ||
              !core_stricmp(machine.system().name, "ssf2t") ||
              !core_stricmp(machine.system().parent, "ssf2t") ||
              !core_stricmp(machine.system().name, "vhunt2") ||
              !core_stricmp(machine.system().parent, "vhunt2") ||
              !core_stricmp(machine.system().name, "vsav") ||
              !core_stricmp(machine.system().parent, "vsav") ||
              !core_stricmp(machine.system().name, "vsav2") ||
              !core_stricmp(machine.system().parent, "vsav2") ||
              !core_stricmp(machine.system().name, "xmcota") ||
              !core_stricmp(machine.system().parent, "xmcota") ||
              !core_stricmp(machine.system().name, "xmvsf") ||
              !core_stricmp(machine.system().parent, "xmvsf") ||
              
              !core_stricmp(machine.system().name, "astrass") ||
              !core_stricmp(machine.system().parent, "astrass") ||
              !core_stricmp(machine.system().name, "brival") ||
              !core_stricmp(machine.system().parent, "brival") ||
              !core_stricmp(machine.system().name, "btlkroad") ||
              !core_stricmp(machine.system().parent, "btlkroad") ||
              !core_stricmp(machine.system().name, "dankuga") ||
              !core_stricmp(machine.system().parent, "dankuga") ||
              !core_stricmp(machine.system().name, "dragoona") ||
              !core_stricmp(machine.system().parent, "dragoona") ||
              !core_stricmp(machine.system().name, "ffreveng") ||
              !core_stricmp(machine.system().parent, "ffreveng") ||
              !core_stricmp(machine.system().name, "fghthist") ||
              !core_stricmp(machine.system().parent, "fghthist") ||
              !core_stricmp(machine.system().name, "fgtlayer") ||
              !core_stricmp(machine.system().parent, "fgtlayer") ||
              !core_stricmp(machine.system().name, "gaxeduel") ||
              !core_stricmp(machine.system().parent, "gaxeduel") ||
              !core_stricmp(machine.system().name, "groovef") ||
              !core_stricmp(machine.system().parent, "groovef") ||
              !core_stricmp(machine.system().name, "kaiserkn") ||
              !core_stricmp(machine.system().parent, "kaiserkn") ||
              !core_stricmp(machine.system().name, "ssoldier") ||
              !core_stricmp(machine.system().parent, "ssoldier")
           )
   {
      /* 6-button fighting games (Mainly Capcom (CPS-1, CPS-2, CPS-3, ZN-1, ZN-2) + Others)*/

      Buttons_mapping[0]=RETROPAD_Y;
      Buttons_mapping[1]=RETROPAD_X;
      Buttons_mapping[2]=RETROPAD_L;
      Buttons_mapping[3]=RETROPAD_B;
      Buttons_mapping[4]=RETROPAD_A;
      Buttons_mapping[5]=RETROPAD_R;

   }
   else if (
              !core_stricmp(machine.system().parent, "aof") ||
              !core_stricmp(machine.system().parent, "aof2") ||
              !core_stricmp(machine.system().parent, "aof3") ||
              !core_stricmp(machine.system().parent, "breakers") ||
              !core_stricmp(machine.system().parent, "breakrev") ||
              !core_stricmp(machine.system().parent, "doubledr") ||
              !core_stricmp(machine.system().parent, "fatfury1") ||
              !core_stricmp(machine.system().parent, "fatfury2") ||
              !core_stricmp(machine.system().parent, "fatfury3") ||
              !core_stricmp(machine.system().parent, "fatfursp") ||
              !core_stricmp(machine.system().parent, "fightfev") ||
              !core_stricmp(machine.system().parent, "galaxyfg") ||
              !core_stricmp(machine.system().parent, "garou") ||
              !core_stricmp(machine.system().parent, "gowcaizr") ||
              !core_stricmp(machine.system().parent, "neogeo") ||
              !core_stricmp(machine.system().parent, "karnovr") ||
              !core_stricmp(machine.system().parent, "kizuna") ||
              !core_stricmp(machine.system().parent, "kabukikl") ||
              !core_stricmp(machine.system().parent, "matrim") ||
              !core_stricmp(machine.system().parent, "mslug") ||
              !core_stricmp(machine.system().parent, "mslug2") ||
              !core_stricmp(machine.system().parent, "mslugx") ||
              !core_stricmp(machine.system().parent, "mslug3") ||
              !core_stricmp(machine.system().parent, "mslug4") ||
              !core_stricmp(machine.system().parent, "mslug5") ||
              !core_stricmp(machine.system().parent, "kof94") ||
              !core_stricmp(machine.system().parent, "kof95") ||
              !core_stricmp(machine.system().parent, "kof96") ||
              !core_stricmp(machine.system().parent, "kof97") ||
              !core_stricmp(machine.system().parent, "kof98") ||
              !core_stricmp(machine.system().parent, "kof99") ||
              !core_stricmp(machine.system().parent, "kof2000") ||
              !core_stricmp(machine.system().parent, "kof2001") ||
              !core_stricmp(machine.system().parent, "kof2002") ||
              !core_stricmp(machine.system().parent, "kof2003") ||
              !core_stricmp(machine.system().parent, "lresort") ||
              !core_stricmp(machine.system().parent, "lastblad") ||
              !core_stricmp(machine.system().parent, "lastbld2") ||
              !core_stricmp(machine.system().parent, "ninjamas") ||
              !core_stricmp(machine.system().parent, "rotd") ||
              !core_stricmp(machine.system().parent, "rbff1") ||
              !core_stricmp(machine.system().parent, "rbff2") ||
              !core_stricmp(machine.system().parent, "rbffspec") ||
              !core_stricmp(machine.system().parent, "savagere") ||
              !core_stricmp(machine.system().parent, "sengoku3") ||
              !core_stricmp(machine.system().parent, "samsho") ||
              !core_stricmp(machine.system().parent, "samsho2") ||
              !core_stricmp(machine.system().parent, "samsho3") ||
              !core_stricmp(machine.system().parent, "samsho4") ||
              !core_stricmp(machine.system().parent, "samsho5") ||
              !core_stricmp(machine.system().parent, "samsh5sp") ||
              !core_stricmp(machine.system().parent, "svc") ||
              !core_stricmp(machine.system().parent, "viewpoin") ||
              !core_stricmp(machine.system().parent, "wakuwak7") ||
              !core_stricmp(machine.system().parent, "wh1") ||
              !core_stricmp(machine.system().parent, "wh2") ||
              !core_stricmp(machine.system().parent, "wh2j") ||
              !core_stricmp(machine.system().parent, "whp")
           )
   {
      /* Neo Geo */

      Buttons_mapping[0]=RETROPAD_B;
      Buttons_mapping[1]=RETROPAD_A;
      Buttons_mapping[2]=RETROPAD_Y;
      Buttons_mapping[3]=RETROPAD_X;
      Buttons_mapping[4]=RETROPAD_L;
      Buttons_mapping[5]=RETROPAD_R;
   }
   else if (
              !core_stricmp(machine.system().name, "kinst") ||
              !core_stricmp(machine.system().parent, "kinst")
           )
   {
      /* Killer Instinct 1 */

      Buttons_mapping[0]=RETROPAD_L;
      Buttons_mapping[1]=RETROPAD_Y;
      Buttons_mapping[2]=RETROPAD_X;
      Buttons_mapping[3]=RETROPAD_R;
      Buttons_mapping[4]=RETROPAD_B;
      Buttons_mapping[5]=RETROPAD_A;

   }
   else if (
              !core_stricmp(machine.system().name, "kinst2") ||
              !core_stricmp(machine.system().parent, "kinst2")
           )
   {
      /* Killer Instinct 2 */

      Buttons_mapping[0]=RETROPAD_L;
      Buttons_mapping[1]=RETROPAD_Y;
      Buttons_mapping[2]=RETROPAD_X;
      Buttons_mapping[3]=RETROPAD_B;
      Buttons_mapping[4]=RETROPAD_A;
      Buttons_mapping[5]=RETROPAD_R;

   }
   else if (
              !core_stricmp(machine.system().name, "tektagt")   ||
              !core_stricmp(machine.system().parent, "tektagt") ||
              !core_stricmp(machine.system().name, "tekken3")   ||
              !core_stricmp(machine.system().parent, "tekken3")
           )
   {
      /* Tekken 3/Tekken Tag Tournament */

      Buttons_mapping[0]=RETROPAD_Y;
      Buttons_mapping[1]=RETROPAD_X;
      Buttons_mapping[2]=RETROPAD_R;
      Buttons_mapping[3]=RETROPAD_B;
      Buttons_mapping[4]=RETROPAD_A;
      Buttons_mapping[5]=RETROPAD_L;

   }
   else if (
              !core_stricmp(machine.system().name, "mk")       ||
              !core_stricmp(machine.system().parent, "mk")     ||
              !core_stricmp(machine.system().name, "mk2")      ||
              !core_stricmp(machine.system().parent, "mk2")    ||
              !core_stricmp(machine.system().name, "mk3")      ||
              !core_stricmp(machine.system().parent, "mk3")    ||
              !core_stricmp(machine.system().name, "umk3")     ||
              !core_stricmp(machine.system().parent, "umk3")   ||
              !core_stricmp(machine.system().name, "wwfmania") ||
              !core_stricmp(machine.system().parent, "wwfmania")
           )
   {
      /* Mortal Kombat 1/2/3/Ultimate/WWF: Wrestlemania */

      Buttons_mapping[0]=RETROPAD_Y;
      Buttons_mapping[1]=RETROPAD_L;
      Buttons_mapping[2]=RETROPAD_X;
      Buttons_mapping[3]=RETROPAD_B;
      Buttons_mapping[4]=RETROPAD_A;
      Buttons_mapping[5]=RETROPAD_R;

   }
}


bool retro_osd_interface::should_hide_mouse()
{
	// if we are paused, no
	if (machine().paused())
		return false;

	// if neither mice nor lightguns are enabled in the core, then no
	if (!options().mouse() && !options().lightgun())
		return false;
#if 0
	if (!mouse_over_window())
		return false;
#endif
	// otherwise, yes
	return true;
}

void retro_osd_interface::release_keys()
{
	auto const keybd = dynamic_cast<input_module_base*>(m_keyboard_input);
	if (keybd)
		keybd->reset_devices();
}

void retro_keyboard_event(bool down, unsigned code,
      uint32_t character, uint16_t mod)
{
   switch (code)
   {
      case RETROK_UNKNOWN:
      case RETROK_PAUSE:
         return;
   }

   retro_key_event_state[code] = down;
}

void retro_osd_interface::process_keyboard_state(running_machine &machine)
{
   unsigned short i = 0;

   do
   {
      if (retro_key_event_state[ktable[i].retro_key_name] && !retro_key_state[ktable[i].retro_key_name])
      {
         retro_key_state[ktable[i].retro_key_name] = 0x80;
         machine.ui_input().push_char_event(osd_common_t::s_window_list.front()->target(), ktable[i].retro_key_name);
      }
      else if (!retro_key_event_state[ktable[i].retro_key_name] && retro_key_state[ktable[i].retro_key_name])
      {
         retro_key_state[ktable[i].retro_key_name] = 0;
      }

      i++;
   } while (ktable[i].retro_key_name != -1);
}

void retro_osd_interface::process_joypad_state(running_machine &machine)
{
   unsigned i, j;
   int analog_l2, analog_r2;
   int16_t ret[8];

   if (libretro_supports_bitmasks)
   {
      for (j = 0; j < 8; j++)
         ret[j] = input_state_cb(j, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
   }
   else
   {
      for (j = 0; j < 8; j++)
      {
         ret[j] = 0;
         for (i = 0; i < RETRO_MAX_BUTTONS; i++)
            if (input_state_cb(j, RETRO_DEVICE_JOYPAD, 0, i))
               ret[j] |= (1 << i);
      }
   }

   for (j = 0; j < 8; j++)
   {
      for (i = 0; i < RETRO_MAX_BUTTONS; i++)
      {
         if (ret[j] & (1 << i))
            joystate[j].button[i] = 0x80;
         else
            joystate[j].button[i] = 0;
      }

      joystate[j].a1[0] = normalize_absolute_axis((input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X)), -32767, 32767);
      joystate[j].a1[1] = normalize_absolute_axis((input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y)), -32767, 32767);
      joystate[j].a2[0] = normalize_absolute_axis((input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X)), -32767, 32767);
      joystate[j].a2[1] = normalize_absolute_axis((input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y)), -32767, 32767);

      analog_l2 = input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_BUTTON, RETRO_DEVICE_ID_JOYPAD_L2);
      analog_r2 = input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_BUTTON, RETRO_DEVICE_ID_JOYPAD_R2);
      /* Fallback, if no analog trigger support, use digital */
      if (analog_l2 == 0)
      {
         if (ret[j] & (1 << RETRO_DEVICE_ID_JOYPAD_L2))
            analog_l2 = 32767;
      }
      if (analog_r2 == 0)
      {
         if (ret[j] & (1 << RETRO_DEVICE_ID_JOYPAD_R2))
            analog_r2 = 32767;
      }
      joystate[j].a3[0] = -normalize_absolute_axis(analog_l2, -32767, 32767);
      joystate[j].a3[1] = -normalize_absolute_axis(analog_r2, -32767, 32767);
   }
}

void retro_osd_interface::process_mouse_state(running_machine &machine)
{
   unsigned i;
   auto &window = osd_common_t::window_list().front();

   for(i = 0;i < 8; i++)
   {
         static int mbL[8] = {0}, mbR[8] = {0}, mbM[8] = {0};
         int mouse_l[8];
         int mouse_r[8];
	     int mouse_m[8];
         int16_t mouse_x[8];
         int16_t mouse_y[8];

         if (!mouse_enable)
            return;

         mouse_x[i] = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
         mouse_y[i] = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);
         mouse_l[i] = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT);
         mouse_r[i] = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT);
         mouse_m[i] = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_MIDDLE);
         mouseLX[i] = mouse_x[i] * osd::input_device::RELATIVE_PER_PIXEL;
         mouseLY[i] = mouse_y[i] * osd::input_device::RELATIVE_PER_PIXEL;

         static int vmx=fb_width/2,vmy=fb_height/2;
         static int ovmx=fb_width/2,ovmy=fb_height/2;

         vmx+=mouse_x[0];
         vmy+=mouse_y[0];
         if(vmx>fb_width)vmx=fb_width-1;
         if(vmy>fb_height)vmy=fb_height-1;
         if(vmx<0)vmx=0;
         if(vmy<0)vmy=0;

         if (vmx != ovmx || vmy != ovmy)
         {
	        int cx = -1, cy = -1;
	        if (window != nullptr && window->renderer().xy_to_render_target(vmx, vmy, &cx, &cy))
					machine.ui_input().push_mouse_move_event(window->target(), cx, cy);
         }
         ovmx=vmx;
         ovmy=vmy;

         if(mbL[i]==0 && mouse_l[i])
         {
            mbL[i]=1;
            mousestate[i].mouseBUT[0]=0x80;

			if(i==0)
			{
				int cx = -1, cy = -1;
				//FIXME doubleclick
				if (window != nullptr && window->renderer().xy_to_render_target(vmx, vmy, &cx, &cy))
						machine.ui_input().push_mouse_down_event(window->target(), cx, cy);
			}
         }
         else if(mbL[i]==1 && !mouse_l[i])
         {
            mousestate[i].mouseBUT[0]=0;
            mbL[i]=0;

			if(i==0)
			{
				int cx = -1, cy = -1;
				if (window != nullptr && window->renderer().xy_to_render_target(vmx, vmy, &cx, &cy))
						machine.ui_input().push_mouse_up_event(window->target(), cx, cy);
			}
		 }

         if(mbR[i]==0 && mouse_r[i])
         {
            mbR[i]=1;
            mousestate[i].mouseBUT[1]=0x80;

			if(i==0)
			{
				int cx = -1, cy = -1;
				if (window != nullptr && window->renderer().xy_to_render_target(vmx, vmy, &cx, &cy))
						machine.ui_input().push_mouse_rdown_event(window->target(), cx, cy);
			}
		 }
         else if(mbR[i]==1 && !mouse_r[i])
         {
            mousestate[i].mouseBUT[1]=0;
            mbR[i]=0;

			if(i==0)
			{
				int cx = -1, cy = -1;
				if (window != nullptr && window->renderer().xy_to_render_target(vmx, vmy, &cx, &cy))
						machine.ui_input().push_mouse_rup_event(window->target(), cx, cy);
			}
		 }
	   
         if(mbM[i]==0 && mouse_m[i])
         {
            mbM[i]=1;
            mousestate[i].mouseBUT[2]=0x80;
         }
         else if(mbM[i]==1 && !mouse_m[i])
         {
            mousestate[i].mouseBUT[2]=0;
            mbM[i]=0;
         }
   }
}

void retro_osd_interface::process_lightgun_state(running_machine &machine)
{
   unsigned i,j;
   for(j = 0;j < 8; j++)
   {
      int16_t gun_x_raw[8], gun_y_raw[8];

      if ( lightgun_mode == RETRO_SETTING_LIGHTGUN_MODE_DISABLED ) {
         return;
      }

      for (i = 0; i < 4; i++) {
         lightgunstate[j].lightgunBUT[i] = 0;
      }

      if ( lightgun_mode == RETRO_SETTING_LIGHTGUN_MODE_POINTER ) {
         gun_x_raw[j] = input_state_cb(j, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X);
         gun_y_raw[j] = input_state_cb(j, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y);

         // handle pointer presses
         // use multi-touch to support different button inputs
         int touch_count[8];
		 touch_count[j] = input_state_cb( j, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_COUNT );
         if ( touch_count[j] > 0 && touch_count[j] <= 4 ) {
            lightgunstate[j].lightgunBUT[touch_count[j]-1] = 0x80;
         }
      } else { // lightgun is default when enabled
         gun_x_raw[j] = input_state_cb( j, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X );
         gun_y_raw[j] = input_state_cb( j, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y );

         if ( input_state_cb( j, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER ) || input_state_cb( j, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD ) ) {
            lightgunstate[j].lightgunBUT[0] = 0x80;
         }
         if ( input_state_cb( j, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_A ) ) {
            lightgunstate[j].lightgunBUT[1] = 0x80;
         }
      }

      //Place the cursor at a corner of the screen designated by "Lightgun offscreen position" when the cursor touches a min/max value
      //The LIGHTGUN_RELOAD input will fire a shot at the bottom-right corner if "Lightgun offscreen position" is set to "fixed (bottom right)"
	  //That same input will fire a shot at the top-left corner otherwise
	  //The reload feature of some games fails at the top-left corner
      if (input_state_cb( j, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN ) && !input_state_cb( j, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD ))
	  {
		 if (lightgun_offscreen_mode == 1)
		 {
		    lightgunX[j] = -65535;
	        lightgunY[j] = -65535;
		 }
		 else if (lightgun_offscreen_mode == 2)
		 {
		    lightgunX[j] = 65535;
	        lightgunY[j] = 65535;
		 }
		 else
		 {
            lightgunX[j] = gun_x_raw[j] * 2;
            lightgunY[j] = gun_y_raw[j] * 2;
		 }
	  }
	  else if (input_state_cb( j, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN ) && input_state_cb( j, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD ) )
	  {
		 if (lightgun_offscreen_mode == 2)
		 {
		    lightgunX[j] = 65535;
	        lightgunY[j] = 65535;
		 }
		 else
		 {
		    lightgunX[j] = -65535;
	        lightgunY[j] = -65535;
		 }
	  }
	  else if (!input_state_cb( j, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN ) && input_state_cb( j, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD ) )
	  {
		 if (lightgun_offscreen_mode == 2)
		 {
		    lightgunX[j] = 65535;
	        lightgunY[j] = 65535;
		 }
		 else
		 {
		    lightgunX[j] = -65535;
	        lightgunY[j] = -65535;
		 }
	  }
	  else
	  {
         lightgunX[j] = gun_x_raw[j] * 2;
         lightgunY[j] = gun_y_raw[j] * 2;
	  }
   }
}


namespace osd {

//============================================================
//  retro_keyboard_device
//============================================================

// This device is purely event driven so the implementation is in the module
class retro_keyboard_device : public event_based_device<KeyPressEventArgs>
{
public:
	retro_keyboard_device(std::string &&name, std::string &&id, input_module &module)
		: event_based_device(std::move(name), std::move(id), module)
	{
	}

	virtual void reset() override
	{
		memset(retro_key_state, 0, sizeof(retro_key_state));
		memset(retro_key_event_state, 0, sizeof(retro_key_event_state));
	}

	virtual void configure(input_device &device) override
	{
		int i = 0;
		do {
			device.add_item(
				ktable[i].mame_key_name,
				std::string_view(),
				ktable[i].mame_key,
				generic_button_get_state<std::uint8_t>,
				&retro_key_state[ktable[i].retro_key_name]);
			i++;
		} while (ktable[i].retro_key_name != -1);
	}

protected:
	virtual void process_event(KeyPressEventArgs const &args)
	{
	}
};

//============================================================
//  keyboard_input_retro - retro keyboard input module
//============================================================

class keyboard_input_retro : public retro_input_module<retro_keyboard_device>
{
private:

public:
	keyboard_input_retro()
		: retro_input_module(OSD_KEYBOARDINPUT_PROVIDER, "retro")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		retro_input_module<retro_keyboard_device>::input_init(machine);

		create_device<retro_keyboard_device>(DEVICE_CLASS_KEYBOARD, "RetroKeyboard0", "RetroKeyboard0");

		memset(retro_key_state, 0, sizeof(retro_key_state));
		memset(retro_key_event_state, 0, sizeof(retro_key_event_state));

		m_global_inputs_enabled = true;
	}

	bool handle_input_event(void) override
	{
		if (!input_enabled())
			return false;
		return true;
	}
};


//============================================================
//  retro_mouse_device
//============================================================

// This device is purely event driven so the implementation is in the module
class retro_mouse_device : public event_based_device<KeyPressEventArgs>
{
public:
	retro_mouse_device(std::string &&name, std::string &&id, input_module &module)
		: event_based_device(std::move(name), std::move(id), module)
	{
	}

	virtual void poll(bool relative_reset) override
	{
		event_based_device::poll(relative_reset);
	}

	virtual void reset() override
	{
		for (int j = 0; j < 8; j++)
		{
			mouseLX[j] = fb_width / 2;
			mouseLY[j] = fb_height / 2;

			for (int i = 0; i < 4; i++)
				mousestate[j].mouseBUT[i] = 0;
		}
	}

	virtual void configure(input_device &device) override
	{
		device.add_item(
			"X",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_XAXIS),
			generic_axis_get_state<std::int32_t>,
			&mouseLX[mouse_count]);
        device.add_item(
			"Y",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_YAXIS),
			generic_axis_get_state<std::int32_t>,
			&mouseLY[mouse_count]);

        for (int button = 0; button < 4; button++)
        {
			mousestate[mouse_count].mouseBUT[button] = 0;

			device.add_item(
				default_button_name(button),
                std::string_view(),
                static_cast<input_item_id>(ITEM_ID_BUTTON1 + button),
                generic_button_get_state<std::int32_t>,
                &mousestate[mouse_count].mouseBUT[button]);
        }

        mouse_count++;
	}

protected:
	virtual void process_event(KeyPressEventArgs const &args)
	{
	}
};

//============================================================
//  mouse_input_retro - retro mouse input module
//============================================================

class mouse_input_retro : public retro_input_module<retro_mouse_device>
{
private:

public:
	mouse_input_retro()
		: retro_input_module(OSD_MOUSEINPUT_PROVIDER, "retro")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		retro_input_module<retro_mouse_device>::input_init(machine);

		if (!input_enabled() || !options()->mouse())
			return;

		char defname[32];

		mouse_count = 0;

		for (int i = 0; i < 8; i++)
		{
			sprintf(defname, "RetroMouse%d", i);
			create_device<retro_mouse_device>(DEVICE_CLASS_MOUSE, defname, defname);

			mouseLX[i] = fb_width / 2;
			mouseLY[i] = fb_height / 2;
		}

		m_global_inputs_enabled = true;
	}

	bool handle_input_event(void) override
	{
		if (!input_enabled() || !options()->mouse())
			return false;
		return true;
	}
};


//============================================================
//  retro_joystick_device
//============================================================

// This device is purely event driven so the implementation is in the module
class retro_joystick_device : public event_based_device<KeyPressEventArgs>, protected joystick_assignment_helper
{
public:
	retro_joystick_device(std::string &&name, std::string &&id, input_module &module)
		: event_based_device(std::move(name), std::move(id), module)
	{
	}

	virtual void poll(bool relative_reset)
	{
		event_based_device::poll(relative_reset);
	}

	virtual void reset() override
	{
		memset(&joystate, 0, sizeof(joystate));
	}

	virtual void configure(osd::input_device &device)
	{
		// track item IDs for setting up default assignments
		input_device::assignment_vector assignments;
		input_item_id axis_ids[AXIS_TOTAL];
		input_item_id switch_ids[SWITCH_TOTAL];
		std::fill(std::begin(switch_ids), std::end(switch_ids), ITEM_ID_INVALID);

		// axes
		axis_ids[AXIS_LSX] = device.add_item(
			"LSX",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_XAXIS),
			generic_axis_get_state<std::int32_t>,
			&joystate[joy_count].a1[0]);
		axis_ids[AXIS_LSY] = device.add_item(
			"LSY",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_YAXIS),
			generic_axis_get_state<std::int32_t>,
			&joystate[joy_count].a1[1]);

		axis_ids[AXIS_RSX] = device.add_item(
			"RSX",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_RXAXIS),
			generic_axis_get_state<std::int32_t>,
			&joystate[joy_count].a2[0]);
		axis_ids[AXIS_RSY] = device.add_item(
			"RSY",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_RYAXIS),
			generic_axis_get_state<std::int32_t>,
			&joystate[joy_count].a2[1]);

		axis_ids[AXIS_L2] = device.add_item(
			"L2",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_RZAXIS),
			generic_axis_get_state<std::int32_t>,
			&joystate[joy_count].a3[0]);
		axis_ids[AXIS_R2] = device.add_item(
			"R2",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_ZAXIS),
			generic_axis_get_state<std::int32_t>,
			&joystate[joy_count].a3[1]);

		for (int j = 0; j < 6; j++)
		{
			switch_ids[j] = device.add_item(
				Buttons_Name[Buttons_mapping[j]],
				std::string_view(),
				(input_item_id)(ITEM_ID_BUTTON1 + j),
				generic_button_get_state<std::int32_t>,
				&joystate[joy_count].button[Buttons_mapping[j]]);

			add_button_assignment(assignments, ioport_type(IPT_BUTTON1 + j), { switch_ids[j] });
		}

		switch_ids[SWITCH_START] = device.add_item(
			Buttons_Name[RETROPAD_START],
			std::string_view(),
			ITEM_ID_START,
			generic_button_get_state<std::int32_t>,
			&joystate[joy_count].button[RETROPAD_START]);
		add_button_assignment(assignments, IPT_START, { switch_ids[SWITCH_START] });

		switch_ids[SWITCH_SELECT] = device.add_item(
			Buttons_Name[RETROPAD_SELECT],
			std::string_view(),
			ITEM_ID_SELECT,
			generic_button_get_state<std::int32_t>,
			&joystate[joy_count].button[RETROPAD_SELECT]);
		add_button_assignment(assignments, IPT_SELECT, { switch_ids[SWITCH_SELECT] });

		switch_ids[SWITCH_L2] = device.add_item(
			Buttons_Name[RETROPAD_L2],
			std::string_view(),
			ITEM_ID_BUTTON7,
			generic_button_get_state<std::int32_t>,
			&joystate[joy_count].button[RETROPAD_L2]);
		add_button_assignment(assignments, ioport_type(IPT_BUTTON7), { switch_ids[SWITCH_L2] });

		switch_ids[SWITCH_R2] = device.add_item(
			Buttons_Name[RETROPAD_R2],
			std::string_view(),
			ITEM_ID_BUTTON8,
			generic_button_get_state<std::int32_t>,
			&joystate[joy_count].button[RETROPAD_R2]);
		add_button_assignment(assignments, ioport_type(IPT_BUTTON8), { switch_ids[SWITCH_R2] });

		switch_ids[SWITCH_L3] = device.add_item(
			Buttons_Name[RETROPAD_L3],
			std::string_view(),
			ITEM_ID_BUTTON9,
			generic_button_get_state<std::int32_t>,
			&joystate[joy_count].button[RETROPAD_L3]);
		add_button_assignment(assignments, IPT_BUTTON9, { switch_ids[SWITCH_L3] });

		switch_ids[SWITCH_R3] = device.add_item(
			Buttons_Name[RETROPAD_R3],
			std::string_view(),
			ITEM_ID_BUTTON10,
			generic_button_get_state<std::int32_t>,
			&joystate[joy_count].button[RETROPAD_R3]);
		add_button_assignment(assignments, IPT_BUTTON10, { switch_ids[SWITCH_R3] });

		// d-pad
		switch_ids[SWITCH_DPAD_UP] = device.add_item(
			Buttons_Name[RETROPAD_PAD_UP],
			std::string_view(),
			ITEM_ID_HAT1UP,
			generic_button_get_state<std::uint8_t>,
			&joystate[joy_count].button[RETROPAD_PAD_UP]);

		switch_ids[SWITCH_DPAD_DOWN] = device.add_item(
			Buttons_Name[RETROPAD_PAD_DOWN],
			std::string_view(),
			ITEM_ID_HAT1DOWN,
			generic_button_get_state<std::uint8_t>,
			&joystate[joy_count].button[RETROPAD_PAD_DOWN]);

		switch_ids[SWITCH_DPAD_LEFT] = device.add_item(
			Buttons_Name[RETROPAD_PAD_LEFT],
			std::string_view(),
			ITEM_ID_HAT1LEFT,
			generic_button_get_state<std::uint8_t>,
			&joystate[joy_count].button[RETROPAD_PAD_LEFT]);

		switch_ids[SWITCH_DPAD_RIGHT] = device.add_item(
			Buttons_Name[RETROPAD_PAD_RIGHT],
			std::string_view(),
			ITEM_ID_HAT1RIGHT,
			generic_button_get_state<std::uint8_t>,
			&joystate[joy_count].button[RETROPAD_PAD_RIGHT]);

		joy_count++;

		// directions, analog stick
		add_directional_assignments(
				assignments,
				axis_ids[AXIS_LSX],
				axis_ids[AXIS_LSY],
				ITEM_ID_INVALID,
				ITEM_ID_INVALID,
				ITEM_ID_INVALID,
				ITEM_ID_INVALID);

		// directions, d-pad
		add_directional_assignments(
				assignments,
				ITEM_ID_INVALID,
				ITEM_ID_INVALID,
				switch_ids[SWITCH_DPAD_LEFT],
				switch_ids[SWITCH_DPAD_RIGHT],
				switch_ids[SWITCH_DPAD_UP],
				switch_ids[SWITCH_DPAD_DOWN]);

		// twin stick
		add_twin_stick_assignments(
				assignments,
				axis_ids[AXIS_LSX],
				axis_ids[AXIS_LSY],
				axis_ids[AXIS_RSX],
				axis_ids[AXIS_RSY],
				switch_ids[SWITCH_DPAD_LEFT],
				switch_ids[SWITCH_DPAD_RIGHT],
				switch_ids[SWITCH_DPAD_UP],
				switch_ids[SWITCH_DPAD_DOWN],
				switch_ids[SWITCH_Y],
				switch_ids[SWITCH_A],
				switch_ids[SWITCH_X],
				switch_ids[SWITCH_B]);

		// bonus AD stick Z
		assignments.emplace_back(
				IPT_AD_STICK_Z,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, axis_ids[AXIS_RSY])));

		// trigger pedals
		assignments.emplace_back(
				IPT_PEDAL,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, axis_ids[AXIS_R2])));
		assignments.emplace_back(
				IPT_PEDAL2,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, axis_ids[AXIS_L2])));

		// button pedals
		assignments.emplace_back(
				IPT_PEDAL,
				SEQ_TYPE_INCREMENT,
				input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_B])));
		assignments.emplace_back(
				IPT_PEDAL2,
				SEQ_TYPE_INCREMENT,
				input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_A])));

		// UI assignments
		add_button_assignment(assignments, IPT_UI_SELECT, { switch_ids[SWITCH_START] });
		add_button_assignment(assignments, IPT_UI_SELECT, { switch_ids[SWITCH_B] });
		add_button_assignment(assignments, IPT_UI_BACK, { switch_ids[SWITCH_A] });
		add_button_assignment(assignments, IPT_UI_CLEAR, { switch_ids[SWITCH_Y] });
		add_button_assignment(assignments, IPT_UI_FOCUS_NEXT, { switch_ids[SWITCH_X] });
		add_button_assignment(assignments, IPT_UI_PAGE_UP, { switch_ids[SWITCH_L1] });
		add_button_assignment(assignments, IPT_UI_PAGE_DOWN, { switch_ids[SWITCH_R1] });
		add_button_assignment(assignments, IPT_UI_FOCUS_NEXT, { switch_ids[SWITCH_L2] });
		add_button_assignment(assignments, IPT_UI_FOCUS_PREV, { switch_ids[SWITCH_R2] });
		add_button_assignment(assignments, IPT_UI_HOME, { switch_ids[SWITCH_L3] });
		add_button_assignment(assignments, IPT_UI_END, { switch_ids[SWITCH_R3] });

		// set default assignments
		device.set_default_assignments(std::move(assignments));
	}

protected:
	virtual void process_event(KeyPressEventArgs const &args)
	{
	}
};

//============================================================
//  joystick_input_retro - retro joystick input module
//============================================================

class joystick_input_retro : public retro_input_module<retro_joystick_device>
{
private:

public:
	joystick_input_retro()
		: retro_input_module(OSD_JOYSTICKINPUT_PROVIDER, "retro")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		retro_input_module<retro_joystick_device>::input_init(machine);

		char defname[32];

		joy_count = 0;

		if (buttons_profiles)
			Input_Binding(machine);

		for (int i = 0; i < 8; i++)
		{
 			sprintf(defname, "RetroPad%d", i);
			create_device<retro_joystick_device>(DEVICE_CLASS_JOYSTICK, defname, defname);
		}

		m_global_inputs_enabled = true;
	}

	bool handle_input_event(void) override
	{
		if (!input_enabled() /*|| !joystick_enabled()*/)
			return false;
		return true;
	}
};

//============================================================
//  retro_lightgun_device
//============================================================

// This device is purely event driven so the implementation is in the module
class retro_lightgun_device : public event_based_device<KeyPressEventArgs>
{
public:
	retro_lightgun_device(std::string &&name, std::string &&id, input_module &module)
		: event_based_device(std::move(name), std::move(id), module)
	{
	}

	void poll(bool relative_reset) override
	{
		event_based_device::poll(relative_reset);
	}

	virtual void reset() override
	{
		for (int j = 0; j < 8; j++)
		{
			lightgunX[j] = fb_width / 2;
			lightgunY[j] = fb_height / 2;

			for (int i = 0; i < 4; i++)
				lightgunstate[j].lightgunBUT[i] = 0;
		}
	}

	virtual void configure(osd::input_device &device)
	{
		device.add_item(
			"X",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_XAXIS),
			generic_axis_get_state<std::int32_t>,
			&lightgunX[lightgun_count]);
		device.add_item(
			"Y",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_YAXIS),
			generic_axis_get_state<std::int32_t>,
			&lightgunY[lightgun_count]);

		for (int button = 0; button < 4; button++)
		{
			lightgunstate[lightgun_count].lightgunBUT[button] = 0;

			device.add_item(
				default_button_name(button),
				std::string_view(),
				static_cast<input_item_id>(ITEM_ID_BUTTON1 + button),
				generic_button_get_state<std::int32_t>,
				&lightgunstate[lightgun_count].lightgunBUT[button]);
		}

		lightgun_count++;
	}

protected:
	virtual void process_event(KeyPressEventArgs const &args)
	{
	}
};

//============================================================
//  lightgun_input_retro - retro lightgun input module
//============================================================

class lightgun_input_retro : public retro_input_module<retro_lightgun_device>
{
private:

public:
	lightgun_input_retro()
		: retro_input_module(OSD_LIGHTGUNINPUT_PROVIDER, "retro")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		retro_input_module<retro_lightgun_device>::input_init(machine);

		if (!input_enabled() || !options()->lightgun())
			return;

		char defname[32];

		lightgun_count = 0;

		for (int i = 0; i < 8; i++)
		{
			sprintf(defname, "RetroLightgun%d", i);
			create_device<retro_lightgun_device>(DEVICE_CLASS_LIGHTGUN, defname, defname);

			lightgunX[i] = fb_width / 2;
			lightgunY[i] = fb_height / 2;
		}

		m_global_inputs_enabled = true;
	}

	bool handle_input_event(void) override
	{
		if (!input_enabled() || !options()->lightgun())
			return false;
		return true;
	}
};

} // namespace osd

void retro_osd_interface::process_events_buf()
{
	input_poll_cb();
}

void retro_osd_interface::poll_inputs(running_machine &machine)
{
	process_mouse_state(machine);
	process_keyboard_state(machine);
	process_joypad_state(machine);
	process_lightgun_state(machine);
}

MODULE_DEFINITION(KEYBOARDINPUT_RETRO, osd::keyboard_input_retro)
MODULE_DEFINITION(MOUSEINPUT_RETRO, osd::mouse_input_retro)
MODULE_DEFINITION(JOYSTICKINPUT_RETRO, osd::joystick_input_retro)
MODULE_DEFINITION(LIGHTGUNINPUT_RETRO, osd::lightgun_input_retro)
