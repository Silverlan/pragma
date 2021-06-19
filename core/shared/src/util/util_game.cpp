/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/util/util_game.hpp"
#include "pragma/networkstate/networkstate.h"
#include "pragma/util/lookup_identifier.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include <sharedutils/util_library.hpp>
#include <sharedutils/util_path.hpp>
#include <udm.hpp>

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
	f->Seek(0);
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
			Con::cwar<<"  > ";
		else
			Con::cwar<<"    ";
		Con::cwar<<line<<'\n';

		if(it == lines.end() -1)
			lineOffset = line.length() -len;
	}
	Con::cwar<<std::string(static_cast<size_t>(charIdx) +4 +lineOffset,' ')<<'^'<<Con::endl;

	Con::cwar<<Con::endl;
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
			Con::cwar<<"[UDM] Failed to load UDM asset";
			if constexpr(std::is_same_v<std::remove_const_t<std::remove_reference_t<T>>,std::string>)
				Con::cout<<" '"<<f<<"'";
			else
			{
				auto *ptr = static_cast<VFilePtrInternalReal*>(f.get());
				if(ptr)
				{
					auto path = util::Path::CreateFile(ptr->GetPath());
					path.MakeRelative(util::get_program_path());
					Con::cwar<<" '"<<path.GetString()<<"'";
				}
			}
			Con::cwar<<": "<<e.what()<<":\n";
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
std::shared_ptr<udm::Data> util::load_udm_asset(std::shared_ptr<VFilePtrInternal> f,std::string *optOutErr)
{
	return ::load_udm_asset(f,optOutErr);
}

void util::write_udm_entity(udm::LinkedPropertyWrapperArg udm,EntityHandle &hEnt)
{
	if(hEnt.IsValid())
		udm = util::uuid_to_string(hEnt->GetUuid());
}
EntityHandle util::read_udm_entity(Game &game,udm::LinkedPropertyWrapperArg udm)
{
	std::string uuid;
	udm(uuid);
	if(util::is_uuid(uuid))
	{
		EntityIterator entIt {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterUuid>(util::uuid_string_to_bytes(uuid));
		auto it = entIt.begin();
		auto *ent = (it != entIt.end()) ? *it : nullptr;
		return ent ? ent->GetHandle() : EntityHandle{};
	}
	return EntityHandle{};
}
EntityHandle util::read_udm_entity(::pragma::BaseEntityComponent &c,udm::LinkedPropertyWrapperArg udm) {return read_udm_entity(*c.GetEntity().GetNetworkState()->GetGameState(),udm);}

std::shared_ptr<util::HairFile> util::HairFile::Load(const udm::AssetData &data,std::string &outErr)
{
	auto anim = Create();
	if(anim->LoadFromAssetData(data,outErr) == false)
		return nullptr;
	return anim;
}
std::shared_ptr<util::HairFile> util::HairFile::Create() {return std::shared_ptr<HairFile>{new HairFile{}};}
bool util::HairFile::Save(udm::AssetData &outData,std::string &outErr) const
{
	outData.SetAssetType(PHAIR_IDENTIFIER);
	outData.SetAssetVersion(PHAIR_VERSION);
	auto udm = *outData;

	auto udmConfig = udm["config"];
	udmConfig["numSegments"] = m_hairConfig.numSegments;
	udmConfig["hairPerSquareMeter"] = m_hairConfig.hairPerSquareMeter;
	udmConfig["defaultThickness"] = m_hairConfig.defaultThickness;
	udmConfig["defaultLength"] = m_hairConfig.defaultLength;
	udmConfig["defaultHairStrength"] = m_hairConfig.defaultHairStrength;
	udmConfig["randomHairLengthFactor"] = m_hairConfig.randomHairLengthFactor;
	udmConfig["curvature"] = m_hairConfig.curvature;

	auto udmData = udm["data"];
	if(!m_hairData.hairPoints.empty())
		udmData["points"] = udm::compress_lz4_blob(m_hairData.hairPoints);
	if(!m_hairData.hairUvs.empty())
		udmData["uvs"] = udm::compress_lz4_blob(m_hairData.hairUvs);
	if(!m_hairData.hairNormals.empty())
		udmData["normals"] = udm::compress_lz4_blob(m_hairData.hairNormals);
	if(!m_hairData.hairPointTriangles.empty())
		udmData["triangles"] = udm::compress_lz4_blob(m_hairData.hairPointTriangles);
	if(!m_hairData.hairPointBarycentric.empty())
		udmData["barycentricCoords"] = udm::compress_lz4_blob(m_hairData.hairPointBarycentric);
	return true;
}
bool util::HairFile::LoadFromAssetData(const udm::AssetData &data,std::string &outErr)
{
	if(data.GetAssetType() != PHAIR_IDENTIFIER)
	{
		outErr = "Incorrect format!";
		return false;
	}

	auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1)
	{
		outErr = "Invalid version!";
		return false;
	}
	
	auto udmConfig = udm["config"];
	udmConfig["numSegments"](m_hairConfig.numSegments);
	udmConfig["hairPerSquareMeter"](m_hairConfig.hairPerSquareMeter);
	udmConfig["defaultThickness"](m_hairConfig.defaultThickness);
	udmConfig["defaultLength"](m_hairConfig.defaultLength);
	udmConfig["defaultHairStrength"](m_hairConfig.defaultHairStrength);
	udmConfig["randomHairLengthFactor"](m_hairConfig.randomHairLengthFactor);
	udmConfig["curvature"](m_hairConfig.curvature);

	auto udmData = udm["data"];
	udmData["points"].GetBlobData(m_hairData.hairPoints);
	udmData["uvs"].GetBlobData(m_hairData.hairUvs);
	udmData["normals"].GetBlobData(m_hairData.hairNormals);
	udmData["triangles"].GetBlobData(m_hairData.hairPointTriangles);
	udmData["barycentricCoords"].GetBlobData(m_hairData.hairPointBarycentric);
	return true;
}
