// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :map.world_data;

void pragma::asset::Output::Read(fs::VFilePtr &f)
{
	name = f->ReadString();
	target = f->ReadString();
	input = f->ReadString();
	param = f->ReadString();
	delay = f->Read<float>();
	times = f->Read<int32_t>();
}

/////////

bool pragma::asset::WorldData::Read(fs::VFilePtr &f, EntityData::Flags entMask, std::string *errMsg)
{
	auto header = f->Read<std::array<char, 3>>();
	if(string::compare(header.data(), "WLD", true, 3) == false) {
		if(errMsg)
			*errMsg = "Invalid file format!";
		return false;
	}
	auto version = f->Read<uint32_t>();
	if(version < 11 || version > WLD_VERSION) {
		if(errMsg)
			*errMsg = "Unsupported map version '" + std::to_string(version) + "'!";
		return false;
	}

#pragma pack(push, 1)
	struct HeaderData {
		DataFlags flags = DataFlags::None;
		uint64_t offsetMaterials = 0;
		uint64_t offsetBSPTree = 0;
		uint64_t offsetLightMapData = 0;
		uint64_t offsetEntities = 0;
	};
#pragma pack(pop)
	auto headerData = f->Read<HeaderData>();

	auto materials = ReadMaterials(f);
	if(math::is_flag_set(headerData.flags, DataFlags::HasBSPTree))
		ReadBSPTree(f, version);
	if(math::is_flag_set(headerData.flags, DataFlags::HasLightmapAtlas)) {
		m_lightMapIntensity = f->Read<float>();
		m_lightMapExposure = f->Read<float>();
	}
	ReadEntities(f, materials, entMask);
	return true;
}
std::vector<pragma::material::MaterialHandle> pragma::asset::WorldData::ReadMaterials(fs::VFilePtr &f)
{
	auto numMaterials = f->Read<uint32_t>();
	m_materialTable.resize(numMaterials);
	std::vector<material::MaterialHandle> materials {};
	materials.reserve(numMaterials);
	for(auto &str : m_materialTable) {
		str = f->ReadString();
		auto *mat = m_nw.LoadMaterial(str);
		materials.push_back(mat ? mat->GetHandle() : material::MaterialHandle {});
	}
	return materials;
}
void pragma::asset::WorldData::ReadBSPTree(fs::VFilePtr &f, uint32_t version)
{
	m_bspTree = util::BSPTree::Create();
	auto &nodes = m_bspTree->GetNodes();
	std::function<void(util::BSPTree::Node &)> fReadNode = nullptr;
	fReadNode = [this, &fReadNode, &nodes, &f](util::BSPTree::Node &node) {
		node.leaf = f->Read<bool>();
		node.min = f->Read<Vector3>();
		node.max = f->Read<Vector3>();
		node.firstFace = f->Read<int32_t>();
		node.numFaces = f->Read<int32_t>();
		node.originalNodeIndex = f->Read<int32_t>();
		if(node.leaf) {
			node.cluster = f->Read<uint16_t>();
			node.minVisible = f->Read<Vector3>();
			node.maxVisible = f->Read<Vector3>();
			return;
		}
		auto normal = f->Read<Vector3>();
		auto d = f->Read<float>();
		node.plane = math::Plane {normal, static_cast<double>(d)};

		auto idx = node.index;
		m_bspTree->GetNodes().reserve(m_bspTree->GetNodes().size() + 2); // Note: This may invalidate 'node'!
		auto idx0 = m_bspTree->CreateNode().index;
		auto idx1 = m_bspTree->CreateNode().index;
		nodes[idx].children.at(0) = idx0;
		nodes[idx].children.at(1) = idx1;
		fReadNode(nodes[idx0]);
		fReadNode(nodes[idx1]);
	};
	fReadNode(m_bspTree->GetRootNode());

	auto numClusters = f->Read<uint64_t>();
	auto numCompressedClusters = math::pow2(numClusters);
	numCompressedClusters = numCompressedClusters / 8u + ((numCompressedClusters % 8u) > 0u ? 1u : 0u);
	auto &compressedClusterData = m_bspTree->GetClusterVisibility();
	compressedClusterData.resize(numCompressedClusters);
	f->Read(compressedClusterData.data(), compressedClusterData.size() * sizeof(compressedClusterData.front()));
	m_bspTree->SetClusterCount(numClusters);

	if(version <= 11)
		return;
	auto hasClusterMeshList = f->Read<bool>();
	if(hasClusterMeshList == false)
		return;
	m_meshesPerCluster.resize(numClusters);
	for(auto i = decltype(numClusters) {0u}; i < numClusters; ++i) {
		auto &meshIndices = m_meshesPerCluster.at(i);
		auto n = f->Read<uint32_t>();
		meshIndices.resize(n);
		f->Read(meshIndices.data(), meshIndices.size() * sizeof(meshIndices.front()));
	}
}
void pragma::asset::WorldData::ReadEntities(fs::VFilePtr &f, const std::vector<material::MaterialHandle> &materials, EntityData::Flags entMask)
{
	auto numEnts = f->Read<uint32_t>();
	m_entities.reserve(numEnts);
	for(auto i = decltype(numEnts) {0u}; i < numEnts; ++i) {
		auto entData = EntityData::Create();
		auto startOffset = f->Tell();
		auto offsetToEndOfEntity = startOffset + f->Read<uint64_t>();
		auto offsetMeshes = f->Tell();
		offsetMeshes += f->Read<uint64_t>();

		auto offsetLeaves = f->Tell();
		offsetLeaves += f->Read<uint64_t>();

		entData->SetFlags(static_cast<EntityData::Flags>(f->Read<uint64_t>()));
		if(entMask != EntityData::Flags::None && (entData->GetFlags() & entMask) == EntityData::Flags::None) {
			// We don't need this entity; Skip it
			f->Seek(offsetToEndOfEntity);
			continue;
		}
		m_entities.push_back(entData);
		entData->m_mapIndex = i + 1; // Map indices always start at 1!
		entData->SetClassName(f->ReadString());
		auto pose = math::ScaledTransform();
		pose.SetOrigin(f->Read<Vector3>());
		entData->SetPose(pose);

		auto numKeyValues = f->Read<uint32_t>();
		auto &keyValues = entData->GetKeyValues();
		keyValues.reserve(numKeyValues);
		for(auto i = decltype(numKeyValues) {0u}; i < numKeyValues; ++i) {
			auto key = f->ReadString();
			auto val = f->ReadString();
			keyValues[key] = val;
		}

		auto numOutputs = f->Read<uint32_t>();
		auto &outputs = entData->GetOutputs();
		outputs.resize(numOutputs);
		for(auto &output : outputs) {
			output.name = f->ReadString();
			output.target = f->ReadString();
			output.input = f->ReadString();
			output.param = f->ReadString();
			output.delay = f->Read<float>();
			output.times = f->Read<int>();
		}

		auto &components = entData->GetComponents();
		auto numComponents = f->Read<uint32_t>();
		components.reserve(numComponents);
		for(auto &c : components) {
			auto componentType = f->ReadString();
			entData->AddComponent(componentType);
		}

		auto numLeaves = f->Read<uint32_t>();
		auto &leaves = entData->GetLeaves();
		leaves.resize(numLeaves);
		f->Read(leaves.data(), leaves.size() * sizeof(leaves.front()));

		f->Seek(offsetToEndOfEntity);
	}
}
