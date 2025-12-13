// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:game.savegame;

export import :types;

export {
	namespace pragma::game {
		namespace savegame {
			constexpr uint32_t FORMAT_VERSION = 1u;
			constexpr auto PSAV_IDENTIFIER = "PSAV";
			bool save(Game &game, const std::string &fileName, std::string &outErr);
			bool load(Game &game, const std::string &fileName, std::string &outErr);
		};
	};
};
