/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/level/mapgeometry.h"
#include "pragma/model/modelmesh.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/model/model.h"
#include "pragma/util/util_bsp_tree.hpp"

static uint32_t get_next_row_neighbor(uint32_t vertId,uint32_t power,uint8_t decimateAmount)
{
	return vertId +power *decimateAmount;
}

static uint32_t get_next_column_neighbor(uint32_t vertId,uint32_t,uint8_t decimateAmount)
{
	return vertId +1 *decimateAmount;
}

static void get_vertex_quad(uint32_t vertId,uint32_t power,uint8_t decimateAmount,std::vector<uint16_t> &r)
{
	r.reserve(r.size() +6);

	// Triangle #1
	auto offset = r.size();
	r.push_back(static_cast<uint16_t>(vertId));
	r.push_back(static_cast<uint16_t>(get_next_column_neighbor(vertId,power,decimateAmount)));
	r.push_back(static_cast<uint16_t>(get_next_row_neighbor(vertId,power,decimateAmount)));

	// Triangle #2
	r.push_back(static_cast<uint16_t>(r[offset +2]));
	r.push_back(static_cast<uint16_t>(r[offset +1]));
	r.push_back(static_cast<uint16_t>(get_next_column_neighbor(r[offset +2],power,decimateAmount)));
}

bool decimate_displacement_geometry(const std::vector<uint16_t> &indices,uint32_t power,std::vector<uint16_t> &outIndices,uint8_t decimateAmount)
{
	UNUSED(indices);
	assert(decimateAmount > 1 && (indices.size() %decimateAmount) == 0);
	if(decimateAmount <= 1)
		return false;
	auto &indicesDecimated = outIndices;
	auto iterations = (power *(power -1)) -decimateAmount;
	indicesDecimated.reserve(iterations *6);
	for(auto i=decltype(iterations){0};i<iterations;)
	{
		get_vertex_quad(i,power,decimateAmount,indicesDecimated);
		if(((i +1 +decimateAmount) %power) == 0)
			i += decimateAmount +1 +power *(decimateAmount -1);
		else
			i += decimateAmount;
	}
	return true;
}
