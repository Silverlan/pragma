#include "stdafx_shared.h"
#include "pragma/localization.h"
#include <fsys/filesystem.h>
#include <sharedutils/util_string.h>

decltype(Locale::m_localization) Locale::m_localization;
decltype(Locale::m_language) Locale::m_language;
decltype(Locale::m_loadedFiles) Locale::m_loadedFiles;

Localization::Localization()
{}

//////////////////////////

bool Locale::Load(const std::string &file,const std::string &lan,bool bReload)
{
	std::string fPath = "scripts/localization/";
	fPath += lan;
	fPath += "/";
	fPath += file;
	fPath = FileManager::GetCanonicalizedPath(fPath);
	auto it = std::find(m_loadedFiles.begin(),m_loadedFiles.end(),fPath);
	if(it != m_loadedFiles.end())
		return true;
	auto f = FileManager::OpenFile(fPath.c_str(),"r");
	if(f != nullptr)
	{
		while(!f->Eof())
		{
			auto l = f->ReadLine();
			std::string key;
			std::string val;
			if(ustring::get_key_value(l,key,val))
				m_localization.texts[key] = val;
		}
		m_loadedFiles.push_back(fPath);
		return true;
	}
	return false;
}

bool Locale::Load(const std::string &file,bool bReload)
{
	auto r = Load(file,"en",bReload);
	if(m_language == "en")
		return r;
	r = Load(file,m_language,bReload);
	return r;
}

void Locale::Initialize(std::string lan)
{
	ustring::to_lower(lan);
	m_language = lan;
}
const std::string &Locale::GetLanguage() {return m_language;}
std::string Locale::GetText(const std::string &id)
{
	std::string r;
	GetText(id,r);
	return r;
}
std::string Locale::GetText(const std::string &id,const std::vector<std::string> &args)
{
	auto out = GetText(id);
	for(auto i=decltype(args.size()){0};i<args.size();++i)
	{
		std::string sarg = "{";
		sarg += std::to_string(i);
		sarg += "}";
		auto pos = out.find(sarg.c_str());
		if(pos != std::string::npos)
			out = out.replace(pos,3,args[i]);
	}
	return out;
}
bool Locale::GetText(const std::string &id,std::string &r)
{
	auto it = m_localization.texts.find(id);
	if(it == m_localization.texts.end())
	{
		r = std::string("<MISSING LOCALIZATION: ") +id +std::string(">");
		return false;
	}
	r = it->second;
	return true;
}
