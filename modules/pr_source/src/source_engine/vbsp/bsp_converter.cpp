#include <iostream>
#include "source_engine/vbsp/bsp_converter.hpp"
#include "source_engine/source_engine.hpp"
#include <pragma/asset_types/world.hpp>
#include <pragma/game/game.h>
#include <pragma/networkstate/networkstate.h>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/model/brush/brushmesh.h>
#include <pragma/physics/collisionmesh.h>
#include <sharedutils/util_file.h>
#include <sharedutils/util.h>
#include <vmf_poly.hpp>
#include <vmf_poly_mesh.hpp>
#include <vmf_entity_data.hpp>
#include <util_bsp.hpp>
#include <util_fgd.hpp>
#include <util_archive.hpp>
#include <pragma/engine.h>

#pragma optimize("",off)
extern DLLENGINE Engine *engine;
const decltype(pragma::asset::vbsp::BSPConverter::WLD_DEFAULT_AMBIENT_COLOR) pragma::asset::vbsp::BSPConverter::WLD_DEFAULT_AMBIENT_COLOR = Color{255,255,255,80};
Vector3 pragma::asset::vbsp::BSPConverter::BSPVertexToPragma(const Vector3 &inPos)
{
	static Quat s_rotationConversion = uquat::create(EulerAngles{0.f,-90.f,0.f});
	auto pos = inPos;
	uvec::rotate(&pos,s_rotationConversion);
	return pos;
}

Vector3 pragma::asset::vbsp::BSPConverter::BSPTextureVecToPragma(const Vector3 &vec)
{
	// In the source engine the x-axis points forward, but in Pragma it's the z-axis.
	// We can just rotate everything by 90 degrees to adapt.
	static Quat s_rotationConversion2 = uquat::create(EulerAngles{0.f,0.f,-90.f});
	auto newPos = vec;
	uvec::rotate(&newPos,s_rotationConversion2);
	return newPos;
}

void pragma::asset::vbsp::BSPConverter::SetMessageLogger(const std::function<void(const std::string&)> &msgLogger)
{
	m_messageLogger = msgLogger ? msgLogger : [](const std::string&) {};
}
bool pragma::asset::vbsp::BSPConverter::StartConversion()
{
	m_outputWorldData = pragma::asset::WorldData::Create(m_nw);
	m_outputWorldData->SetMessageLogger(m_messageLogger);
	Con::cwar<<"----------- BSP Conversion LOG -----------"<<Con::endl;

	LoadFGDData();
	ExtractBSPFiles();

	ConvertEntityData();

	auto &texStringData = m_bsp->GetTranslatedTexDataStrings();
	std::vector<MaterialHandle> materials {};
	materials.reserve(texStringData.size());
	for(auto &str : texStringData)
	{
		auto lstr = str;
		ustring::to_lower(lstr);
		m_nw.LoadMaterial(lstr); // Note: This is on purpose, m_nw may be ClientState
		auto *mat = m_game.GetNetworkState()->LoadMaterial(lstr);
		materials.push_back(mat ? mat->GetHandle() : MaterialHandle{});
	}

	auto lightmapData = LoadLightmapData(m_nw,*m_bsp);
	GenerateLightMapAtlas(*lightmapData,GetMapName());
	ConvertWorldGeometry();

	struct ModelInfo
	{
		std::shared_ptr<Model> model = nullptr;
		pragma::asset::EntityData *entityData = nullptr;
	};
	std::vector<ModelInfo> models {};
	for(auto &pair : m_bspEntityData)
	{
		auto mdl = GenerateModel(*pair.first,*lightmapData,materials);
		if(mdl == nullptr)
			continue;
		models.push_back(ModelInfo{mdl,pair.first});
	}

	// Convert vertices to Pragma's coordinate system
	for(auto &mdlInfo : models)
	{
		for(auto &meshGroup : mdlInfo.model->GetMeshGroups())
		{
			for(auto &mesh : meshGroup->GetMeshes())
			{
				for(auto &subMesh : mesh->GetSubMeshes())
				{
					for(auto &v : subMesh->GetVertices())
					{
						v.position = {-v.position.z,v.position.y,v.position.x};
						v.normal = {-v.normal.z,v.normal.y,v.normal.x};
					}
				}
			}
		}
		for(auto &colMesh : mdlInfo.model->GetCollisionMeshes())
		{
			for(auto &v : colMesh->GetVertices())
				v = {-v.z,v.y,v.x};
		}
		mdlInfo.model->Update(ModelUpdateFlags::All);

		auto name = mdlInfo.entityData->GetClassName() +"_" +std::to_string(mdlInfo.entityData->GetMapIndex());
		auto path = "maps/" +GetMapName() +"/" +name;
		mdlInfo.entityData->SetKeyValue("model",path);
		mdlInfo.model->Save(&m_game,path);
	}

	ScopeGuard sg([]() {
		Con::cwar<<"----------- BSP Conversion LOG -----------"<<Con::endl;
	});

	m_messageLogger("Creating output file '" +m_path +"'...");
	std::string errMsg;
	if(m_outputWorldData->Write(m_path,&errMsg) == false)
		m_messageLogger("ERROR: " +errMsg);
	return true;
}

void pragma::asset::vbsp::BSPConverter::ParseEntityGeometryData(pragma::asset::EntityData &entData,const std::unordered_set<uint32_t> &materialRemovalTable,int32_t skyTexIdx,int32_t skyTex2dIdx)
{
	auto *bspData = FindBSPEntityData(entData);
	if(bspData == nullptr)
		return;

	BSPEntityData *bspSkybox = nullptr;
	BSPEntityData *bspWater = nullptr;

	auto fInitEntityBSPData = [this](BSPEntityData **outPtr,const std::string &className) -> BSPEntityData* {
		if(*outPtr)
			return *outPtr;
		auto &ents = m_outputWorldData->GetEntities();
		auto it = std::find_if(ents.begin(),ents.end(),[className](const std::shared_ptr<pragma::asset::EntityData> &entData) {
			return ustring::compare(entData->GetClassName(),className,false);
		});
		EntityData *entData = nullptr;
		if(it != ents.end())
			entData = it->get();
		if(it == ents.end())
		{
			auto ent = pragma::asset::EntityData::Create();
			ent->SetClassName(className);
			m_outputWorldData->AddEntity(*ent);
			entData = ent.get();
		}
		auto *bspData = FindBSPEntityData(*entData);
		if(bspData == nullptr)
		{
			auto it = m_bspEntityData.insert(std::make_pair(entData,std::make_shared<BSPEntityData>())).first;
			bspData = it->second.get();
		}
		*outPtr = bspData;
		return *outPtr;
	};
	auto fAddSkyboxFace = [&fInitEntityBSPData,bspSkybox](int32_t faceIndex) mutable {
		auto *bspData = fInitEntityBSPData(&bspSkybox,"skybox");
		auto &faceIndices = bspData->faceIndices;
		if(faceIndices.size() == faceIndices.capacity())
			faceIndices.reserve(faceIndices.size() *1.5f +100);
		faceIndices.push_back(faceIndex);
	};
	auto fAddWaterFace = [&fInitEntityBSPData,bspWater](int32_t faceIndex) mutable {
		auto *bspData = fInitEntityBSPData(&bspWater,"water");
		auto &faceIndices = bspData->faceIndices;
		if(faceIndices.size() == faceIndices.capacity())
			faceIndices.reserve(faceIndices.size() *1.5f +100);
		faceIndices.push_back(faceIndex);
	};

	// Iterate the brushes for this entity and collect the faces
	auto &models = m_bsp->GetModels();
	auto &faces = m_bsp->GetFaces();
	auto &brushes = m_bsp->GetBrushes();
	auto &texInfo = m_bsp->GetTexInfo();
	for(auto mdlIdx : bspData->modelIndices)
	{
		auto &mdl = models.at(mdlIdx);
		auto &faceIndices = bspData->faceIndices;
		faceIndices.reserve(mdl.numfaces);
		for(auto faceIndex=mdl.firstface;faceIndex<(mdl.firstface +mdl.numfaces);++faceIndex)
		{
			auto &face = faces.at(faceIndex);
			auto &faceTexInfo = texInfo.at(face.texinfo);
			if(faceTexInfo.flags &umath::to_integral(SurfFlags::Sky))
			{
				// This is a skybox face; Add it to the skybox entity instead
				fAddSkyboxFace(faceIndex);
			}
			else
			{
				// TODO: Is 'Warp' surface flag a reliable way of determining
				// whether this is a water face?
				constexpr auto skipFlags = umath::to_integral(SurfFlags::Warp) | 
					umath::to_integral(SurfFlags::Skip) | 
					umath::to_integral(SurfFlags::Hint) | 
					umath::to_integral(SurfFlags::Nodraw);
				if((faceTexInfo.flags &skipFlags) == 0) // Only add the face if it's not a water face
					faceIndices.push_back(faceIndex);
			}
		}
	}

	if(entData.IsWorld())
	{
		// The world brushes include special types like water or the skybox, but we want to
		// split that into separate entities
		auto &brushes = m_bsp->GetBrushes();
		auto &brushSides = m_bsp->GetBrushSides();
		auto &texData = m_bsp->GetTexData();
		auto &texStringTable = m_bsp->GetTexDataStringIndices();
		auto &texStringData = m_bsp->GetTranslatedTexDataStrings();

		for(auto it=bspData->brushIndices.begin();it!=bspData->brushIndices.end();)
		{
			auto brushIdx = *it;
			auto &brush = brushes.at(brushIdx);
			enum class BrushType : uint32_t
			{
				World = 0,
				Skybox,
				Water,
				Remove
			};
			auto type = BrushType::World;
			if(brush.contents &umath::to_integral(ContentsFlags::Water))
				type = BrushType::Water;
			// else if(brush.contents &umath::to_integral(ContentsFlags::Detail))
			// 	type = BrushType::Remove;
			else
			{
				for(auto i=brush.firstside;i<brush.firstside +brush.numsides;++i)
				{
					auto &brushSide = brushSides.at(i);
					if(brushSide.texinfo == -1)
					{
						//brushSide.texinfo = iNodraw;
						//break;
						continue;
					}
					auto &brushTexInfo = texInfo.at(brushSide.texinfo);
					if(brushTexInfo.texdata == -1)
						continue;
					auto &brushTexData = texData.at(brushTexInfo.texdata);
					auto brushTexStringId = texStringTable.at(brushTexData.nameStringTableID);
					auto &brushTexString = texStringData.at(brushTexStringId);

					if(brushTexStringId == skyTexIdx || brushTexStringId == skyTex2dIdx)
					{
						type = BrushType::Skybox;
						break;
					}
					else
					{
						auto it = materialRemovalTable.find(brushTexStringId);
						if(it != materialRemovalTable.end())
						{
							type = BrushType::Remove;
							break;
						}
					}
				}
			}

			if(type == BrushType::Skybox)
			{
				fInitEntityBSPData(&bspSkybox,"skybox")->brushIndices.insert(brushIdx);
				it = bspData->brushIndices.erase(it);
			}
			else if(type == BrushType::Water)
			{
				fInitEntityBSPData(&bspSkybox,"water")->brushIndices.insert(brushIdx);
				it = bspData->brushIndices.erase(it);
			}
			else if(type == BrushType::Remove)
				it = bspData->brushIndices.erase(it); // Remove all brushes of this type entirely
			else
				++it;
		}
	}
}

