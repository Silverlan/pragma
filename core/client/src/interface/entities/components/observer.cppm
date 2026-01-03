// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.observer;

export import :entities.components.entity;

export namespace pragma {
	namespace cObserverComponent {
		CLASS_ENUM_COMPAT ComponentEventId EVENT_CALC_VIEW;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_CALC_VIEW_OFFSET;
	}
	class DLLCLIENT CObserverComponent final : public BaseObserverComponent, public CBaseNetComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		CObserverComponent(ecs::BaseEntity &ent);
		virtual ~CObserverComponent() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
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
		virtual void PushArguments(lua::State *l) override;
		Vector3 &pos;
		Quat &rot;
		Quat &rotModifier;
	};

	struct DLLCLIENT CECalcViewOffset : public ComponentEvent {
		CECalcViewOffset(Vector3 &pos, Quat &rot);
		virtual void PushArguments(lua::State *l) override;
		Vector3 &pos;
		Quat &rot;
	};
};
