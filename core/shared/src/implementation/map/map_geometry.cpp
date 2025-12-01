// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :map.map_geometry;

static uint32_t get_next_row_neighbor(uint32_t vertId, uint32_t power, uint8_t decimateAmount) { return vertId + power * decimateAmount; }

static uint32_t get_next_column_neighbor(uint32_t vertId, uint32_t, uint8_t decimateAmount) { return vertId + 1 * decimateAmount; }

static void get_vertex_quad(uint32_t vertId, uint32_t power, uint8_t decimateAmount, std::vector<uint16_t> &r)
{
	r.reserve(r.size() + 6);

	// Triangle #1
	auto offset = r.size();
	r.push_back(static_cast<uint16_t>(vertId));
	r.push_back(static_cast<uint16_t>(get_next_column_neighbor(vertId, power, decimateAmount)));
	r.push_back(static_cast<uint16_t>(get_next_row_neighbor(vertId, power, decimateAmount)));

	// Triangle #2
	r.push_back(static_cast<uint16_t>(r[offset + 2]));
	r.push_back(static_cast<uint16_t>(r[offset + 1]));
	r.push_back(static_cast<uint16_t>(get_next_column_neighbor(r[offset + 2], power, decimateAmount)));
}

bool decimate_displacement_geometry(const std::vector<uint16_t> &indices, uint32_t power, std::vector<uint16_t> &outIndices, uint8_t decimateAmount)
{
	assert(decimateAmount > 1 && (indices.size() % decimateAmount) == 0);
	if(decimateAmount <= 1)
		return false;
	auto &indicesDecimated = outIndices;
	auto iterations = (power * (power - 1)) - decimateAmount;
	indicesDecimated.reserve(iterations * 6);
	for(auto i = decltype(iterations) {0}; i < iterations;) {
		get_vertex_quad(i, power, decimateAmount, indicesDecimated);
		if(((i + 1 + decimateAmount) % power) == 0)
			i += decimateAmount + 1 + power * (decimateAmount - 1);
		else
			i += decimateAmount;
	}
	return true;
}
