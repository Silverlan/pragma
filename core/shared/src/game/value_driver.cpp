/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/game/value_driver.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/logging.hpp"
#include <sharedutils/util_uri.hpp>

using namespace pragma;

pragma::ValueDriverDescriptor::ValueDriverDescriptor(lua_State *l, std::string expression, std::unordered_map<std::string, std::string> variables, std::unordered_map<std::string, udm::PProperty> constants)
    : m_luaState {l}, m_expression {std::move(expression)}, m_variables {std::move(variables)}, m_constants {std::move(constants)}
{
}
void pragma::ValueDriverDescriptor::SetExpression(const std::string &expression)
{
	m_expression = expression;
	m_expressionDirty = true;
}
void pragma::ValueDriverDescriptor::RebuildLuaExpression() const
{
	if(!m_luaState)
		return;
	std::string argList = "value,self";
	for(auto &pair : m_variables)
		argList += ',' + pair.first;

	auto *l = m_luaState;
	std::string luaStr = "return function(" + argList + ") " + m_expression + " end";
	auto r = Lua::RunString(l, luaStr, 1, "internal", &Lua::HandleTracebackError); /* 1 */
	if(r == Lua::StatusCode::Ok) {
		luabind::object oFunc {luabind::from_stack(l, -1)};
		m_luaExpression = oFunc;
		Lua::Pop(l, 1); /* 0 */
	}
	m_expressionDirty = false;
}
const luabind::object &pragma::ValueDriverDescriptor::GetLuaExpression() const
{
	if(m_expressionDirty) {
		RebuildLuaExpression();
		m_expressionDirty = false;
	}
	return m_luaExpression;
}
void pragma::ValueDriverDescriptor::AddConstant(const std::string &name, const udm::PProperty &prop)
{
	m_constants[name] = prop;
	m_expressionDirty = true;
}
void pragma::ValueDriverDescriptor::AddReference(const std::string &name, std::string path)
{
	m_variables[name] = std::move(path);
	m_expressionDirty = true;
}
void pragma::ValueDriverDescriptor::ClearConstants()
{
	m_constants.clear();
	m_expressionDirty = true;
}
void pragma::ValueDriverDescriptor::ClearReferences()
{
	m_variables.clear();
	m_expressionDirty = true;
}

std::ostream &operator<<(std::ostream &out, const pragma::ValueDriverDescriptor &descriptor)
{
	out << "ValueDriverDescriptor[Expr:" << descriptor.GetExpression() << "][Vars:" << descriptor.GetReferences().size() << "][Constants:" << descriptor.GetConstants().size() << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ValueDriver &driver)
{
	out << "ValueDriver[Expr:" << driver.GetDescriptor() << "][State:" << ((driver.IsFailureFlagSet()) ? "failed" : "base") << "]";
	return out;
}

////////////