void pragma::asset::vbsp::BSPConverter::ConvertEntityData()
{
	auto entities = m_bsp->GetEntities();
	auto &brushes = m_bsp->GetBrushes();
	auto &brushSides = m_bsp->GetBrushSides();
	auto &planes = m_bsp->GetPlanes();
	auto &texInfo = m_bsp->GetTexInfo();
	auto &texData = m_bsp->GetTexData();
	auto &texStringTable = m_bsp->GetTexDataStringIndices();
	auto &texStringData = m_bsp->GetTranslatedTexDataStrings();
	auto &models = m_bsp->GetModels();
	auto &faces = m_bsp->GetHDRFaces().empty() ? m_bsp->GetFaces() : m_bsp->GetHDRFaces();
	auto &origFaces = m_bsp->GetOriginalFaces();
	auto &nodes = m_bsp->GetNodes();
	auto &leaves = m_bsp->GetLeaves();
	auto &leafBrushes = m_bsp->GetLeafBrushes();
	auto &cubemapSamples = m_bsp->GetCubemapSamples();

	std::unordered_map<uint32_t,std::pair<uint32_t,uint32_t>> staticPropLeafRanges {};
	auto &staticPropData = m_bsp->GetStaticPropData();

	for(auto &lump : staticPropData.staticPropLumps)
	{
		auto &name = staticPropData.dictionaryModelNames.at(lump.PropType);
		entities.push_back(std::make_shared<vmf::DataFileBlock>());
		auto &entData = entities.back();
		entData->keyvalues["classname"] = {"prop_static"};
		entData->keyvalues["origin"] = {std::to_string(lump.Origin.x) +" " +std::to_string(lump.Origin.y) +" " +std::to_string(lump.Origin.z)};
		entData->keyvalues["angles"] = {std::to_string(lump.Angles.p) +" " +std::to_string(lump.Angles.y) +" " +std::to_string(lump.Angles.r)};
		entData->keyvalues["skin"] = {std::to_string(lump.Skin)};
		entData->keyvalues["model"] = {name};
		auto idx = entities.size() -1;
		staticPropLeafRanges[idx] = {lump.FirstLeaf,lump.LeafCount};
	}

	for(auto &cubemapSample : cubemapSamples)
	{
		entities.push_back(std::make_shared<vmf::DataFileBlock>());
		auto &entData = entities.back();
		entData->keyvalues["classname"] = {"env_cubemap"};
		entData->keyvalues["origin"] = {std::to_string(cubemapSample.origin.at(0)) +" " +std::to_string(cubemapSample.origin.at(1)) +" " +std::to_string(cubemapSample.origin.at(2))};
	}

	auto itNodraw = std::find(texStringData.begin(),texStringData.end(),std::string("TOOLS/TOOLSNODRAW"));
	if(itNodraw == texStringData.end())
		const_cast<std::vector<std::string>&>(texStringData).push_back("TOOLS/TOOLSNODRAW");

	const std::array<std::string,10> materialsRemove = {
		"TOOLS/TOOLSHINT",
		"TOOLS/TOOLSSKIP",
		"TOOLS/TOOLSAREAPORTAL",
		"TOOLS/TOOLSBLOCKLIGHT",
		"TOOLS/TOOLSINVISIBLELADDER",
		"TOOLS/TOOLSOCCLUDER",
		"TOOLS/TOOLSBLOCK_LOS",
		"TOOLS/TOOLSBLOCKBULLETS",
		"TOOLS/TOOLSFOG",

		"TOOLS/TOOLSCLIP"
	};
	std::unordered_set<uint32_t> materialRemovalTable;
	for(auto &matName : materialsRemove)
	{
		auto it = std::find(texStringData.begin(),texStringData.end(),matName);
		if(it == texStringData.end())
			continue;
		materialRemovalTable.insert(it -texStringData.begin());
	}
	const std::unordered_map<std::string,std::string> materialsReplace = {
		//{"TOOLS/TOOLSCLIP","TOOLS/TOOLSNODRAW"}
	};
	for(auto &pair : materialsReplace)
	{
		auto it = std::find(texStringData.begin(),texStringData.end(),pair.first);
		if(it == texStringData.end())
			continue;
		const_cast<std::string&>(*it) = pair.second;
	}

	const auto fGetKeyValue = [](const bsp::EntityBlock &entBlock,const std::string &key) -> const std::string& {
		auto it = std::find_if(entBlock->keyvalues.begin(),entBlock->keyvalues.end(),[&key](const std::pair<std::string,std::vector<std::string>> &pair) {
			return ustring::compare(key,pair.first,false);
		});
		if(it == entBlock->keyvalues.end() || it->second.empty())
		{
			static std::string r = "";
			return r;
		}
		return it->second.front();
	};

	// Make sure the world is the first in the list
	auto itWorld = std::find_if(entities.begin(),entities.end(),[&fGetKeyValue](const bsp::EntityBlock &entBlock) {
		auto &className = fGetKeyValue(entBlock,"classname");
		return ustring::compare(className,"worldspawn",false);
	});
	if(itWorld != entities.end())
	{
		auto worldEntBlock = *itWorld;
		entities.erase(itWorld);
		entities.insert(entities.begin(),worldEntBlock);
	}

	const std::array<std::string,7> classIgnoreList = {
		//"worldspawn",
		"env_fog_controller", // TODO
		"func_brush", // TODO
		"info_node"
	};
	m_messageLogger("Transforming entity data...");

	auto itSkybox = std::find(texStringData.begin(),texStringData.end(),std::string("TOOLS/TOOLSSKYBOX"));
	auto iTexSkybox = (itSkybox != texStringData.end()) ? (itSkybox -texStringData.begin()) : std::string::npos;
	auto itSkybox2D = std::find(texStringData.begin(),texStringData.end(),std::string("TOOLS/TOOLSSKYBOX2D"));
	auto iTexSkybox2D = (itSkybox2D != texStringData.end()) ? (itSkybox2D -texStringData.begin()) : std::string::npos;

	auto idx = 1u;
	for(auto &entBlock : entities)
	{
		auto entIdx = idx++ -1;
		auto className = fGetKeyValue(entBlock,"classname");
		if(className.empty())
			continue;
		auto isWorld = ustring::compare(className,"worldspawn",false);
		if(isWorld == true)
		{
			entBlock->keyvalues.insert(std::make_pair("color",std::vector<std::string>{WLD_DEFAULT_AMBIENT_COLOR.ToString()}));
			auto skybox = fGetKeyValue(entBlock,"skyname");
			if(skybox == "painted") // Painted skyboxes are currently not supported, so we'll default to another one
				skybox = "skybox/dawn2";
			else
				skybox = "skybox/" +skybox;
			if(itSkybox != texStringData.end())
				const_cast<std::string&>(*itSkybox) = skybox;
			if(itSkybox2D != texStringData.end())
				const_cast<std::string&>(*itSkybox2D) = skybox;
			className = "world";
		}
		auto it = std::find_if(classIgnoreList.begin(),classIgnoreList.end(),[&className](const std::string &classNameOther) {
			return ustring::compare(className,classNameOther,false);
		});
		if(it != classIgnoreList.end())
			continue;

		int32_t brushModelId = -1;
		auto brushModel = fGetKeyValue(entBlock,"model");
		if(brushModel.size() > 1 && brushModel.front() == '*')
			brushModelId = util::to_int(ustring::substr(brushModel,1));
		if(isWorld)
			brushModelId = 0u; // First brush model is always worldspawn geometry
		if(brushModelId != -1)
		{
			auto &model = models.at(brushModelId);
			auto itOrigin = entBlock->keyvalues.find("origin");
			if(itOrigin == entBlock->keyvalues.end())
				entBlock->keyvalues["origin"] = {std::to_string(model.origin.x) +" " +std::to_string(model.origin.y) +" " +std::to_string(model.origin.z)};
		}

		if(ustring::compare(className,"func_detail",false) == true)
		{
			// We'll add the func_detail geometry to the map entity
			if(brushModelId != -1)
			{
				auto *entWorld = m_outputWorldData->FindWorld();
				if(entWorld)
				{
					auto *bspData = FindBSPEntityData(*entWorld);
					if(bspData)
						FindModelBrushes(*m_bsp,models.at(brushModelId),bspData->brushIndices);
				}
			}
			continue;
		}

		auto bWorld = className == "world";
		auto prEntity = pragma::asset::EntityData::Create();
		m_outputWorldData->AddEntity(*prEntity,bWorld);

		if(brushModelId != -1)
		{
			// Collect brushes used by this entity
			auto bspEntData = std::make_shared<BSPEntityData>();
			m_bspEntityData.insert(std::make_pair(prEntity.get(),bspEntData));
			FindModelBrushes(*m_bsp,models.at(brushModelId),bspEntData->brushIndices);

			auto it = m_bspEntityData.find(prEntity.get());
			if(it == m_bspEntityData.end())
				it = m_bspEntityData.insert(std::make_pair(prEntity.get(),std::make_shared<BSPEntityData>())).first;
			it->second->modelIndices.push_back(brushModelId);
		}

		auto itLeafRange = staticPropLeafRanges.find(entIdx);
		if(itLeafRange != staticPropLeafRanges.end())
			prEntity->SetLeafData(itLeafRange->second.first,itLeafRange->second.second);

		auto lClassName = className;
		ustring::to_lower(lClassName);
		prEntity->SetClassName(lClassName);
		for(auto &pair : entBlock->keyvalues)
		{
			if(pair.first.empty())
				continue;
			auto bKeyValue = true;
			for(auto &fgd : m_fgdData)
			{
				auto itClassDef = fgd.classDefinitions.find(lClassName);
				if(itClassDef == fgd.classDefinitions.end())
					continue;
				if(itClassDef->second->FindOutput(fgd,pair.first) != nullptr)
				{
					if(itClassDef->second->FindKeyValue(fgd,pair.first) == nullptr)
						bKeyValue = false;
					else
						m_messageLogger("WARNING: Unable to determine whether '" +pair.first +"' of entity class '" +className +"' is keyvalue or output because definition could not be located in any FGD file! Treating as keyvalue...");
					break;
				}
			}
			if(bKeyValue == true)
			{
				if(pair.second.empty() == false)
					prEntity->SetKeyValue(pair.first,pair.second.front());
				continue;
			}
			auto name = pair.first;
			ustring::to_lower(name);
			for(auto &val : pair.second)
			{
				pragma::asset::Output output {};
				output.name = name;

				std::vector<std::string> args;
				ustring::explode(val,",",args);
				auto numArgs = args.size();
				if(numArgs >= 2)
				{
					output.target = args.at(0);
					output.input = args.at(1);
					output.param = "";
					output.delay = 0.f;
					output.times = 1;
					if(numArgs > 2)
					{
						output.param = args.at(2);
						if(numArgs > 3)
						{
							output.delay = util::to_float(args.at(3));
							if(numArgs > 4)
								output.times = util::to_int(args.at(4));
						}
					}
				}
				prEntity->AddOutput(output);
			}
		}
	}
	const std::unordered_map<SurfFlags,std::string> surfFlagBitToName = {
		{SurfFlags::Light,"Light"},
		{SurfFlags::Sky2D,"Sky2D"},
		{SurfFlags::Sky,"Sky"},
		{SurfFlags::Warp,"Warp"},
		{SurfFlags::Trans,"Trans"},
		{SurfFlags::NoPortal,"NoPortal"},
		{SurfFlags::Trigger,"Trigger"},
		{SurfFlags::Nodraw,"Nodraw"},
		{SurfFlags::Hint,"Hint"},
		{SurfFlags::Skip,"Skip"},
		{SurfFlags::NoLight,"NoLight"},
		{SurfFlags::BumpLight,"BumpLight"},
		{SurfFlags::NoShadows,"NoShadows"},
		{SurfFlags::NoDecals,"NoDecals"},
		{SurfFlags::NoChop,"NoChop"},
		{SurfFlags::Hitbox,"Hitbox"}
	};
	auto &ents = m_outputWorldData->GetEntities();
	auto itEntWorld = std::find_if(ents.begin(),ents.end(),[](const std::shared_ptr<pragma::asset::EntityData> &ent) {
		return ent->IsWorld();
	});
	auto itEntSkybox = std::find_if(ents.begin(),ents.end(),[](const std::shared_ptr<pragma::asset::EntityData> &ent) {
		return ent->IsSkybox();
	});
	auto entWorld = (itEntWorld != ents.end()) ? *itEntWorld : nullptr;
	auto entSkybox = (itEntSkybox != ents.end()) ? *itEntSkybox : nullptr;
	auto *bspWorld = entWorld ? FindBSPEntityData(*entWorld) : nullptr;

	if(bspWorld)
	{
		// Add displacements to list of world brushes
		auto &displacements = m_bsp->GetDisplacements();
		for(auto &disp : displacements)
			BuildDisplacement(*m_bsp,&m_nw,disp,bspWorld->polyMeshes);
	}

	// HACK: ParseEntityGeometryData may add new entities to the list while we're iterating, which
	// would invalidate the array. To prevent that from happening, we make sure it's large enough to
	// encompass the new entities, which should only be 2 max (water +skybox), but we'll add a buffer to be sure.
	const_cast<std::vector<std::shared_ptr<EntityData>>&>(ents).reserve(ents.size() +100);
	for(auto &entData : ents)
		ParseEntityGeometryData(*entData,materialRemovalTable,iTexSkybox,iTexSkybox2D);

	// Build poly meshes from brushes (required for collisions)
	for(auto &pair : m_bspEntityData)
		BuildBrushMesh(*m_bsp,&m_nw,pair.second->brushIndices,pair.second->polyMeshes);

	for(auto &pair : m_bspEntityData)
	{
		auto &bspData = pair.second;
		auto &polyMeshes = bspData->polyMeshes;
		for(auto mesh : polyMeshes)
		{
			if(mesh->HasDisplacements() == false)
				continue;
			std::vector<vmf::Poly*> *polys;
			mesh->GetPolys(&polys);
			for(auto *poly : *polys)
			{
				auto &polyInfo = poly->GetCompiledData();
				if(poly->IsDisplacement() == false)
					continue;
				auto &displacement = *polyInfo.displacement.get();
				vmf::PolyMesh::BuildDisplacementNormals(polyMeshes,mesh.get(),poly,polyInfo,displacement.normals);
			}
		}
	}
	//
	//}
	//else
	source_engine::translate_entity_data(*m_outputWorldData,m_fgdData,false,m_messageLogger,&m_msgCache);
}

