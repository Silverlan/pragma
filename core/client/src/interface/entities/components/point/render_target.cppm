// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <texturemanager/texture.h>
#include "pragma/lua/core.hpp"




export module pragma.client:entities.components.point_render_target;

import :entities.base_entity;
import :entities.components.camera;
import :entities.components.entity;
import :rendering.enums;

export {
	namespace pragma {
		class DLLCLIENT CRenderTargetComponent final : public BasePointRenderTargetComponent, public CBaseNetComponent {
		public:
			CRenderTargetComponent(pragma::ecs::BaseEntity &ent) : BasePointRenderTargetComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua_State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		protected:
			void SetRenderSize(Vector2 &size);
			void SetRenderSize(float w, float h);
			Vector2 GetRenderSize();
			void GetRenderSize(float *w, float *h);

			void SetRenderMaterial(Material *mat);
			Material *GetRenderMaterial();
			void SetRenderMaterial(std::string mat);

			void SetRefreshRate(float f);
			float GetRefreshRate();

			void SetRenderFOV(float fov);
			float GetRenderFOV();

			void SetRenderDepth(unsigned int depth);
			unsigned int GetRenderDepth();

			void Spawn();
			unsigned int GetTextureBuffer();
			void Render(pragma::rendering::SceneRenderPass renderMode);

			std::unique_ptr<Texture> m_texture = nullptr;
			Material *m_matRender = nullptr;
			float m_refreshRate = 0.f;
			double m_tLastRefresh;
			unsigned int m_curDepth = 0u;
			util::WeakHandle<pragma::CCameraComponent> m_cam = {};
		};
	};

	class DLLCLIENT CPointRenderTarget : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
