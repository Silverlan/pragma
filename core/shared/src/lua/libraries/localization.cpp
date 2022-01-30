/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/localization.h"
#include <fsys/directory_watcher.h>
#include <fsys/filesystem.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util_path.hpp>

#undef CreateFile

decltype(Locale::m_localization) Locale::m_localization;
decltype(Locale::m_language) Locale::m_language;
decltype(Locale::m_loadedFiles) Locale::m_loadedFiles;

Localization::Localization()
{}

//////////////////////////

static constexpr auto LOCALIZATION_ROOT_PATH = "scripts/localization/";

static std::unique_ptr<DirectoryWatcherCallback> g_locFileWatcher = nullptr;
void Locale::Init() {}
void Locale::Clear()
{
	g_locFileWatcher = nullptr;
	m_loadedFiles.clear();
	m_localization.texts.clear();
	m_language.clear();
}
bool Locale::Load(const std::string &file,const std::string &lan,bool bReload)
{
	auto filePath = util::Path::CreateFile(file);
	auto it = std::find(m_loadedFiles.begin(),m_loadedFiles.end(),filePath.GetString());
	if(it != m_loadedFiles.end())
	{
		if(bReload == false)
			return true;
		m_loadedFiles.erase(it);
	}
	return LoadFile(filePath.GetString(),lan);
}

bool Locale::LoadFile(const std::string &file,const std::string &lan)
{
	auto filePath = LOCALIZATION_ROOT_PATH +lan +'/' +file;
	auto f = FileManager::OpenFile(filePath.c_str(),"r");
	if(f != nullptr)
	{
		while(!f->Eof())
		{
			auto l = f->ReadLine();
			std::string key;
			std::string val;
			if(ustring::get_key_value(l,key,val))
			{
				ustring::replace(val,"\\\"","\"");
				m_localization.texts[key] = val;
			}
		}
		m_loadedFiles.push_back(file);
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

void Locale::Poll()
{
	if(g_locFileWatcher)
		g_locFileWatcher->Poll();
}

void Locale::ReloadFiles()
{
	auto files = m_loadedFiles;
	for(auto &f : files)
		Load(f,true);
}

void Locale::SetLanguage(std::string lan)
{
	ustring::to_lower(lan);
	m_language = lan;

	auto loadedFiles = m_loadedFiles;
	m_loadedFiles.clear();
	for(auto &fpath : loadedFiles)
		LoadFile(fpath,lan);

	try
	{
		g_locFileWatcher = std::make_unique<DirectoryWatcherCallback>(LOCALIZATION_ROOT_PATH +lan +'/',[](const std::string &str) {
			auto filePath = util::Path::CreateFile(str);
			auto it = std::find(m_loadedFiles.begin(),m_loadedFiles.end(),filePath.GetString());
			if(it == m_loadedFiles.end())
				return;
			Con::cout<<"Reloading localization file '"<<str<<"'..."<<Con::endl;
			Load(str,true);
		});
	}
	catch(const std::runtime_error &err)
	{}
}
const std::string &Locale::GetLanguage() {return m_language;}
std::unordered_map<std::string,std::string> Locale::GetLanguages()
{
	auto f = FileManager::OpenFile("scripts/localization/languages.txt","r");
	std::unordered_map<std::string,std::string> lanOptions;
	if(f != nullptr)
	{
		while(!f->Eof())
		{
			std::string l = f->ReadLine();
			std::string key;
			std::string val;
			if(ustring::get_key_value(l,key,val))
				lanOptions.insert(std::unordered_map<std::string,std::string>::value_type(key,val));
		}
	}
	return lanOptions;
}
bool Locale::SetLocalization(const std::string &id,const std::string &text,bool overwriteIfExists)
{
	if(!overwriteIfExists && m_localization.texts.find(id) != m_localization.texts.end())
		return false;
	m_localization.texts[id] = text;
	return true;
}
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
	{
		Con::cwar<<"WARNING: Missing localization for '"<<id<<"'!"<<Con::endl;
		return std::string("<MISSING LOCALIZATION: ") +id +std::string(">");
	}
	auto r = it->second;
	insert_arguments(args,r);
	return r;
}
