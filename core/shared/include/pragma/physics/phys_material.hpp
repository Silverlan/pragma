/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PHYS_MATERIAL_HPP__
#define __PHYS_MATERIAL_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/base.hpp"

class SurfaceMaterial;
namespace pragma::physics {
	class DLLNETWORK IMaterial : public IBase {
	  public:
		using IBase::IBase;
		virtual ~IMaterial() = default;
		void SetFriction(float friction);
		virtual float GetStaticFriction() const = 0;
		virtual void SetStaticFriction(float friction) = 0;
		virtual float GetDynamicFriction() const = 0;
		virtual void SetDynamicFriction(float friction) = 0;
		virtual float GetRestitution() const = 0;
		virtual void SetRestitution(float restitution) = 0;

		void SetSurfaceMaterial(SurfaceMaterial &surfMat);
		SurfaceMaterial *GetSurfaceMaterial() const;
		virtual void InitializeLuaObject(lua_State *lua) override;
	  private:
		uint32_t m_surfMatIdx = std::numeric_limits<uint32_t>::max();
	};
};

#endif
