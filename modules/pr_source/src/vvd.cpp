#include "mdl.h"
#include "vvd.h"
#include "mdl_optimize.h"

bool import::mdl::load_vvd(const VFilePtr &f,std::vector<Vertex> &verts,std::vector<VertexWeight> &vertWeights,std::vector<std::vector<uint32_t>> &fixedLodVertexIndices)
{
	auto offset = f->Tell();
	auto header = f->Read<vvd::vertexFileHeader_t>();

	f->Seek(offset +header.fixupTableStart);
	auto fixup = f->Read<vvd::vertexFileFixup_t>();

	f->Seek(offset +header.vertexDataStart);

	if(header.numLODs > 0)
	{
		auto numVerts = header.numLODVertexes.front();
		verts.reserve(numVerts);
		vertWeights.reserve(numVerts);
		for(auto i=decltype(numVerts){0};i<numVerts;++i)
		{
			auto stdVert = f->Read<vvd::mstudiovertex_t>();
			verts.push_back(Vertex{stdVert.m_vecPosition,stdVert.m_vecTexCoord,stdVert.m_vecNormal});
			vertWeights.push_back(VertexWeight{{-1,-1,-1,-1},{}});
			auto &weight = vertWeights.back();
			auto weightSum = 0.f;
			for(auto j=decltype(stdVert.m_BoneWeights.numbones){0};j<umath::min(stdVert.m_BoneWeights.numbones,static_cast<byte>(4));++j)
			{
				weight.boneIds[j] = stdVert.m_BoneWeights.bone[j];
				weight.weights[j] = stdVert.m_BoneWeights.weight[j];

				weightSum += stdVert.m_BoneWeights.weight[j];
			}
		}
	}

	// Read fixups
	if(header.numFixups > 0)
	{
		f->Seek(header.fixupTableStart);
		std::vector<vvd::vertexFileFixup_t> fixups(header.numFixups);
		f->Read(fixups.data(),fixups.size() *sizeof(fixups.front()));
		
		f->Seek(header.vertexDataStart);
		fixedLodVertexIndices.reserve(header.numLODs);
		for(auto lod=decltype(header.numLODs){0};lod<header.numLODs;++lod)
		{
			fixedLodVertexIndices.push_back({});
			auto &lodIndices = fixedLodVertexIndices.back();
			for(auto &fixup : fixups)
			{
				if(fixup.lod >= lod)
				{
					lodIndices.reserve(lodIndices.size() +fixup.numVertexes);
					for(auto i=decltype(fixup.numVertexes){0};i<fixup.numVertexes;++i)
						lodIndices.push_back(fixup.sourceVertexID +i);
				}
			}
		}
	}
	return true;
}
