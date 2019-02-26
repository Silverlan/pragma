#ifndef __VVD_H__
#define __VVD_H__

#include <string>
#include <vector>
#include <mathutil/uvec.h>
#include <pragma/model/vertex.h>
#include "mdl_shared.h"

#pragma pack(push,1)
namespace import
{
	namespace mdl
	{ 
		namespace vvd
		{
			struct vertexFileHeader_t
			{
				int32_t id;
				int32_t	version;
				int32_t checksum;
				int32_t numLODs;
				std::array<int32_t,MAX_NUM_LODS> numLODVertexes;
				int32_t numFixups;
				int32_t fixupTableStart;
				int32_t vertexDataStart;
				int32_t tangentDataStart;
			};

			struct mstudioboneweight_t
			{
				std::array<float,MAX_NUM_BONES_PER_VERT> weight;
				std::array<uint8_t,MAX_NUM_BONES_PER_VERT> bone; 
				byte numbones;
			};

			struct mstudiovertex_t
			{
				mstudioboneweight_t m_BoneWeights;
				Vector3 m_vecPosition;
				Vector3 m_vecNormal;
				Vector2 m_vecTexCoord;
			};

			struct vertexFileFixup_t
			{
				int32_t lod;
				int32_t sourceVertexID;
				int32_t numVertexes;
			};
		};
		bool load_vvd(const VFilePtr &f,std::vector<Vertex> &verts,std::vector<VertexWeight> &vertWeights,std::vector<std::vector<uint32_t>> &fixedLodVertexIndices);
	};
};
#pragma pack(pop)

#endif
