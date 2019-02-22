#ifndef __C_ENV_MICROPHONE_H__
#define __C_ENV_MICROPHONE_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/env_microphone_base.h"

namespace pragma
{
	class DLLCLIENT CMicrophoneComponent final
		: public BaseEnvMicrophoneComponent
	{
	public:
		CMicrophoneComponent(BaseEntity &ent) : BaseEnvMicrophoneComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CEnvMicrophone
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif