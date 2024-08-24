/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/asset_types/world.hpp"
#include "pragma/level/level_info.hpp"
#include "pragma/model/modelmanager.h"
#include <util_image.hpp>
#include <util_texture_info.hpp>
#include <sharedutils/util_file.h>
#include <sharedutils/datastream.h>
#include <udm.hpp>

extern DLLNETWORK Engine *engine;

void pragma::asset::Output::Write(VFilePtrReal &f)
{
	auto lname = name;
	ustring::to_lower(lname);
	f->WriteString(lname);
	f->WriteString(target);
	f->WriteString(input);
	f->WriteString(param);
	f->Write<float>(delay);
	f->Write<int32_t>(times);
}

/////////

void pragma::asset::WorldData::WriteDataOffset(VFilePtrReal &f, uint64_t offsetToOffset)
{
	auto offset = f->Tell();
	f->Seek(offsetToOffset);
	f->Write<uint64_t>(offset);
	f->Seek(offset);
}

bool pragma::asset::WorldData::Write(const std::string &fileName, std::string *optOutErrMsg)
{
	auto nFileName = fileName;
	ufile::remove_extension_from_filename(nFileName);
	nFileName += ".wld";
	auto fullPath = util::CONVERT_PATH + nFileName;
	auto pathNoFile = ufile::get_path_from_filename(fullPath);
	if(FileManager::CreatePath(pathNoFile.c_str()) == false) {
		if(optOutErrMsg)
			*optOutErrMsg = "Unable to create path '" + pathNoFile + "'!";
		return false;
	}
	auto fOut = FileManager::OpenFile<VFilePtrReal>(fullPath.c_str(), "wb");
	if(fOut == nullptr) {
		if(optOutErrMsg)
			*optOutErrMsg = "Unable to write file '" + nFileName + "'!";
		return false;
	}
	Write(fOut);
	return true;
}

static void preprocess_bsp_data(util::BSPTree &bspTree, std::vector<std::vector<size_t>> &outClusterNodes, std::vector<std::vector<uint16_t>> &outClusterToClusterVisibility)
{
	auto numClusters = bspTree.GetClusterCount();
	auto &bspNodes = bspTree.GetNodes();

	// Pre-processing to speed up some calculations
	{
		// Nodes per cluster
		outClusterNodes.resize(numClusters);
		for(auto i = decltype(bspNodes.size()) {0u}; i < bspNodes.size(); ++i) {
			auto &bspNode = bspNodes.at(i);
			if(bspNode.cluster >= outClusterNodes.size())
				continue;
			auto &nodeList = outClusterNodes.at(bspNode.cluster);
			if(nodeList.size() == nodeList.capacity())
				nodeList.reserve(nodeList.size() * 1.5f + 100);
			nodeList.push_back(i);
		}

		// List of clusters visible from every other cluster
		outClusterToClusterVisibility.resize(numClusters);
		for(auto cluster0 = decltype(numClusters) {0u}; cluster0 < numClusters; ++cluster0) {
			auto &visibleClusters = outClusterToClusterVisibility.at(cluster0);
			for(auto cluster1 = decltype(numClusters) {0u}; cluster1 < numClusters; ++cluster1) {
				if(bspTree.IsClusterVisible(cluster0, cluster1) == false)
					continue;
				if(visibleClusters.size() == visibleClusters.capacity())
					visibleClusters.reserve(visibleClusters.size() * 1.5f + 50);
				visibleClusters.push_back(cluster1);
			}
		}
	}
	//
}

