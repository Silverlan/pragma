// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.parent_info;

export import :entities.enums;

export {
	struct DLLNETWORK AttachmentInfo {
		std::optional<Vector3> offset = {};
		std::optional<Quat> rotation = {};
		pragma::FAttachmentMode flags = pragma::FAttachmentMode::None;
	};
	struct DLLNETWORK AttachmentData {
		AttachmentData() = default;
		~AttachmentData() = default;
		Vector3 offset = {};
		Quat rotation = uquat::identity();
		pragma::FAttachmentMode flags = pragma::FAttachmentMode::None;
		int32_t bone = -1;
		int32_t attachment = -1;
		std::vector<int32_t> boneMapping = {};
	};
};
