// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:game.value_driver;

export import :entities.enums;
export import :entities.universal_reference;
export import :entities.member_reference;
export import pragma.lua;

export {
	namespace pragma::game {
		struct DLLNETWORK ValueDriverDescriptor {
			ValueDriverDescriptor() = default;
			ValueDriverDescriptor(lua::State *l, std::string expression, std::unordered_map<std::string, std::string> variables, std::unordered_map<std::string, udm::PProperty> constants);
			ValueDriverDescriptor(lua::State *l, std::string expression, std::unordered_map<std::string, std::string> variables) : ValueDriverDescriptor {l, expression, std::move(variables), {}} {}
			ValueDriverDescriptor(lua::State *l, std::string expression) : ValueDriverDescriptor {l, expression, {}, {}} {}
			ValueDriverDescriptor(lua::State *l) : ValueDriverDescriptor {l, {}, {}, {}} {}
			void SetExpression(const std::string &expression);
			const luabind::object &GetLuaExpression() const;
			const std::string &GetExpression() const { return m_expression; }

			template<typename T>
			    requires(udm::type_to_enum_s<T>() != udm::Type::Invalid)
			void AddConstant(const std::string &name, T &&value)
			{
				return AddConstant(name, udm::Property::Create<T>(std::forward<T>(value)));
			}
			void AddConstant(const std::string &name, const udm::PProperty &prop);
			void AddReference(const std::string &name, std::string path);

			void ClearConstants();
			void ClearReferences();

			const std::unordered_map<std::string, udm::PProperty> &GetConstants() const { return m_constants; }
			const std::unordered_map<std::string, std::string> &GetReferences() const { return m_variables; }
		  private:
			void RebuildLuaExpression() const;
			std::string m_expression;
			mutable bool m_expressionDirty = true;
			mutable luabind::object m_luaExpression;
			lua::State *m_luaState = nullptr;

			std::unordered_map<std::string, udm::PProperty> m_constants;
			std::unordered_map<std::string, std::string> m_variables;
		};
		struct DLLNETWORK ValueDriverVariable {
			// Path format: pragma:game/entity/ec/<componentTypeName>/<memberName>?entity_uuid=<uuid>
			static std::optional<ValueDriverVariable> Create(std::string path, const util::Uuid &self);
			ValueDriverVariable(util::Uuid entUuid, std::string var);
			ValueDriverVariable(EntityUComponentMemberRef memberRef);
			ValueDriverVariable(const ValueDriverVariable &) = default;
			EntityUComponentMemberRef memberRef;
		};
		struct DLLNETWORK ValueDriver {
			enum class StateFlags : uint32_t { None = 0u, MemberRefFailed = 1u, ComponentRefFailed = MemberRefFailed << 1u, EntityRefFailed = ComponentRefFailed << 1u };
			enum class Result : uint32_t { Success = 0, ErrorNoExpression, ErrorComponentNotFound, ErrorMemberNotFound, ErrorInvalidMemberType, ErrorInvalidParameterReference, ErrorExpressionExecutionFailed, ErrorNoExpressionReturnValue, ErrorUnexpectedExpressionReturnValueType };
			ValueDriver() = default;
			ValueDriver(ComponentId componentId, ComponentMemberReference memberRef, ValueDriverDescriptor descriptor, const util::Uuid &self);
			const ComponentMemberReference &GetMemberReference() const { return m_memberReference; }
			const ValueDriverDescriptor &GetDescriptor() const { return m_descriptor; }
			ComponentId GetComponentId() const { return m_componentId; }

			Result Apply(ecs::BaseEntity &ent);
			void ResetFailureState();
			bool IsFailureFlagSet() const;
		  private:
			ValueDriverDescriptor m_descriptor;
			std::unordered_map<std::string, ValueDriverVariable> m_variables;
			ComponentId m_componentId = std::numeric_limits<ComponentId>::max();
			ComponentMemberReference m_memberReference;
			StateFlags m_stateFlags = StateFlags::None;
		};
		using namespace pragma::math::scoped_enum::bitwise;

		std::ostream &operator<<(std::ostream &out, const ValueDriverDescriptor &descriptor);
		std::ostream &operator<<(std::ostream &out, const ValueDriver &driver);
	};
	REGISTER_ENUM_FLAGS(pragma::game::ValueDriver::StateFlags)
};
