// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.velocity;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK VelocityComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		VelocityComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		void SetVelocity(const Vector3 &vel);
		void AddVelocity(const Vector3 &vel);
		const Vector3 &GetVelocity() const;
		void SetAngularVelocity(const Vector3 &vel);
		void AddAngularVelocity(const Vector3 &vel);
		const Vector3 &GetAngularVelocity() const;
		void SetLocalAngularVelocity(Vector3 vel);
		void AddLocalAngularVelocity(Vector3 vel);
		Vector3 GetLocalAngularVelocity() const;

		void SetLocalVelocity(Vector3 vel);
		void AddLocalVelocity(Vector3 vel);
		Vector3 GetLocalVelocity() const;

		const util::PVector3Property &GetVelocityProperty() const;
		const util::PVector3Property &GetAngularVelocityProperty() const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;

		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void InitializeLuaObject(lua::State *lua) override;

		// Set member variables directly, without any other influences
		void SetRawVelocity(const Vector3 &vel);
		void SetRawAngularVelocity(const Vector3 &vel);
	  protected:
		util::PVector3Property m_velocity;
		util::PVector3Property m_angVelocity;
	};
};
