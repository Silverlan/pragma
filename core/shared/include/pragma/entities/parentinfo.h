/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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
