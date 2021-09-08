/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/game/value_driver.hpp"
#include "pragma/lua/lua_call.hpp"

using namespace pragma;
#pragma optimize("",off)
pragma::ValueDriverDescriptor::ValueDriverDescriptor(
	lua_State *l,std::string expression,std::unordered_map<std::string,util::Path> variables,
	std::unordered_map<std::string,udm::PProperty> constants
) : m_luaState{l},m_expression{std::move(expression)},m_variables{std::move(variables)},m_constants{std::move(constants)}
{}
void pragma::ValueDriverDescriptor::SetExpression(const std::string &expression)
{
	m_expression = expression;
	m_expressionDirty = true;
}
void pragma::ValueDriverDescriptor::RebuildLuaExpression() const
{
	if(!m_luaState)
		return;
	std::string argList = "value";
	for(auto &pair : m_variables)
		argList += ',' +pair.first;

	auto *l = m_luaState;
	std::string luaStr = "return function(" +argList +") return " +m_expression +" end";
	auto r = Lua::RunString(l,luaStr,1,"internal"); /* 1 */
	if(r == Lua::StatusCode::Ok)
	{
		luabind::object oFunc {luabind::from_stack(l,-1)};
		m_luaExpression = oFunc;
		Lua::Pop(l,1); /* 0 */
	}
	else
		Lua::HandleLuaError(l);
	m_expressionDirty = false;
}
const luabind::object &pragma::ValueDriverDescriptor::GetLuaExpression() const
{
	if(m_expressionDirty)
	{
		RebuildLuaExpression();
		m_expressionDirty = false;
	}
	return m_luaExpression;
}
void pragma::ValueDriverDescriptor::AddConstant(const std::string &name,const udm::PProperty &prop)
{
	m_constants[name] = prop;
	m_expressionDirty = true;
}
void pragma::ValueDriverDescriptor::AddReference(const std::string &name,util::Path path)
{
	m_variables[name] = std::move(path);
	m_expressionDirty = true;
}

////////////

ValueDriver::ValueDriver(pragma::ComponentId componentId,ComponentMemberReference memberRef,ValueDriverDescriptor descriptor)
	: m_componentId{componentId},m_memberReference{std::move(memberRef)},m_descriptor{std::move(descriptor)}
{
	auto &references = m_descriptor.GetReferences();
	m_variables.reserve(references.size());
	for(auto &pair : references)
	{
		auto var = ValueDriverVariable::Create(pair.second);
		if(!var.has_value())
			continue;
		m_variables.insert(std::make_pair(pair.first,*var));
	}
}
bool pragma::ValueDriver::Apply(BaseEntity &ent)
{
	auto &luaExpression = m_descriptor.GetLuaExpression();
	if(!luaExpression)
		return false;
	auto *l = luaExpression.interpreter();
	auto &game = *pragma::get_engine()->GetNetworkState(l)->GetGameState();
	auto component = ent.FindComponent(m_componentId);
	if(component.expired())
		return false;
	auto *member = m_memberReference.GetMemberInfo(*component);
	if(!member)
		return false;
	luabind::object arg;
	udm::visit_ng(member->type,[l,&arg,member,&component](auto tag) {
		using T = decltype(tag)::type;
		T value;
		member->getterFunction(*member,*component,&value);
		arg = luabind::object{l,value};
	});
	luaExpression.push(l);
	arg.push(l);
	uint32_t numPushed = 2;
	auto argsValid = true;
	for(auto &pair : m_variables)
	{
		auto &var = pair.second;
		auto *memInfo = var.memberRef.GetMemberInfo(game);
		if(memInfo)
		{
			auto *c = var.memberRef.GetComponent(game);
			auto o = udm::visit_ng(memInfo->type,[memInfo,c,l](auto tag) {
				using T = decltype(tag)::type;
				T value;
				memInfo->getterFunction(*memInfo,*c,&value);
				return luabind::object{l,value};
			});
			o.push(l);
			++numPushed;
			continue;
		}
		if(var.memberRef.HasMemberReference())
		{
			// Is a member reference, but member is not valid
			argsValid = false;
			break;
		}

		auto *c = var.memberRef.GetComponent(game);
		if(c)
		{
			c->PushLuaObject(l);
			++numPushed;
			continue;
		}

		if(var.memberRef.HasComponentReference())
		{
			// Is a component reference, but component is not valid
			argsValid = false;
			break;
		}

		auto *e = var.memberRef.GetEntity(game);
		if(e)
		{
			e->PushLuaObject(l);
			++numPushed;
			continue;
		}
		
		// Is either an invalid reference altogether or an invalid entity reference
		argsValid = false;
		break;
	}
	if(!argsValid)
	{
		// Can't execute the driver if one or more of the arguments
		// couldn't be determined
		Lua::Pop(l,numPushed);
		return false;
	}
	auto c = Lua::ProtectedCall(l,numPushed -1,1);
	if(c != Lua::StatusCode::Ok)
	{
		Lua::HandleLuaError(l,c);
		return false;
	}
	luabind::object result {luabind::from_stack(l,-1)};
	Lua::Pop(l,1);

	if(!result)
		return false;
	udm::visit_ng(member->type,[l,this,&result,&member,&component](auto tag) {
		using T = decltype(tag)::type;
		try
		{
			auto ret = luabind::object_cast<T>(result);
			member->setterFunction(*member,*component,&ret);
		}
		catch(const luabind::cast_failed &e)
		{
			Con::cwar<<"WARNING: Driver expression '"<<m_descriptor.GetExpression()<<"' return value is incompatible with expected type '"<<magic_enum::enum_name(member->type)<<"'!"<<Con::endl;
		}
	});
	return true;
}

////////////

static EntityUuidComponentMemberRef get_member_ref(util::Uuid entUuid,util::Path var)
{
	auto componentName = var.GetFront();
	var.PopFront();
	auto &memberName = var.GetString();
	return EntityUuidComponentMemberRef{entUuid,std::string{componentName},memberName};
}
ValueDriverVariable::ValueDriverVariable(util::Uuid entUuid,const util::Path &var)
	: memberRef{get_member_ref(entUuid,var)}
{}
std::optional<ValueDriverVariable> ValueDriverVariable::Create(util::Path path)
{
	auto uuid = path.GetFront();
	path.PopFront();
	return ValueDriverVariable{util::uuid_string_to_bytes(std::string{uuid}),std::move(path)};
}
#pragma optimize("",on)
