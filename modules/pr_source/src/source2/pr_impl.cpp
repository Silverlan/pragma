#include "source2/source2.hpp"
#include "source2/pr_impl.hpp"
#include <util_source2.hpp>
#include <source2/resource.hpp>
#include <source2/resource_data.hpp>
#include <source2/resource_edit_info.hpp>
#include <pragma/model/modelmesh.h>
#include <pragma/game/game_resources.hpp>
#include <pragma/networkstate/networkstate.h>
#include <fsys/filesystem.h>

#pragma optimize("",off)
std::shared_ptr<source2::resource::Resource> source2::impl::load_resource(NetworkState &nw,std::shared_ptr<VFilePtrInternal> &f)
{
	return source2::load_resource(f,[&nw](const std::string &path) -> VFilePtr {
		if(::util::port_file(&nw,path) == false)
			return nullptr;
		return FileManager::OpenFile(path.c_str(),"rb");
	});
}

uint64_t source2::impl::vertex_attr_value_to_index(uint32_t size,const uint8_t *data)
{
	if(size == sizeof(uint16_t))
		return *reinterpret_cast<const uint16_t*>(data);
	else if(size == sizeof(uint32_t))
		return *reinterpret_cast<const uint32_t*>(data);
	throw std::invalid_argument{"Unsupported index type"};
}

static Vector4 vertex_attr_value_to_vec4(const source2::resource::VBIB::VertexBuffer &vbuf,const source2::resource::VBIB::VertexAttribute &attr,uint32_t offset)
{
	std::vector<float> data;
	vbuf.ReadVertexAttribute(offset,attr,data);
	Vector4 result {};
	for(uint8_t i=0;i<data.size();++i)
		result[i] = data.at(i);
	return result;
}

static Vector3 convert_normal(int32_t x,int32_t y)
{
	// Source unknown
	Vector3 outNormal {};
	auto z_sing = -umath::floor((x - 128.f) / 128.f);
	auto t_sing = -umath::floor((y - 128.f) / 128.f);
	auto x_abs = abs(x - 128.f) - z_sing;
	auto y_abs = abs(y - 128.f) - t_sing;
	auto x_sing = -umath::floor((x_abs - 64.f) / 64.f);
	auto y_sing = -umath::floor((y_abs - 64.f) / 64.f);
	outNormal.x = (abs(x_abs - 64.f) - x_sing) / 64.f;
	outNormal.y = (abs(y_abs - 64.f) - y_sing) / 64.f;
	outNormal.z = 1 - outNormal.x - outNormal.y;
	uvec::normalize(&outNormal);
	outNormal.x *= umath::lerp(1.f, -1.f, x_sing);
	outNormal.y *= umath::lerp(1.f, -1.f, y_sing);
	outNormal.z *= umath::lerp(1.f, -1.f, z_sing);
	return outNormal;
}

static bool is_integral_type(source2::resource::DXGI_FORMAT format)
{
	switch(format)
	{
	case source2::resource::DXGI_FORMAT::R8_SINT:
	case source2::resource::DXGI_FORMAT::R8_UINT:
	case source2::resource::DXGI_FORMAT::R8_UNORM:
	case source2::resource::DXGI_FORMAT::R8G8_SINT:
	case source2::resource::DXGI_FORMAT::R8G8_UINT:
	case source2::resource::DXGI_FORMAT::R8G8_UNORM:
	case source2::resource::DXGI_FORMAT::R8G8B8A8_SINT:
	case source2::resource::DXGI_FORMAT::R8G8B8A8_UINT:
	case source2::resource::DXGI_FORMAT::R8G8B8A8_UNORM:
	case source2::resource::DXGI_FORMAT::R16_SINT:
	case source2::resource::DXGI_FORMAT::R16_UINT:
	case source2::resource::DXGI_FORMAT::R16_UNORM:
	case source2::resource::DXGI_FORMAT::R16G16_SINT:
	case source2::resource::DXGI_FORMAT::R16G16_UINT:
	case source2::resource::DXGI_FORMAT::R16G16_UNORM:
	case source2::resource::DXGI_FORMAT::R16G16B16A16_SINT:
	case source2::resource::DXGI_FORMAT::R16G16B16A16_UINT:
	case source2::resource::DXGI_FORMAT::R16G16B16A16_UNORM:
	case source2::resource::DXGI_FORMAT::R32_SINT:
	case source2::resource::DXGI_FORMAT::R32_UINT:
	case source2::resource::DXGI_FORMAT::R32G32_SINT:
	case source2::resource::DXGI_FORMAT::R32G32_UINT:
	case source2::resource::DXGI_FORMAT::R32G32B32_SINT:
	case source2::resource::DXGI_FORMAT::R32G32B32_UINT:
	case source2::resource::DXGI_FORMAT::R32G32B32A32_SINT:
	case source2::resource::DXGI_FORMAT::R32G32B32A32_UINT:
		return true;
	};
	return false;
}

