// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/util/global_string_table.hpp"
#include "pragma/game/game_coordinate_system.hpp"
#include <sharedutils/util_string.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <optional>
#include <sharedutils/util_path.hpp>

export module pragma.shared:model.animation.bone;

export {
	namespace pragma::animation {
		using BoneId = uint16_t;
		struct DLLNETWORK Bone : public std::enable_shared_from_this<Bone> {
			Bone();
			Bone(const Bone &other); // Parent has to be updated by caller!
			pragma::GString name;
			std::unordered_map<pragma::animation::BoneId, std::shared_ptr<Bone>> children;
			std::weak_ptr<Bone> parent;
			BoneId ID;

			bool IsAncestorOf(const Bone &other) const;
			bool IsDescendantOf(const Bone &other) const;

			bool operator==(const Bone &other) const;
			bool operator!=(const Bone &other) const { return !operator==(other); }
		};
	};

	std::ostream &operator<<(std::ostream &out, const pragma::animation::Bone &o);
};
