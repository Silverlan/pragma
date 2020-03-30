#ifndef __C_SKYBOX_H__
#define __C_SKYBOX_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/baseskybox.h>

namespace pragma
{
	class DLLCLIENT CSkyboxComponent final
		: public BaseSkyboxComponent,
		public CBaseNetComponent
	{
	public:
		CSkyboxComponent(BaseEntity &ent) : BaseSkyboxComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;

		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	private:
		bool CreateCubemapFromIndividualTextures(const std::string &materialPath,const std::string &postfix="") const;
		void ValidateMaterials();
		CallbackHandle m_cbOnModelMaterialsLoaded = {};
	};
};

class DLLCLIENT CSkybox
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif