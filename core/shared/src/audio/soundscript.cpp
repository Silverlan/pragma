#include "stdafx_shared.h"
#include <fsys/filesystem.h>
#include "datasystem.h"
#include "pragma/audio/soundscript.h"
#include "pragma/audio/soundscript_events.h"
#include "datasystem.h"
#include <mathutil/umath.h>
#include <sharedutils/util_string.h>

#undef CreateEvent

SoundScript::SoundScript(SoundScriptManager *manager,const std::string &identifier)
	: SoundScriptEventContainer(manager),m_identifier(identifier)
{}

SoundScript::~SoundScript()
{}

const std::string &SoundScript::GetIdentifier() const {return m_identifier;}

//////////////////////////////

SoundScriptManager::SoundScriptManager()
{}

SoundScriptManager::~SoundScriptManager()
{
	Clear();
}

const std::string &SoundScriptManager::GetSoundScriptPath()
{
	static std::string r = "scripts\\sounds\\";
	return r;
}

const std::unordered_map<std::string,std::shared_ptr<SoundScript>> &SoundScriptManager::GetScripts() const {return m_soundScripts;}
const std::vector<std::string> &SoundScriptManager::GetSoundScriptFiles() const {return m_soundScriptFiles;}

void SoundScriptManager::Clear()
{
	m_soundScripts.clear();
}

bool SoundScriptManager::Load(const char *fname,std::vector<std::shared_ptr<SoundScript>> *scripts)
{
	return Load<SoundScript>(fname,scripts);
}

SoundScript *SoundScriptManager::FindScript(const char *name)
{
	std::string sname = name;
	StringToLower(sname);
	auto it = m_soundScripts.find(sname);
	if(it != m_soundScripts.end())
		return it->second.get();
	return NULL;
}

SoundScriptEvent *SoundScriptManager::CreateEvent(std::string name)
{
	if(name == "playsound")
		return new SSEPlaySound(this);
	else if(name == "lua")
		return new SSELua(this);
	return new SoundScriptEvent(this);
}
SoundScriptEvent *SoundScriptManager::CreateEvent() {return CreateEvent("");}