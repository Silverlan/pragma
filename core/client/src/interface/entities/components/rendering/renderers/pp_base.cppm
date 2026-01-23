// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.pp_base;

export import :entities.components.rasterization_renderer;
import :entities.components.renderer;

export namespace pragma {
	class DLLCLIENT CRendererPpBaseComponent : public BaseEntityComponent {
	  public:
		CRendererPpBaseComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnRemove() override;
		virtual std::string GetIdentifier() const = 0;
		virtual uint32_t GetPostProcessingWeight() const = 0;
		virtual PostProcessingEffectData::Flags GetFlags() const;
	  protected:
		void RenderEffect(const rendering::DrawSceneInfo &drawSceneInfo);
		virtual void DoRenderEffect(const rendering::DrawSceneInfo &drawSceneInfo) = 0;
		ComponentHandle<CRasterizationRendererComponent> m_renderer {};
		CallbackHandle m_cbEffect;
	};
};
