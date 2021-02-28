/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/asset_types/world.hpp"
#include "pragma/level/level_info.hpp"
#include <util_image.hpp>
#include <util_texture_info.hpp>
#include <sharedutils/util_file.h>
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

void pragma::asset::WorldData::WriteDataOffset(VFilePtrReal &f,uint64_t offsetToOffset)
{
	auto offset = f->Tell();
	f->Seek(offsetToOffset);
	f->Write<uint64_t>(offset);
	f->Seek(offset);
}

bool pragma::asset::WorldData::Write(const std::string &fileName,std::string *optOutErrMsg)
{
	auto nFileName = fileName;
	ufile::remove_extension_from_filename(nFileName);
	nFileName += ".wld";
	auto fullPath = util::CONVERT_PATH +nFileName;
	auto pathNoFile = ufile::get_path_from_filename(fullPath);
	if(FileManager::CreatePath(pathNoFile.c_str()) == false)
	{
		if(optOutErrMsg)
			*optOutErrMsg = "Unable to create path '" +pathNoFile +"'!";
		return false;
	}
	auto fOut = FileManager::OpenFile<VFilePtrReal>(fullPath.c_str(),"wb");
	if(fOut == nullptr)
	{
		if(optOutErrMsg)
			*optOutErrMsg = "Unable to write file '" +nFileName +"'!";
		return false;
	}
	Write(fOut);
	return true;
}

static void preprocess_bsp_data(util::BSPTree &bspTree,std::vector<std::vector<size_t>> &outClusterNodes,std::vector<std::vector<uint16_t>> &outClusterToClusterVisibility)
{
	auto numClusters = bspTree.GetClusterCount();
	auto &bspNodes = bspTree.GetNodes();

	// Pre-processing to speed up some calculations
	{
		// Nodes per cluster
		outClusterNodes.resize(numClusters);
		for(auto i=decltype(bspNodes.size()){0u};i<bspNodes.size();++i)
		{
			auto &bspNode = bspNodes.at(i);
			if(bspNode->cluster >= outClusterNodes.size())
				continue;
			auto &nodeList = outClusterNodes.at(bspNode->cluster);
			if(nodeList.size() == nodeList.capacity())
				nodeList.reserve(nodeList.size() *1.5f +100);
			nodeList.push_back(i);
		}

		// List of clusters visible from every other cluster
		outClusterToClusterVisibility.resize(numClusters);
		for(auto cluster0=decltype(numClusters){0u};cluster0<numClusters;++cluster0)
		{
			auto &visibleClusters = outClusterToClusterVisibility.at(cluster0);
			for(auto cluster1=decltype(numClusters){0u};cluster1<numClusters;++cluster1)
			{
				if(bspTree.IsClusterVisible(cluster0,cluster1) == false)
					continue;
				if(visibleClusters.size() == visibleClusters.capacity())
					visibleClusters.reserve(visibleClusters.size() *1.5f +50);
				visibleClusters.push_back(cluster1);
			}
		}
	}
	//
}