void pragma::asset::vbsp::BSPConverter::ConvertWorldGeometry()
{
	// Which geometry type to load from the BSP
	auto &texStringTable = m_bsp->GetTexDataStringIndices();
	auto &texStringData = m_bsp->GetTranslatedTexDataStrings();

	auto &texInfo = m_bsp->GetTexInfo();
	auto &texData = m_bsp->GetTexData();
	auto &staticPropData = m_bsp->GetStaticPropData();

	m_outputWorldData->GetStaticPropLeaves() = staticPropData.leaves;
	m_outputWorldData->GetMaterialTable() = texStringData;

	auto bspTree = BSPTree::Create(*m_bsp);
	m_outputWorldData->SetBSPTree(*bspTree);
}

void pragma::asset::vbsp::BSPConverter::LoadFGDData()
{
	m_messageLogger("Loading FGD data...");
	m_fgdData = source_engine::load_fgds(m_nw,m_messageLogger);
}

pragma::asset::vbsp::BSPEntityData *pragma::asset::vbsp::BSPConverter::FindBSPEntityData(pragma::asset::EntityData &entData)
{
	auto it = m_bspEntityData.find(&entData);
	if(it == m_bspEntityData.end())
		return nullptr;
	return it->second.get();
}

void pragma::asset::vbsp::BSPConverter::ExtractBSPFiles()
{
	std::string mapName = ufile::get_file_from_filename(m_path);
	ufile::remove_extension_from_filename(mapName);
	auto mapMatPath = "materials/maps/" +mapName +'/';

	m_messageLogger("Extracing embedded files...");
	uint32_t numExtracted = 0;
	uint32_t numSkippedLump = 0;
	uint32_t numSkippedMapMats = 0;
	uint32_t numSkippedVhvFiles = 0;
	auto &bsp = *m_bsp;
	for(auto &fName : bsp.GetFilenames())
	{
		if(ustring::compare(fName.c_str(),"lumps/",false,6))
		{
			++numSkippedLump;
			continue;
		}
		if(ustring::compare(fName.c_str(),mapMatPath.c_str(),false,mapMatPath.length()))
		{
			++numSkippedMapMats;
			continue;
		}
		std::string ext;
		if(ufile::get_extension(fName,&ext) && ustring::compare(ext,"vhv",false))
		{
			++numSkippedVhvFiles;
			continue;
		}
		std::vector<uint8_t> data;
		if(bsp.ReadFile(fName,data) == false)
			m_messageLogger("WARNING: Unable to extract file '" +fName +"' from BSP!");
		else
		{
			m_messageLogger("Extracting file '" +fName +"'...");
			std::string extractPath = "addons/imported/" +fName;
			auto relPath = ufile::get_path_from_filename(extractPath);
			FileManager::CreatePath(relPath.c_str());
			auto fOut = FileManager::OpenFile<VFilePtrReal>(extractPath.c_str(),"wb");
			if(fOut == nullptr)
			{
				m_messageLogger("WARNING: Unable to extract file '" +fName +"' from BSP: Cannot write output file!");
				continue;
			}
			fOut->Write(data.data(),data.size() *sizeof(data.front()));
			++numExtracted;
		}
	}
	m_messageLogger(std::to_string(numExtracted) +" files have been extracted! " +std::to_string(numSkippedLump) +" lump files, " +std::to_string(numSkippedMapMats) +" map material files and " +std::to_string(numSkippedVhvFiles) +" vhv files have been skipped!");
}

