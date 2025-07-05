// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SOUNDSCRIPT_H__
#define __C_SOUNDSCRIPT_H__

#include "pragma/clientdefinitions.h"
#include <pragma/audio/soundscript.h>

#undef CreateEvent

class CSoundScriptManager;

namespace al {
	class IEffect;
};
class DLLCLIENT CSSEPlaySound : public SSEPlaySound {
  protected:
	std::shared_ptr<al::IEffect> m_dspEffect = nullptr;
	virtual void PrecacheSound(const char *name) override;
  public:
	CSSEPlaySound(SoundScriptManager *manager);
	std::vector<std::shared_ptr<al::IEffect>> effects;
	virtual void Initialize(udm::LinkedPropertyWrapper &prop) override;
	virtual SSESound *CreateSound(double tStart, const std::function<std::shared_ptr<ALSound>(const std::string &, ALChannel, ALCreateFlags)> &createSound) override;
};

class DLLCLIENT CSoundScript : public SoundScript {
  public:
	friend SoundScriptManager;
	friend CSoundScriptManager;
	CSoundScript(SoundScriptManager *manager, std::string identifier);
	virtual ~CSoundScript() override;
};

class DLLCLIENT CSoundScriptManager : public SoundScriptManager {
  public:
	CSoundScriptManager();
	virtual ~CSoundScriptManager() override;
	virtual bool Load(const char *fname, std::vector<std::shared_ptr<SoundScript>> *scripts = NULL) override;
	virtual SoundScriptEvent *CreateEvent(std::string name) override;
};

#endif
