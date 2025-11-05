// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.client:entities.components.pp_dof;

export import :entities.components.pp_base;

export namespace pragma {
	class CRasterizationRendererComponent;
	class DLLCLIENT CRendererPpDoFComponent final : public CRendererPpBaseComponent {
	  public:
		CRendererPpDoFComponent(pragma::ecs::BaseEntity &ent);
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual std::string GetIdentifier() const override { return "dof"; }
		virtual uint32_t GetPostProcessingWeight() const override { return umath::to_integral(CRendererComponent::StandardPostProcessingWeight::DoF); }
	  private:
		virtual void DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) override;
	};
};