std::string pragma::asset::vbsp::BSPConverter::GetMapName() const
{
	auto mapName = ufile::get_file_from_filename(m_path);
	ufile::remove_extension_from_filename(mapName);
	ustring::to_lower(mapName);
	return mapName;
}

std::shared_ptr<pragma::asset::vbsp::BSPConverter> pragma::asset::vbsp::BSPConverter::Open(Game &game,const std::string &path)
{
	auto bspPath = path;
	ufile::remove_extension_from_filename(bspPath);
	bspPath += ".bsp";

	auto f = FileManager::OpenFile(bspPath.c_str(),"rb");
	if(f == nullptr)
		f = uarch::load(bspPath);

	if(f == nullptr)
		return nullptr;
	bsp::ResultCode code;
	auto bsp = bsp::File::Open(f,code);
	if(bsp == nullptr)
		return nullptr;
	auto *clNw = engine->GetClientState();
	auto *nw = clNw ? clNw : game.GetNetworkState();
	return std::shared_ptr<BSPConverter>{new BSPConverter{*nw,game,std::move(bsp),path}};
}

pragma::asset::vbsp::BSPConverter::BSPConverter(NetworkState &nw,Game &game,std::unique_ptr<bsp::File> &&bsp,const std::string &path)
	: m_bsp{std::move(bsp)},m_game{game},m_path{path},m_nw{nw}
{
	SetMessageLogger(nullptr);
}

