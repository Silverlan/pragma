/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/util_bvh.hpp"
#include "pragma/entities/components/bvh_data.hpp"

std::unique_ptr<pragma::bvh::BvhData> pragma::bvh::create_bvh_data(std::vector<pragma::bvh::Primitive> &&triangles)
{
	auto bvhData = std::make_unique<BvhData>();
	bvhData->primitives = std::move(triangles);
	bvhData->InitializeIntersectorData();
	return bvhData;
}
