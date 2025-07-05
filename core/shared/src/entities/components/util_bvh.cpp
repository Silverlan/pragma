// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/entities/components/util_bvh.hpp"
#include "pragma/entities/components/bvh_data.hpp"

std::unique_ptr<pragma::bvh::MeshBvhTree> pragma::bvh::create_bvh_data(std::vector<pragma::bvh::Primitive> &&triangles)
{
	auto bvhData = std::make_unique<MeshBvhTree>();
	bvhData->primitives = std::move(triangles);
	bvhData->InitializeBvh();
	return bvhData;
}
