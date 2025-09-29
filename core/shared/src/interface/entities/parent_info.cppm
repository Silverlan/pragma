// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include <sharedutils/util_shared_handle.hpp>
#include <vector>
#include <optional>
#include "mathutil/uquat.h"
#include "mathutil/uvec.h"


export module pragma.shared:entities.parent_info;

export import :entities.enums;

export {
	struct DLLNETWORK AttachmentInfo {
		std::optional<Vector3> offset = {};
		std::optional<Quat> rotation = {};
		FAttachmentMode flags = FAttachmentMode::None;
	};
	struct DLLNETWORK AttachmentData {
		AttachmentData() = default;
		~AttachmentData() = default;
		Vector3 offset = {};
		Quat rotation = uquat::identity();
		FAttachmentMode flags = FAttachmentMode::None;
		int32_t bone = -1;
		int32_t attachment = -1;
		std::vector<int32_t> boneMapping = {};
	};
};