static Vector4 vertex_attr_value_to_vec4(source2::resource::DXGI_FORMAT format,const uint8_t *data)
{
	uint8_t numChannels = 0;
	auto baseFormat = source2::resource::DXGI_FORMAT::UNKNOWN;

	switch(format)
	{
	case source2::resource::DXGI_FORMAT::R8_SNORM:
		numChannels = 1;
		baseFormat = source2::resource::DXGI_FORMAT::R8_SNORM;
		break;
	case source2::resource::DXGI_FORMAT::R8_UNORM:
		numChannels = 1;
		baseFormat = source2::resource::DXGI_FORMAT::R8_UNORM;
		break;
	case source2::resource::DXGI_FORMAT::R8G8_SNORM:
		numChannels = 2;
		baseFormat = source2::resource::DXGI_FORMAT::R8_SNORM;
		break;
	case source2::resource::DXGI_FORMAT::R8G8_UNORM:
		numChannels = 2;
		baseFormat = source2::resource::DXGI_FORMAT::R8_UNORM;
		break;
	case source2::resource::DXGI_FORMAT::R8G8B8A8_SNORM:
		numChannels = 4;
		baseFormat = source2::resource::DXGI_FORMAT::R8_SNORM;
		break;
	case source2::resource::DXGI_FORMAT::R8G8B8A8_UNORM:
		numChannels = 4;
		baseFormat = source2::resource::DXGI_FORMAT::R8_UNORM;
		break;
	case source2::resource::DXGI_FORMAT::R16_SNORM:
		numChannels = 1;
		baseFormat = source2::resource::DXGI_FORMAT::R16_SNORM;
		break;
	case source2::resource::DXGI_FORMAT::R16_UNORM:
		numChannels = 1;
		baseFormat = source2::resource::DXGI_FORMAT::R16_UNORM;
		break;
	case source2::resource::DXGI_FORMAT::R16_FLOAT:
		numChannels = 1;
		baseFormat = source2::resource::DXGI_FORMAT::R16_FLOAT;
		break;
	case source2::resource::DXGI_FORMAT::R16G16_SNORM:
		numChannels = 2;
		baseFormat = source2::resource::DXGI_FORMAT::R16_SNORM;
		break;
	case source2::resource::DXGI_FORMAT::R16G16_UNORM:
		numChannels = 2;
		baseFormat = source2::resource::DXGI_FORMAT::R16_UNORM;
		break;
	case source2::resource::DXGI_FORMAT::R16G16_FLOAT:
		numChannels = 2;
		baseFormat = source2::resource::DXGI_FORMAT::R16_FLOAT;
		break;
	case source2::resource::DXGI_FORMAT::R16G16B16A16_SNORM:
		numChannels = 4;
		baseFormat = source2::resource::DXGI_FORMAT::R16_SNORM;
		break;
	case source2::resource::DXGI_FORMAT::R16G16B16A16_UNORM:
		numChannels = 4;
		baseFormat = source2::resource::DXGI_FORMAT::R16_UNORM;
		break;
	case source2::resource::DXGI_FORMAT::R16G16B16A16_FLOAT:
		numChannels = 4;
		baseFormat = source2::resource::DXGI_FORMAT::R16_FLOAT;
		break;
	case source2::resource::DXGI_FORMAT::R32_FLOAT:
		numChannels = 1;
		baseFormat = source2::resource::DXGI_FORMAT::R32_FLOAT;
		break;
	case source2::resource::DXGI_FORMAT::R32G32_FLOAT:
		numChannels = 2;
		baseFormat = source2::resource::DXGI_FORMAT::R32_FLOAT;
		break;
	case source2::resource::DXGI_FORMAT::R32G32B32_FLOAT:
		numChannels = 3;
		baseFormat = source2::resource::DXGI_FORMAT::R32_FLOAT;
		break;
	case source2::resource::DXGI_FORMAT::R32G32B32A32_FLOAT:
		numChannels = 4;
		baseFormat = source2::resource::DXGI_FORMAT::R32_FLOAT;
		break;
	default:
		throw std::invalid_argument{"Unknown attribute format " +std::to_string(umath::to_integral(format))};
	}

	Vector4 result {};
	for(uint8_t i=0;i<numChannels;++i)
	{
		switch(baseFormat)
		{
		case source2::resource::DXGI_FORMAT::R8_SNORM:
			result[i] = static_cast<uint8_t>(*reinterpret_cast<const int8_t*>(data +sizeof(int8_t) *i)) /static_cast<float>(std::numeric_limits<uint8_t>::max());
			break;
		case source2::resource::DXGI_FORMAT::R8_UNORM:
			result[i] = *reinterpret_cast<const uint8_t*>(data +sizeof(uint8_t) *i) /static_cast<float>(std::numeric_limits<uint8_t>::max());
			break;
		case source2::resource::DXGI_FORMAT::R16_SNORM:
			result[i] = static_cast<uint16_t>(*reinterpret_cast<const int16_t*>(data +sizeof(int16_t) *i)) /static_cast<float>(std::numeric_limits<uint16_t>::max());
			break;
		case source2::resource::DXGI_FORMAT::R16_UNORM:
			result[i] = *reinterpret_cast<const uint16_t*>(data +sizeof(uint16_t) *i) /static_cast<float>(std::numeric_limits<uint16_t>::max());
			break;
		case source2::resource::DXGI_FORMAT::R16_FLOAT:
			result[i] = umath::float16_to_float32_glm(*reinterpret_cast<const uint16_t*>(data +sizeof(uint16_t) *i));
			break;
		case source2::resource::DXGI_FORMAT::R32_FLOAT:
			result[i] = *reinterpret_cast<const float*>(data +sizeof(float) *i);
			break;
		}
	}
	return result;
}

