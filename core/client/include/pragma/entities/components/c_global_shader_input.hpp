/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __C_GLOBAL_SHADER_INPUT_HPP__
#define __C_GLOBAL_SHADER_INPUT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/global_shader_input_manager.hpp"
#include "pragma/entities/components/base_entity_component_member_register.hpp"
#include <pragma/entities/components/base_entity_component.hpp>

namespace pragma {
	struct DLLCLIENT UdmPropertyList {
	  public:
		UdmPropertyList();
		const udm::PProperty &GetUdmData() const;
		udm::LinkedPropertyWrapper GetPropertyData() const { return (*m_properties)["properties"]; }
		void AddProperty(const pragma::ComponentMemberInfo &memberInfo);
		void UpdateNameCache();
		bool HasProperty(const std::string &name) const;
		std::optional<size_t> GetPropertyIndex(const std::string &name) const;
		const std::string *GetPropertyName(size_t index) const;
	  private:
		udm::PProperty m_properties;
		std::unordered_map<std::string, size_t> m_nameToIndex;
		std::vector<std::string> m_indexToName;
	};

	namespace rendering {
		class GlobalShaderInputDataManager;
	};
	class DLLCLIENT CGlobalShaderInputComponent final : public BaseEntityComponent, public DynamicMemberRegister {
	  public:
		static void RegisterLuaBindings(lua_State *l, luabind::module_ &modEnts);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		static void DebugPrintProperties();

		CGlobalShaderInputComponent(BaseEntity &ent);

		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;

		template<typename T>
		bool SetShaderInputValue(const std::string_view &name, const T &val)
		{
			auto &inputManager = GetInputManager();
			return inputManager.SetValue<T>(name, val);
		}

		template<typename T>
		bool GetShaderInputValue(const std::string_view &name, T &outVal) const
		{
			auto &inputManager = GetInputManager();
			return inputManager.GetValue<T>(name, outVal);
		}

		virtual const ComponentMemberInfo *GetMemberInfo(ComponentMemberIndex idx) const override;
	  private:
		virtual std::optional<ComponentMemberIndex> DoGetMemberIndex(const std::string &name) const override;
		rendering::GlobalShaderInputDataManager &GetInputManager();
		const rendering::GlobalShaderInputDataManager &GetInputManager() const { return const_cast<CGlobalShaderInputComponent *>(this)->GetInputManager(); }
		void UpdateMembers();
		void SyncShaderVarsToProperties();
		void UpdatePropertyCache();

		UdmPropertyList m_propertyList;
	};
};

#endif
