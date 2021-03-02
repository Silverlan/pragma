/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PARTICLE_MODIFIER_COMPONENT_EASE_HPP__
#define __C_PARTICLE_MODIFIER_COMPONENT_EASE_HPP__

#include "pragma/clientdefinitions.h"
#include <string>
#include <unordered_map>

namespace umath {enum class EaseType : uint32_t;};
class DLLCLIENT CParticleModifierComponentEase
{
protected:
	CParticleModifierComponentEase();
	void Initialize(const std::unordered_map<std::string,std::string> &values);

	float Ease(float t) const;
	float EaseIn(float t) const;
	float EaseOut(float t) const;
	float EaseInOut(float t) const;
private:
	enum class EaseFunc : uint32_t
	{
		None = 0u,
		In,
		Out,
		InOut
	};
	umath::EaseType m_type;
	EaseFunc m_easeFunc = EaseFunc::InOut;
};

#endif
