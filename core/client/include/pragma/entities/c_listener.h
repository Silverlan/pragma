// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LISTENER_H__
#define __C_LISTENER_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/lua/c_listener_handle.hpp"

namespace al {
	class IListener;
};
namespace pragma {
	class DLLCLIENT CListenerComponent final : public BaseEntityComponent {
	  public:
		CListenerComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		CListenerComponent();
		virtual void Initialize() override;
		virtual void OnTick(double dt) override;
		float GetGain();
		void SetGain(float gain);
		virtual void InitializeLuaObject(lua_State *l) override;
	  private:
		al::IListener *m_listener = nullptr;
	};
};

class DLLCLIENT CListener : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
