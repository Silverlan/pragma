// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/entities/components/panima_component_channel_submitter_def.hpp"

import panima;

#pragma optimize("", off)
DLLNETWORK void __panima_cvs_5()
{
	udm::visit_ng({}, [](auto tag) {
		using TChannel = typename decltype(tag)::type;
		if constexpr(std::is_same_v<TChannel, udm::Vector2> || std::is_same_v<TChannel, udm::Vector3> || std::is_same_v<TChannel, udm::Vector4>)
			instantiate_get_member_channel_submitter<TChannel>();
	});
}
#pragma optimize("", on)