bool pragma::asset::WorldData::LoadFromAssetData(udm::LinkedPropertyWrapper &udm, EntityData::Flags entMask, std::string &outErr)
{
	udm["materials"] >> m_materialTable;
	for(auto &str : m_materialTable)
		m_nw.PrecacheMaterial(str);

	auto udmLightmap = udm["lightmap"];
	if(udmLightmap) {
		m_useLegacyLightmapDefinition = true;
		udm["lightmap"]["intensity"] >> m_lightMapIntensity;
		udm["lightmap"]["exposure"] >> m_lightMapExposure;
	}

	uint32_t nextEntIdx = 0;
	for(auto udmEnt : udm["entities"]) {
		auto entIdx = nextEntIdx++;
		auto entData = EntityData::Create();

		if(PMAP_VERSION > 1) {
			EntityData::Flags flags;
			if(udmEnt["flags"] >> flags)
				entData->SetFlags(flags);
		}
		else {
			if(udmEnt["flags"]) {
				auto udmClientsideOnly = udmEnt["flags"]["clientsideOnly"];
				if(udmClientsideOnly.ToValue<bool>(false))
					entData->SetFlags(EntityData::Flags::ClientsideOnly);
			}
		}

		if(entMask != EntityData::Flags::None && (entData->GetFlags() & entMask) == EntityData::Flags::None)
			continue;

		m_entities.push_back(entData);
		entData->m_mapIndex = entIdx + 1; // Map indices always start at 1!
		entData->SetClassName(udmEnt["className"].ToValue<std::string>(""));

		auto pose = udmEnt["pose"].ToValue<umath::ScaledTransform>(umath::ScaledTransform {});
		entData->SetPose(pose);

		auto &keyValues = entData->GetKeyValues();
		udmEnt["keyValues"] >> keyValues;

		auto itMdl = keyValues.find("model");
		if(itMdl != keyValues.end())
			m_nw.GetModelManager().PreloadAsset(itMdl->second);

		auto &outputs = entData->GetOutputs();
		auto udmOutputs = udmEnt["outputs"];
		outputs.reserve(udmOutputs.GetSize());
		for(auto udmOutput : udmOutputs) {
			outputs.push_back({});
			auto &output = outputs.back();
			udmOutput["name"](output.name);
			udmOutput["target"](output.target);
			udmOutput["input"](output.input);
			udmOutput["param"](output.param);
			udmOutput["delay"](output.delay);
			udmOutput["times"](output.times);
		}

		auto &components = entData->GetComponents();
		auto udmComponents = udmEnt["components"];
		components.reserve(udmComponents.GetSize());
		if(udm::is_array_type(udmComponents.GetType())) {
			for(auto udmComponent : udmComponents) {
				auto *typeName = udmComponent.GetValuePtr<udm::String>();
				if(typeName)
					entData->AddComponent(std::string {*typeName});
			}
		}
		else {
			for(auto pair : udmComponents.ElIt()) {
				auto name = pair.key;
				auto &udmComponent = pair.property;
				auto component = entData->AddComponent(std::string {name});

				auto flags = component->GetFlags();
				udmComponent["flags"] >> flags;
				component->SetFlags(flags);

				auto udmProperties = udmComponent["properties"];
				auto *elProperties = udmProperties->GetValuePtr<udm::Element>();
				if(elProperties) {
					auto udmData = component->GetData();
					udmData->GetValue<udm::Element>().Merge(*elProperties);
				}
			}
		}

		auto &leaves = entData->GetLeaves();
		udmEnt["bspLeaves"].GetBlobData(leaves);
	}

	auto udmBsp = udm["bsp"];
	if(udmBsp) {
		m_bspTree = util::BSPTree::Load(udm::AssetData {udmBsp["tree"]}, outErr);
		if(m_bspTree == nullptr)
			return false;

		auto numClusters = m_bspTree->GetClusterCount();
		std::vector<uint8_t> clusterMeshIndexData;
		udmBsp["clusterMeshIndexData"].GetBlobData(clusterMeshIndexData);
		auto &clusterMeshIndices = GetClusterMeshIndices();
		clusterMeshIndices.resize(numClusters);
		if(!clusterMeshIndexData.empty()) {
			auto *ptr = clusterMeshIndexData.data();
			uint32_t idx = 0;
			do {
				auto &numIndices = *reinterpret_cast<uint32_t *>(ptr);
				ptr += sizeof(numIndices);
				auto &meshIndices = clusterMeshIndices[idx++];
				meshIndices.resize(numIndices);
				memcpy(meshIndices.data(), ptr, numIndices * sizeof(meshIndices.front()));
				ptr += numIndices * sizeof(meshIndices.front());
			} while(ptr < &clusterMeshIndexData.back());
		}
	}
	return true;
}

bool pragma::asset::WorldData::LoadFromAssetData(const udm::AssetData &data, EntityData::Flags entMask, std::string &outErr)
{
	if(data.GetAssetType() != PMAP_IDENTIFIER) {
		outErr = "Incorrect format!";
		return false;
	}

	auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1) {
		outErr = "Invalid version!";
		return false;
	}

	return LoadFromAssetData(udm, entMask, outErr);
}

