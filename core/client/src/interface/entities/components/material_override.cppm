// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.material_override;

export import :rendering.material_property_block;
export import pragma.cmaterialsystem;
export import pragma.shared;

export namespace pragma {
	namespace cMaterialOverrideComponent {
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_MATERIAL_OVERRIDES_CLEARED;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_MATERIAL_OVERRIDE_CHANGED;
	}
	class DLLCLIENT CMaterialOverrideComponent final : public BaseEntityComponent, public DynamicMemberRegister {
	  public:
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		CMaterialOverrideComponent(ecs::BaseEntity &ent);
		virtual ~CMaterialOverrideComponent() override;
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;

		void SetMaterialOverride(uint32_t idx, const std::string &matOverride);
		void SetMaterialOverride(uint32_t idx, material::CMaterial &mat);
		void ClearMaterialOverride(uint32_t idx);
		void ClearMaterialOverrides();
		material::CMaterial *GetMaterialOverride(uint32_t idx) const;
		material::CMaterial *GetRenderMaterial(uint32_t idx) const;
		size_t GetMaterialOverrideCount() const;

		virtual const ComponentMemberInfo *GetMemberInfo(ComponentMemberIndex idx) const override;
	  protected:
		struct MaterialOverride {
			material::MaterialHandle materialOverride;
		};
		virtual std::optional<ComponentMemberIndex> DoGetMemberIndex(const std::string &name) const override;
		void PopulateProperties();
		void UpdateMaterialOverride(uint32_t matIdx, material::CMaterial &mat);
		std::vector<MaterialOverride> m_materialOverrides = {};
	};

	struct DLLCLIENT CEOnMaterialOverrideChanged : public ComponentEvent {
		CEOnMaterialOverrideChanged(uint32_t idx, material::CMaterial &mat);
		virtual void PushArguments(lua::State *l) override;
		uint32_t materialIndex;
		material::CMaterial &material;
	};
};
