#ifndef __PR_SOURCE_SOURCE2_IMPL_HPP__
#define __PR_SOURCE_SOURCE2_IMPL_HPP__

#include <memory>
#include <functional>
#include <unordered_map>
#include <source2/resource_data.hpp>

class ModelSubMesh;
class VFilePtrInternal;
namespace source2
{
	namespace impl
	{
		struct MeshData
		{
			std::vector<Vertex> verts;
			std::vector<VertexWeight> vertWeights;
			std::vector<Vector2> lightmapUvs;
		};
		MeshData initialize_vertices(
			const source2::resource::VBIB::VertexBuffer &vbuf,std::unordered_map<int32_t,uint32_t> *optSkinIndexToBoneIndex=nullptr
		);
		std::shared_ptr<resource::Resource> load_resource(NetworkState &nw,std::shared_ptr<VFilePtrInternal> &f);
		uint64_t vertex_attr_value_to_index(uint32_t size,const uint8_t *data);
		Vector3 convert_source2_vector_to_pragma(const Vector3 &v);
	};
};

#endif
