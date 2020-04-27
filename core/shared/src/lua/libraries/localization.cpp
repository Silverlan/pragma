/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

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
bool Locale::GetText(const std::string &id,std::string &outText) {return GetText(id,{},outText);}
static void insert_arguments(const std::vector<std::string> &args,std::string &inOutText)
{
	for(auto i=decltype(args.size()){0};i<args.size();++i)
	{
		std::string sarg = "{";
		sarg += std::to_string(i);
		sarg += "}";
		auto pos = inOutText.find(sarg.c_str());
		if(pos != std::string::npos)
			inOutText = inOutText.replace(pos,3,args[i]);
	}
}
bool Locale::GetText(const std::string &id,const std::vector<std::string> &args,std::string &outText)
{
	auto it = m_localization.texts.find(id);
	if(it == m_localization.texts.end())
		return false;
	outText = it->second;
	insert_arguments(args,outText);
	return true;
}
std::string Locale::GetText(const std::string &id,const std::vector<std::string> &args)
{
	auto it = m_localization.texts.find(id);
	if(it == m_localization.texts.end())
		return std::string("<MISSING LOCALIZATION: ") +id +std::string(">");
	auto r = it->second;
	insert_arguments(args,r);
	return r;
}