bool pragma::asset::WorldData::LoadFromAssetData(const udm::AssetData &data,EntityData::Flags entMask,std::string &outErr)
{
	if(data.GetAssetType() != PMAP_IDENTIFIER)
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

	m_materialTable = udm["materials"](m_materialTable);
	std::vector<MaterialHandle> materials {};
	materials.reserve(m_materialTable.size());
	for(auto &str : m_materialTable)
	{
		auto *mat = m_nw.LoadMaterial(str);
		materials.push_back(mat ? mat->GetHandle() : MaterialHandle{});
	}

	auto udmLightmap = udm["lightmap"];
	if(udmLightmap)
	{
		m_lightMapIntensity = udm["lightmap.intensity"](m_lightMapIntensity);
		m_lightMapExposure = udm["lightmap.exposure"](m_lightMapExposure);
	}

	uint32_t nextEntIdx = 0;
	for(auto udmEnt : udm["entities"])
	{
		auto entIdx = nextEntIdx++;
		auto entData = EntityData::Create();

		auto udmClientsideOnly = udmEnt["flags.clientsideOnly"];
		if(udmClientsideOnly.ToValue<bool>(false))
			entData->SetFlags(EntityData::Flags::ClientsideOnly);

		if(entMask != EntityData::Flags::None && (entData->GetFlags() &entMask) == EntityData::Flags::None)
			continue;

		m_entities.push_back(entData);
		entData->m_mapIndex = entIdx +1; // Map indices always start at 1!
		entData->SetClassName(udmEnt["className"].ToValue<std::string>(""));

		auto pose = udmEnt["pose"].ToValue<umath::Transform>(umath::Transform{});
		entData->SetOrigin(pose.GetOrigin());

		auto &keyValues = entData->GetKeyValues();
		keyValues = udmEnt["keyValues"](keyValues);
		
		auto &outputs = entData->GetOutputs();
		auto udmOutputs = udmEnt["outputs"];
		outputs.reserve(udmOutputs.GetSize());
		for(auto udmOutput : udmOutputs)
		{
			outputs.push_back({});
			auto &output = outputs.back();
			output.name = udmOutput["name"](output.name);
			output.target = udmOutput["target"](output.target);
			output.input = udmOutput["input"](output.input);
			output.param = udmOutput["param"](output.param);
			output.delay = udmOutput["delay"](output.delay);
			output.times = udmOutput["times"](output.times);
		}
		
		auto &components = entData->GetComponents();
		components = udmEnt["components"](components);
		
		auto &leaves = entData->GetLeaves();
		udmEnt["bspLeaves"].GetBlobData(leaves);
	}

	auto udmBsp = udm["bsp"];
	if(udmBsp)
	{
		auto udmRootNode = udmBsp["rootNode"];
		if(udmRootNode)
		{
			std::function<void(util::BSPTree::Node&,udm::LinkedPropertyWrapper&)> fReadNode = nullptr;
			fReadNode = [this,&fReadNode](util::BSPTree::Node &node,udm::LinkedPropertyWrapper &udm) {
				node.leaf = udm["leaf"](node.leaf);
				node.min = udm["bounds.min"](node.min);
				node.max = udm["bounds.max"](node.max);
				node.firstFace = udm["firstFace"](node.firstFace);
				node.numFaces = udm["numFaces"](node.numFaces);
				node.originalNodeIndex = udm["originalNodeIndex"](node.originalNodeIndex);

				if(node.leaf)
				{
					node.cluster = udm["cluster"](node.cluster);
					node.minVisible = udm["clusterBounds.min"](node.minVisible);
					node.maxVisible = udm["clusterBounds.max"](node.maxVisible);
					return;
				}

				node.plane = umath::Plane{udm["plane.normal"](uvec::FORWARD),udm["plane.distance"](0.f)};
				
				node.children.at(0) = m_bspTree->CreateNode();
				node.children.at(1) = m_bspTree->CreateNode();
				auto udmLeft = udm["leftChild"];
				if(udmLeft)
					fReadNode(*node.children.at(0),udmLeft);
				auto udmRight = udm["rightChild"];
				if(udmRight)
					fReadNode(*node.children.at(1),udmRight);
			};
			fReadNode(m_bspTree->GetRootNode(),udmRootNode);
		}

		auto &clusterVisibility = m_bspTree->GetClusterVisibility();
		udmBsp["clusterVisibility"].GetBlobData(clusterVisibility);

		auto numClusters = udmBsp["clusters"].ToValue<uint32_t>(0);
		std::vector<uint8_t> clusterMeshIndexData;
		udmBsp["clusterMeshIndexData"].GetBlobData(clusterMeshIndexData);
		auto &clusterMeshIndices = GetClusterMeshIndices();
		clusterMeshIndices.resize(numClusters);
		if(!clusterMeshIndexData.empty())
		{
			auto *ptr = clusterMeshIndexData.data();
			uint32_t idx = 0;
			do
			{
				auto &numIndices = *reinterpret_cast<uint32_t*>(ptr);
				ptr += sizeof(numIndices);
				auto &meshIndices = clusterMeshIndices[idx++];
				meshIndices.resize(numIndices);
				memcpy(meshIndices.data(),ptr,numIndices *sizeof(meshIndices.front()));
				ptr += numIndices *sizeof(meshIndices.front());
			}
			while(ptr < &clusterMeshIndexData.back());
		}
	}

	if(m_bspTree && m_bspTree->GetNodes().empty() == false && m_bspTree->GetClusterCount() > 0)
	{
		auto udmBspTree = udm["bsp"];
		auto &bspTree = *m_bspTree;
		std::vector<std::vector<size_t>> clusterNodes;
		std::vector<std::vector<uint16_t>> clusterToClusterVisibility;
		preprocess_bsp_data(bspTree,clusterNodes,clusterToClusterVisibility);
	
		auto &bspNodes = bspTree.GetNodes();
		auto numClusters = bspTree.GetClusterCount();
		auto &clusterVisibility = bspTree.GetClusterVisibility();
		std::function<void(const util::BSPTree::Node&,udm::LinkedPropertyWrapper&)> fWriteNode = nullptr;
		fWriteNode = [&fWriteNode,&clusterVisibility,&clusterToClusterVisibility,&bspNodes,&clusterNodes,&bspTree,numClusters](const util::BSPTree::Node &node,udm::LinkedPropertyWrapper &udm) {
			udm["leaf"] = node.leaf;
			udm["bounds.min"] = node.min;
			udm["bounds.max"] = node.max;
			udm["firstFace"] = node.firstFace;
			udm["numFaces"] = node.numFaces;
			udm["originalNodeIndex"] = node.originalNodeIndex;

			if(node.leaf)
			{
				udm["cluster"] = node.cluster;
				auto itNode = std::find_if(bspNodes.begin(),bspNodes.end(),[&node](const std::shared_ptr<util::BSPTree::Node> &nodeOther) {
					return nodeOther.get() == &node;
				});
				// Calculate AABB encompassing all nodes visible by this node
				auto min = node.min;
				auto max = node.max;
				if(itNode != bspNodes.end() && node.cluster != std::numeric_limits<uint16_t>::max())
				{
					for(auto clusterDst : clusterToClusterVisibility.at(node.cluster))
					{
						for(auto nodeOtherIdx : clusterNodes.at(clusterDst))
						{
							auto &nodeOther = bspNodes.at(nodeOtherIdx);
							uvec::to_min_max(min,max,nodeOther->min,nodeOther->max);
						}
					}
				}
				uvec::to_min_max(min,max); // Vertex conversion rotates the vectors, which will change the signs, so we have to re-order the vector components
				udm["clusterBounds.min"] = min;
				udm["clusterBounds.max"] = max;
				return;
			}

			udm["plane.normal"] = node.plane.GetNormal();
			udm["plane.distance"] = node.plane.GetDistance();

			auto udmLeft = udm["leftChild"];
			auto udmRight = udm["rightChild"];
			fWriteNode(*node.children.at(0),udmLeft);
			fWriteNode(*node.children.at(1),udmRight);
		};
		auto udmRootNode = udmBspTree["rootNode"];
		fWriteNode(bspTree.GetRootNode(),udmRootNode);

		udmBspTree["clusterVisibility"] = udm::compress_lz4_blob(clusterVisibility);

		auto &clusterMeshIndices = GetClusterMeshIndices();
		if(clusterMeshIndices.empty() == false)
		{
			assert(clusterMeshIndices.size() == m_bspTree->GetClusterCount());
			if(clusterMeshIndices.size() != m_bspTree->GetClusterCount())
			{
				m_messageLogger("Error: Number of items in cluster mesh list mismatches number of BSP tree clusters!");
				return false;
			}

			auto numClusters = m_bspTree->GetClusterCount();
			uint32_t numIndices = 0;
			for(auto &meshIndices : clusterMeshIndices)
				numIndices += meshIndices.size();
			std::vector<uint8_t> clusterData {};
			clusterData.resize(numClusters *sizeof(uint32_t) +numIndices *sizeof(WorldModelMeshIndex));
			auto *ptr = clusterData.data();
			for(auto &meshIndices : clusterMeshIndices)
			{
				uint32_t numIndices = meshIndices.size();
				memcpy(ptr,&numIndices,sizeof(numIndices));
				ptr += sizeof(numIndices);
				memcpy(ptr,meshIndices.data(),meshIndices.size() *sizeof(meshIndices.front()));
				ptr += meshIndices.size() *sizeof(meshIndices.front());
			}
			udmBspTree["clusters"] = numClusters;
			udmBspTree["clusterMeshIndexData"] = udm::compress_lz4_blob(clusterData);
		}
	}

	return true;
}

