// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/entities/components/panima_component_channel_submitter_def.hpp"

module pragma.shared;

import :entities.components.panima;

import panima;

#pragma optimize("", off)
DLLNETWORK void __panima_cvs_3()
{
	udm::visit_ng({}, [](auto tag) {
		using TChannel = typename decltype(tag)::type;
		if constexpr(udm::is_matrix_type<TChannel>)
			instantiate_get_member_channel_submitter<TChannel>();
	});
}
#pragma optimize("", on)
