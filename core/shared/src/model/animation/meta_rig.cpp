/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "pragma/model/animation/meta_rig.hpp"

pragma::animation::BoneId pragma::animation::MetaRig::GetBoneId(const pragma::GString &type) const
{
	auto eType = get_meta_rig_bone_type_enum(type);
	if(!eType)
		return pragma::animation::INVALID_BONE_INDEX;
	auto i = umath::to_integral(*eType);
	if(i >= bones.size())
		return pragma::animation::INVALID_BONE_INDEX;
	return bones[i].boneId;
}
