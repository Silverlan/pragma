/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENV_DECAL_H__
#define __ENV_DECAL_H__

#include "pragma/networkdefinitions.h"
#include <string>

namespace pragma
{
	class DLLNETWORK BaseEnvDecalComponent
		: public BaseEntityComponent
	{
	public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;

		float GetSize() const;
		const std::string &GetMaterial() const;
		bool GetStartDisabled() const;
	protected:
		float m_size = 1.f;
		std::string m_material = "";
		bool m_startDisabled = false;
	};
};

#endif