bool pragma::asset::WorldData::Save(udm::AssetData &outData,const std::string &mapName,std::string &outErr)
{
	outData.SetAssetType(PMAP_IDENTIFIER);
	outData.SetAssetVersion(PMAP_VERSION);
	auto udm = *outData;
	
	// Materials
	std::vector<std::string> normalizedMaterials;
	normalizedMaterials.reserve(m_materialTable.size());
	for(auto &str : m_materialTable)
	{
		util::Path path{str};
		if(ustring::compare(path.GetFront(),"materials",false))
			path.PopFront();

		auto strPath = path.GetString();
		ustring::to_lower(strPath);
		normalizedMaterials.push_back(strPath);
	}
	udm["materials"] = normalizedMaterials;

	// Entities
	auto udmEntities = udm.AddArray("entities",m_entities.size());
	uint32_t entIdx = 0;
	for(auto &entData : m_entities)
	{
		auto udmEnt = udmEntities[entIdx++];
		udmEnt["className"] = entData->GetClassName();

		if(umath::is_flag_set(entData->GetFlags(),EntityData::Flags::ClientsideOnly))
			udmEnt["flags.clientsideOnly"] = true;

		umath::ScaledTransform pose {};
		pose.SetOrigin(entData->GetOrigin());
		udmEnt["pose"] = pose;
		udmEnt["keyValues"] = entData->GetKeyValues();

		auto &outputs = entData->GetOutputs();
		auto udmOutputs = udmEnt.AddArray("outputs",outputs.size());
		uint32_t outputIdx = 0;
		for(auto &output : outputs)
		{
			auto udmOutput = udmOutputs[outputIdx++];
			udmOutput["name"] = output.name;
			udmOutput["target"] = output.target;
			udmOutput["input"] = output.input;
			udmOutput["param"] = output.param;
			udmOutput["delay"] = output.delay;
			udmOutput["times"] = output.times;
		}

		udmEnt["components"] = entData->GetComponents();

		uint32_t firstLeaf,numLeaves;
		entData->GetLeafData(firstLeaf,numLeaves);
		if(numLeaves > 0)
		{
			std::vector<uint16_t> leaves {};
			leaves.resize(numLeaves);
			if(numLeaves > 0u)
				memcpy(leaves.data(),GetStaticPropLeaves().data() +firstLeaf,leaves.size() *sizeof(leaves.front()));
			udmEnt["bspLeaves"] = leaves;
		}
	}

	if(m_lightMapAtlasEnabled)
	{
		auto strMapName = mapName;
		ufile::remove_extension_from_filename(strMapName); // TODO: Specify extensions
		ustring::to_lower(strMapName);
		SaveLightmapAtlas(strMapName);
		udm["lightmap.intensity"] = m_lightMapIntensity;
		udm["lightmap.exposure"] = m_lightMapExposure;
	}

	if(m_bspTree && m_bspTree->GetNodes().empty() == false && m_bspTree->GetClusterCount() > 0)
	{
		auto udmBspTree = udm["bsp"];
		auto &bspTree = *m_bspTree;
		std::vector<std::vector<size_t>> clusterNodes;
		std::vector<std::vector<uint16_t>> clusterToClusterVisibility;
		preprocess_bsp_data(bspTree,clusterNodes,clusterToClusterVisibility);
	
		// TODO: This wastes a lot of space due to the large amount of nodes that most maps have;
		// Store the data as LZ4 compressed data streams instead!
		// TODO: Store key string table for keyvalues?

		// TODO
#if 0
		struct BaseNode
		{
			Vector3 min = {};
			Vector3 max = {};
			int32_t originalNodeIndex = -1;
			int32_t firstFace = 0u;
			int32_t numFaces = 0u;
		};

		struct LeafNode
			: public BaseNode
		{
			util::BSPTree::ClusterIndex cluster = std::numeric_limits<util::BSPTree::ClusterIndex>::max();
			Vector3 minVisible = {};
			Vector3 maxVisible = {};
		};

		struct ParentNode
			: public BaseNode
		{
			std::array<LeafNode,2> children;
			umath::Plane plane = {};
		};

		std::vector<LeafNode> leafNodes;
		std::vector<ParentNode> parentNodes;

		auto &bspNodes = bspTree.GetNodes();
		leafNodes.reserve(bspNodes.size());
		parentNodes.reserve(bspNodes.size());
		for(auto &node : bspNodes)
		{
			BaseNode *dnode = nullptr;
			if(node->leaf)
			{
				leafNodes.push_back({});
				auto &leafNode = leafNodes.back();
				leafNode.cluster = node->cluster;
				leafNode.minVisible = node->minVisible;
				leafNode.maxVisible = node->maxVisible;
				dnode = &leafNode;
			}
			else
			{
				parentNodes.push_back({});
				auto &parentNode = parentNodes.back();
				parentNode.plane = node->plane;
				parentNode.children = node->children; // ??
				dnode = &parentNode;
			}
			dnode->min = node->min;
			dnode->max = node->max;
			dnode->originalNodeIndex = node->originalNodeIndex;
			dnode->numFaces = node->numFaces;
		}

		auto numClusters = bspTree.GetClusterCount();
		auto &clusterVisibility = bspTree.GetClusterVisibility();
		std::function<void(const util::BSPTree::Node&,udm::LinkedPropertyWrapper&)> fWriteNode = nullptr;
		fWriteNode = [&fWriteNode,&clusterVisibility,&clusterToClusterVisibility,&bspNodes,&clusterNodes,&bspTree,numClusters](const util::BSPTree::Node &node,udm::LinkedPropertyWrapper &udm) {
			udm["leaf"] = node.leaf;
			udm["bounds.min"] = node.min;
			udm["bounds.max"] = node.max;
			udm["firstFace"] = node.firstFace;
			udm["numFaces"] = node.numFaces;
			udm["originalNodeIndex"] = node.originalNodeIndex;

			if(node.leaf)
			{
				udm["cluster"] = node.cluster;
				auto itNode = std::find_if(bspNodes.begin(),bspNodes.end(),[&node](const std::shared_ptr<util::BSPTree::Node> &nodeOther) {
					return nodeOther.get() == &node;
				});
				// Calculate AABB encompassing all nodes visible by this node
				auto min = node.min;
				auto max = node.max;
				if(itNode != bspNodes.end() && node.cluster != std::numeric_limits<uint16_t>::max())
				{
					for(auto clusterDst : clusterToClusterVisibility.at(node.cluster))
					{
						for(auto nodeOtherIdx : clusterNodes.at(clusterDst))
						{
							auto &nodeOther = bspNodes.at(nodeOtherIdx);
							uvec::to_min_max(min,max,nodeOther->min,nodeOther->max);
						}
					}
				}
				uvec::to_min_max(min,max); // Vertex conversion rotates the vectors, which will change the signs, so we have to re-order the vector components
				udm["clusterBounds.min"] = min;
				udm["clusterBounds.max"] = max;
				return;
			}

			udm["plane.normal"] = node.plane.GetNormal();
			udm["plane.distance"] = node.plane.GetDistance();

			auto udmLeft = udm["leftChild"];
			auto udmRight = udm["rightChild"];
			fWriteNode(*node.children.at(0),udmLeft);
			fWriteNode(*node.children.at(1),udmRight);
		};
		auto udmRootNode = udmBspTree["rootNode"];
		fWriteNode(bspTree.GetRootNode(),udmRootNode);

		udmBspTree["clusterVisibility"] = udm::compress_lz4_blob(clusterVisibility);
#endif

		auto &clusterMeshIndices = GetClusterMeshIndices();
		if(clusterMeshIndices.empty() == false)
		{
			assert(clusterMeshIndices.size() == m_bspTree->GetClusterCount());
			if(clusterMeshIndices.size() != m_bspTree->GetClusterCount())
			{
				m_messageLogger("Error: Number of items in cluster mesh list mismatches number of BSP tree clusters!");
				return false;
			}

			auto numClusters = m_bspTree->GetClusterCount();
			uint32_t numIndices = 0;
			for(auto &meshIndices : clusterMeshIndices)
				numIndices += meshIndices.size();
			std::vector<uint8_t> clusterData {};
			clusterData.resize(numClusters *sizeof(uint32_t) +numIndices *sizeof(WorldModelMeshIndex));
			auto *ptr = clusterData.data();
			for(auto &meshIndices : clusterMeshIndices)
			{
				uint32_t numIndices = meshIndices.size();
				memcpy(ptr,&numIndices,sizeof(numIndices));
				ptr += sizeof(numIndices);
				memcpy(ptr,meshIndices.data(),meshIndices.size() *sizeof(meshIndices.front()));
				ptr += meshIndices.size() *sizeof(meshIndices.front());
			}
			udmBspTree["clusters"] = numClusters;
			udmBspTree["clusterMeshIndexData"] = udm::compress_lz4_blob(clusterData);
		}
	}
	return true;
}