bool pragma::asset::WorldData::Save(const std::string &fileName, const std::string &mapName, std::string &outErr)
{
	auto udmData = udm::Data::Create();
	if(!udmData) {
		outErr = "Failed to create UDM data.";
		return false;
	}
	if(!Save(udmData->GetAssetData(), mapName, outErr))
		return false;

	auto normFileName = fileName;
	auto exts = get_supported_extensions();
	auto ext = ufile::remove_extension_from_filename(normFileName, exts);
	if(!ext)
		ext = PMAP_EXTENSION_BINARY;

	auto asciiFormat = ustring::compare(*ext, std::string {PMAP_EXTENSION_ASCII}, false);
	auto filePath = util::Path::CreateFile(fileName);
	auto fileMode = filemanager::FileMode::Write;
	if(!asciiFormat)
		fileMode |= filemanager::FileMode::Binary;
	auto absFilePath = filePath.GetString();
	filemanager::find_local_path(absFilePath, absFilePath);
	auto f = filemanager::open_file(absFilePath, fileMode);
	if(!f) {
		outErr = "Failed to open file '" + filePath.GetString() + "' for writing.";
		return false;
	}

	auto res = false;
	if(asciiFormat)
		res = udmData->SaveAscii(f, udm::AsciiSaveFlags::IncludeHeader);
	else
		res = udmData->Save(f);
	f = nullptr;
	if(res == false) {
		outErr = "Failed to save world data.";
		return false;
	}
	return true;
}

