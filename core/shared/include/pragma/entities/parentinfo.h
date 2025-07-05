// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PARENTINFO_H__
#define __PARENTINFO_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include <pragma/math/angle/wvquaternion.h>
#include "pragma/entities/parentmode.h"
#include <sharedutils/util_shared_handle.hpp>
#include <vector>
#include <optional>

namespace pragma {
	class ParentComponent;
};
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

#endif
