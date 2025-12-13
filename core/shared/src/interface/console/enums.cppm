// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:console.enums;

export import pragma.math;

export namespace pragma::console {
	enum class MESSAGE : int { PRINTCONSOLE, PRINTCHAT };

	enum class ConType : uint32_t {
		Var = 0,
		Variable = 0,

		Cmd = 1,
		Command = 1,

		LuaCmd = 2,
		LuaCommand = 2
	};

	enum class ConVarFlags : uint32_t {
		None = 0,
		Cheat = 1,
		Singleplayer = Cheat << 1,
		Userinfo = Singleplayer << 1,
		Replicated = Userinfo << 1,
		Archive = Replicated << 1,
		Notify = Archive << 1,
		JoystickAxisContinuous = Notify << 1,
		JoystickAxisSingle = JoystickAxisContinuous << 1,
		Hidden = JoystickAxisSingle << 1,
		Password = Hidden << 1u,

		Last = Hidden
	};
	enum class ConsoleColorFlags : uint32_t {
		None = 0u,
		Red = 1u,
		Green = Red << 1u,
		Blue = Green << 1u,
		Intensity = Blue << 1u,

		BackgroundRed = Intensity << 1u,
		BackgroundGreen = BackgroundRed << 1u,
		BackgroundBlue = BackgroundGreen << 1u,
		BackgroundIntensity = BackgroundBlue << 1u,

		Reset = BackgroundIntensity << 1u,

		Yellow = Red | Green,
		Magenta = Red | Blue,
		Cyan = Blue | Green,
		White = Red | Green | Blue,
		Black = None,

		BackgroundYellow = BackgroundRed | BackgroundGreen,
		BackgroundMagenta = BackgroundRed | BackgroundBlue,
		BackgroundCyan = BackgroundBlue | BackgroundGreen,
		BackgroundWhite = BackgroundRed | BackgroundGreen | BackgroundBlue,
		BackgroundBlack = None
	};
	enum class ConsoleDecoratorFlags : uint32_t {
		None = 0,
		Bold = 1,
		Underline = Bold << 1,
		SlowBlink = Underline << 1,
		Framed = SlowBlink << 1,
		Encircled = Framed << 1,
		Overlined = Encircled << 1,
		Reset = Overlined << 1,
	};

	DLLNETWORK bool set_console_color(ConsoleColorFlags flags);
	DLLNETWORK bool reset_console_color();
	DLLNETWORK ConsoleColorFlags get_active_console_color_flags();
	DLLNETWORK std::optional<Color> console_color_flags_to_color(ConsoleColorFlags flags);
	DLLNETWORK ConsoleColorFlags color_to_console_color_flags(const Color &color);
	DLLNETWORK std::string get_ansi_color_code(ConsoleColorFlags flags);

	DLLNETWORK std::string get_true_color_code(std::optional<Color> foregroundColor, std::optional<Color> backgroundColor = {}, ConsoleDecoratorFlags flags = ConsoleDecoratorFlags::None);
	DLLNETWORK std::string get_reset_color_code();

	enum class MessageFlags : uint8_t {
		None = 0u,
		Generic = 1u,
		Warning = Generic << 1u,
		Error = Warning << 1u,
		Critical = Error << 1u,

		ServerSide = Critical << 1u,
		ClientSide = ServerSide << 1u
	};

	using namespace pragma::math::scoped_enum::bitwise;
}
export {
	REGISTER_ENUM_FLAGS(pragma::console::ConVarFlags)
	REGISTER_ENUM_FLAGS(pragma::console::ConsoleColorFlags)
	REGISTER_ENUM_FLAGS(pragma::console::ConsoleDecoratorFlags)
}