bool pragma::asset::WorldData::Save(udm::AssetDataArg outData, const std::string &mapName, std::string &outErr)
{
	outData.SetAssetType(PMAP_IDENTIFIER);
	outData.SetAssetVersion(PMAP_VERSION);
	auto udm = *outData;

	// Materials
	std::vector<std::string> normalizedMaterials;
	normalizedMaterials.reserve(m_materialTable.size());
	for(auto &str : m_materialTable) {
		util::Path path {str};
		if(ustring::compare<std::string_view>(path.GetFront(), "materials", false))
			path.PopFront();

		auto strPath = path.GetString();
		ustring::to_lower(strPath);
		normalizedMaterials.push_back(strPath);
	}
	udm["materials"] << normalizedMaterials;

	// Entities
	auto udmEntities = udm.AddArray("entities", m_entities.size());
	uint32_t entIdx = 0;
	for(auto &entData : m_entities) {
		auto udmEnt = udmEntities[entIdx++];
		udmEnt["className"] << entData->GetClassName();

		if(PMAP_VERSION > 1)
			udmEnt["flags"] << entData->GetFlags();
		else {
			if(umath::is_flag_set(entData->GetFlags(), EntityData::Flags::ClientsideOnly))
				udmEnt["flags"]["clientsideOnly"] << true;
		}

		auto &pose = entData->GetPose();
		if(pose)
			udmEnt["pose"] << *pose;
		udmEnt["keyValues"] << entData->GetKeyValues();

		auto &outputs = entData->GetOutputs();
		auto udmOutputs = udmEnt.AddArray("outputs", outputs.size());
		uint32_t outputIdx = 0;
		for(auto &output : outputs) {
			auto udmOutput = udmOutputs[outputIdx++];
			udmOutput["name"] << output.name;
			udmOutput["target"] << output.target;
			udmOutput["input"] << output.input;
			udmOutput["param"] << output.param;
			udmOutput["delay"] << output.delay;
			udmOutput["times"] << output.times;
		}

		auto udmComponents = udmEnt["components"];
		for(auto &pair : entData->GetComponents()) {
			if(pair.first.empty()) {
				outErr = "Encountered empty component name!";
				return false;
			}
			auto &componentData = *pair.second;
			auto udmComponent = udmComponents[pair.first];
			udmComponent["flags"] << componentData.GetFlags();
			auto udmProperties = udmComponent.Add("properties");
			udmProperties.Merge(udm::LinkedPropertyWrapper {*componentData.GetData()});
		}

		uint32_t firstLeaf, numLeaves;
		entData->GetLeafData(firstLeaf, numLeaves);
		if(numLeaves > 0) {
			std::vector<uint16_t> leaves {};
			leaves.resize(numLeaves);
			if(numLeaves > 0u)
				memcpy(leaves.data(), GetStaticPropLeaves().data() + firstLeaf, leaves.size() * sizeof(leaves.front()));
			udmEnt["bspLeaves"] << leaves;
		}
	}

	if(m_lightMapAtlasEnabled) {
		auto strMapName = mapName;
		ufile::remove_extension_from_filename(strMapName); // TODO: Specify extensions
		ustring::to_lower(strMapName);
		SaveLightmapAtlas(strMapName);
		udm["lightmap"]["intensity"] << m_lightMapIntensity;
		udm["lightmap"]["exposure"] << m_lightMapExposure;
	}

	if(m_bspTree && m_bspTree->GetNodes().empty() == false && m_bspTree->GetClusterCount() > 0) {
		auto udmBsp = udm["bsp"];
		auto &bspTree = *m_bspTree;

		if(bspTree.Save(udm::AssetData {udmBsp["tree"]}, outErr) == false)
			return false;

		std::vector<std::vector<size_t>> clusterNodes;
		std::vector<std::vector<uint16_t>> clusterToClusterVisibility;
		preprocess_bsp_data(bspTree, clusterNodes, clusterToClusterVisibility);

		auto &clusterMeshIndices = GetClusterMeshIndices();
		if(clusterMeshIndices.empty() == false) {
			assert(clusterMeshIndices.size() == m_bspTree->GetClusterCount());
			if(clusterMeshIndices.size() != m_bspTree->GetClusterCount()) {
				m_messageLogger("Error: Number of items in cluster mesh list mismatches number of BSP tree clusters!");
				return false;
			}

			auto numClusters = m_bspTree->GetClusterCount();
			uint32_t numIndices = 0;
			for(auto &meshIndices : clusterMeshIndices)
				numIndices += meshIndices.size();
			std::vector<uint8_t> clusterData {};
			clusterData.resize(numClusters * sizeof(uint32_t) + numIndices * sizeof(WorldModelMeshIndex));
			auto *ptr = clusterData.data();
			for(auto &meshIndices : clusterMeshIndices) {
				uint32_t numIndices = meshIndices.size();
				memcpy(ptr, &numIndices, sizeof(numIndices));
				ptr += sizeof(numIndices);
				memcpy(ptr, meshIndices.data(), meshIndices.size() * sizeof(meshIndices.front()));
				ptr += meshIndices.size() * sizeof(meshIndices.front());
			}
			udmBsp["clusterMeshIndexData"] << udm::compress_lz4_blob(clusterData);
		}
	}
	return true;
}

