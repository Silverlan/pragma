/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SKYBOX_H__
#define __C_SKYBOX_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/baseskybox.h>

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

#endif
