/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/game/value_driver.hpp"
#include "pragma/lua/lua_call.hpp"
#include <sharedutils/util_uri.hpp>

using namespace pragma;
#pragma optimize("",off)
pragma::ValueDriverDescriptor::ValueDriverDescriptor(
	lua_State *l,std::string expression,std::unordered_map<std::string,std::string> variables,
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
	std::string argList = "value,self";
	for(auto &pair : m_variables)
		argList += ',' +pair.first;

	auto *l = m_luaState;
	std::string luaStr = "return function(" +argList +") return " +m_expression +" end";
	auto r = Lua::RunString(l,luaStr,1,"internal",&Lua::HandleTracebackError); /* 1 */
	if(r == Lua::StatusCode::Ok)
	{
		luabind::object oFunc {luabind::from_stack(l,-1)};
		m_luaExpression = oFunc;
		Lua::Pop(l,1); /* 0 */
	}
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
void pragma::ValueDriverDescriptor::AddReference(const std::string &name,std::string path)
{
	m_variables[name] = std::move(path);
	m_expressionDirty = true;
}

////////////

ValueDriver::ValueDriver(pragma::ComponentId componentId,ComponentMemberReference memberRef,ValueDriverDescriptor descriptor,const util::Uuid &self)
	: m_componentId{componentId},m_memberReference{std::move(memberRef)},m_descriptor{std::move(descriptor)}
{
	auto &references = m_descriptor.GetReferences();
	m_variables.reserve(references.size());
	for(auto &pair : references)
	{
		auto var = ValueDriverVariable::Create(pair.second,self);
		if(!var.has_value())
			continue;
		m_variables.insert(std::make_pair(pair.first,*var));
	}
}
void pragma::ValueDriver::ResetFailureState()
{
	umath::set_flag(m_stateFlags,StateFlags::MemberRefFailed | StateFlags::ComponentRefFailed | StateFlags::EntityRefFailed,false);
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
	component->PushLuaObject(l);
	uint32_t numPushed = 3;
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

			if(!umath::is_flag_set(m_stateFlags,StateFlags::MemberRefFailed))
			{
				ResetFailureState(); // Clear other failure flags
				Con::cwar<<"WARNING: Unable to apply value driver with expression '"<<m_descriptor.GetExpression()<<"': Variable '"<<pair.first<<"' is member reference, but is pointing to invalid ";
				if(!var.memberRef.GetEntity(game))
					Con::cwar<<"entity";
				else if(!var.memberRef.GetComponent(game))
					Con::cwar<<"component";
				else
					Con::cwar<<"member";
				Con::cwar<<"!"<<Con::endl;
				umath::set_flag(m_stateFlags,StateFlags::MemberRefFailed);
			}
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
			
			if(!umath::is_flag_set(m_stateFlags,StateFlags::ComponentRefFailed))
			{
				ResetFailureState(); // Clear other failure flags
				Con::cwar<<"WARNING: Unable to apply value driver with expression '"<<m_descriptor.GetExpression()<<"': Variable '"<<pair.first<<"' is component reference, but is pointing to invalid ";
				if(!var.memberRef.GetEntity(game))
					Con::cwar<<"entity";
				else
					Con::cwar<<"component";
				Con::cwar<<"!"<<Con::endl;
				umath::set_flag(m_stateFlags,StateFlags::ComponentRefFailed);
			}
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
		
		if(!umath::is_flag_set(m_stateFlags,StateFlags::EntityRefFailed))
		{
			ResetFailureState(); // Clear other failure flags
			Con::cwar<<"WARNING: Unable to apply value driver with expression '"<<m_descriptor.GetExpression()<<"': Variable '"<<pair.first<<"' is entity reference, but is pointing to invalid entity!"<<Con::endl;
			umath::set_flag(m_stateFlags,StateFlags::EntityRefFailed);
		}
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

static EntityUuidComponentMemberRef get_member_ref(util::Uuid entUuid,std::string var)
{
	if(var.empty())
		return EntityUuidComponentMemberRef{entUuid,"",""};
	util::Path path {std::move(var)};
	size_t offset = 0;
	if(path.GetComponent(offset,&offset) != "ec")
		return EntityUuidComponentMemberRef{entUuid,"",""};
	auto componentName = path.GetComponent(offset,&offset);
	auto memberName = path.GetComponent(offset,&offset);
	return EntityUuidComponentMemberRef{entUuid,std::string{componentName},std::string{memberName}};
}
ValueDriverVariable::ValueDriverVariable(util::Uuid entUuid,std::string var)
	: memberRef{get_member_ref(entUuid,std::move(var))}
{}
std::optional<ValueDriverVariable> ValueDriverVariable::Create(std::string uriPath,const util::Uuid &self)
{
	uriparser::Uri uri {std::move(uriPath)};
	auto scheme = uri.scheme();
	if(!scheme.empty() && uri.scheme() != "pragma")
		return {};
	util::Path path {uri.path()};
	size_t offset = 0;
	if(path.GetComponent(offset,&offset) != "game" || path.GetComponent(offset,&offset) != "entity")
		return {};
	std::unordered_map<std::string_view,std::string_view> query;
	uriparser::parse_uri_query(uri.query(),query);
	util::Uuid uuid {};
	auto itUuid = query.find("entity_uuid");
	if(itUuid != query.end())
		uuid = util::uuid_string_to_bytes(std::string{itUuid->second}); // TODO: uuid_string_to_bytes should take a string_view argument!
	else
		uuid = util::uuid_string_to_bytes(util::uuid_to_string(self));
	auto &str = path.GetString();
	return ValueDriverVariable{uuid,(offset < str.size()) ? str.substr(offset) : std::string{}};
}
#pragma optimize("",on)
