// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.pp_dof;

export import :entities.components.pp_base;

export namespace pragma {
	class CRasterizationRendererComponent;
	class DLLCLIENT CRendererPpDoFComponent final : public CRendererPpBaseComponent {
	  public:
		CRendererPpDoFComponent(ecs::BaseEntity &ent);
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual std::string GetIdentifier() const override { return "dof"; }
		virtual uint32_t GetPostProcessingWeight() const override { return math::to_integral(CRendererComponent::StandardPostProcessingWeight::DoF); }
	  private:
		virtual void DoRenderEffect(const rendering::DrawSceneInfo &drawSceneInfo) override;
	};
};
