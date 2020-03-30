#include "source2/source2.hpp"
#include "source2/pr_impl.hpp"
#include <util_source2.hpp>
#include <source2/resource.hpp>
#include <source2/resource_data.hpp>
#include <source2/resource_edit_info.hpp>
#include <fsys/filesystem.h>
#include <util_archive.hpp>
#include <sharedutils/util_string.h>
#include <sharedutils/util_path.hpp>
#include <pragma/model/vertex.h>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/networkstate/networkstate.h>
#include <pragma/game/game_resources.hpp>
#include <pragma/asset_types/world.hpp>
#include <unordered_set>

#pragma optimize("",off)
// TODO: Remove this file
#if 0
std::shared_ptr<ModelSubMesh> source2::convert::convert_mesh(
	NetworkState &nw,const source2::resource::Mesh &s2Mesh,std::unordered_map<int32_t,uint32_t> *optSkinIndexToBoneIndex
)
{
	auto prSubMesh = std::shared_ptr<ModelSubMesh>(nw.CreateSubMesh());
	prSubMesh->SetSkinTextureIndex(0);

	auto &vbib = *s2Mesh.GetVBIB();
	auto &vertexBuffers = vbib.GetVertexBuffers();
	auto &indexBuffers = vbib.GetIndexBuffers();
	for(auto &vbuf : vertexBuffers)
	{
		impl::initialize_vertices(vbuf,*prSubMesh,optSkinIndexToBoneIndex);
		break; // TODO: Handle multiple vertex buffers!
	}

	auto &prIndices = prSubMesh->GetTriangles();
	for(auto &ibuf : indexBuffers)
	{
		auto numIndices = ibuf.count;
		auto sizePerIndex = ibuf.size;
		prIndices.resize(numIndices);
		for(auto i=decltype(numIndices){0u};i<numIndices;++i)
		{
			auto index = impl::vertex_attr_value_to_index(sizePerIndex,ibuf.buffer.data() +i *sizePerIndex);
			prIndices.at(i) = static_cast<uint16_t>(index);
		}
		break; // TODO: Handle multiple index buffers!
	}
	return prSubMesh;
}
#endif
#pragma optimize("",on)
