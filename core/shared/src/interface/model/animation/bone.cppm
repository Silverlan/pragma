// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:model.animation.bone;

export import :util.global_string_table;

export import :model.animation.enums;

export {
	namespace pragma::animation {
		struct DLLNETWORK Bone : public std::enable_shared_from_this<Bone> {
			Bone();
			Bone(const Bone &other); // Parent has to be updated by caller!
			GString name;
			std::unordered_map<BoneId, std::shared_ptr<Bone>> children;
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
