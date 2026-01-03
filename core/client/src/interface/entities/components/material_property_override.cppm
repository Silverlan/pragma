// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.material_property_override;

import :rendering.material_property_block;
import :rendering.shader_graph.node_shader_material;

export namespace pragma {
	class DLLCLIENT CMaterialPropertyOverrideComponent final : public BaseEntityComponent, public DynamicMemberRegister {
	  public:
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		CMaterialPropertyOverrideComponent(ecs::BaseEntity &ent);
		virtual ~CMaterialPropertyOverrideComponent() override;
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;
		material::CMaterial *GetRenderMaterial(uint32_t matIdx) const;

		template<typename T, bool TEXTURE = false>
		void ApplyMaterialProperty(uint32_t matIdx, const char *key, const T &value);
		void ApplyMaterialProperty(uint32_t matIdx, const char *key);
		template<typename T, bool TEXTURE = false>
		void SetMaterialProperty(uint32_t matIdx, const char *key, const T &value);
		template<typename T>
		bool GetMaterialProperty(uint32_t matIdx, const char *key, T &outValue) const;

		void SetTextureProperty(uint32_t matIdx, const char *key, const std::string &tex);
		std::string GetTextureProperty(uint32_t matIdx, const char *key) const;

		virtual const ComponentMemberInfo *GetMemberInfo(ComponentMemberIndex idx) const override;
	  protected:
		static std::string GetNormalizedMaterialName(std::string name);
		static std::string GetNormalizedMaterialName(const material::CMaterial &mat);
		static const rendering::shader_material::ShaderMaterial *GetShaderMaterial(const material::CMaterial &mat);
		static std::string GetPropertyName(const material::CMaterial &mat, const char *key, bool texture);
		static std::string NormalizeTexturePath(const std::string &path);

		void AddMaterialPropertyMember(const rendering::Property &prop, const std::string &propPath, const std::string &name, std::optional<uint32_t> matIdx = {});
		void AddMaterialTexturePropertyMember(const std::string &propPath, const rendering::shader_material::Texture &tex, std::optional<uint32_t> matIdx = {});

		struct PropertyInfo {
			udm::PProperty property;
			bool enabled = false;
			bool texture = false;
		};
		struct MaterialData {
			std::shared_ptr<material::Material> material;
			std::unordered_map<std::string, PropertyInfo> properties;

			std::string shaderOverride;
			bool shaderOverrideEnabled = false;
		};
		void ReloadMaterialOverride(uint32_t idx);
		void UpdateMaterialOverride(uint32_t idx, const material::CMaterial &mat, bool forceInitialize = false);
		void UpdateRenderBuffers(prosper::IPrimaryCommandBuffer &drawCmd);

		PropertyInfo *InitializeMaterialProperty(uint32_t matIdx, const char *key);
		void ClearMaterialProperty(uint32_t matIdx, const char *key);

		PropertyInfo *FindMaterialPropertyInfo(uint32_t matIdx, const char *key);
		const PropertyInfo *FindMaterialPropertyInfo(uint32_t matIdx, const char *key) const { return const_cast<CMaterialPropertyOverrideComponent *>(this)->FindMaterialPropertyInfo(matIdx, key); }
		udm::Property *FindMaterialProperty(uint32_t matIdx, const char *key);
		material::Material *GetTargetMaterial(uint32_t matIdx);

		static void SetPropertyEnabled(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, bool enabled);
		static void GetPropertyEnabled(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, bool &outEnabled);
		static void SetShader(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, std::string shader);
		static void GetShader(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, std::string &outShader);
		static void SetShaderEnabled(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, bool enabled);
		static void GetShaderEnabled(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, bool &outEnabled);
		void UpdateShaderOverride(uint32_t matIdx);
		virtual std::optional<ComponentMemberIndex> DoGetMemberIndex(const std::string &name) const override;
		template<typename T>
		void SetMaterialPropertyBufferValue(material::Material &mat, const rendering::shader_material::ShaderMaterial &shaderMat, const char *keyName, const T &newVal);
		void PopulateProperties();
		void PopulateProperties(std::string matName, material::CMaterial &mat, uint32_t matIdx);
		void ClearProperties(uint32_t matIdx);
		struct BufferUpdateInfo {
			std::shared_ptr<prosper::IBuffer> buffer;
			size_t startOffset;
			size_t size;
			std::unique_ptr<std::byte[]> data;
		};
		void ApplyMaterialPropertyOverride(material::Material &mat, const rendering::MaterialPropertyBlock &matPropOverride);
		void UpdateMaterialOverride(material::Material &mat);

		struct ShaderMaterialPropertyInfo {
			ShaderMaterialPropertyInfo();
			ShaderMaterialPropertyInfo(const ShaderMaterialPropertyInfo &other);
			void Invalidate()
			{
				name = nullptr;
				materialIndex = std::numeric_limits<uint32_t>::max();
			}
			bool IsValid() const { return name != nullptr; }
			const char *name;
			uint32_t materialIndex = std::numeric_limits<uint32_t>::max();
		};

		std::vector<MaterialData> m_materialOverrides;
		std::unordered_map<std::string, PropertyInfo> m_basePropertyOverrides;
		std::queue<BufferUpdateInfo> m_bufferUpdateQueue;

		std::vector<ShaderMaterialPropertyInfo> m_shaderMaterialPropertyInfos;
		std::queue<uint32_t> m_freePropertyInfoIndices;
	};
};
