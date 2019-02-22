#ifndef __C_SOUNDSCRIPT_H__
#define __C_SOUNDSCRIPT_H__

#include "pragma/clientdefinitions.h"
#include <pragma/audio/soundscript.h>

#undef CreateEvent

class CSoundScriptManager;

namespace al {class Effect;};
class DLLCLIENT CSSEPlaySound
	: public SSEPlaySound
{
protected:
	std::shared_ptr<al::Effect> m_dspEffect = nullptr;
	void PrecacheSound(const char *name);
public:
	CSSEPlaySound(SoundScriptManager *manager);
	std::vector<std::shared_ptr<al::Effect>> effects;
	virtual void Initialize(const std::shared_ptr<ds::Block> &data) override;
	virtual SSESound *CreateSound(double tStart,const std::function<std::shared_ptr<ALSound>(const std::string&,ALChannel,ALCreateFlags)> &createSound) override;
};

class DLLCLIENT CSoundScript
	: public SoundScript
{
public:
	friend SoundScriptManager;
	friend CSoundScriptManager;
protected:
	CSoundScript(SoundScriptManager *manager,std::string identifier);
	virtual ~CSoundScript() override;
};

class DLLCLIENT CSoundScriptManager
	: public SoundScriptManager
{
public:
	CSoundScriptManager();
	virtual ~CSoundScriptManager() override;
	bool Load(const char *fname,std::vector<std::shared_ptr<SoundScript>> *scripts=NULL);
	SoundScriptEvent *CreateEvent(std::string name);
};

#endif