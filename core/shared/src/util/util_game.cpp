/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/util/util_game.hpp"
#include "pragma/networkstate/networkstate.h"
#include "pragma/util/lookup_identifier.hpp"
#include <sharedutils/util_library.hpp>
#include <sharedutils/util_path.hpp>
#include <udm.hpp>
#pragma optimize("",off)
util::ParallelJob<std::vector<Vector2>&> util::generate_lightmap_uvs(NetworkState &nwState,uint32_t atlastWidth,uint32_t atlasHeight,const std::vector<Vertex> &verts,const std::vector<uint32_t> &tris)
{
	auto lib = nwState.InitializeLibrary("pr_uvatlas");
	if(lib == nullptr)
		return {};
	auto *fGenerateAtlasUvs = lib->FindSymbolAddress<void(*)(uint32_t,uint32_t,const std::vector<Vertex>&,const std::vector<uint32_t>&,util::ParallelJob<std::vector<Vector2>&>&)>("pr_uvatlas_generate_atlas_uvs");
	if(fGenerateAtlasUvs == nullptr)
		return {};
	util::ParallelJob<std::vector<Vector2>&> job {};
	fGenerateAtlasUvs(atlastWidth,atlasHeight,verts,tris,job);
	if(job.IsValid() == false)
		return {};
	return job;
}

static bool print_code_snippet(VFilePtr &f,uint32_t lineIdx,uint32_t charIdx)
{
	uint32_t numLinesPrint = 3;
	if(lineIdx < 2)
		numLinesPrint = lineIdx +1;
	auto startLineIdx = lineIdx +1 -numLinesPrint;

	uint32_t curLine = 0;
	for(;;)
	{
		if(curLine == startLineIdx)
			break;
		auto c = f->ReadChar();
		if(c == '\n')
		{
			++curLine;
			continue;
		}
		if(c == EOF)
			return false;
	}

	std::vector<std::string> lines;
	lines.resize(numLinesPrint);
	for(auto &l : lines)
		l = f->ReadLine();

	uint32_t lineOffset = 0;
	for(auto it=lines.begin();it!=lines.end();++it)
	{
		auto &line = *it;
		size_t pos = 0;
		auto len = line.length();
		while(pos < line.length())
		{
			auto c = line[pos];
			if(c == '\t')
				line = line.substr(0,pos) +"    " +line.substr(pos +1);
			++pos;
		}
		if(it == lines.end() -1)
			Con::cout<<"  > ";
		else
			Con::cout<<"    ";
		Con::cout<<line<<'\n';

		if(it == lines.end() -1)
			lineOffset = line.length() -len;
	}
	Con::cout<<std::string(static_cast<size_t>(charIdx) +4 +lineOffset,' ')<<'^'<<Con::endl;

	Con::cout<<Con::endl;
	return true;
}
template<typename T>
	static std::shared_ptr<udm::Data> load_udm_asset(T &f,std::string *optOutErr)
	{
		try
		{
			return udm::Data::Load(f);
		}
		catch(const udm::AsciiException &e)
		{
			if(optOutErr)
				*optOutErr = e.what();
			Con::cout<<"[UDM] Failed to load UDM asset";
			if constexpr(std::is_same_v<std::remove_const_t<std::remove_reference_t<T>>,std::string>)
				Con::cout<<" '"<<f<<"'";
			else
			{
				auto *ptr = static_cast<VFilePtrInternalReal*>(f.get());
				if(ptr)
				{
					auto path = util::Path::CreateFile(ptr->GetPath());
					path.MakeRelative(util::get_program_path());
					Con::cout<<" '"<<path.GetString()<<"'";
				}
			}
			Con::cout<<": "<<e.what()<<":\n";
			if constexpr(std::is_same_v<std::remove_const_t<std::remove_reference_t<T>>,std::string>)
			{
				auto fptr = FileManager::OpenFile(f.c_str(),"r");
				if(fptr)
					print_code_snippet(fptr,e.lineIndex,e.charIndex);
			}
			else
				print_code_snippet(f,e.lineIndex,e.charIndex);
		}
		catch(const udm::Exception &e)
		{
			if(optOutErr)
				*optOutErr = e.what();
			return nullptr;
		}
		return nullptr;
	}
std::shared_ptr<udm::Data> util::load_udm_asset(const std::string &fileName,std::string *optOutErr)
{
	return ::load_udm_asset(fileName,optOutErr);
}
std::shared_ptr<udm::Data> util::load_udm_asset(std::shared_ptr<VFilePtrInternal> &f,std::string *optOutErr)
{
	return ::load_udm_asset(f,optOutErr);
}
#pragma optimize("",on)
