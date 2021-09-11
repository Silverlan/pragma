/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PRAGMA_VALUE_DRIVER_HPP__
#define __PRAGMA_VALUE_DRIVER_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/types.hpp"
#include "pragma/entities/entity_uuid_ref.hpp"
#include <luabind/object.hpp>
#include <sharedutils/util_path.hpp>
#include <udm.hpp>

namespace pragma
{
	struct DLLNETWORK ValueDriverDescriptor
	{
		ValueDriverDescriptor()=default;
		ValueDriverDescriptor(
			lua_State *l,std::string expression,std::unordered_map<std::string,std::string> variables,
			std::unordered_map<std::string,udm::PProperty> constants
		);
		ValueDriverDescriptor(
			lua_State *l,std::string expression,std::unordered_map<std::string,std::string> variables
		) : ValueDriverDescriptor{l,expression,std::move(variables),{}} {}
		ValueDriverDescriptor(
			lua_State *l,std::string expression
		) : ValueDriverDescriptor{l,expression,{},{}} {}
		void SetExpression(const std::string &expression);
		const luabind::object &GetLuaExpression() const;
		const std::string &GetExpression() const {return m_expression;}

		template<typename T> requires(udm::type_to_enum_s<T>() != udm::Type::Invalid)
			void AddConstant(const std::string &name,T &&value)
		{
			return AddConstant(name,udm::Property::Create<T>(std::forward<T>(value)));
		}
		void AddConstant(const std::string &name,const udm::PProperty &prop);
		void AddReference(const std::string &name,std::string path);

		const std::unordered_map<std::string,udm::PProperty> &GetConstants() const {return m_constants;}
		const std::unordered_map<std::string,std::string> &GetReferences() const {return m_variables;}
	private:
		void RebuildLuaExpression() const;
		std::string m_expression;
		mutable bool m_expressionDirty = true;
		mutable luabind::object m_luaExpression;
		lua_State *m_luaState = nullptr;

		std::unordered_map<std::string,udm::PProperty> m_constants;
		std::unordered_map<std::string,std::string> m_variables;
	};
	struct DLLNETWORK ValueDriverVariable
	{
		// Path format: pragma:game/entity/ec/<componentTypeName>/<memberName>?entity_uuid=<uuid>
		static std::optional<ValueDriverVariable> Create(std::string path);
		ValueDriverVariable(util::Uuid entUuid,const std::string &var);
		ValueDriverVariable(const ValueDriverVariable&)=default;
		EntityUuidComponentMemberRef memberRef;
	};
	struct DLLNETWORK ValueDriver
	{
		ValueDriver()=default;
		ValueDriver(pragma::ComponentId componentId,ComponentMemberReference memberRef,ValueDriverDescriptor descriptor);
		const ComponentMemberReference &GetMemberReference() const {return m_memberReference;}
		const ValueDriverDescriptor &GetDescriptor() const {return m_descriptor;}
		pragma::ComponentId GetComponentId() const {return m_componentId;}

		bool Apply(BaseEntity &ent);
	private:
		ValueDriverDescriptor m_descriptor;
		std::unordered_map<std::string,ValueDriverVariable> m_variables;
		pragma::ComponentId m_componentId = std::numeric_limits<pragma::ComponentId>::max();
		ComponentMemberReference m_memberReference;
	};
};

#endif
