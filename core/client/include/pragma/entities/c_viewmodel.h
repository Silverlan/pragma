/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_VIEWMODEL_H__
#define __C_VIEWMODEL_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"

namespace pragma
{
	class CPlayerComponent;
	class CWeaponComponent;
	class DLLCLIENT CViewModelComponent final
		: public BaseEntityComponent
	{
	public:
		CViewModelComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		void SetViewModelOffset(const Vector3 &offset);
		const Vector3 &GetViewModelOffset() const;
		void SetViewFOV(float fov);
		float GetViewFOV() const;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;

		CPlayerComponent *GetPlayer();
		CWeaponComponent *GetWeapon();
	protected:
		Vector3 m_viewModelOffset;
		float m_viewFov = std::numeric_limits<float>::quiet_NaN();
	};
};

class DLLCLIENT CViewModel
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif