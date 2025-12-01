// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

module pragma.shared;

import :entities.components.panima;
import :entities.components.panima_channel_submitter;

import panima;

#pragma optimize("", off)
DLLNETWORK void __panima_cvs_7()
{
	udm::visit_ng({}, [](auto tag) {
		using TChannel = typename decltype(tag)::type;
		if constexpr(std::is_same_v<TChannel, udm::Int16> || std::is_same_v<TChannel, udm::UInt16>)
			instantiate_get_member_channel_submitter<TChannel>();
	});
}
#pragma optimize("", on)
