/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ANIMATION_DRIVER_COMPONENT_HPP__
#define __ANIMATION_DRIVER_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

struct AnimationEvent;
namespace util {class Path;};
namespace pragma
{
	struct DLLNETWORK AnimationDataValue
	{
		udm::Type type;
		std::unique_ptr<void,void(*)(void*)> data = std::unique_ptr<void,void(*)(void*)>{nullptr,[](void*) {}};
	};
	struct DLLNETWORK AnimationDriver
	{
		std::string expression;
		luabind::object luaExpression {};
		udm::PProperty constants = nullptr;
		AnimationDataValue dataValue {};
		pragma::ComponentId componentId = std::numeric_limits<pragma::ComponentId>::max();
		pragma::ComponentMemberIndex memberIndex = std::numeric_limits<pragma::ComponentMemberIndex>::max();

		const ComponentMemberInfo *GetMemberInfo(const BaseEntity &ent) const;
	};
	class DLLNETWORK AnimationDriverComponent final
		: public BaseEntityComponent
	{
	public:
		using AnimationDriverHash = uint64_t; // ComponentId +ComponentMemberIndex
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);
		
		AnimationDriverComponent(BaseEntity &ent);

		virtual void Initialize() override;
		virtual void OnRemove() override;

		void ClearDrivers();
		void AddDriver(ComponentId componentId,ComponentMemberIndex memberIdx,const std::string &expression,udm::PProperty constants=nullptr);
		void ApplyDrivers();

		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;

		AnimationDriver *FindDriver(ComponentId componentId,ComponentMemberIndex memberIdx);
		const AnimationDriver *FindDriver(ComponentId componentId,ComponentMemberIndex memberIdx) const {return const_cast<AnimationDriverComponent*>(this)->FindDriver(componentId,memberIdx);}
		bool HasDriver(ComponentId componentId,ComponentMemberIndex memberIdx) const;
		
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		using BaseEntityComponent::Load;
	protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm,uint32_t version) override;
		std::unordered_map<AnimationDriverHash,AnimationDriver> m_drivers;
	};
};

#endif
