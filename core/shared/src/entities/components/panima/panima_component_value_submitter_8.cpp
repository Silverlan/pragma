/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/panima_component_channel_submitter_def.hpp"

import panima;

#pragma optimize("", off)
void __panima_cvs_8()
{
	udm::visit_ng({}, [](auto tag) {
		using TChannel = typename decltype(tag)::type;
		if constexpr(std::is_same_v<TChannel, udm::Int32> || std::is_same_v<TChannel, udm::UInt32>)
			instantiate_get_member_channel_submitter<TChannel>();
	});
}
#pragma optimize("", on)
