/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/input/inputhelper.h"
#include <pragma/localization.h>

DLLNETWORK bool KeyToString(short c, std::string *key)
{
	if(c <= GLFW_KEY_WORLD_2 && c != GLFW_KEY_SPACE) {
		*key = static_cast<char>(c);
		return true;
	}
	switch(c) {
	case GLFW_KEY_SPACE:
		*key = "space";
		break;
	case GLFW_KEY_ESCAPE:
		*key = "escape";
		break;
	case GLFW_KEY_F1:
		*key = "f1";
		break;
	case GLFW_KEY_F2:
		*key = "f2";
		break;
	case GLFW_KEY_F3:
		*key = "f3";
		break;
	case GLFW_KEY_F4:
		*key = "f4";
		break;
	case GLFW_KEY_F5:
		*key = "f5";
		break;
	case GLFW_KEY_F6:
		*key = "f6";
		break;
	case GLFW_KEY_F7:
		*key = "f7";
		break;
	case GLFW_KEY_F8:
		*key = "f8";
		break;
	case GLFW_KEY_F9:
		*key = "f9";
		break;
	case GLFW_KEY_F10:
		*key = "f10";
		break;
	case GLFW_KEY_F11:
		*key = "f11";
		break;
	case GLFW_KEY_F12:
		*key = "f12";
		break;
	case GLFW_KEY_F13:
		*key = "f13";
		break;
	case GLFW_KEY_F14:
		*key = "f14";
		break;
	case GLFW_KEY_F15:
		*key = "f15";
		break;
	case GLFW_KEY_F16:
		*key = "f16";
		break;
	case GLFW_KEY_F17:
		*key = "f17";
		break;
	case GLFW_KEY_F18:
		*key = "f18";
		break;
	case GLFW_KEY_F19:
		*key = "f19";
		break;
	case GLFW_KEY_F20:
		*key = "f20";
		break;
	case GLFW_KEY_F21:
		*key = "f21";
		break;
	case GLFW_KEY_F22:
		*key = "f22";
		break;
	case GLFW_KEY_F23:
		*key = "f23";
		break;
	case GLFW_KEY_F24:
		*key = "f24";
		break;
	case GLFW_KEY_F25:
		*key = "f25";
		break;
	case GLFW_KEY_UP:
		*key = "uparrow";
		break;
	case GLFW_KEY_DOWN:
		*key = "downarrow";
		break;
	case GLFW_KEY_LEFT:
		*key = "leftarrow";
		break;
	case GLFW_KEY_RIGHT:
		*key = "rightarrow";
		break;
	case GLFW_KEY_LEFT_SHIFT:
		*key = "lshift";
		break;
	case GLFW_KEY_RIGHT_SHIFT:
		*key = "rshift";
		break;
	case GLFW_KEY_LEFT_CONTROL:
		*key = "lctrl";
		break;
	case GLFW_KEY_RIGHT_CONTROL:
		*key = "rctrl";
		break;
	case GLFW_KEY_LEFT_ALT:
		*key = "lalt";
		break;
	case GLFW_KEY_RIGHT_ALT:
		*key = "ralt";
		break;
	case GLFW_KEY_TAB:
		*key = "tab";
		break;
	case GLFW_KEY_ENTER:
		*key = "enter";
		break;
	case GLFW_KEY_BACKSPACE:
		*key = "backspace";
		break;
	case GLFW_KEY_INSERT:
		*key = "ins";
		break;
	case GLFW_KEY_DELETE:
		*key = "del";
		break;
	case GLFW_KEY_PAGE_UP:
		*key = "pgup";
		break;
	case GLFW_KEY_PAGE_DOWN:
		*key = "pgdn";
		break;
	case GLFW_KEY_HOME:
		*key = "home";
		break;
	case GLFW_KEY_END:
		*key = "end";
		break;
	case GLFW_KEY_KP_0:
		*key = "kp_0";
		break;
	case GLFW_KEY_KP_1:
		*key = "kp_1";
		break;
	case GLFW_KEY_KP_2:
		*key = "kp_2";
		break;
	case GLFW_KEY_KP_3:
		*key = "kp_3";
		break;
	case GLFW_KEY_KP_4:
		*key = "kp_4";
		break;
	case GLFW_KEY_KP_5:
		*key = "kp_5";
		break;
	case GLFW_KEY_KP_6:
		*key = "kp_6";
		break;
	case GLFW_KEY_KP_7:
		*key = "kp_7";
		break;
	case GLFW_KEY_KP_8:
		*key = "kp_8";
		break;
	case GLFW_KEY_KP_9:
		*key = "kp_9";
		break;
	case GLFW_KEY_KP_DIVIDE:
		*key = "kp_slash";
		break;
	case GLFW_KEY_KP_MULTIPLY:
		*key = "kp_multiply";
		break;
	case GLFW_KEY_KP_SUBTRACT:
		*key = "kp_minus";
		break;
	case GLFW_KEY_KP_ADD:
		*key = "kp_plus";
		break;
	case GLFW_KEY_KP_DECIMAL:
		*key = "kp_del";
		break;
	case GLFW_KEY_KP_EQUAL:
		*key = "kp_equal";
		break;
	case GLFW_KEY_KP_ENTER:
		*key = "kp_enter";
		break;
	case GLFW_KEY_NUM_LOCK:
		*key = "kp_numlock";
		break;
	case GLFW_KEY_CAPS_LOCK:
		*key = "capslock";
		break;
	case GLFW_KEY_SCROLL_LOCK:
		*key = "scrolllock";
		break;
	case GLFW_KEY_PAUSE:
		*key = "pause";
		break;
	case GLFW_KEY_LEFT_SUPER:
		*key = "lsuper";
		break;
	case GLFW_KEY_RIGHT_SUPER:
		*key = "rsuper";
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_1:
		*key = "mouse1";
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_2:
		*key = "mouse2";
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_3:
		*key = "mouse3";
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_4:
		*key = "mouse4";
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_5:
		*key = "mouse5";
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_6:
		*key = "mouse6";
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_7:
		*key = "mouse7";
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_8:
		*key = "mouse8";
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_9:
		*key = "mouse9";
		break;
	case GLFW_KEY_PRINT_SCREEN:
		*key = "printscreen";
		break;
	case GLFW_CUSTOM_KEY_SCRL_UP:
		*key = "scrlup";
		break;
	case GLFW_CUSTOM_KEY_SCRL_DOWN:
		*key = "scrldn";
		break;
	default:
		if(c >= GLFW_CUSTOM_KEY_VR_CONTROLLER_0_KEY_START) {
			auto btId = c;
			uint32_t controllerId = 0;
			auto bAxis = false;
			auto r = get_controller_button(btId, controllerId, bAxis);
			assert(r == true);
			if(bAxis == false)
				*key = "vr" + std::to_string(controllerId) + "bt" + std::to_string(btId);
			else
				*key = "vr" + std::to_string(controllerId) + "axis" + std::to_string(btId);
		}
		else if(c >= GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START) {
			auto btId = c;
			uint32_t controllerId = 0;
			auto bAxis = false;
			auto r = get_controller_button(btId, controllerId, bAxis);
			assert(r == true);
			if(bAxis == false)
				*key = "joy" + std::to_string(controllerId) + "bt" + std::to_string(btId);
			else
				*key = "joy" + std::to_string(controllerId) + "axis" + std::to_string(btId);
		}
		else
			return false;
	}
	return true;
}

