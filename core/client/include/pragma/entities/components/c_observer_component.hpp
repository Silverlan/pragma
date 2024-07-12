/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __C_OBSERVER_COMPONENT_HPP__
#define __C_OBSERVER_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_observer_component.hpp>
#include <pragma/util/util_handled.hpp>
#include <pragma/math/deltaoffset.h>
#include <pragma/math/deltatransform.h>

class CBaseEntity;
struct DeltaTransform;
namespace pragma {
	class DLLCLIENT CObserverComponent final : public BaseObserverComponent, public CBaseNetComponent {
	  public:
		static ComponentEventId EVENT_CALC_VIEW;
		static ComponentEventId EVENT_CALC_VIEW_OFFSET;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		CObserverComponent(BaseEntity &ent);
		virtual ~CObserverComponent() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
	  public:
		virtual void SetObserverMode(ObserverMode mode) override;
		virtual void SetObserverTarget(BaseObservableComponent *ent) override;
		virtual void DoSetObserverMode(ObserverMode mode) override;
		void UpdateCharacterViewOrientationFromMouseMovement();
		void UpdateCameraPose();
		void ApplyCameraObservationMode(Vector3 &pos, Quat &rot, Quat &rotModifier);

		// Returns true if observer mode is first-person and observer target is this player (Only works for the local player)
		bool IsInFirstPersonMode() const;
	  protected:
		Quat m_curFrameRotationModifier = uquat::identity();
	};

	struct DLLCLIENT CECalcView : public ComponentEvent {
		CECalcView(Vector3 &pos, Quat &rot, Quat &rotModifier);
		virtual void PushArguments(lua_State *l) override;
		Vector3 &pos;
		Quat &rot;
		Quat &rotModifier;
	};

	struct DLLCLIENT CECalcViewOffset : public ComponentEvent {
		CECalcViewOffset(Vector3 &pos, Quat &rot);
		virtual void PushArguments(lua_State *l) override;
		Vector3 &pos;
		Quat &rot;
	};
};

#endif
