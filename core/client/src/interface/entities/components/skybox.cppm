// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"




export module pragma.client:entities.components.skybox;

import :entities.base_entity;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CSkyboxComponent final : public BaseSkyboxComponent, public CBaseNetComponent {
		public:
			CSkyboxComponent(BaseEntity &ent) : BaseSkyboxComponent(ent) {}
			virtual void Initialize() override;
			virtual void OnRemove() override;
			virtual void InitializeLuaObject(lua_State *l) override;

			virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
			virtual void ReceiveData(NetPacket &packet) override;
			virtual bool ShouldTransmitNetData() const override { return true; }

			virtual void SetSkyAngles(const EulerAngles &ang) override;
			const EulerAngles &GetSkyAngles() const;
			const Vector4 &GetRenderSkyAngles() const;

			void SetSkyMaterial(Material *mat);
			void ValidateMaterials();
		private:
			bool CreateCubemapFromIndividualTextures(const std::string &materialPath, const std::string &postfix = "") const;
			CallbackHandle m_cbOnModelMaterialsLoaded = {};
			EulerAngles m_skyAngles;
			Vector4 m_renderSkyAngles;
		};
	};

	class DLLCLIENT CSkybox : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