DLLNETWORK bool KeyToText(short c, std::string *key)
{
	if(c <= GLFW_KEY_WORLD_2 && c != GLFW_KEY_SPACE) {
		*key = static_cast<char>(c);
		return true;
	}
	switch(c) {
	case GLFW_KEY_SPACE:
		*key = Locale::GetText("key_space");
		break;
	case GLFW_KEY_ESCAPE:
		*key = Locale::GetText("key_escape");
		break;
	case GLFW_KEY_F1:
		*key = Locale::GetText("key_function") + "1";
		break;
	case GLFW_KEY_F2:
		*key = Locale::GetText("key_function") + "2";
		break;
	case GLFW_KEY_F3:
		*key = Locale::GetText("key_function") + "3";
		break;
	case GLFW_KEY_F4:
		*key = Locale::GetText("key_function") + "4";
		break;
	case GLFW_KEY_F5:
		*key = Locale::GetText("key_function") + "5";
		break;
	case GLFW_KEY_F6:
		*key = Locale::GetText("key_function") + "6";
		break;
	case GLFW_KEY_F7:
		*key = Locale::GetText("key_function") + "7";
		break;
	case GLFW_KEY_F8:
		*key = Locale::GetText("key_function") + "8";
		break;
	case GLFW_KEY_F9:
		*key = Locale::GetText("key_function") + "9";
		break;
	case GLFW_KEY_F10:
		*key = Locale::GetText("key_function") + "10";
		break;
	case GLFW_KEY_F11:
		*key = Locale::GetText("key_function") + "11";
		break;
	case GLFW_KEY_F12:
		*key = Locale::GetText("key_function") + "12";
		break;
	case GLFW_KEY_F13:
		*key = Locale::GetText("key_function") + "13";
		break;
	case GLFW_KEY_F14:
		*key = Locale::GetText("key_function") + "14";
		break;
	case GLFW_KEY_F15:
		*key = Locale::GetText("key_function") + "15";
		break;
	case GLFW_KEY_F16:
		*key = Locale::GetText("key_function") + "16";
		break;
	case GLFW_KEY_F17:
		*key = Locale::GetText("key_function") + "17";
		break;
	case GLFW_KEY_F18:
		*key = Locale::GetText("key_function") + "18";
		break;
	case GLFW_KEY_F19:
		*key = Locale::GetText("key_function") + "19";
		break;
	case GLFW_KEY_F20:
		*key = Locale::GetText("key_function") + "20";
		break;
	case GLFW_KEY_F21:
		*key = Locale::GetText("key_function") + "21";
		break;
	case GLFW_KEY_F22:
		*key = Locale::GetText("key_function") + "22";
		break;
	case GLFW_KEY_F23:
		*key = Locale::GetText("key_function") + "23";
		break;
	case GLFW_KEY_F24:
		*key = Locale::GetText("key_function") + "24";
		break;
	case GLFW_KEY_F25:
		*key = Locale::GetText("key_function") + "25";
		break;
	case GLFW_KEY_UP:
		*key = Locale::GetText("key_uparrow");
		break;
	case GLFW_KEY_DOWN:
		*key = Locale::GetText("key_downarrow");
		break;
	case GLFW_KEY_LEFT:
		*key = Locale::GetText("key_leftarrow");
		break;
	case GLFW_KEY_RIGHT:
		*key = Locale::GetText("key_rightarrow");
		break;
	case GLFW_KEY_LEFT_SHIFT:
		*key = Locale::GetText("key_lshift");
		break;
	case GLFW_KEY_RIGHT_SHIFT:
		*key = Locale::GetText("key_rshift");
		break;
	case GLFW_KEY_LEFT_CONTROL:
		*key = Locale::GetText("key_lctrl");
		break;
	case GLFW_KEY_RIGHT_CONTROL:
		*key = Locale::GetText("key_rctrl");
		break;
	case GLFW_KEY_LEFT_ALT:
		*key = Locale::GetText("key_lalt");
		break;
	case GLFW_KEY_RIGHT_ALT:
		*key = Locale::GetText("key_ralt");
		break;
	case GLFW_KEY_TAB:
		*key = Locale::GetText("key_tab");
		break;
	case GLFW_KEY_ENTER:
		*key = Locale::GetText("key_enter");
		break;
	case GLFW_KEY_BACKSPACE:
		*key = Locale::GetText("key_backspace");
		break;
	case GLFW_KEY_INSERT:
		*key = Locale::GetText("key_ins");
		break;
	case GLFW_KEY_DELETE:
		*key = Locale::GetText("key_del");
		break;
	case GLFW_KEY_PAGE_UP:
		*key = Locale::GetText("key_pgup");
		break;
	case GLFW_KEY_PAGE_DOWN:
		*key = Locale::GetText("key_pgdn");
		break;
	case GLFW_KEY_HOME:
		*key = Locale::GetText("key_home");
		break;
	case GLFW_KEY_END:
		*key = Locale::GetText("key_end");
		break;
	case GLFW_KEY_KP_0:
		*key = Locale::GetText("key_kp") + " 0";
		break;
	case GLFW_KEY_KP_1:
		*key = Locale::GetText("key_kp") + " 1";
		break;
	case GLFW_KEY_KP_2:
		*key = Locale::GetText("key_kp") + " 2";
		break;
	case GLFW_KEY_KP_3:
		*key = Locale::GetText("key_kp") + " 3";
		break;
	case GLFW_KEY_KP_4:
		*key = Locale::GetText("key_kp") + " 4";
		break;
	case GLFW_KEY_KP_5:
		*key = Locale::GetText("key_kp") + " 5";
		break;
	case GLFW_KEY_KP_6:
		*key = Locale::GetText("key_kp") + " 6";
		break;
	case GLFW_KEY_KP_7:
		*key = Locale::GetText("key_kp") + " 7";
		break;
	case GLFW_KEY_KP_8:
		*key = Locale::GetText("key_kp") + " 8";
		break;
	case GLFW_KEY_KP_9:
		*key = Locale::GetText("key_kp") + " 9";
		break;
	case GLFW_KEY_KP_DIVIDE:
		*key = Locale::GetText("key_kp_slash");
		break;
	case GLFW_KEY_KP_MULTIPLY:
		*key = Locale::GetText("key_kp_multiply");
		break;
	case GLFW_KEY_KP_SUBTRACT:
		*key = Locale::GetText("key_kp_minus");
		break;
	case GLFW_KEY_KP_ADD:
		*key = Locale::GetText("key_kp_plus");
		break;
	case GLFW_KEY_KP_DECIMAL:
		*key = Locale::GetText("key_kp_del");
		break;
	case GLFW_KEY_KP_EQUAL:
		*key = Locale::GetText("key_kp_equal");
		break;
	case GLFW_KEY_KP_ENTER:
		*key = Locale::GetText("key_kp_enter");
		break;
	case GLFW_KEY_NUM_LOCK:
		*key = Locale::GetText("key_numlock");
		break;
	case GLFW_KEY_CAPS_LOCK:
		*key = Locale::GetText("key_capslock");
		break;
	case GLFW_KEY_SCROLL_LOCK:
		*key = Locale::GetText("key_scrolllock");
		break;
	case GLFW_KEY_PAUSE:
		*key = Locale::GetText("key_pause");
		break;
	case GLFW_KEY_LEFT_SUPER:
		*key = Locale::GetText("key_lsuper");
		break;
	case GLFW_KEY_RIGHT_SUPER:
		*key = Locale::GetText("key_rsuper");
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_1:
		*key = Locale::GetText("mouse_left");
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_2:
		*key = Locale::GetText("mouse_right");
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_3:
		*key = Locale::GetText("mouse_middle");
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_4:
		*key = Locale::GetText("mouse") + " 4";
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_5:
		*key = Locale::GetText("mouse") + " 5";
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_6:
		*key = Locale::GetText("mouse") + " 6";
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_7:
		*key = Locale::GetText("mouse") + " 7";
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_8:
		*key = Locale::GetText("mouse") + " 8";
		break;
	case GLFW_KEY_SPECIAL_MOUSE_BUTTON_9:
		*key = Locale::GetText("mouse") + " 9";
		break;
	case GLFW_KEY_PRINT_SCREEN:
		*key = Locale::GetText("key_printscreen");
		break;
	case GLFW_CUSTOM_KEY_SCRL_UP:
		*key = Locale::GetText("key_scroll_up");
		break;
	case GLFW_CUSTOM_KEY_SCRL_DOWN:
		*key = Locale::GetText("key_scroll_down");
		break;
	default:
		{
			auto btId = c;
			uint32_t controllerId = 0;
			auto bAxis = false;
			if(get_controller_button(btId, controllerId, bAxis) == true) {
				std::string identifier;
				if((c >= GLFW_CUSTOM_KEY_VR_CONTROLLER_0_KEY_START) ? true : false)
					identifier = (bAxis == false) ? "key_vr" : "key_vr_axis";
				else
					identifier = (bAxis == false) ? "key_joystick" : "key_joystick_axis";
				*key = Locale::GetText(identifier, {std::to_string(controllerId + 1), std::to_string(btId + 1)});
			}
			else
				return false;
		}
	}
	return true;
}