static Vector4i vertex_attr_value_to_ivec4(source2::resource::DXGI_FORMAT format,const uint8_t *data,const Vector4i &default={})
{
	uint8_t numChannels = 0;
	auto baseFormat = source2::resource::DXGI_FORMAT::UNKNOWN;

	switch(format)
	{
	case source2::resource::DXGI_FORMAT::R8_SINT:
		numChannels = 1;
		baseFormat = source2::resource::DXGI_FORMAT::R8_SINT;
		break;
	case source2::resource::DXGI_FORMAT::R8_UINT:
	case source2::resource::DXGI_FORMAT::R8_UNORM:
		numChannels = 1;
		baseFormat = source2::resource::DXGI_FORMAT::R8_UINT;
		break;
	case source2::resource::DXGI_FORMAT::R8G8_SINT:
		numChannels = 2;
		baseFormat = source2::resource::DXGI_FORMAT::R8_SINT;
		break;
	case source2::resource::DXGI_FORMAT::R8G8_UINT:
	case source2::resource::DXGI_FORMAT::R8G8_UNORM:
		numChannels = 2;
		baseFormat = source2::resource::DXGI_FORMAT::R8_UINT;
		break;
	case source2::resource::DXGI_FORMAT::R8G8B8A8_SINT:
		numChannels = 4;
		baseFormat = source2::resource::DXGI_FORMAT::R8_SINT;
		break;
	case source2::resource::DXGI_FORMAT::R8G8B8A8_UINT:
	case source2::resource::DXGI_FORMAT::R8G8B8A8_UNORM:
		numChannels = 4;
		baseFormat = source2::resource::DXGI_FORMAT::R8_UINT;
		break;
	case source2::resource::DXGI_FORMAT::R16_SINT:
		numChannels = 1;
		baseFormat = source2::resource::DXGI_FORMAT::R16_SINT;
		break;
	case source2::resource::DXGI_FORMAT::R16_UINT:
		numChannels = 1;
		baseFormat = source2::resource::DXGI_FORMAT::R16_UINT;
		break;
	case source2::resource::DXGI_FORMAT::R16G16_SINT:
		numChannels = 2;
		baseFormat = source2::resource::DXGI_FORMAT::R16_SINT;
		break;
	case source2::resource::DXGI_FORMAT::R16G16_UINT:
		numChannels = 2;
		baseFormat = source2::resource::DXGI_FORMAT::R16_UINT;
		break;
	case source2::resource::DXGI_FORMAT::R16G16B16A16_SINT:
		numChannels = 4;
		baseFormat = source2::resource::DXGI_FORMAT::R16_SINT;
		break;
	case source2::resource::DXGI_FORMAT::R16G16B16A16_UINT:
		numChannels = 4;
		baseFormat = source2::resource::DXGI_FORMAT::R16_UINT;
		break;
	case source2::resource::DXGI_FORMAT::R32_SINT:
		numChannels = 1;
		baseFormat = source2::resource::DXGI_FORMAT::R32_SINT;
		break;
	case source2::resource::DXGI_FORMAT::R32_UINT:
		numChannels = 1;
		baseFormat = source2::resource::DXGI_FORMAT::R32_UINT;
		break;
	case source2::resource::DXGI_FORMAT::R32G32_SINT:
		numChannels = 2;
		baseFormat = source2::resource::DXGI_FORMAT::R32_SINT;
		break;
	case source2::resource::DXGI_FORMAT::R32G32_UINT:
		numChannels = 2;
		baseFormat = source2::resource::DXGI_FORMAT::R32_UINT;
		break;
	case source2::resource::DXGI_FORMAT::R32G32B32_SINT:
		numChannels = 3;
		baseFormat = source2::resource::DXGI_FORMAT::R32_SINT;
		break;
	case source2::resource::DXGI_FORMAT::R32G32B32_UINT:
		numChannels = 3;
		baseFormat = source2::resource::DXGI_FORMAT::R32_UINT;
		break;
	case source2::resource::DXGI_FORMAT::R32G32B32A32_SINT:
		numChannels = 4;
		baseFormat = source2::resource::DXGI_FORMAT::R32_SINT;
		break;
	case source2::resource::DXGI_FORMAT::R32G32B32A32_UINT:
		numChannels = 4;
		baseFormat = source2::resource::DXGI_FORMAT::R32_UINT;
		break;
	default:
		throw std::invalid_argument{"Unknown attribute format " +std::to_string(umath::to_integral(format))};
	}

	auto result = default;
	for(uint8_t i=0;i<numChannels;++i)
	{
		switch(baseFormat)
		{
		case source2::resource::DXGI_FORMAT::R8_SINT:
			result[i] = *reinterpret_cast<const int8_t*>(data +sizeof(int8_t) *i);
			break;
		case source2::resource::DXGI_FORMAT::R8_UINT:
			result[i] = *reinterpret_cast<const uint8_t*>(data +sizeof(uint8_t) *i);
			break;
		case source2::resource::DXGI_FORMAT::R16_SINT:
			result[i] = *reinterpret_cast<const int16_t*>(data +sizeof(int16_t) *i);
			break;
		case source2::resource::DXGI_FORMAT::R16_UINT:
			result[i] = *reinterpret_cast<const uint16_t*>(data +sizeof(uint16_t) *i);
			break;
		case source2::resource::DXGI_FORMAT::R32_SINT:
			result[i] = *reinterpret_cast<const int32_t*>(data +sizeof(int32_t) *i);
			break;
		case source2::resource::DXGI_FORMAT::R32_UINT:
			result[i] = *reinterpret_cast<const uint32_t*>(data +sizeof(uint32_t) *i);
			break;
		}
	}
	return result;
}

