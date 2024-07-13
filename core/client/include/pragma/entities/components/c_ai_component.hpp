/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_AI_COMPONENT_HPP__
#define __C_AI_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_ai_component.hpp>
#include <pragma/util/util_handled.hpp>

class CBaseEntity;
namespace pragma {
	class DLLCLIENT CAIComponent final : public BaseAIComponent, public CBaseSnapshotComponent {
	  private:
		static std::vector<CAIComponent *> s_npcs;
	  protected:
		Vector3 OnCalcMovementDirection() const;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
	  public:
		static unsigned int GetNPCCount();
		static const std::vector<CAIComponent *> &GetAll();
		CAIComponent(BaseEntity &ent);
		virtual ~CAIComponent() override;
		virtual void Initialize() override;
		virtual void UpdateMovementProperties(MovementComponent &movementC) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual bool ShouldTransmitNetData() const override { return false; }
		virtual void ReceiveSnapshotData(NetPacket &packet) override;
	};
};

#endif