void pragma::asset::WorldData::Write(VFilePtrReal &f)
{
	auto mapName = util::Path::CreateFile(f->GetPath());
	while(ustring::compare<std::string_view>(mapName.GetFront(), "maps", false) == false)
		mapName.PopFront();
	mapName.PopFront(); // Pop "maps"
	auto strMapName = mapName.GetString();
	ufile::remove_extension_from_filename(strMapName);
	ustring::to_lower(strMapName);

	const std::array<char, 3> header = {'W', 'L', 'D'};
	f->Write(header.data(), header.size());

	f->Write<uint32_t>(WLD_VERSION);
	static_assert(WLD_VERSION == 12);
	auto offsetToDataFlags = f->Tell();
	f->Write<DataFlags>(DataFlags::None);
	auto offsetMaterials = f->Tell();
	f->Write<uint64_t>(0ull);
	auto offsetBSPTree = f->Tell();
	f->Write<uint64_t>(0ull);
	auto offsetLightMapData = f->Tell();
	f->Write<uint64_t>(0ull);
	auto offsetEntities = f->Tell();
	f->Write<uint64_t>(0ull);

	auto flags = DataFlags::None;
	m_messageLogger("Writing materials...");
	WriteDataOffset(f, offsetMaterials);
	WriteMaterials(f);

	m_messageLogger("Writing BSP Tree...");
	if(m_bspTree && m_bspTree->GetNodes().empty() == false && m_bspTree->GetClusterCount() > 0) {
		WriteDataOffset(f, offsetBSPTree);
		flags |= DataFlags::HasBSPTree;
		WriteBSPTree(f);

		auto &clusterMeshIndices = GetClusterMeshIndices();
		f->Write<bool>(!clusterMeshIndices.empty());
		if(clusterMeshIndices.empty() == false) {
			assert(clusterMeshIndices.size() == m_bspTree->GetClusterCount());
			if(clusterMeshIndices.size() != m_bspTree->GetClusterCount()) {
				m_messageLogger("Error: Number of items in cluster mesh list mismatches number of BSP tree clusters!");
				return;
			}

			auto numClusters = m_bspTree->GetClusterCount();
			for(auto i = decltype(numClusters) {0u}; i < numClusters; ++i) {
				auto &meshIndices = clusterMeshIndices.at(i);
				f->Write<uint32_t>(meshIndices.size());
				f->Write(meshIndices.data(), meshIndices.size() * sizeof(meshIndices.front()));
			}
		}
	}

	m_messageLogger("Saving lightmap atlas...");
	SaveLightmapAtlas(strMapName);
	if(m_lightMapAtlasEnabled)
		flags |= DataFlags::HasLightmapAtlas;
	if(m_lightMapAtlasEnabled) {
		WriteDataOffset(f, offsetLightMapData);
		f->Write<float>(m_lightMapIntensity);
		f->Write<float>(m_lightMapExposure);
	}

	m_messageLogger("Writing entity data...");
	WriteDataOffset(f, offsetEntities);
	WriteEntities(f);

	// Update flags
	auto cur = f->Tell();
	f->Seek(offsetToDataFlags);
	f->Write<DataFlags>(flags);
	f->Seek(cur);

	m_messageLogger("Done!");
	m_messageLogger("All operations are complete!");
}

void pragma::asset::WorldData::WriteMaterials(VFilePtrReal &f)
{
	f->Write<uint32_t>(m_materialTable.size());
	for(auto &str : m_materialTable) {
		util::Path path {str};
		if(ustring::compare<std::string_view>(path.GetFront(), "materials", false))
			path.PopFront();

		auto strPath = path.GetString();
		ustring::to_lower(strPath);
		f->WriteString(strPath);
	}
}

void pragma::asset::WorldData::WriteBSPTree(VFilePtrReal &f)
{
	auto &bspTree = *m_bspTree;
	std::vector<std::vector<size_t>> clusterNodes;
	std::vector<std::vector<uint16_t>> clusterToClusterVisibility;
	preprocess_bsp_data(bspTree, clusterNodes, clusterToClusterVisibility);

	auto &bspNodes = bspTree.GetNodes();
	auto numClusters = bspTree.GetClusterCount();
	auto &clusterVisibility = bspTree.GetClusterVisibility();
	std::function<void(const util::BSPTree::Node &)> fWriteNode = nullptr;
	fWriteNode = [&f, &fWriteNode, &clusterVisibility, &clusterToClusterVisibility, &bspNodes, &clusterNodes, &bspTree, numClusters](const util::BSPTree::Node &node) {
		f->Write<bool>(node.leaf);
		f->Write<Vector3>(node.min);
		f->Write<Vector3>(node.max);
		f->Write<int32_t>(node.firstFace);
		f->Write<int32_t>(node.numFaces);
		f->Write<int32_t>(node.originalNodeIndex);
		if(node.leaf) {
			f->Write<uint16_t>(node.cluster);
			auto itNode = std::find_if(bspNodes.begin(), bspNodes.end(), [&node](const util::BSPTree::Node &nodeOther) { return &nodeOther == &node; });
			// Calculate AABB encompassing all nodes visible by this node
			auto min = node.min;
			auto max = node.max;
			if(itNode != bspNodes.end() && node.cluster != std::numeric_limits<uint16_t>::max()) {
				for(auto clusterDst : clusterToClusterVisibility.at(node.cluster)) {
					for(auto nodeOtherIdx : clusterNodes.at(clusterDst)) {
						auto &nodeOther = bspNodes.at(nodeOtherIdx);
						uvec::to_min_max(min, max, nodeOther.min, nodeOther.max);
					}
				}
			}
			uvec::to_min_max(min, max); // Vertex conversion rotates the vectors, which will change the signs, so we have to re-order the vector components
			f->Write<Vector3>(min);
			f->Write<Vector3>(max);
			return;
		}
		f->Write<Vector3>(node.plane.GetNormal());
		f->Write<float>(node.plane.GetDistance());

		fWriteNode(bspNodes[node.children.at(0)]);
		fWriteNode(bspNodes[node.children.at(1)]);
	};
	fWriteNode(bspTree.GetRootNode());

	f->Write<uint64_t>(bspTree.GetClusterCount());
	f->Write(clusterVisibility.data(), clusterVisibility.size() * sizeof(clusterVisibility.front()));
}

