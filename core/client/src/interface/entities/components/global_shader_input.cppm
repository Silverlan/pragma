// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.global_shader_input;

export import :rendering.global_shader_input_manager;

export namespace pragma {
	struct DLLCLIENT UdmPropertyList {
	  public:
		UdmPropertyList();
		const udm::PProperty &GetUdmData() const;
		udm::LinkedPropertyWrapper GetPropertyData() const { return (*m_properties)["properties"]; }
		void AddProperty(const ComponentMemberInfo &memberInfo);
		void UpdateNameCache();
		bool HasProperty(const std::string &name) const;
		std::optional<size_t> GetPropertyIndex(const std::string &name) const;
		const std::string *GetPropertyName(size_t index) const;
	  private:
		udm::PProperty m_properties;
		std::unordered_map<std::string, size_t> m_nameToIndex;
		std::vector<std::string> m_indexToName;
	};

	class DLLCLIENT CGlobalShaderInputComponent final : public BaseEntityComponent, public DynamicMemberRegister {
	  public:
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		static void DebugPrintProperties();

		CGlobalShaderInputComponent(ecs::BaseEntity &ent);

		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
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
