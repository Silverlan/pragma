#include "mdl.h"
#include "wv_source.hpp"
#include <util_source2.hpp>
#include <source2/resource.hpp>
#include <source2/resource_data.hpp>
#include <source2/resource_edit_info.hpp>
#include <fsys/filesystem.h>
#include <util_archive.hpp>
#include <sharedutils/util_string.h>
#include <pragma/model/vertex.h>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/networkstate/networkstate.h>

#pragma optimize("",off)
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

static Vector4i vertex_attr_value_to_ivec4(source2::resource::DXGI_FORMAT format,const uint8_t *data)
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
		numChannels = 1;
		baseFormat = source2::resource::DXGI_FORMAT::R8_UINT;
		break;
	case source2::resource::DXGI_FORMAT::R8G8_SINT:
		numChannels = 2;
		baseFormat = source2::resource::DXGI_FORMAT::R8_SINT;
		break;
	case source2::resource::DXGI_FORMAT::R8G8_UINT:
		numChannels = 2;
		baseFormat = source2::resource::DXGI_FORMAT::R8_UINT;
		break;
	case source2::resource::DXGI_FORMAT::R8G8B8A8_SINT:
		numChannels = 4;
		baseFormat = source2::resource::DXGI_FORMAT::R8_SINT;
		break;
	case source2::resource::DXGI_FORMAT::R8G8B8A8_UINT:
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

	Vector4i result {};
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

static uint64_t vertex_attr_value_to_index(uint32_t size,const uint8_t *data)
{
	if(size == sizeof(uint16_t))
		return *reinterpret_cast<const uint16_t*>(data);
	else if(size == sizeof(uint32_t))
		return *reinterpret_cast<const uint32_t*>(data);
	throw std::invalid_argument{"Unsupported index type"};
}

