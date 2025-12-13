// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.animation_driver;

export import :entities.components.base;
export import :entities.universal_reference;
import :game.value_driver;

export namespace pragma {
	class DLLNETWORK AnimationDriverComponent final : public BaseEntityComponent {
	  public:
		using ValueDriverHash = uint64_t; // ComponentId +ComponentMemberIndex
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		AnimationDriverComponent(ecs::BaseEntity &ent);

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

		void SetDrivenObject(const EntityUComponentMemberRef &drivenObject);
		const EntityUComponentMemberRef &GetDrivenObject() const;

		udm::PProperty &GetParameters();
		void UpdateParameters();

		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		using BaseEntityComponent::Load;
	  protected:
		std::optional<ComponentMemberIndex> FindComponentMember(ComponentId componentId, const std::string &memberName);
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		EntityUComponentMemberRef m_drivenObject;
		std::unique_ptr<game::ValueDriver> m_driver;
		game::ValueDriverDescriptor m_descriptor;
		bool m_driverDirty = true;
		udm::PProperty m_parameters;
	};
};
