// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/entities/components/panima_component_channel_submitter_def.hpp"

#include "udm.hpp"

#include "pragma/networkdefinitions.h"

module pragma.shared;

import :entities.components.panima;

import panima;

#pragma optimize("", off)
DLLNETWORK void __panima_cvs_2()
{
	udm::visit_ng({}, [](auto tag) {
		using TChannel = typename decltype(tag)::type;
		if constexpr(std::is_floating_point_v<TChannel> || std::is_same_v<TChannel, udm::Boolean>)
			instantiate_get_member_channel_submitter<TChannel>();
	});
}
#pragma optimize("", on)