source2::impl::MeshData source2::impl::initialize_vertices(
	const source2::resource::VBIB::VertexBuffer &vbuf,std::unordered_map<int32_t,uint32_t> *optSkinIndexToBoneIndex
)
{
	auto numVerts = vbuf.count;
	auto sizePerVertex = vbuf.size;

	auto fFindAttribute = [](const std::string &name,const source2::resource::VBIB::VertexBuffer &vbuf) -> const source2::resource::VBIB::VertexAttribute* {
		auto it = std::find_if(vbuf.attributes.begin(),vbuf.attributes.end(),[&name](const source2::resource::VBIB::VertexAttribute &va) {
			return ustring::compare(va.name,name);
			});
		if(it == vbuf.attributes.end())
			return nullptr;
		return &*it;
	};
	auto *attrPos = fFindAttribute("POSITION",vbuf);
	auto *attrNorm = fFindAttribute("NORMAL",vbuf);
	auto *attrTex = fFindAttribute("TEXCOORD",vbuf);
	auto *attrBlendIndices = fFindAttribute("BLENDINDICES",vbuf);
	auto *attrBlendWeights = fFindAttribute("BLENDWEIGHT",vbuf);
	auto *attrTex2 = fFindAttribute("texcoord",vbuf); // Lightmap uvs?
	auto &vertexData = vbuf.buffer;

	MeshData meshData {};
	auto &prVerts = meshData.verts;
	prVerts.resize(numVerts);

	auto &lightmapUvs = meshData.lightmapUvs;
	if(attrTex2)
		lightmapUvs.resize(numVerts);

	auto &vertWeights = meshData.vertWeights;
	if(attrBlendIndices)
		vertWeights.resize(numVerts);

	for(auto i=decltype(numVerts){0u};i<numVerts;++i)
	{
		auto offset = i *sizePerVertex;
		auto *data = vertexData.data() +offset;
		auto &v = prVerts.at(i);
		auto *vw = attrBlendIndices ? &vertWeights.at(i) : nullptr;
		if(attrPos)
		{
			auto pos = vertex_attr_value_to_vec4(vbuf,*attrPos,i);
			//auto pos = vertex_attr_value_to_vec4(attrPos->type,data +attrPos->offset);
			v.position = reinterpret_cast<Vector3&>(pos);
			//v.position = Vector3{v.position.y,v.position.z,v.position.x};
			//uvec::rotate(&v.position,rotS2ToPragma);
		}
		if(attrNorm)
		{
			Vector3 n;
			if(is_integral_type(attrNorm->type))
			{
				auto ni = vertex_attr_value_to_ivec4(attrNorm->type,data +attrNorm->offset,{0,0,0,0});
				n = convert_normal(ni.x,ni.y);
			}
			else
				n = vertex_attr_value_to_vec4(vbuf,*attrNorm,i);

			//auto n = vertex_attr_value_to_vec4(attrNorm->type,data +attrNorm->offset);
			v.normal = reinterpret_cast<Vector3&>(n);
		}
		if(attrTex)
		{
			auto uv = vertex_attr_value_to_vec4(vbuf,*attrTex,i);
			//auto uv = vertex_attr_value_to_vec4(attrTex->type,data +attrTex->offset);
			v.uv = reinterpret_cast<Vector2&>(uv);
		}
		if(attrTex2)
		{
			auto uv = vertex_attr_value_to_vec4(vbuf,*attrTex2,i);
			lightmapUvs.at(i) = uv *1.3333333f; // Magic number; Currently unknown why this is needed!
		}
		if(optSkinIndexToBoneIndex && attrBlendIndices)
		{
			vw->boneIds = vertex_attr_value_to_ivec4(attrBlendIndices->type,data +attrBlendIndices->offset,{-1,-1,-1,-1});
			for(uint8_t i=0;i<4;++i)
			{
				if(vw->boneIds[i] == -1)
					continue;
				auto itBoneIdx = optSkinIndexToBoneIndex->find(vw->boneIds[i]);
				vw->boneIds[i] = (itBoneIdx != optSkinIndexToBoneIndex->end()) ? itBoneIdx->second : -1;
			}
		}
		if(attrBlendIndices)
		{
			auto w = attrBlendWeights ? vertex_attr_value_to_vec4(vbuf,*attrBlendWeights,i) : Vector4{1.f,0.f,0.f,0.f};
			//auto w = attrBlendWeights ? vertex_attr_value_to_vec4(attrBlendWeights->type,data +attrBlendWeights->offset) : Vector4{1.f,0.f,0.f,0.f};
			vw->weights = reinterpret_cast<Vector4&>(w);
		}
	}
	return meshData;
}
#pragma optimize("",on)
