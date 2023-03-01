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
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		AnimationDriverComponent(BaseEntity &ent);

		virtual void Initialize() override;
		virtual void OnRemove() override;

		bool ApplyDriver();

		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;

		void SetExpression(const std::string &expression);
		const std::string &GetExpression() const;

		template<typename T>
		    requires(udm::type_to_enum_s<T>() != udm::Type::Invalid)
		void AddConstant(const std::string &name, T &&value)
		{
			return AddConstant(name, udm::Property::Create<T>(std::forward<T>(value)));
		}
		void AddConstant(const std::string &name, const udm::PProperty &prop);
		void AddReference(const std::string &name, std::string path);

		const std::unordered_map<std::string, udm::PProperty> &GetConstants() const;
		const std::unordered_map<std::string, std::string> &GetReferences() const;

		void SetDrivenObject(const pragma::EntityUComponentMemberRef &drivenObject);
		const pragma::EntityUComponentMemberRef &GetDrivenObject() const;

		udm::PProperty &GetParameters();
		void UpdateParameters();

		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		using BaseEntityComponent::Load;
	  protected:
		std::optional<ComponentMemberIndex> FindComponentMember(ComponentId componentId, const std::string &memberName);
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		pragma::EntityUComponentMemberRef m_drivenObject;
		std::unique_ptr<ValueDriver> m_driver;
		ValueDriverDescriptor m_descriptor;
		bool m_driverDirty = true;
		udm::PProperty m_parameters;
	};
};

#endif
