/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan
 */

#ifndef __PRAGMA_MATERIAL_OVERRIDE_HPP__
#define __PRAGMA_MATERIAL_OVERRIDE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/rendering/shader_input_data.hpp"
#include "pragma/rendering/shader_material/shader_material.hpp"
#include <pragma/entities/components/base_time_scale_component.hpp>
#include <material_manager2.hpp>
#include <queue>

import pragma.client.rendering.material_property_block;

namespace pragma {
	class DLLCLIENT CMaterialOverrideComponent final : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_MATERIAL_OVERRIDES_CLEARED;
		static ComponentEventId EVENT_ON_MATERIAL_OVERRIDE_CHANGED;

		static void RegisterLuaBindings(lua_State *l, luabind::module_ &modEnts);
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		CMaterialOverrideComponent(BaseEntity &ent);
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
		void UpdateMaterialOverride(uint32_t matIdx, CMaterial &mat);
		std::unordered_map<size_t, MaterialPropertyOverride> m_materialPropertyOverrides;
		std::unique_ptr<MaterialPropertyOverride> m_globalMaterialPropertyOverride;

		std::vector<MaterialOverride> m_materialOverrides = {};
	};

	struct DLLCLIENT CEOnMaterialOverrideChanged : public ComponentEvent {
		CEOnMaterialOverrideChanged(uint32_t idx, CMaterial &mat);
		virtual void PushArguments(lua_State *l) override;
		uint32_t materialIndex;
		CMaterial &material;
	};
};

#endif