ValueDriver::ValueDriver(pragma::ComponentId componentId, ComponentMemberReference memberRef, ValueDriverDescriptor descriptor, const util::Uuid &self) : m_componentId {componentId}, m_memberReference {std::move(memberRef)}, m_descriptor {std::move(descriptor)}
{
	auto &references = m_descriptor.GetReferences();
	m_variables.reserve(references.size());
	for(auto &pair : references) {
		auto var = ValueDriverVariable::Create(pair.second, self);
		if(!var.has_value())
			continue;
		m_variables.insert(std::make_pair(pair.first, *var));
	}
}
void pragma::ValueDriver::ResetFailureState() { umath::set_flag(m_stateFlags, StateFlags::MemberRefFailed | StateFlags::ComponentRefFailed | StateFlags::EntityRefFailed, false); }
bool pragma::ValueDriver::IsFailureFlagSet() const { return umath::is_flag_set(m_stateFlags, StateFlags::MemberRefFailed | StateFlags::ComponentRefFailed | StateFlags::EntityRefFailed); }
pragma::ValueDriver::Result pragma::ValueDriver::Apply(BaseEntity &ent)
{
	auto &luaExpression = m_descriptor.GetLuaExpression();
	auto &expression = m_descriptor.GetExpression();
	if(!luaExpression) {
		spdlog::trace("Failed to execute value driver: No Lua expression has been specified!");
		return Result::ErrorNoExpression;
	}
	auto *l = luaExpression.interpreter();
	auto &game = *pragma::get_engine()->GetNetworkState(l)->GetGameState();
	auto component = ent.FindComponent(m_componentId);
	if(component.expired()) {
		spdlog::trace("Failed to execute value driver (Expr: '{}'): Component {} could not be found in driver entity '{}'!", expression, m_componentId, ent.ToString());
		return Result::ErrorComponentNotFound;
	}
	auto *member = m_memberReference.GetMemberInfo(*component);
	if(!member) {
		spdlog::trace("Failed to execute value driver (Expr: '{}'): Member '{}' could not be found in component {} of driver entity '{}'!", expression, m_memberReference.GetMemberName(), m_componentId, ent.ToString());
		return Result::ErrorMemberNotFound;
	}
	auto udmType = ents::member_type_to_udm_type(member->type);
	if(udmType == udm::Type::Invalid) {
		spdlog::trace("Failed to execute value driver (Expr: '{}'): Member '{}' of component {} of driver entity '{}' has unsupported type {}!", expression, m_memberReference.GetMemberName(), m_componentId, ent.ToString(), magic_enum::enum_name(member->type));
		return Result::ErrorInvalidMemberType;
	}
	luabind::object arg;
	udm::visit_ng(udmType, [l, &arg, member, &component](auto tag) {
		using T = typename decltype(tag)::type;
		T value;
		member->getterFunction(*member, *component, &value);
		arg = luabind::object {l, value};
	});
	luaExpression.push(l);
	arg.push(l);
	component->PushLuaObject(l);
	uint32_t numPushed = 3;
	auto argsValid = true;
	for(auto &pair : m_variables) {
		auto &var = pair.second;
		auto *memInfo = var.memberRef.GetMemberInfo(game);
		if(memInfo) {
			auto *c = var.memberRef.GetComponent(game);
			auto o = udm::visit(udmType, [memInfo, c, l](auto tag) {
				using T = typename decltype(tag)::type;
				if constexpr(pragma::is_valid_component_property_type_v<T>) {
					T value;
					memInfo->getterFunction(*memInfo, *c, &value);
					return luabind::object {l, value};
				}
				else
					return Lua::nil;
			});
			o.push(l);
			++numPushed;
			continue;
		}
		if(var.memberRef.HasMemberReference()) {
			// Is a member reference, but member is not valid
			argsValid = false;

			if(!umath::is_flag_set(m_stateFlags, StateFlags::MemberRefFailed)) {
				ResetFailureState(); // Clear other failure flags
				std::string type;
				if(!var.memberRef.GetEntity(game))
					type = "entity";
				else if(!var.memberRef.GetComponent(game))
					type = "component";
				else
					type = "member";
				spdlog::trace("Failed to execute value driver (Expr: '{}') for member '{}' of component {} of driver entity '{}': Variable '{}' is member reference, but is pointing to invalid {}!", expression, m_memberReference.GetMemberName(), m_componentId, ent.ToString(), pair.first,
				  type);
				umath::set_flag(m_stateFlags, StateFlags::MemberRefFailed);
			}
			break;
		}

		auto *c = var.memberRef.GetComponent(game);
		if(c) {
			c->PushLuaObject(l);
			++numPushed;
			continue;
		}

		if(var.memberRef.HasComponentReference()) {
			// Is a component reference, but component is not valid
			argsValid = false;

			if(!umath::is_flag_set(m_stateFlags, StateFlags::ComponentRefFailed)) {
				ResetFailureState(); // Clear other failure flags
				std::string type;
				if(!var.memberRef.GetEntity(game))
					type = "entity";
				else
					type = "component";
				spdlog::trace("Failed to execute value driver (Expr: '{}') for member '{}' of component {} of driver entity '{}': Variable '{}' is component reference, but is pointing to invalid {}!", expression, m_memberReference.GetMemberName(), m_componentId, ent.ToString(), pair.first,
				  type);
				umath::set_flag(m_stateFlags, StateFlags::ComponentRefFailed);
			}
			break;
		}

		auto *e = var.memberRef.GetEntity(game);
		if(e) {
			e->PushLuaObject(l);
			++numPushed;
			continue;
		}

		// Is either an invalid reference altogether or an invalid entity reference
		argsValid = false;

		spdlog::trace("Failed to execute value driver (Expr: '{}') for member '{}' of component {} of driver entity '{}': Parameter '{}' is invalid!", expression, m_memberReference.GetMemberName(), m_componentId, ent.ToString(), var.memberRef.ToString());

		if(!umath::is_flag_set(m_stateFlags, StateFlags::EntityRefFailed)) {
			ResetFailureState(); // Clear other failure flags
			spdlog::trace("Failed to execute value driver (Expr: '{}') for member '{}' of component {} of driver entity '{}': Variable '{}' is entity reference, but is pointing to invalid entity!", expression, m_memberReference.GetMemberName(), m_componentId, ent.ToString(), pair.first);
			umath::set_flag(m_stateFlags, StateFlags::EntityRefFailed);
		}
		break;
	}
	if(!argsValid) {
		// Can't execute the driver if one or more of the arguments
		// couldn't be determined
		Lua::Pop(l, numPushed);
		return Result::ErrorInvalidParameterReference;
	}
	auto c = Lua::ProtectedCall(l, numPushed - 1, 1);
	if(c != Lua::StatusCode::Ok) {
		Lua::HandleLuaError(l, c);
		spdlog::trace("Failed to execute value driver (Expr: '{}') for member '{}' of component {} of driver entity '{}': Failed to execute expression!", expression, m_memberReference.GetMemberName(), m_componentId, ent.ToString());
		return Result::ErrorExpressionExecutionFailed;
	}
	luabind::object result {luabind::from_stack(l, -1)};
	Lua::Pop(l, 1);

	if(!result) {
		spdlog::trace("Failed to execute value driver (Expr: '{}') for member '{}' of component {} of driver entity '{}': Expression has no return value!", expression, m_memberReference.GetMemberName(), m_componentId, ent.ToString());
		return Result::ErrorNoExpressionReturnValue;
	}
	return udm::visit_ng(udmType, [l, this, &result, &member, &component, &ent, &expression](auto tag) {
		using T = typename decltype(tag)::type;
		try {
			auto ret = luabind::object_cast<T>(result);
			member->setterFunction(*member, *component, &ret);
		}
		catch(const luabind::cast_failed &e) {
			spdlog::trace("Failed to execute value driver (Expr: '{}') for member '{}' of component {} of driver entity '{}': Expression return value has type which is incompatible with expected type '{}'!", expression, m_memberReference.GetMemberName(), m_componentId, ent.ToString(),
			  magic_enum::enum_name(member->type));
			return Result::ErrorUnexpectedExpressionReturnValueType;
		}
		return Result::Success;
	});
}

////////////

ValueDriverVariable::ValueDriverVariable(EntityUComponentMemberRef memberRef) : memberRef {std::move(memberRef)} {}
ValueDriverVariable::ValueDriverVariable(util::Uuid entUuid, std::string var) { BaseEntity::CreateMemberReference(entUuid, std::move(var), memberRef); }
std::optional<ValueDriverVariable> ValueDriverVariable::Create(std::string uriPath, const util::Uuid &self)
{
	EntityUComponentMemberRef ref;
	if(BaseEntity::ParseUri(std::move(uriPath), ref, &self) == false)
		return {};
	return ValueDriverVariable {std::move(ref)};
}
