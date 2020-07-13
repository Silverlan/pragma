/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/c_skybox.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/model/c_model.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/util_file.h>
#include <util_image_buffer.hpp>
#include <util_texture_info.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

using namespace pragma;
#pragma optimize("",off)
LINK_ENTITY_TO_CLASS(skybox,CSkybox);

void CSkyboxComponent::Initialize()
{
	BaseSkyboxComponent::Initialize();
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto pRenderComponent = ent.GetComponent<pragma::CRenderComponent>();
	if(pRenderComponent.valid())
	{
		pRenderComponent->SetRenderMode(RenderMode::Skybox);
		pRenderComponent->SetCastShadows(false);
		pRenderComponent->SetDepthPassEnabled(false);
	}

	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(m_cbOnModelMaterialsLoaded.IsValid())
			m_cbOnModelMaterialsLoaded.Remove();
		auto &evMdl = static_cast<pragma::CEOnModelChanged&>(evData.get());
		if(evMdl.model == nullptr)
			return;
		m_cbOnModelMaterialsLoaded = evMdl.model->CallOnMaterialsLoaded([this]() {
			ValidateMaterials();
		});
	});
}
void CSkyboxComponent::ReceiveData(NetPacket &packet)
{
	m_skyAngles = packet->Read<EulerAngles>();
}
Bool CSkyboxComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetSkyAngles)
		SetSkyAngles(packet->Read<EulerAngles>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId,packet);
	return true;
}
void CSkyboxComponent::OnRemove()
{
	BaseSkyboxComponent::OnRemove();
	if(m_cbOnModelMaterialsLoaded.IsValid())
		m_cbOnModelMaterialsLoaded.Remove();
}
luabind::object CSkyboxComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CSkyboxComponentHandleWrapper>(l);}
bool CSkyboxComponent::CreateCubemapFromIndividualTextures(const std::string &materialPath,const std::string &postfix) const
{
	// Check if this skybox is made of individual textures
	// (e.g. if it came from the source engine)
	auto matName = materialPath;
	ufile::remove_extension_from_filename(matName);

	auto containerFormat = uimg::TextureInfo::ContainerFormat::DDS;
	auto ext = "dds";
	if(FileManager::Exists("materials/" +matName +"." +ext))
		return true; // Skybox texture already exists; There's nothing for us to do

	// Load all sides as textures
	const std::array<std::string,6> sidePostfixes {
		"ft","bk","up","dn","rt","lf"
	};
	constexpr uint32_t numLayers = 6u;
	std::array<std::shared_ptr<prosper::IImage>,numLayers> cubemapImages {};
	auto bAllValid = true;
	uint32_t largestWidth = 0u;
	uint32_t largestHeight = 0u;
	for(auto i=decltype(sidePostfixes.size()){0u};i<sidePostfixes.size();++i)
	{
		auto &sidePostFix = sidePostfixes.at(i);
		auto *matSide = client->LoadMaterial(matName +postfix +sidePostFix,true,false);
		if(matSide == nullptr)
			return false;
		auto *diffuseMapSide = matSide->GetDiffuseMap();
		if(diffuseMapSide == nullptr || diffuseMapSide->texture == nullptr)
			return false;
		auto texture = std::static_pointer_cast<Texture>(diffuseMapSide->texture);
		if(texture->HasValidVkTexture() == false || texture->IsError())
			return false;
		auto &img = texture->GetVkTexture()->GetImage();
		cubemapImages.at(i) = img.shared_from_this();

		auto extents = img.GetExtents();
		largestWidth = umath::max(largestWidth,extents.width);
		largestHeight = umath::max(largestHeight,extents.height);
	}
	Con::cout<<"Found individual skybox textures for skybox '"<<materialPath<<"'! Generating cubemap texture..."<<Con::endl;

	// Merge textures into cubemap image

	// Note: We need to transfer the individual images from their compressed format to RGBA8UNORM, as well as
	// from optimal tiling to linear tiling. This can't be done in one step, because some GPUs do not support
	// images with linear tiling and mipmaps, so we need to create a new image which will contain the RGBA8 data and
	// optimal tiling, and then copy that data into a host-readable buffer.
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.flags = prosper::util::ImageCreateInfo::Flags::Cubemap | prosper::util::ImageCreateInfo::Flags::FullMipmapChain;
	imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	imgCreateInfo.width = largestWidth;
	imgCreateInfo.height = largestHeight;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::TransferDstOptimal;
	imgCreateInfo.tiling = prosper::ImageTiling::Optimal;
	imgCreateInfo.usage = prosper::ImageUsageFlags::TransferSrcBit;
	auto imgCubemap = c_engine->GetRenderContext().CreateImage(imgCreateInfo);
	auto numMipmaps = imgCubemap->GetMipmapCount();

	struct ImageBufferInfo
	{
		uint32_t layerIndex = 0;
		uint32_t mipmapIndex = 0;
		uint64_t bufferOffset = 0;
		uint64_t bufferSize = 0;

		uint32_t width = 0;
		uint32_t height = 0;
	};
	auto numBytesPerPixel = prosper::util::get_byte_size(imgCreateInfo.format);
	std::vector<ImageBufferInfo> imageBufferInfos {};
	imageBufferInfos.resize(numLayers *numMipmaps);
	uint32_t i = 0;
	uint64_t offset = 0;
	for(auto iLayer=decltype(numLayers){0u};iLayer<numLayers;++iLayer)
	{
		for(auto iMipmap=decltype(numMipmaps){0u};iMipmap<numMipmaps;++iMipmap)
		{
			uint32_t wMipmap,hMipmap;
			prosper::util::calculate_mipmap_size(largestWidth,largestHeight,&wMipmap,&hMipmap,iMipmap);
			auto &imgBufferInfo = imageBufferInfos.at(i++);
			imgBufferInfo.width = wMipmap;
			imgBufferInfo.height = hMipmap;
			imgBufferInfo.layerIndex = iLayer;
			imgBufferInfo.mipmapIndex = iMipmap;
			imgBufferInfo.bufferOffset = offset;
			imgBufferInfo.bufferSize = wMipmap *hMipmap *numBytesPerPixel;

			offset += imgBufferInfo.bufferSize;
		}
	}

	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUToCPU;
	bufCreateInfo.size = offset;
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::TransferDstBit;
	auto buf = c_engine->GetRenderContext().CreateBuffer(bufCreateInfo);

	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	for(auto &imgBufferInfo : imageBufferInfos)
	{
		auto &img = cubemapImages.at(imgBufferInfo.layerIndex);

		// Blit image into cubemap image
		prosper::util::ImageSubresourceRange range {};
		range.baseMipLevel = imgBufferInfo.mipmapIndex;
		setupCmd->RecordImageBarrier(*img,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferSrcOptimal,range);
		prosper::util::BlitInfo blitInfo {};
		blitInfo.dstSubresourceLayer.mipLevel = imgBufferInfo.mipmapIndex;
		setupCmd->RecordBlitImage(blitInfo,*img,*imgCubemap);
		setupCmd->RecordImageBarrier(*img,prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal,range);

		// Copy image data to host-readable buffer
		setupCmd->RecordImageBarrier(*imgCubemap,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::TransferSrcOptimal,range);
		// Note: No buffer barrier required, since we're writing to non-intersecting sections of the buffer
		prosper::util::BufferImageCopyInfo copyInfo {};
		copyInfo.width = imgBufferInfo.width;
		copyInfo.height = imgBufferInfo.height;
		copyInfo.mipLevel = imgBufferInfo.mipmapIndex;
		copyInfo.bufferOffset = imgBufferInfo.bufferOffset;
		setupCmd->RecordCopyImageToBuffer(copyInfo,*imgCubemap,prosper::ImageLayout::TransferSrcOptimal,*buf);
		setupCmd->RecordImageBarrier(*imgCubemap,prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::TransferDstOptimal,range);
	}
	c_engine->FlushSetupCommandBuffer();

	std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> cubemapBuffers {};
	cubemapBuffers.resize(numLayers);
	for(auto iLayer=decltype(numLayers){0u};iLayer<numLayers;++iLayer)
	{
		auto &mipmapBuffers = cubemapBuffers.at(iLayer);
		mipmapBuffers.reserve(numMipmaps);
		for(auto iMipmap=decltype(numMipmaps){0u};iMipmap<numMipmaps;++iMipmap)
		{
			auto &imgBufferInfo = imageBufferInfos.at(iLayer *numMipmaps +iMipmap);

			auto imgBuffer = uimg::ImageBuffer::Create(imgBufferInfo.width,imgBufferInfo.height,uimg::ImageBuffer::Format::RGBA8);
			buf->Read(imgBufferInfo.bufferOffset,imgBufferInfo.bufferSize,imgBuffer->GetData());
			mipmapBuffers.push_back(imgBuffer);
		}
	}

	std::vector<std::vector<const void*>> ptrCubemapBuffers {};
	ptrCubemapBuffers.resize(numLayers);
	for(auto iLayer=decltype(numLayers){0u};iLayer<numLayers;++iLayer)
	{
		ptrCubemapBuffers.at(iLayer).resize(numMipmaps);
		for(auto iMipmap=decltype(numMipmaps){0u};iMipmap<numMipmaps;++iMipmap)
			ptrCubemapBuffers.at(iLayer).at(iMipmap) = cubemapBuffers.at(iLayer).at(iMipmap)->GetData();
	}

	// Save the cubemap image on disk; It will automatically be reloaded
	uimg::TextureInfo imgWriteInfo {};
	imgWriteInfo.containerFormat = containerFormat;
	imgWriteInfo.inputFormat = uimg::TextureInfo::InputFormat::R8G8B8A8_UInt;
	imgWriteInfo.outputFormat = uimg::TextureInfo::OutputFormat::ColorMap;
	imgWriteInfo.wrapMode = uimg::TextureInfo::WrapMode::Clamp;
	auto fullPath = "addons/converted/materials/" +matName;
	auto szPerPixel = prosper::util::get_byte_size(prosper::Format::R8G8B8A8_UNorm);
	if(c_game->SaveImage(ptrCubemapBuffers,largestWidth,largestHeight,szPerPixel,fullPath,imgWriteInfo,true))
	{
		Con::cout<<"Skybox cubemap texture saved as '"<<fullPath<<"'! Generating material..."<<Con::endl;
		auto *mat = client->CreateMaterial("skybox");
		mat->GetDataBlock()->AddValue("texture","skybox",matName);
		if(mat->Save(matName,"addons/converted/"))
		{
			client->LoadMaterial(matName,true);
			Con::cout<<"Skybox material saved as '"<<(matName +".wmi")<<"'"<<Con::endl;
			return true;
		}
		else
			Con::cwar<<"WARNING: Unable to save skybox material as '"<<(matName +".wmi")<<"'!"<<Con::endl;
	}
	else
		Con::cwar<<"WARNING: Unable to save skybox cubemap texture as '"<<fullPath<<"'!"<<Con::endl;
	return false;
}
void CSkyboxComponent::ValidateMaterials()
{
	// TODO: Move this to Source Engine porting module?
	auto mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	auto &textures = mdl->GetMetaInfo().textures;
	auto &materials = mdl->GetMaterials();
	auto &texturePaths = mdl->GetMetaInfo().texturePaths;
	if(materials.empty() || textures.empty() || texturePaths.empty())
		return;
	auto &mat = materials.front();
	auto &texture = textures.front();
	auto &texturePath = texturePaths.front();
	if(mat.IsValid() && mat->GetTextureInfo("skybox"))
		return; // Skybox is valid; Skip the material
	// Attempt to use HDR textures, otherwise LDR
	if(CreateCubemapFromIndividualTextures(texturePath +texture +".wmi","_hdr") || CreateCubemapFromIndividualTextures(texturePath +texture +".wmi"))
	{
		mdl->LoadMaterials([](const std::string &str,bool b) -> Material* {
			return client->LoadMaterial(str,b);
		});
	}
}

void CSkybox::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSkyboxComponent>();
}
#pragma optimize("",on)
