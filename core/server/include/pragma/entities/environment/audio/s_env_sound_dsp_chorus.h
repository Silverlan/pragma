#ifndef __S_ENV_SOUND_DSP_CHORUS_H__
#define __S_ENV_SOUND_DSP_CHORUS_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp.h"
#include "pragma/entities/environment/audio/env_sound_dsp_chorus.h"

namespace pragma
{
	class DLLSERVER SSoundDspChorusComponent final
		: public SBaseSoundDspComponent,
		public BaseEnvSoundDspChorus
	{
	public:
		SSoundDspChorusComponent(BaseEntity &ent) : SBaseSoundDspComponent(ent) {}
		virtual bool OnSetKeyValue(const std::string &key,const std::string &val) override;
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvSoundDspChorus
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