std::shared_ptr<Model> import::load_source2_mdl(
	NetworkState *nw,VFilePtr f,const std::function<std::shared_ptr<Model>()> &fCreateModel,
	const std::function<bool(const std::shared_ptr<Model>&,const std::string&,const std::string&)> &fCallback,bool bCollision,
	std::vector<std::string> &textures,std::ostream *optLog
)
{
	auto resource = source2::load_resource(f);
	if(resource == nullptr)
		return nullptr;
	auto *dataBlock = dynamic_cast<source2::resource::BinaryKV3*>(resource->FindBlock(source2::BlockType::DATA));
	if(dataBlock == nullptr)
		return nullptr;
	auto &stringArray = dataBlock->GetStringArray();
	auto &data = dataBlock->GetData();
	auto &values = data->GetValues();

	auto itMdlInfo = values.find("m_modelInfo");
	if(itMdlInfo != values.end())
	{
		auto &mdlInfo = *itMdlInfo;

	}
	auto ptrMdl = fCreateModel();
	auto &mdl = *ptrMdl;

	// TODO
	mdl.AddTexturePath("");
	import::util::add_texture(*nw,mdl,"batrider");

	auto &meshGroup = *mdl.AddMeshGroup("model");
	mdl.GetBaseMeshes().push_back(0u);

	auto *meshes = data->FindValue("m_refMeshes");
	if(meshes && meshes->GetType() == source2::resource::KVType::ARRAY_TYPED)
	{
		auto *meshesArray = static_cast<source2::resource::KVObject*>(meshes->GetObject());
		auto numMeshes = meshesArray->GetArrayCount();
		for(auto i=decltype(numMeshes){0u};i<numMeshes;++i)
		{
			auto *val = meshesArray->GetArrayValue<source2::resource::KVValueString>(i);
			if(val == nullptr)
				continue;
			auto meshName = val->GetValue() +"_c";

			std::cout<<"Mesh Name: "<<meshName<<std::endl;
			auto fMesh = FileManager::OpenFile(meshName.c_str(),"rb");
			if(fMesh == nullptr)
				fMesh = uarch::load(meshName);
			auto mesh = fMesh ? source2::load_resource(fMesh) : nullptr;
			if(mesh == nullptr)
				continue;

			auto *vbib = dynamic_cast<source2::resource::VBIB*>(mesh->FindBlock(source2::BlockType::VBIB));
			if(vbib == nullptr)
				continue;

			auto prMesh = std::shared_ptr<ModelMesh>{nw->CreateMesh()};
			meshGroup.AddMesh(prMesh);
			auto prSubMesh = std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh());
			prSubMesh->SetSkinTextureIndex(0);
			prMesh->AddSubMesh(prSubMesh);

			auto &vertexBuffers = vbib->GetVertexBuffers();
			auto &indexBuffers = vbib->GetIndexBuffers();
			auto fFindAttribute = [](const std::string &name,const source2::resource::VBIB::VertexBuffer &vbuf) -> const source2::resource::VBIB::VertexAttribute* {
				auto it = std::find_if(vbuf.attributes.begin(),vbuf.attributes.end(),[&name](const source2::resource::VBIB::VertexAttribute &va) {
					return ustring::compare(va.name,name,false);
				});
				if(it == vbuf.attributes.end())
					return nullptr;
				return &*it;
			};
			for(auto &vbuf : vertexBuffers)
			{
				auto numVerts = vbuf.count;
				auto sizePerVertex = vbuf.size;
									
				auto *attrPos = fFindAttribute("POSITION",vbuf);
				auto *attrNorm = fFindAttribute("NORMAL",vbuf);
				auto *attrTex = fFindAttribute("TEXCOORD",vbuf);
				auto *attrBlendIndices = fFindAttribute("BLENDINDICES",vbuf);
				auto *attrBlendWeights = fFindAttribute("BLENDWEIGHT",vbuf);
				auto &vertexData = vbuf.buffer;
				auto &prVerts = prSubMesh->GetVertices();
				prVerts.resize(numVerts);
				std::vector<VertexWeight> *vertWeights = nullptr;
				if(attrBlendIndices)
				{
					vertWeights = &prSubMesh->GetVertexWeights();
					vertWeights->resize(numVerts);
				}
				for(auto i=decltype(numVerts){0u};i<numVerts;++i)
				{
					auto offset = i *sizePerVertex;
					auto *data = vertexData.data() +offset;
					auto &v = prVerts.at(i);
					auto *vw = vertWeights ? &vertWeights->at(i) : nullptr;
					if(attrPos)
					{
						auto pos = vertex_attr_value_to_vec4(attrPos->type,data +attrPos->offset);
						v.position = reinterpret_cast<Vector3&>(pos);
					}
					if(attrNorm)
					{
						auto n = vertex_attr_value_to_vec4(attrNorm->type,data +attrNorm->offset);
						v.normal = reinterpret_cast<Vector3&>(n);
					}
					if(attrTex)
					{
						auto uv = vertex_attr_value_to_vec4(attrTex->type,data +attrTex->offset);
						v.uv = reinterpret_cast<Vector2&>(uv);
					}
					if(attrBlendIndices && vertWeights)
						vw->boneIds = vertex_attr_value_to_ivec4(attrBlendIndices->type,data +attrBlendIndices->offset);
					if(attrBlendWeights && vertWeights)
					{
						auto w = vertex_attr_value_to_vec4(attrBlendWeights->type,data +attrBlendWeights->offset);
						vw->weights = reinterpret_cast<Vector4&>(w);
					}
				}
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
					auto index = vertex_attr_value_to_index(sizePerIndex,ibuf.buffer.data() +i *sizePerIndex);
					prIndices.at(i) = static_cast<uint16_t>(index);
				}
				break; // TODO: Handle multiple index buffers!
			}
		}
	}
	mdl.Update(ModelUpdateFlags::All);
	return ptrMdl;
}
#pragma optimize("",on)
