/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ANIMATION_DRIVER_COMPONENT_HPP__
#define __ANIMATION_DRIVER_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/entity_uuid_ref.hpp"

struct AnimationEvent;
namespace util {class Path;};
namespace pragma
{
	struct DLLNETWORK AnimationDataValue
	{
		udm::Type type;
		std::unique_ptr<void,void(*)(void*)> data = std::unique_ptr<void,void(*)(void*)>{nullptr,[](void*) {}};
	};
	struct DLLNETWORK AnimationDriverVariable
	{
		AnimationDriverVariable(util::Uuid entUuid,const util::Path &var);
		AnimationDriverVariable(const AnimationDriverVariable&)=default;
		AnimationDriverVariable &operator=(const AnimationDriverVariable&)=default;
		EntityUuidComponentMemberRef memberRef;
	};
	using AnimationDriverVariableList = std::unordered_map<std::string,AnimationDriverVariable>;
	struct DLLNETWORK AnimationDriver
	{
		std::string expression;
		luabind::object luaExpression {};
		AnimationDataValue dataValue {};
		pragma::ComponentId componentId = std::numeric_limits<pragma::ComponentId>::max();
		AnimationDriverVariableList variables;
		
		const ComponentMemberInfo *GetMemberInfo(const BaseEntityComponent &component) const;
		const ComponentMemberInfo *GetMemberInfo(const BaseEntity &ent) const;
		ComponentMemberReference memberReference;
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
		void AddDriver(ComponentId componentId,ComponentMemberIndex memberIdx,const std::string &expression,AnimationDriverVariableList &&vars={});
		bool AddDriver(ComponentId componentId,const std::string &memberName,const std::string &expression,AnimationDriverVariableList &&vars={});
		void RemoveDriver(ComponentId componentId,ComponentMemberIndex memberIdx);
		void RemoveDriver(ComponentId componentId,const std::string &memberName);
		void RemoveDrivers(ComponentId componentId);
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
		std::optional<ComponentMemberIndex> FindComponentMember(ComponentId componentId,const std::string &memberName);
		virtual void Load(udm::LinkedPropertyWrapperArg udm,uint32_t version) override;
		std::unordered_map<AnimationDriverHash,AnimationDriver> m_drivers;
	};
};

#endif