void pragma::asset::WorldData::Write(VFilePtrReal &f)
{
	auto mapName = util::Path::CreateFile(f->GetPath());
	while(ustring::compare(mapName.GetFront(),"maps",false) == false)
		mapName.PopFront();
	mapName.PopFront(); // Pop "maps"
	auto strMapName = mapName.GetString();
	ufile::remove_extension_from_filename(strMapName);
	ustring::to_lower(strMapName);

	const std::array<char,3> header = {'W','L','D'};
	f->Write(header.data(),header.size());

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
	WriteDataOffset(f,offsetMaterials);
	WriteMaterials(f);

	m_messageLogger("Writing BSP Tree...");
	if(m_bspTree && m_bspTree->GetNodes().empty() == false && m_bspTree->GetClusterCount() > 0)
	{
		WriteDataOffset(f,offsetBSPTree);
		flags |= DataFlags::HasBSPTree;
		WriteBSPTree(f);

		auto &clusterMeshIndices = GetClusterMeshIndices();
		f->Write<bool>(!clusterMeshIndices.empty());
		if(clusterMeshIndices.empty() == false)
		{
			assert(clusterMeshIndices.size() == m_bspTree->GetClusterCount());
			if(clusterMeshIndices.size() != m_bspTree->GetClusterCount())
			{
				m_messageLogger("Error: Number of items in cluster mesh list mismatches number of BSP tree clusters!");
				return;
			}

			auto numClusters = m_bspTree->GetClusterCount();
			for(auto i=decltype(numClusters){0u};i<numClusters;++i)
			{
				auto &meshIndices = clusterMeshIndices.at(i);
				f->Write<uint32_t>(meshIndices.size());
				f->Write(meshIndices.data(),meshIndices.size() *sizeof(meshIndices.front()));
			}
		}
	}

	m_messageLogger("Saving lightmap atlas...");
	SaveLightmapAtlas(strMapName);
	if(m_lightMapAtlasEnabled)
		flags |= DataFlags::HasLightmapAtlas;
	if(m_lightMapAtlasEnabled)
	{
		WriteDataOffset(f,offsetLightMapData);
		f->Write<float>(m_lightMapIntensity);
		f->Write<float>(m_lightMapExposure);
	}

	m_messageLogger("Writing entity data...");
	WriteDataOffset(f,offsetEntities);
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
	for(auto &str : m_materialTable)
	{
		util::Path path{str};
		if(ustring::compare(path.GetFront(),"materials",false))
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
	preprocess_bsp_data(bspTree,clusterNodes,clusterToClusterVisibility);
	
	auto &bspNodes = bspTree.GetNodes();
	auto numClusters = bspTree.GetClusterCount();
	auto &clusterVisibility = bspTree.GetClusterVisibility();
	std::function<void(const util::BSPTree::Node&)> fWriteNode = nullptr;
	fWriteNode = [&f,&fWriteNode,&clusterVisibility,&clusterToClusterVisibility,&bspNodes,&clusterNodes,&bspTree,numClusters](const util::BSPTree::Node &node) {
		f->Write<bool>(node.leaf);
		f->Write<Vector3>(node.min);
		f->Write<Vector3>(node.max);
		f->Write<int32_t>(node.firstFace);
		f->Write<int32_t>(node.numFaces);
		f->Write<int32_t>(node.originalNodeIndex);
		if(node.leaf)
		{
			f->Write<uint16_t>(node.cluster);
			auto itNode = std::find_if(bspNodes.begin(),bspNodes.end(),[&node](const std::shared_ptr<util::BSPTree::Node> &nodeOther) {
				return nodeOther.get() == &node;
				});
			// Calculate AABB encompassing all nodes visible by this node
			auto min = node.min;
			auto max = node.max;
			if(itNode != bspNodes.end() && node.cluster != std::numeric_limits<uint16_t>::max())
			{
				for(auto clusterDst : clusterToClusterVisibility.at(node.cluster))
				{
					for(auto nodeOtherIdx : clusterNodes.at(clusterDst))
					{
						auto &nodeOther = bspNodes.at(nodeOtherIdx);
						uvec::to_min_max(min,max,nodeOther->min,nodeOther->max);
					}
				}
			}
			uvec::to_min_max(min,max); // Vertex conversion rotates the vectors, which will change the signs, so we have to re-order the vector components
			f->Write<Vector3>(min);
			f->Write<Vector3>(max);
			return;
		}
		f->Write<Vector3>(node.plane.GetNormal());
		f->Write<float>(node.plane.GetDistance());

		fWriteNode(*node.children.at(0));
		fWriteNode(*node.children.at(1));
	};
	fWriteNode(bspTree.GetRootNode());

	f->Write<uint64_t>(bspTree.GetClusterCount());
	f->Write(clusterVisibility.data(),clusterVisibility.size() *sizeof(clusterVisibility.front()));
}

void pragma::asset::WorldData::WriteEntities(VFilePtrReal &f)
{
	f->Write<uint32_t>(m_entities.size());

	for(auto &entData : m_entities)
	{
		auto offsetEndOfEntity = f->Tell();
		f->Write<uint64_t>(0u); // Offset to end of entity
		auto offsetEntityMeshes = f->Tell();
		f->Write<uint64_t>(0u); // Offset to entity meshes
		auto offsetEntityLeaves = f->Tell();
		f->Write<uint64_t>(0u); // Offset to entity leaves

		f->Write<uint64_t>(umath::to_integral(entData->GetFlags()));

		f->WriteString(entData->GetClassName());
		f->Write<Vector3>(entData->GetOrigin());

		// Keyvalues
		auto &keyValues = entData->GetKeyValues();
		f->Write<uint32_t>(keyValues.size());
		for(auto &pair : keyValues)
		{
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
		for(auto &name : components)
			f->WriteString(name);

		// Leaves
		auto cur = f->Tell();
		f->Seek(offsetEntityLeaves);
		f->Write<uint64_t>(cur -offsetEntityLeaves);
		f->Seek(cur);

		uint32_t firstLeaf,numLeaves;
		entData->GetLeafData(firstLeaf,numLeaves);
		f->Write<uint32_t>(numLeaves);
		std::vector<uint16_t> leaves {};
		leaves.resize(numLeaves);
		if(numLeaves > 0u)
			memcpy(leaves.data(),GetStaticPropLeaves().data() +firstLeaf,leaves.size() *sizeof(leaves.front()));
		f->Write(leaves.data(),leaves.size() *sizeof(leaves.front()));

		cur = f->Tell();
		f->Seek(offsetEndOfEntity);
		f->Write<uint64_t>(cur -offsetEndOfEntity);
		f->Seek(cur);
	}
}

bool pragma::asset::WorldData::SaveLightmapAtlas(const std::string &mapName)
{
	if(m_lightMapAtlas == nullptr)
	{
		m_messageLogger("No lightmap atlas has been specified! Lightmaps will not be available.");
		return false;
	}
	// Build lightmap texture
	uimg::TextureInfo texInfo {};
	texInfo.containerFormat = uimg::TextureInfo::ContainerFormat::DDS;
	// texInfo.flags = uimg::TextureInfo::Flags::SRGB;
	texInfo.inputFormat = uimg::TextureInfo::InputFormat::R16G16B16A16_Float;
	texInfo.outputFormat = uimg::TextureInfo::OutputFormat::HDRColorMap;
	auto matPath = "materials/maps/" +mapName;
	FileManager::CreatePath(matPath.c_str());
	auto filePath = matPath +"/lightmap_atlas.dds";
	auto result = uimg::save_texture(filePath,*m_lightMapAtlas,texInfo,false,[](const std::string &msg) {
		Con::cwar<<"WARNING: Unable to save lightmap atlas: "<<msg<<Con::endl;
	});
	if(result)
		m_messageLogger("Lightmap atlas has been saved as '" +filePath +"'!");
	else
		m_messageLogger("Lightmap atlas could not be saved as '" +filePath +"'! Lightmaps will not be available.");
	return result;
}