Vector2 pragma::asset::vbsp::BSPConverter::CalcDispSurfCoords(const std::array<Vector2,4> &texCoords,uint32_t power,uint32_t x,uint32_t y)
{
	// See CCoreDispInfo::CalcDispSurfCoords from source sdk code
	auto postSpacing = ((1<<power) +1);
	auto ooInt = 1.0f /static_cast<float>(postSpacing -1);
	std::array<Vector2,2> edgeInt = {
		texCoords.at(1) -texCoords.at(0),
		texCoords.at(2) -texCoords.at(3)
	};
	edgeInt.at(0) *= ooInt;
	edgeInt.at(1) *= ooInt;

	std::array<Vector2,2> endPts = {
		edgeInt.at(0) *static_cast<float>(x) +texCoords.at(0),
		edgeInt.at(1) *static_cast<float>(x) +texCoords.at(3)
	};
	auto seg = endPts.at(1) -endPts.at(0);
	auto segInt = seg *ooInt;
	seg = segInt *static_cast<float>(y);
	return endPts.at(0) +seg;
}

std::vector<std::shared_ptr<CollisionMesh>> pragma::asset::vbsp::BSPConverter::GenerateCollisionMeshes(Game &game,std::vector<std::shared_ptr<BrushMesh>> &meshes,const std::vector<SurfaceMaterial> &surfaceMaterials)
{
	std::vector<std::shared_ptr<CollisionMesh>> collisionMeshes {};
	std::vector<std::unordered_map<uint32_t,uint32_t>> lods;
	Vector3 min(std::numeric_limits<Vector3::value_type>::max(),std::numeric_limits<Vector3::value_type>::max(),std::numeric_limits<Vector3::value_type>::max());
	Vector3 max(std::numeric_limits<Vector3::value_type>::lowest(),std::numeric_limits<Vector3::value_type>::lowest(),std::numeric_limits<Vector3::value_type>::lowest());
	collisionMeshes.reserve(meshes.size());
	for(auto &brushMesh : meshes)
	{
		auto colMesh = CollisionMesh::Create(&game);
		collisionMeshes.push_back(colMesh);

		std::vector<Vector3> &colVerts = colMesh->GetVertices();
		auto &colTriangles = colMesh->GetTriangles();
		auto &colMaterials = colMesh->GetSurfaceMaterials();
		colMesh->SetBoneParent(0);

		auto bConvex = brushMesh->IsConvex();
		colMesh->SetConvex(bConvex);
		Vector3 brushMin;
		Vector3 brushMax;
		brushMesh->GetBounds(&brushMin,&brushMax);
		uvec::min(&min,brushMin);
		uvec::max(&max,brushMax);
		auto &sides = brushMesh->GetSides();
		struct SurfaceMaterialInfo
		{
			SurfaceMaterialInfo(int id)
				: surfaceMaterialId(id),count(0)
			{}
			int surfaceMaterialId;
			int count;
		};
		std::vector<SurfaceMaterialInfo> surfaceMaterialInfo;
		auto surfaceMaterialId = 0;
		auto maxSurfaceMaterialCount = 0;

		for(auto &side : sides)
		{
			if(bConvex == side->IsConvex())
			{
				Material *mat = side->GetMaterial();
				auto surfaceMaterial = -1;
				if(mat != NULL)
				{
					auto &data = mat->GetDataBlock();
					if(data != nullptr)
					{
						std::string surfaceIdentifier;
						if(data->GetString("surfacematerial",&surfaceIdentifier) == true)
						{
							auto it = std::find_if(surfaceMaterials.begin(),surfaceMaterials.end(),[&surfaceIdentifier](const SurfaceMaterial &mat) {
								return (mat.GetIdentifier() == surfaceIdentifier) ? true : false;
								});
							if(it != surfaceMaterials.end())
								surfaceMaterial = static_cast<int>(it->GetIndex());
						}
					}
				}
				if(bConvex == true && surfaceMaterial != -1) // Determine most used surface material if this is a convex mesh
				{
					auto itInfo = std::find_if(surfaceMaterialInfo.begin(),surfaceMaterialInfo.end(),[surfaceMaterial](SurfaceMaterialInfo &info) {
						return (info.surfaceMaterialId == surfaceMaterial) ? true : false;
						});
					if(itInfo == surfaceMaterialInfo.end())
					{
						surfaceMaterialInfo.push_back(SurfaceMaterialInfo(surfaceMaterial));
						itInfo = surfaceMaterialInfo.end() -1;
					}
					itInfo->count++;
					if(itInfo->count > maxSurfaceMaterialCount)
					{
						maxSurfaceMaterialCount = itInfo->count;
						surfaceMaterialId = surfaceMaterial;
					}
				}

				auto &vertsSide = side->GetVertices();
				auto &normalsSide = side->GetNormals();
				auto &trianglesSide = *side->GetTriangles();
				auto &uvsSide = side->GetUVMapping();
				auto &alphasSide = side->GetAlphas();
				auto colVertOffset = colVerts.size();
				auto cv = [](Vector3 &v) -> Vector3 {
					return Vector3{v.z,v.y,-v.x};
				};
				if(bConvex == false)
				{
					if(surfaceMaterial == -1)
						surfaceMaterial = 0;
					colVerts.reserve(colVerts.size() +trianglesSide.size());
					colMaterials.reserve(colMaterials.size() +trianglesSide.size());
					for(auto it=trianglesSide.begin();it!=trianglesSide.end();++it)
						colVerts.push_back(cv(vertsSide.at(*it)));

					colTriangles.reserve(colTriangles.size() +trianglesSide.size());
					for(auto i=decltype(trianglesSide.size()){0};i<trianglesSide.size();++i)
						colTriangles.push_back(colVertOffset +i);

					colMaterials.reserve(colMaterials.size() +trianglesSide.size() /3);
					for(UInt i=0;i<trianglesSide.size();i+=3)
						colMaterials.push_back(surfaceMaterial);
				}
				else
					colVerts.reserve(colVerts.size() +vertsSide.size());

				for(unsigned int k=0;k<vertsSide.size();k++)
				{
					if(bConvex == true)
						colVerts.push_back(cv(vertsSide[k]));
				}
				if(bConvex == true)
				{
					colTriangles.reserve(colTriangles.size() +trianglesSide.size());
					for(auto i=decltype(trianglesSide.size()){0};i<trianglesSide.size();++i)
						colTriangles.push_back(colVertOffset +trianglesSide.at(i));
				}
			}
		}

		if(bConvex)
			colMesh->SetSurfaceMaterial(surfaceMaterialId); // Global surface material for entire mesh

		colMesh->CalculateBounds();
		colMesh->CalculateVolumeAndCom();
	}
	return collisionMeshes;
}
#pragma optimize("",on)
