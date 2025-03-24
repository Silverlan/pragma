/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan
 */

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_time_scale_component.hpp>
#include <material_manager2.hpp>

export module pragma.client.entities.components.material_override;

import pragma.client.rendering.material_property_block;

export namespace pragma {
	class DLLCLIENT CMaterialOverrideComponent final : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_MATERIAL_OVERRIDES_CLEARED;

		static void RegisterLuaBindings(lua_State *l, luabind::module_ &modEnts);
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		CMaterialOverrideComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual ~CMaterialOverrideComponent() override;
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;

		void SetMaterialOverride(uint32_t idx, const std::string &matOverride);
		void SetMaterialOverride(uint32_t idx, CMaterial &mat);
		void ClearMaterialOverride(uint32_t idx);
		void ClearMaterialOverrides();
		CMaterial *GetMaterialOverride(uint32_t idx) const;
		CMaterial *GetRenderMaterial(uint32_t idx) const;
		size_t GetMaterialOverrideCount() const;

		void ClearMaterialPropertyOverrides();
		void ClearMaterialPropertyOverride(size_t materialIndex);
		void ClearMaterialPropertyOverride();

		void SetMaterialPropertyOverride(size_t materialIndex, const pragma::rendering::MaterialPropertyBlock &block);
		const pragma::rendering::MaterialPropertyBlock *GetMaterialPropertyOverride(size_t materialIndex) const;

		void SetMaterialPropertyOverride(const pragma::rendering::MaterialPropertyBlock &block);
		const pragma::rendering::MaterialPropertyBlock *GetMaterialPropertyOverride() const;
	  protected:
		struct MaterialPropertyOverride {
			pragma::rendering::MaterialPropertyBlock block;
		};
		struct MaterialOverride {
			msys::MaterialHandle materialOverride; // Optionally defined by user
			// Acts as an additional "layer" and allows the user to overwrite specific material properties
			// on top of the main material override
			std::shared_ptr<Material> propertyOverride;
		};
		void UpdateMaterialPropertyOverride(size_t matIdx);
		void ApplyMaterialPropertyOverride(Material &mat, const pragma::rendering::MaterialPropertyBlock &matPropOverride);
		void UpdateMaterialOverride(Material &mat);
		std::unordered_map<size_t, MaterialPropertyOverride> m_materialPropertyOverrides;
		std::unique_ptr<MaterialPropertyOverride> m_globalMaterialPropertyOverride;

		std::vector<MaterialOverride> m_materialOverrides = {};
	};
};
