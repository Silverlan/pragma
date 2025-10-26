// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include <string>

#include <cinttypes>

export module pragma.shared:input.enums;

import pragma.math;

export {
	constexpr uint32_t GLFW_KEY_MENU = 348;
	constexpr uint32_t GLFW_KEY_LAST = GLFW_KEY_MENU;
	/* Printable keys */
	constexpr uint32_t GLFW_KEY_SPACE = 32;
	constexpr uint32_t GLFW_KEY_APOSTROPHE = 39; /* ' */
	constexpr uint32_t GLFW_KEY_COMMA = 44;      /* , */
	constexpr uint32_t GLFW_KEY_MINUS = 45;      /* - */
	constexpr uint32_t GLFW_KEY_PERIOD = 46;     /* . */
	constexpr uint32_t GLFW_KEY_SLASH = 47;      /* / */
	constexpr uint32_t GLFW_KEY_0 = 48;
	constexpr uint32_t GLFW_KEY_1 = 49;
	constexpr uint32_t GLFW_KEY_2 = 50;
	constexpr uint32_t GLFW_KEY_3 = 51;
	constexpr uint32_t GLFW_KEY_4 = 52;
	constexpr uint32_t GLFW_KEY_5 = 53;
	constexpr uint32_t GLFW_KEY_6 = 54;
	constexpr uint32_t GLFW_KEY_7 = 55;
	constexpr uint32_t GLFW_KEY_8 = 56;
	constexpr uint32_t GLFW_KEY_9 = 57;
	constexpr uint32_t GLFW_KEY_SEMICOLON = 59; /* ; */
	constexpr uint32_t GLFW_KEY_EQUAL = 61;     /* = */
	constexpr uint32_t GLFW_KEY_A = 65;
	constexpr uint32_t GLFW_KEY_B = 66;
	constexpr uint32_t GLFW_KEY_C = 67;
	constexpr uint32_t GLFW_KEY_D = 68;
	constexpr uint32_t GLFW_KEY_E = 69;
	constexpr uint32_t GLFW_KEY_F = 70;
	constexpr uint32_t GLFW_KEY_G = 71;
	constexpr uint32_t GLFW_KEY_H = 72;
	constexpr uint32_t GLFW_KEY_I = 73;
	constexpr uint32_t GLFW_KEY_J = 74;
	constexpr uint32_t GLFW_KEY_K = 75;
	constexpr uint32_t GLFW_KEY_L = 76;
	constexpr uint32_t GLFW_KEY_M = 77;
	constexpr uint32_t GLFW_KEY_N = 78;
	constexpr uint32_t GLFW_KEY_O = 79;
	constexpr uint32_t GLFW_KEY_P = 80;
	constexpr uint32_t GLFW_KEY_Q = 81;
	constexpr uint32_t GLFW_KEY_R = 82;
	constexpr uint32_t GLFW_KEY_S = 83;
	constexpr uint32_t GLFW_KEY_T = 84;
	constexpr uint32_t GLFW_KEY_U = 85;
	constexpr uint32_t GLFW_KEY_V = 86;
	constexpr uint32_t GLFW_KEY_W = 87;
	constexpr uint32_t GLFW_KEY_X = 88;
	constexpr uint32_t GLFW_KEY_Y = 89;
	constexpr uint32_t GLFW_KEY_Z = 90;
	constexpr uint32_t GLFW_KEY_LEFT_BRACKET = 91;  /* [ */
	constexpr uint32_t GLFW_KEY_BACKSLASH = 92;     /* \ */
	constexpr uint32_t GLFW_KEY_RIGHT_BRACKET = 93; /* ] */
	constexpr uint32_t GLFW_KEY_GRAVE_ACCENT = 96;  /* ` */
	constexpr uint32_t GLFW_KEY_WORLD_1 = 161;      /* non-US #1 */
	constexpr uint32_t GLFW_KEY_WORLD_2 = 162;      /* non-US #2 */

	/* Function keys */
	constexpr uint32_t GLFW_KEY_ESCAPE = 256;
	constexpr uint32_t GLFW_KEY_ENTER = 257;
	constexpr uint32_t GLFW_KEY_TAB = 258;
	constexpr uint32_t GLFW_KEY_BACKSPACE = 259;
	constexpr uint32_t GLFW_KEY_INSERT = 260;
	constexpr uint32_t GLFW_KEY_DELETE = 261;
	constexpr uint32_t GLFW_KEY_RIGHT = 262;
	constexpr uint32_t GLFW_KEY_LEFT = 263;
	constexpr uint32_t GLFW_KEY_DOWN = 264;
	constexpr uint32_t GLFW_KEY_UP = 265;
	constexpr uint32_t GLFW_KEY_PAGE_UP = 266;
	constexpr uint32_t GLFW_KEY_PAGE_DOWN = 267;
	constexpr uint32_t GLFW_KEY_HOME = 268;
	constexpr uint32_t GLFW_KEY_END = 269;
	constexpr uint32_t GLFW_KEY_CAPS_LOCK = 280;
	constexpr uint32_t GLFW_KEY_SCROLL_LOCK = 281;
	constexpr uint32_t GLFW_KEY_NUM_LOCK = 282;
	constexpr uint32_t GLFW_KEY_PRINT_SCREEN = 283;
	constexpr uint32_t GLFW_KEY_PAUSE = 284;
	constexpr uint32_t GLFW_KEY_F1 = 290;
	constexpr uint32_t GLFW_KEY_F2 = 291;
	constexpr uint32_t GLFW_KEY_F3 = 292;
	constexpr uint32_t GLFW_KEY_F4 = 293;
	constexpr uint32_t GLFW_KEY_F5 = 294;
	constexpr uint32_t GLFW_KEY_F6 = 295;
	constexpr uint32_t GLFW_KEY_F7 = 296;
	constexpr uint32_t GLFW_KEY_F8 = 297;
	constexpr uint32_t GLFW_KEY_F9 = 298;
	constexpr uint32_t GLFW_KEY_F10 = 299;
	constexpr uint32_t GLFW_KEY_F11 = 300;
	constexpr uint32_t GLFW_KEY_F12 = 301;
	constexpr uint32_t GLFW_KEY_F13 = 302;
	constexpr uint32_t GLFW_KEY_F14 = 303;
	constexpr uint32_t GLFW_KEY_F15 = 304;
	constexpr uint32_t GLFW_KEY_F16 = 305;
	constexpr uint32_t GLFW_KEY_F17 = 306;
	constexpr uint32_t GLFW_KEY_F18 = 307;
	constexpr uint32_t GLFW_KEY_F19 = 308;
	constexpr uint32_t GLFW_KEY_F20 = 309;
	constexpr uint32_t GLFW_KEY_F21 = 310;
	constexpr uint32_t GLFW_KEY_F22 = 311;
	constexpr uint32_t GLFW_KEY_F23 = 312;
	constexpr uint32_t GLFW_KEY_F24 = 313;
	constexpr uint32_t GLFW_KEY_F25 = 314;
	constexpr uint32_t GLFW_KEY_KP_0 = 320;
	constexpr uint32_t GLFW_KEY_KP_1 = 321;
	constexpr uint32_t GLFW_KEY_KP_2 = 322;
	constexpr uint32_t GLFW_KEY_KP_3 = 323;
	constexpr uint32_t GLFW_KEY_KP_4 = 324;
	constexpr uint32_t GLFW_KEY_KP_5 = 325;
	constexpr uint32_t GLFW_KEY_KP_6 = 326;
	constexpr uint32_t GLFW_KEY_KP_7 = 327;
	constexpr uint32_t GLFW_KEY_KP_8 = 328;
	constexpr uint32_t GLFW_KEY_KP_9 = 329;
	constexpr uint32_t GLFW_KEY_KP_DECIMAL = 330;
	constexpr uint32_t GLFW_KEY_KP_DIVIDE = 331;
	constexpr uint32_t GLFW_KEY_KP_MULTIPLY = 332;
	constexpr uint32_t GLFW_KEY_KP_SUBTRACT = 333;
	constexpr uint32_t GLFW_KEY_KP_ADD = 334;
	constexpr uint32_t GLFW_KEY_KP_ENTER = 335;
	constexpr uint32_t GLFW_KEY_KP_EQUAL = 336;
	constexpr uint32_t GLFW_KEY_LEFT_SHIFT = 340;
	constexpr uint32_t GLFW_KEY_LEFT_CONTROL = 341;
	constexpr uint32_t GLFW_KEY_LEFT_ALT = 342;
	constexpr uint32_t GLFW_KEY_LEFT_SUPER = 343;
	constexpr uint32_t GLFW_KEY_RIGHT_SHIFT = 344;
	constexpr uint32_t GLFW_KEY_RIGHT_CONTROL = 345;
	constexpr uint32_t GLFW_KEY_RIGHT_ALT = 346;
	constexpr uint32_t GLFW_KEY_RIGHT_SUPER = 347;

	constexpr uint32_t GLFW_KEY_SPECIAL_MOUSE_BUTTON_1 = (GLFW_KEY_LAST + 0);
	constexpr uint32_t GLFW_KEY_SPECIAL_MOUSE_BUTTON_2 = (GLFW_KEY_LAST + 1);
	constexpr uint32_t GLFW_KEY_SPECIAL_MOUSE_BUTTON_3 = (GLFW_KEY_LAST + 2);
	constexpr uint32_t GLFW_KEY_SPECIAL_MOUSE_BUTTON_4 = (GLFW_KEY_LAST + 3);
	constexpr uint32_t GLFW_KEY_SPECIAL_MOUSE_BUTTON_5 = (GLFW_KEY_LAST + 4);
	constexpr uint32_t GLFW_KEY_SPECIAL_MOUSE_BUTTON_6 = (GLFW_KEY_LAST + 5);
	constexpr uint32_t GLFW_KEY_SPECIAL_MOUSE_BUTTON_7 = (GLFW_KEY_LAST + 6);
	constexpr uint32_t GLFW_KEY_SPECIAL_MOUSE_BUTTON_8 = (GLFW_KEY_LAST + 7);
	constexpr uint32_t GLFW_KEY_SPECIAL_MOUSE_BUTTON_9 = (GLFW_KEY_LAST + 8);

	constexpr uint32_t GLFW_CUSTOM_KEY_SCRL_UP = (GLFW_KEY_SPECIAL_MOUSE_BUTTON_9 + 1);
	constexpr uint32_t GLFW_CUSTOM_KEY_SCRL_DOWN = (GLFW_CUSTOM_KEY_SCRL_UP + 1);

	// Maximum amount of controls (keys and axes) per controller, has to be dividable by 2!
	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT = 250;
	constexpr uint32_t GLFW_MAX_JOYSTICK_COUNT = 10;
	constexpr uint32_t GLFW_MAX_VR_CONTROLLER_COUNT = 10;
	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_KEY_COUNT = (GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT / 2);
	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT = (GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT / 2);

	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START = (GLFW_CUSTOM_KEY_SCRL_DOWN + 1000);
	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_0_AXIS_START = (GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_1_KEY_START = (GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_1_AXIS_START = (GLFW_CUSTOM_KEY_JOYSTICK_1_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_2_KEY_START = (GLFW_CUSTOM_KEY_JOYSTICK_1_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_2_AXIS_START = (GLFW_CUSTOM_KEY_JOYSTICK_2_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_3_KEY_START = (GLFW_CUSTOM_KEY_JOYSTICK_2_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_3_AXIS_START = (GLFW_CUSTOM_KEY_JOYSTICK_3_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_4_KEY_START = (GLFW_CUSTOM_KEY_JOYSTICK_3_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_4_AXIS_START = (GLFW_CUSTOM_KEY_JOYSTICK_4_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_5_KEY_START = (GLFW_CUSTOM_KEY_JOYSTICK_4_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_5_AXIS_START = (GLFW_CUSTOM_KEY_JOYSTICK_5_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_6_KEY_START = (GLFW_CUSTOM_KEY_JOYSTICK_5_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_6_AXIS_START = (GLFW_CUSTOM_KEY_JOYSTICK_6_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_7_KEY_START = (GLFW_CUSTOM_KEY_JOYSTICK_6_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_7_AXIS_START = (GLFW_CUSTOM_KEY_JOYSTICK_7_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_8_KEY_START = (GLFW_CUSTOM_KEY_JOYSTICK_7_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_8_AXIS_START = (GLFW_CUSTOM_KEY_JOYSTICK_8_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_9_KEY_START = (GLFW_CUSTOM_KEY_JOYSTICK_8_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_JOYSTICK_9_AXIS_START = (GLFW_CUSTOM_KEY_JOYSTICK_9_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_0_KEY_START = (GLFW_CUSTOM_KEY_JOYSTICK_9_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_0_AXIS_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_0_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_1_KEY_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_0_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_1_AXIS_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_1_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_2_KEY_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_1_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_2_AXIS_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_2_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_3_KEY_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_2_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_3_AXIS_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_3_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_4_KEY_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_3_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_4_AXIS_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_4_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_5_KEY_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_4_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_5_AXIS_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_5_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_6_KEY_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_5_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_6_AXIS_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_6_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_7_KEY_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_6_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_7_AXIS_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_7_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_8_KEY_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_7_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_8_AXIS_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_8_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_9_KEY_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_8_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT);
	constexpr uint32_t GLFW_CUSTOM_KEY_VR_CONTROLLER_9_AXIS_START = (GLFW_CUSTOM_KEY_VR_CONTROLLER_9_KEY_START + GLFW_CUSTOM_KEY_JOYSTICK_AXIS_COUNT);

	namespace pragma {
		enum class Action : uint32_t {
			None = 0,
			MoveForward = 1,
			MoveBackward = MoveForward << 1,
			MoveLeft = MoveBackward << 1,
			MoveRight = MoveLeft << 1,
			Sprint = MoveRight << 1,
			Walk = Sprint << 1,
			Jump = Walk << 1,
			Crouch = Jump << 1,
			Attack = Crouch << 1,
			Attack2 = Attack << 1,
			Attack3 = Attack2 << 1,
			Attack4 = Attack3 << 1,
			Reload = Attack4 << 1,
			Use = Reload << 1,

			Last = Use,
		};
		using namespace umath::scoped_enum::bitwise;
	}
	namespace umath::scoped_enum::bitwise {
		template<>
		struct enable_bitwise_operators<pragma::Action> : std::true_type {};
	};

	struct DLLNETWORK InputAction {
		pragma::Action action;
		unsigned char task;
	};

	DLLNETWORK bool get_controller_button(short &btId, uint32_t &controllerId, bool &axis);

	DLLNETWORK bool KeyToString(short c, std::string *key);
	DLLNETWORK bool KeyToText(short c, std::string *key);
	DLLNETWORK bool StringToKey(std::string key, short *c);
	const std::string BIND_KEYS[]
	= {"space", "escape", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15", "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23", "f24", "f25", "uparrow", "downarrow", "leftarrow", "rightarrow", "lshift", "rshift", "lctrl", "rctrl", "lalt",
		"ralt", "tab", "enter", "backspace", "ins", "del", "pgup", "pgdn", "home", "end", "kp_0", "kp_1", "kp_2", "kp_3", "kp_4", "kp_5", "kp_6", "kp_7", "kp_8", "kp_9", "kp_slash", "kp_multiply", "kp_minus", "kp_plus", "kp_del", "kp_equal", "kp_enter", "kp_numlock", "capslock", "scrolllock",
		"printscreen", "pause", "lsuper", "rsuper", "'", ",", ".", "/", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "[", "\\", "]", "`", ";", "=", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",

		"mouse1", "mouse2", "mouse3", "mouse4", "mouse5", "mouse6", "mouse7", "mouse8", "mouse9", "scrlup", "scrldn"};

	// These have to correspond to GLFW::KeyState!
	enum class KeyState : uint32_t { Invalid = std::numeric_limits<std::underlying_type_t<KeyState>>::max(), Press = 1, Release = 0, Repeat = 2, Held = Repeat + 1 };
};