void pragma::asset::WorldData::WriteEntities(VFilePtrReal &f)
{
	f->Write<uint32_t>(m_entities.size());

	for(auto &entData : m_entities) {
		auto offsetEndOfEntity = f->Tell();
		f->Write<uint64_t>(0u); // Offset to end of entity
		auto offsetEntityMeshes = f->Tell();
		f->Write<uint64_t>(0u); // Offset to entity meshes
		auto offsetEntityLeaves = f->Tell();
		f->Write<uint64_t>(0u); // Offset to entity leaves

		f->Write<uint64_t>(umath::to_integral(entData->GetFlags()));

		f->WriteString(entData->GetClassName());
		auto &pose = entData->GetPose();
		f->Write<Vector3>(pose ? pose->GetOrigin() : uvec::ORIGIN);

		// Keyvalues
		auto &keyValues = entData->GetKeyValues();
		f->Write<uint32_t>(keyValues.size());
		for(auto &pair : keyValues) {
			f->WriteString(pair.first);
			f->WriteString(pair.second);
		}

		// Outputs
		auto &outputs = entData->GetOutputs();
		f->Write<uint32_t>(outputs.size());
		for(auto &output : outputs)
			output.Write(f);

		// Custom Components
		auto &components = entData->GetComponents();
		f->Write<uint32_t>(components.size());
		for(auto &pair : components)
			f->WriteString(pair.first);

		// Leaves
		auto cur = f->Tell();
		f->Seek(offsetEntityLeaves);
		f->Write<uint64_t>(cur - offsetEntityLeaves);
		f->Seek(cur);

		uint32_t firstLeaf, numLeaves;
		entData->GetLeafData(firstLeaf, numLeaves);
		f->Write<uint32_t>(numLeaves);
		std::vector<uint16_t> leaves {};
		leaves.resize(numLeaves);
		if(numLeaves > 0u)
			memcpy(leaves.data(), GetStaticPropLeaves().data() + firstLeaf, leaves.size() * sizeof(leaves.front()));
		f->Write(leaves.data(), leaves.size() * sizeof(leaves.front()));

		cur = f->Tell();
		f->Seek(offsetEndOfEntity);
		f->Write<uint64_t>(cur - offsetEndOfEntity);
		f->Seek(cur);
	}
}

bool pragma::asset::WorldData::SaveLightmapAtlas(const std::string &mapName)
{
	if(m_lightMapAtlas == nullptr) {
		m_messageLogger("No lightmap atlas has been specified! Lightmaps will not be available.");
		return false;
	}
	// Build lightmap texture
	uimg::TextureSaveInfo saveInfo {};
	auto &texInfo = saveInfo.texInfo;
	texInfo.containerFormat = uimg::TextureInfo::ContainerFormat::DDS;
	// texInfo.flags = uimg::TextureInfo::Flags::SRGB;
	texInfo.inputFormat = uimg::TextureInfo::InputFormat::R16G16B16A16_Float;
	texInfo.outputFormat = uimg::TextureInfo::OutputFormat::HDRColorMap;
	auto matPath = "materials/maps/" + mapName;
	FileManager::CreatePath(matPath.c_str());
	auto filePath = matPath + "/lightmap_atlas.dds";
	auto result = uimg::save_texture(filePath, *m_lightMapAtlas, saveInfo, [](const std::string &msg) { Con::cwar << "Unable to save lightmap atlas: " << msg << Con::endl; });
	if(result)
		m_messageLogger("Lightmap atlas has been saved as '" + filePath + "'!");
	else
		m_messageLogger("Lightmap atlas could not be saved as '" + filePath + "'! Lightmaps will not be available.");
	return result;
}
