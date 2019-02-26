#include "vtx.h"
#include "mdl_optimize.h"

bool import::mdl::load_vtx(const VFilePtr &f,std::vector<vtx::VtxBodyPart> &bodyParts)
{
	auto offset = f->Tell();
	auto header = f->Read<OptimizedModel::FileHeader_t>();
	if(header.version > OPTIMIZED_MODEL_FILE_VERSION)
		return false;

	f->Seek(offset +header.bodyPartOffset);
	offset = f->Tell();
	auto bodyPartHeader = f->Read<OptimizedModel::BodyPartHeader_t>();

	f->Seek(offset +bodyPartHeader.modelOffset);
	offset = f->Tell();
	auto mdlHeader = f->Read<OptimizedModel::ModelHeader_t>();
	
	f->Seek(offset +mdlHeader.lodOffset);
	offset = f->Tell();
	auto lodHeader = f->Read<OptimizedModel::ModelLODHeader_t>();
	
	f->Seek(offset +lodHeader.meshOffset);
	offset = f->Tell();
	auto meshHeader = f->Read<OptimizedModel::MeshHeader_t>();
	
	f->Seek(offset +meshHeader.stripGroupHeaderOffset);
	offset = f->Tell();
	auto stripGroupHeader = f->Read<OptimizedModel::StripGroupHeader_t>();

	// Read vertices
	f->Seek(offset +stripGroupHeader.vertOffset);
	std::vector<OptimizedModel::Vertex_t> mdlVerts(stripGroupHeader.numVerts);
	f->Read(mdlVerts.data(),sizeof(mdlVerts.front()) *mdlVerts.size());

	std::vector<uint16_t> origMeshIds;
	origMeshIds.reserve(mdlVerts.size());
	for(auto &v : mdlVerts)
		origMeshIds.push_back(v.origMeshVertID);
	//

	// Read indices
	f->Seek(offset +stripGroupHeader.indexOffset);
	/*indices.resize(stripGroupHeader.numIndices);
	f->Read(indices.data(),sizeof(indices.front()) *indices.size());
	for(auto &i : indices)
	{
		i = origMeshIds[i];
	}*/

	auto bStripGroup8BytePadding = false;
	auto fReadVtxStripGroups = [&f,&bStripGroup8BytePadding](uint64_t offsetVtx,mdl::vtx::VtxLodMesh &vtxLodMesh) {
		auto &vtxMeshHeader = vtxLodMesh.header;
		auto &stripGroups = vtxLodMesh.stripGroups;
		stripGroups.clear();
		stripGroups.reserve(vtxMeshHeader.numStripGroups);
		f->Seek(offsetVtx +vtxMeshHeader.stripGroupHeaderOffset);
		for(auto i=decltype(vtxMeshHeader.numStripGroups){0};i<vtxMeshHeader.numStripGroups;++i)
		{
			auto offsetStripGroupStart = f->Tell();

			stripGroups.push_back({});
			auto &stripGroup = stripGroups.back();
			stripGroup.header = f->Read<vtx::StripGroupHeader_t>();
			auto offsetStripGroupHeaderEnd = f->Tell();
			if(bStripGroup8BytePadding == true)
				offsetStripGroupHeaderEnd += sizeof(int32_t) *2;
			auto &sgHeader = stripGroup.header;
			auto &strips = stripGroup.strips;
			auto &stripVerts = stripGroup.stripVerts;
			auto &indices = stripGroup.indices;

			if(sgHeader.numVerts > 0 && sgHeader.vertOffset != 0)
			{
				stripVerts.resize(sgHeader.numVerts);
				f->Seek(offsetStripGroupStart +sgHeader.vertOffset);
				f->Read(stripVerts.data(),stripVerts.size() *sizeof(stripVerts.front()));
			}
			if(sgHeader.numIndices > 0 && sgHeader.indexOffset != 0)
			{
				f->Seek(offsetStripGroupStart +sgHeader.indexOffset);
				indices.resize(indices.size() +sgHeader.numIndices);
				f->Read(indices.data(),sgHeader.numIndices *sizeof(uint16_t));
			}
			if(sgHeader.numStrips > 0 && sgHeader.stripOffset != 0)
			{
				f->Seek(offsetStripGroupStart +sgHeader.stripOffset);
				strips.resize(sgHeader.numStrips);
				f->Read(strips.data(),strips.size() *sizeof(strips.front()));
			}
			f->Seek(offsetStripGroupHeaderEnd);
		}
	};

	auto expectedStartOfSecondStripGroupList = std::numeric_limits<uint64_t>::max();
	mdl::vtx::VtxLodMesh *firstVtxLodMesh = nullptr;
	uint64_t firstVtxLodMeshOffset = std::numeric_limits<uint64_t>::max();
	auto bSecondStripGroupAnalyzed = false;
	f->Seek(header.bodyPartOffset);
	for(auto i=decltype(header.numBodyParts){0};i<header.numBodyParts;++i)
	{
		auto offset = f->Tell();
		auto bodyPartHeader = f->Read<vtx::BodyPartHeader_t>();
		bodyParts.reserve(bodyPartHeader.numModels);
		if(bodyPartHeader.numModels > 0 && bodyPartHeader.modelOffset != 0)
		{
			bodyParts.push_back({});
			auto &vtxBodyPart = bodyParts.back();

			auto &bodyPartMdls = vtxBodyPart.models;
			bodyPartMdls.reserve(bodyPartHeader.numModels);
			for(auto i=decltype(bodyPartHeader.numModels){0};i<bodyPartHeader.numModels;++i)
			{
				f->Seek(offset +bodyPartHeader.modelOffset +sizeof(vtx::ModelHeader_t) *i);
				auto offsetBodyPart = f->Tell();
				bodyPartMdls.push_back({});
				auto &bodyPartMdl = bodyPartMdls.back();
				auto &bodyPartMdlHeader = bodyPartMdl.header = f->Read<vtx::ModelHeader_t>();
				if(bodyPartMdlHeader.numLODs > 0 && bodyPartMdlHeader.lodOffset != 0)
				{
					bodyPartMdl.lods.reserve(bodyPartMdlHeader.numLODs);
					for(auto lod=decltype(bodyPartMdlHeader.numLODs){0};lod<bodyPartMdlHeader.numLODs;++lod)
					{
						bodyPartMdl.lods.push_back({});
						auto &vtxLod = bodyPartMdl.lods.back();
						f->Seek(offsetBodyPart +bodyPartMdlHeader.lodOffset +sizeof(vtx::ModelLODHeader_t) *lod);
						auto &vtxLodMeshes = vtxLod.meshes;
						auto offsetLod = f->Tell();
						auto &mdlLodHeader = vtxLod.header = f->Read<vtx::ModelLODHeader_t>();
						auto offset = f->Tell();
						vtxLodMeshes.reserve(mdlLodHeader.numMeshes);
						if(mdlLodHeader.numMeshes > 0 && mdlLodHeader.meshOffset != 0)
						{
							for(auto i=decltype(mdlLodHeader.numMeshes){0};i<mdlLodHeader.numMeshes;++i)
							{
								f->Seek(offsetLod +mdlLodHeader.meshOffset +sizeof(vtx::MeshHeader_t) *i);
								auto offsetVtx = f->Tell();
								vtxLodMeshes.push_back({});
								auto &vtxLodMesh = vtxLodMeshes.back();
								vtxLodMesh.header = f->Read<vtx::MeshHeader_t>();
								auto &vtxMeshHeader = vtxLodMesh.header;
								auto &stripGroups = vtxLodMesh.stripGroups;

								if(vtxMeshHeader.numStripGroups > 0 && vtxMeshHeader.stripGroupHeaderOffset != 0)
								{
									// Analyze strip groups
									auto bFirst = (expectedStartOfSecondStripGroupList == std::numeric_limits<uint64_t>::max()) ? true : false;
									if(bFirst == true)
									{
										f->Seek(offsetVtx +vtxMeshHeader.stripGroupHeaderOffset);
										for(auto i=decltype(vtxMeshHeader.numStripGroups){0};i<vtxMeshHeader.numStripGroups;++i)
											f->Read<vtx::StripGroupHeader_t>();
										expectedStartOfSecondStripGroupList = f->Tell();
										firstVtxLodMesh = &vtxLodMesh;
										firstVtxLodMeshOffset = offsetVtx;
									}
									else if(bSecondStripGroupAnalyzed == false)
									{
										bSecondStripGroupAnalyzed = true;
										if(expectedStartOfSecondStripGroupList != offsetVtx +vtxLodMesh.header.stripGroupHeaderOffset)
										{
											bStripGroup8BytePadding = true;

											stripGroups.clear();
											fReadVtxStripGroups(firstVtxLodMeshOffset,*firstVtxLodMesh);
										}
									}
									//
									fReadVtxStripGroups(offsetVtx,vtxLodMesh);
								}
							}
						}
					}
				}
			}
		}
		f->Seek(offset +sizeof(vtx::BodyPartHeader_t));
	}
	return true;
}

