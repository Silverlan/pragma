/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ANIMATION_DRIVER_COMPONENT_HPP__
#define __ANIMATION_DRIVER_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/entity_uuid_ref.hpp"
#include "pragma/game/value_driver.hpp"
#include <udm.hpp>

struct AnimationEvent;
namespace util {
	class Path;
};
namespace pragma {
	class DLLNETWORK AnimationDriverComponent final : public BaseEntityComponent {
	  public:
		using ValueDriverHash = uint64_t; // ComponentId +ComponentMemberIndex
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		AnimationDriverComponent(BaseEntity &ent);

		virtual void Initialize() override;
		virtual void OnRemove() override;

		void ClearDrivers();
		void AddDriver(ComponentId componentId, ComponentMemberIndex memberIdx, ValueDriverDescriptor descriptor);
		bool AddDriver(ComponentId componentId, const std::string &memberName, ValueDriverDescriptor descriptor);
		void RemoveDriver(ComponentId componentId, ComponentMemberIndex memberIdx);
		void RemoveDriver(ComponentId componentId, const std::string &memberName);
		void RemoveDrivers(ComponentId componentId);
		void ApplyDrivers();

		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;

		ValueDriver *FindDriver(ComponentId componentId, ComponentMemberIndex memberIdx);
		const ValueDriver *FindDriver(ComponentId componentId, ComponentMemberIndex memberIdx) const { return const_cast<AnimationDriverComponent *>(this)->FindDriver(componentId, memberIdx); }
		bool HasDriver(ComponentId componentId, ComponentMemberIndex memberIdx) const;

		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		using BaseEntityComponent::Load;
	  protected:
		std::optional<ComponentMemberIndex> FindComponentMember(ComponentId componentId, const std::string &memberName);
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		std::unordered_map<ValueDriverHash, ValueDriver> m_drivers;
	};
};

#endif
