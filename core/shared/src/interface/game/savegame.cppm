// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <memory>

export module pragma.shared:game.savegame;

export namespace pragma {
	namespace savegame {
		static constexpr uint32_t FORMAT_VERSION = 1u;
		static constexpr auto PSAV_IDENTIFIER = "PSAV";
		bool save(Game &game, const std::string &fileName, std::string &outErr);
		bool load(Game &game, const std::string &fileName, std::string &outErr);
	};
};
