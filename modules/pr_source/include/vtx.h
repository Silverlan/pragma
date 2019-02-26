#ifndef __VTX_H__
#define __VTX_H__

#include "mdl_shared.h"
#include <array>
#include <vector>
#include <fsys/filesystem.h>

#define OPTIMIZED_MODEL_FILE_VERSION 7

#pragma pack(push,1)
namespace import
{
	namespace mdl
	{ 
		namespace vtx
		{
			struct FileHeader_t
			{
				int32_t version;
 
				int32_t vertCacheSize;
				uint16_t maxBonesPerStrip;
				uint16_t maxBonesPerFace;
				int32_t maxBonesPerVert;
 
				int32_t checkSum;
 
				int32_t numLODs;
 
				int32_t materialReplacementListOffset;
 
				int32_t numBodyParts;
				int32_t bodyPartOffset;
			};
			struct BodyPartHeader_t
			{
				int32_t numModels;
				int32_t modelOffset;
			};
			struct ModelHeader_t
			{
				int32_t numLODs;
				int32_t lodOffset;
			};
			struct ModelLODHeader_t
			{
				int32_t numMeshes;
				int32_t meshOffset;
				float switchPoint;
			};
			struct MeshHeader_t
			{
				int32_t numStripGroups;
				int32_t stripGroupHeaderOffset;
				uint8_t flags;
			};
			struct StripGroupHeader_t
			{
				int32_t numVerts;
				int32_t vertOffset;

				int32_t numIndices;
				int32_t indexOffset;

				int32_t numStrips;
				int32_t stripOffset;

				uint8_t flags;
			};
			struct Vertex_t
			{
				std::array<uint8_t,MAX_NUM_BONES_PER_VERT> boneWeightIndex;
				uint8_t numBones;

				uint16_t origMeshVertID;

				std::array<int8_t,MAX_NUM_BONES_PER_VERT> boneID;
			};
			struct StripHeader_t
			{
				int32_t numIndices;
				int32_t indexOffset;

				int32_t numVerts;
				int32_t vertOffset;

				int16_t numBones;  

				uint8_t flags;

				int32_t numBoneStateChanges;
				int32_t boneStateChangeOffset;
			};
			struct VtxStripGroup
			{
				vtx::StripGroupHeader_t header;
				std::vector<vtx::StripHeader_t> strips;
				std::vector<vtx::Vertex_t> stripVerts;
				std::vector<uint16_t> indices;
			};
			struct VtxLodMesh
			{
				vtx::MeshHeader_t header;
				std::vector<VtxStripGroup> stripGroups;
			};
			struct VtxLod
			{
				vtx::ModelLODHeader_t header;
				std::vector<VtxLodMesh> meshes;
			};
			struct VtxModel
			{
				vtx::ModelHeader_t header;
				std::vector<VtxLod> lods;
			};
			struct VtxBodyPart
			{
				std::vector<VtxModel> models;
			};
			struct VertexInfoData
			{
				uint32_t vertexCount;
				uint32_t offVertexNumTable;
				uint32_t faceVertexCount;
				uint32_t offDrawFaces;
				uint32_t offPlaceholder;
				uint32_t offRefPlaceholder;
				uint8_t placeholder;
			};
		};
		bool load_vtx(const VFilePtr &f,std::vector<vtx::VtxBodyPart> &bodyParts);
	};
};
#pragma pack(pop)

#endif