DLLNETWORK bool get_controller_button(short &btId, uint32_t &controllerId, bool &axis)
{
	if(btId >= GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START) {
		axis = false;
		btId -= GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START;
		controllerId = btId / GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT;
		if(controllerId >= GLFW_MAX_JOYSTICK_COUNT)
			controllerId -= GLFW_MAX_JOYSTICK_COUNT; // It's a VR controller
		btId %= GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT;
		if(btId >= GLFW_CUSTOM_KEY_JOYSTICK_KEY_COUNT) {
			axis = true;
			btId -= GLFW_CUSTOM_KEY_JOYSTICK_KEY_COUNT;
		}
		return true;
	}
	return false;
}

DLLNETWORK bool StringToKey(std::string key, short *c)
{
	if(key.length() == 0)
		return false;
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	static const std::unordered_map<std::string, decltype(GLFW_KEY_SPACE)> keyMap
	  = {{"space", GLFW_KEY_SPACE}, {"escape", GLFW_KEY_ESCAPE}, {"f1", GLFW_KEY_F1}, {"f2", GLFW_KEY_F2}, {"f3", GLFW_KEY_F3}, {"f4", GLFW_KEY_F4}, {"f5", GLFW_KEY_F5}, {"f6", GLFW_KEY_F6}, {"f7", GLFW_KEY_F7}, {"f8", GLFW_KEY_F8}, {"f9", GLFW_KEY_F9}, {"f10", GLFW_KEY_F10},
	    {"f11", GLFW_KEY_F11}, {"f12", GLFW_KEY_F12}, {"f13", GLFW_KEY_F13}, {"f14", GLFW_KEY_F14}, {"f15", GLFW_KEY_F15}, {"f16", GLFW_KEY_F16}, {"f17", GLFW_KEY_F17}, {"f18", GLFW_KEY_F18}, {"f19", GLFW_KEY_F19}, {"f20", GLFW_KEY_F20}, {"f21", GLFW_KEY_F21}, {"f22", GLFW_KEY_F22},
	    {"f23", GLFW_KEY_F23}, {"f24", GLFW_KEY_F24}, {"f25", GLFW_KEY_F25}, {"uparrow", GLFW_KEY_UP}, {"downarrow", GLFW_KEY_DOWN}, {"leftarrow", GLFW_KEY_LEFT}, {"rightarrow", GLFW_KEY_RIGHT}, {"lshift", GLFW_KEY_LEFT_SHIFT}, {"rshift", GLFW_KEY_RIGHT_SHIFT},
	    {"lctrl", GLFW_KEY_LEFT_CONTROL}, {"rctrl", GLFW_KEY_RIGHT_CONTROL}, {"lalt", GLFW_KEY_LEFT_ALT}, {"ralt", GLFW_KEY_RIGHT_ALT}, {"tab", GLFW_KEY_TAB}, {"enter", GLFW_KEY_ENTER}, {"backspace", GLFW_KEY_BACKSPACE}, {"ins", GLFW_KEY_INSERT}, {"del", GLFW_KEY_DELETE},
	    {"pgup", GLFW_KEY_PAGE_UP}, {"pgdn", GLFW_KEY_PAGE_DOWN}, {"home", GLFW_KEY_HOME}, {"end", GLFW_KEY_END}, {"kp_0", GLFW_KEY_KP_0}, {"kp_1", GLFW_KEY_KP_1}, {"kp_2", GLFW_KEY_KP_2}, {"kp_3", GLFW_KEY_KP_3}, {"kp_4", GLFW_KEY_KP_4}, {"kp_5", GLFW_KEY_KP_5}, {"kp_6", GLFW_KEY_KP_6},
	    {"kp_7", GLFW_KEY_KP_7}, {"kp_8", GLFW_KEY_KP_8}, {"kp_9", GLFW_KEY_KP_9}, {"kp_slash", GLFW_KEY_KP_DIVIDE}, {"kp_multiply", GLFW_KEY_KP_MULTIPLY}, {"kp_minus", GLFW_KEY_KP_SUBTRACT}, {"kp_plus", GLFW_KEY_KP_ADD}, {"kp_del", GLFW_KEY_KP_DECIMAL}, {"kp_equal", GLFW_KEY_KP_EQUAL},
	    {"kp_enter", GLFW_KEY_KP_ENTER}, {"kp_numlock", GLFW_KEY_NUM_LOCK}, {"capslock", GLFW_KEY_CAPS_LOCK}, {"scrolllock", GLFW_KEY_SCROLL_LOCK}, {"pause", GLFW_KEY_PAUSE}, {"lsuper", GLFW_KEY_LEFT_SUPER}, {"rsuper", GLFW_KEY_RIGHT_SUPER}, {"mouse1", GLFW_KEY_SPECIAL_MOUSE_BUTTON_1},
	    {"mouse2", GLFW_KEY_SPECIAL_MOUSE_BUTTON_2}, {"mouse3", GLFW_KEY_SPECIAL_MOUSE_BUTTON_3}, {"mouse4", GLFW_KEY_SPECIAL_MOUSE_BUTTON_4}, {"mouse5", GLFW_KEY_SPECIAL_MOUSE_BUTTON_5}, {"mouse6", GLFW_KEY_SPECIAL_MOUSE_BUTTON_6}, {"mouse7", GLFW_KEY_SPECIAL_MOUSE_BUTTON_7},
	    {"mouse8", GLFW_KEY_SPECIAL_MOUSE_BUTTON_8}, {"mouse9", GLFW_KEY_SPECIAL_MOUSE_BUTTON_9}, {"printscreen", GLFW_KEY_PRINT_SCREEN}, {"scrlup", GLFW_CUSTOM_KEY_SCRL_UP}, {"scrldn", GLFW_CUSTOM_KEY_SCRL_DOWN}};
	auto it = keyMap.find(key);
	if(it != keyMap.end()) {
		*c = it->second;
		return true;
	}
	if(key.length() > 1) {
		auto fTranslateKey = [](const std::string &key, short *c, const std::string &id, uint32_t axisStart, uint32_t keyStart) -> bool {
			if(ustring::substr(key, 0, 3) == id) {
				const std::string numbers = "0123456789";
				auto nEnd = key.find_first_not_of(numbers, 3);
				if(nEnd != std::string::npos) {
					auto controllerId = ustring::to_int(ustring::substr(key, 3, nEnd));
					auto btStart = key.find_first_of(numbers, nEnd);
					if(btStart != std::string::npos) {
						auto type = ustring::substr(key, nEnd, btStart - nEnd);
						auto bAxis = (type == "axis") ? true : false;
						if(bAxis == true || type == "bt") {
							auto btId = ustring::to_int(ustring::substr(key, btStart));
							*c = ((bAxis == true) ? axisStart : keyStart) + btId;
							*c += controllerId * GLFW_CUSTOM_KEY_JOYSTICK_CONTROL_COUNT;
							return true;
						}
					}
				}
			}
			return false;
		};
		return fTranslateKey(key, c, "joy", GLFW_CUSTOM_KEY_JOYSTICK_0_AXIS_START, GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START) || fTranslateKey(key, c, "vr", GLFW_CUSTOM_KEY_VR_CONTROLLER_0_AXIS_START, GLFW_CUSTOM_KEY_VR_CONTROLLER_0_KEY_START);
	}
	else
		*c = key.front();
	return true;
}
