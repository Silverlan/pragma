/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan
 */

#ifndef __PRAGMA_MATERIAL_PROPERTY_OVERRIDE_HPP__
#define __PRAGMA_MATERIAL_PROPERTY_OVERRIDE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/entities/components/base_entity_component_member_register.hpp"
#include "pragma/rendering/shader_input_data.hpp"
#include "pragma/rendering/shader_material/shader_material.hpp"
#include <pragma/entities/components/base_time_scale_component.hpp>
#include <sharedutils/util_heterogenous_lookup.hpp>
#include <material_manager2.hpp>
#include <udm.hpp>
#include <queue>

import pragma.client.rendering.material_property_block;

namespace prosper {
	class IPrimaryCommandBuffer;
	class IBuffer;
};

namespace pragma {
	class DLLCLIENT CMaterialPropertyOverrideComponent final : public BaseEntityComponent, public DynamicMemberRegister {
	  public:
		static void RegisterLuaBindings(lua_State *l, luabind::module_ &modEnts);
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		CMaterialPropertyOverrideComponent(BaseEntity &ent);
		virtual ~CMaterialPropertyOverrideComponent() override;
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;
		CMaterial *GetRenderMaterial(uint32_t matIdx) const;

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
		static std::string GetNormalizedMaterialName(const CMaterial &mat);
		static const rendering::shader_material::ShaderMaterial *GetShaderMaterial(const CMaterial &mat);
		static std::string GetPropertyName(const CMaterial &mat, const char *key, bool texture);
		static std::string NormalizeTexturePath(const std::string &path);
		struct PropertyInfo {
			udm::PProperty property;
			bool enabled = false;
			bool texture = false;
		};
		struct MaterialData {
			std::shared_ptr<Material> material;
			std::unordered_map<std::string, PropertyInfo> properties;
		};
		void UpdateMaterialOverride(uint32_t idx, const CMaterial &mat, bool forceInitialize = false);
		void UpdateRenderBuffers(prosper::IPrimaryCommandBuffer &drawCmd);

		PropertyInfo *InitializeMaterialProperty(uint32_t matIdx, const char *key);
		void ClearMaterialProperty(uint32_t matIdx, const char *key);

		PropertyInfo *FindMaterialPropertyInfo(uint32_t matIdx, const char *key);
		const PropertyInfo *FindMaterialPropertyInfo(uint32_t matIdx, const char *key) const { return const_cast<CMaterialPropertyOverrideComponent *>(this)->FindMaterialPropertyInfo(matIdx, key); }
		udm::Property *FindMaterialProperty(uint32_t matIdx, const char *key);
		Material *GetTargetMaterial(uint32_t matIdx);
		std::vector<MaterialData> m_materialOverrides;
		MaterialData m_baseOverride; // TODO

		static void SetPropertyEnabled(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, bool enabled);
		static void GetPropertyEnabled(const ComponentMemberInfo &info, CMaterialPropertyOverrideComponent &component, bool &outEnabled);
		virtual std::optional<ComponentMemberIndex> DoGetMemberIndex(const std::string &name) const override;
		template<typename T>
		void SetMaterialPropertyBufferValue(Material &mat, const pragma::rendering::shader_material::ShaderMaterial &shaderMat, const char *keyName, const T &newVal);
		void PopulateProperties();
		struct BufferUpdateInfo {
			std::shared_ptr<prosper::IBuffer> buffer;
			size_t startOffset;
			size_t size;
			std::unique_ptr<std::byte[]> data;
		};
		struct MaterialPropertyOverride {
			pragma::rendering::MaterialPropertyBlock block;
		};
		struct MaterialOverride {
			msys::MaterialHandle materialOverride; // Optionally defined by user
			// Acts as an additional "layer" and allows the user to overwrite specific material properties
			// on top of the main material override
			std::shared_ptr<Material> propertyOverride;
		};
		void ApplyMaterialPropertyOverride(Material &mat, const pragma::rendering::MaterialPropertyBlock &matPropOverride);
		void UpdateMaterialOverride(Material &mat);

		std::queue<BufferUpdateInfo> m_bufferUpdateQueue;

		struct ShaderMaterialPropertyInfo {
			ShaderMaterialPropertyInfo();
			ShaderMaterialPropertyInfo(const ShaderMaterialPropertyInfo &other);
			const char *name;
			uint32_t materialIndex = std::numeric_limits<uint32_t>::max();
			std::unique_ptr<MaterialPropertyOverride> propertyOverride;
		};
		std::vector<ShaderMaterialPropertyInfo> m_shaderMaterialPropertyInfos;
	};
};

#endif
