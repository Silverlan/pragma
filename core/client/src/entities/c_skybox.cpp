/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/c_skybox.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/rendering/shaders/c_shader_equirectangular_to_cubemap.hpp"
#include "pragma/rendering/shaders/c_shader_cubemap_to_equirectangular.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/model/c_model.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/asset/util_asset.hpp>
#include <pragma/game/game_resources.hpp>
#include <sharedutils/util_file.h>
#include <util_image_buffer.hpp>
#include <util_texture_info.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <image/prosper_image.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <buffers/prosper_buffer.hpp>
#include <cmaterial.h>

import pragma.client.entities.components;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

using namespace pragma;

LINK_ENTITY_TO_CLASS(skybox, CSkybox);

void CSkyboxComponent::Initialize()
{
	BaseSkyboxComponent::Initialize();
	auto &ent = static_cast<CBaseEntity &>(GetEntity());
	auto pRenderComponent = ent.GetComponent<pragma::CRenderComponent>();
	if(pRenderComponent.valid()) {
		pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::Sky);
		pRenderComponent->SetCastShadows(false);
		pRenderComponent->SetDepthPassEnabled(false);
	}

	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(m_cbOnModelMaterialsLoaded.IsValid())
			m_cbOnModelMaterialsLoaded.Remove();
		auto &evMdl = static_cast<pragma::CEOnModelChanged &>(evData.get());
		if(evMdl.model == nullptr)
			return;
		m_cbOnModelMaterialsLoaded = evMdl.model->CallOnMaterialsLoaded([this]() { ValidateMaterials(); });
	});
	SetSkyAngles({});
}
void CSkyboxComponent::ReceiveData(NetPacket &packet) { m_skyAngles = packet->Read<EulerAngles>(); }
Bool CSkyboxComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetSkyAngles)
		SetSkyAngles(packet->Read<EulerAngles>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
void CSkyboxComponent::OnRemove()
{
	BaseSkyboxComponent::OnRemove();
	if(m_cbOnModelMaterialsLoaded.IsValid())
		m_cbOnModelMaterialsLoaded.Remove();
}
void CSkyboxComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CSkyboxComponent::SetSkyAngles(const EulerAngles &ang)
{
	m_skyAngles = ang;
	Vector3 axis;
	float angle;
	uquat::to_axis_angle(uquat::create(ang), axis, angle);
	m_renderSkyAngles = {axis, angle};
}
const EulerAngles &CSkyboxComponent::GetSkyAngles() const { return m_skyAngles; }
const Vector4 &CSkyboxComponent::GetRenderSkyAngles() const { return m_renderSkyAngles; }
bool CSkyboxComponent::CreateCubemapFromIndividualTextures(const std::string &materialPath, const std::string &postfix) const
{
	// Check if this skybox is made of individual textures
	// (e.g. if it came from the source engine)
	auto matName = materialPath;
	ufile::remove_extension_from_filename(matName, pragma::asset::get_supported_extensions(pragma::asset::Type::Material, pragma::asset::FormatType::All));

	auto containerFormat = uimg::TextureInfo::ContainerFormat::DDS;
	auto ext = "dds";
	if(FileManager::Exists("materials/" + matName + "." + ext) && pragma::asset::exists(matName, pragma::asset::Type::Material))
		return true; // Skybox texture already exists; There's nothing for us to do

	// Load all sides as textures
	const std::array<std::string, 6> sidePostfixes {"ft", "bk", "up", "dn", "rt", "lf"};
	constexpr uint32_t numLayers = 6u;
	std::array<std::shared_ptr<prosper::IImage>, numLayers> cubemapImages {};
	auto bAllValid = true;
	uint32_t largestWidth = 0u;
	uint32_t largestHeight = 0u;
	for(auto i = decltype(sidePostfixes.size()) {0u}; i < sidePostfixes.size(); ++i) {
		auto &sidePostFix = sidePostfixes.at(i);
		auto *matSide = client->LoadMaterial(matName + postfix + sidePostFix, nullptr, false, true);
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
		largestWidth = umath::max(largestWidth, extents.width);
		largestHeight = umath::max(largestHeight, extents.height);
	}
	Con::cout << "Found individual skybox textures for skybox '" << materialPath << "'! Generating cubemap texture..." << Con::endl;

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

	struct ImageBufferInfo {
		uint32_t layerIndex = 0;
		uint32_t mipmapIndex = 0;
		uint64_t bufferOffset = 0;
		uint64_t bufferSize = 0;

		uint32_t width = 0;
		uint32_t height = 0;
	};
	auto numBytesPerPixel = prosper::util::get_byte_size(imgCreateInfo.format);
	std::vector<ImageBufferInfo> imageBufferInfos {};
	imageBufferInfos.resize(numLayers * numMipmaps);
	uint32_t i = 0;
	uint64_t offset = 0;
	for(auto iLayer = decltype(numLayers) {0u}; iLayer < numLayers; ++iLayer) {
		for(auto iMipmap = decltype(numMipmaps) {0u}; iMipmap < numMipmaps; ++iMipmap) {
			uint32_t wMipmap, hMipmap;
			prosper::util::calculate_mipmap_size(largestWidth, largestHeight, &wMipmap, &hMipmap, iMipmap);
			auto &imgBufferInfo = imageBufferInfos.at(i++);
			imgBufferInfo.width = wMipmap;
			imgBufferInfo.height = hMipmap;
			imgBufferInfo.layerIndex = iLayer;
			imgBufferInfo.mipmapIndex = iMipmap;
			imgBufferInfo.bufferOffset = offset;
			imgBufferInfo.bufferSize = wMipmap * hMipmap * numBytesPerPixel;

			offset += imgBufferInfo.bufferSize;
		}
	}

	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUToCPU;
	bufCreateInfo.size = offset;
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::TransferDstBit;
	auto buf = c_engine->GetRenderContext().CreateBuffer(bufCreateInfo);

	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	for(auto &imgBufferInfo : imageBufferInfos) {
		auto &img = cubemapImages.at(imgBufferInfo.layerIndex);

		// Blit image into cubemap image
		prosper::util::ImageSubresourceRange range {};
		range.baseMipLevel = imgBufferInfo.mipmapIndex;
		setupCmd->RecordImageBarrier(*img, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferSrcOptimal, range);
		prosper::util::BlitInfo blitInfo {};
		blitInfo.dstSubresourceLayer.mipLevel = imgBufferInfo.mipmapIndex;
		setupCmd->RecordBlitImage(blitInfo, *img, *imgCubemap);
		setupCmd->RecordImageBarrier(*img, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal, range);

		// Copy image data to host-readable buffer
		setupCmd->RecordImageBarrier(*imgCubemap, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::TransferSrcOptimal, range);
		// Note: No buffer barrier required, since we're writing to non-intersecting sections of the buffer
		prosper::util::BufferImageCopyInfo copyInfo {};
		copyInfo.imageExtent = {imgBufferInfo.width, imgBufferInfo.height};
		copyInfo.mipLevel = imgBufferInfo.mipmapIndex;
		copyInfo.bufferOffset = imgBufferInfo.bufferOffset;
		setupCmd->RecordCopyImageToBuffer(copyInfo, *imgCubemap, prosper::ImageLayout::TransferSrcOptimal, *buf);
		setupCmd->RecordImageBarrier(*imgCubemap, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::TransferDstOptimal, range);
	}
	c_engine->FlushSetupCommandBuffer();

	std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> cubemapBuffers {};
	cubemapBuffers.resize(numLayers);
	for(auto iLayer = decltype(numLayers) {0u}; iLayer < numLayers; ++iLayer) {
		auto &mipmapBuffers = cubemapBuffers.at(iLayer);
		mipmapBuffers.reserve(numMipmaps);
		for(auto iMipmap = decltype(numMipmaps) {0u}; iMipmap < numMipmaps; ++iMipmap) {
			auto &imgBufferInfo = imageBufferInfos.at(iLayer * numMipmaps + iMipmap);

			auto imgBuffer = uimg::ImageBuffer::Create(imgBufferInfo.width, imgBufferInfo.height, uimg::Format::RGBA8);
			buf->Read(imgBufferInfo.bufferOffset, imgBufferInfo.bufferSize, imgBuffer->GetData());
			mipmapBuffers.push_back(imgBuffer);
		}
	}

	std::vector<std::vector<const void *>> ptrCubemapBuffers {};
	ptrCubemapBuffers.resize(numLayers);
	for(auto iLayer = decltype(numLayers) {0u}; iLayer < numLayers; ++iLayer) {
		ptrCubemapBuffers.at(iLayer).resize(numMipmaps);
		for(auto iMipmap = decltype(numMipmaps) {0u}; iMipmap < numMipmaps; ++iMipmap)
			ptrCubemapBuffers.at(iLayer).at(iMipmap) = cubemapBuffers.at(iLayer).at(iMipmap)->GetData();
	}

	// Save the cubemap image on disk; It will automatically be reloaded
	uimg::TextureInfo imgWriteInfo {};
	imgWriteInfo.containerFormat = containerFormat;
	imgWriteInfo.inputFormat = uimg::TextureInfo::InputFormat::R8G8B8A8_UInt;
	imgWriteInfo.outputFormat = uimg::TextureInfo::OutputFormat::ColorMap;
	imgWriteInfo.wrapMode = uimg::TextureInfo::WrapMode::Clamp;
	auto fullPath = "addons/converted/materials/" + matName;
	auto szPerPixel = prosper::util::get_byte_size(prosper::Format::R8G8B8A8_UNorm);
	if(c_game->SaveImage(ptrCubemapBuffers, largestWidth, largestHeight, szPerPixel, fullPath, imgWriteInfo, true)) {
		Con::cout << "Skybox cubemap texture saved as '" << fullPath << "'! Generating material..." << Con::endl;
		auto mat = client->CreateMaterial("skybox");
		mat->SetTextureProperty("skybox", matName);
		auto savePath = pragma::asset::relative_path_to_absolute_path(matName, pragma::asset::Type::Material, util::CONVERT_PATH);
		std::string err;
		if(mat->Save(savePath.GetString(), err, true)) {
			client->LoadMaterial(matName, nullptr, true, true);
			Con::cout << "Skybox material saved as '" << (matName + ".wmi") << "'" << Con::endl;
			return true;
		}
		else
			Con::cwar << "Unable to save skybox material as '" << (matName + ".wmi") << "': " << err << "!" << Con::endl;
	}
	else
		Con::cwar << "Unable to save skybox cubemap texture as '" << fullPath << "'!" << Con::endl;
	return false;
}
void CSkyboxComponent::ValidateMaterials()
{
	// TODO: Move this to Source Engine porting module?
	auto mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	auto mdlC = GetEntity().GetComponent<CModelComponent>();
	auto *mat = mdlC->GetRenderMaterial(0);
	if(!mat)
		return;
	auto &metaInfo = mdl->GetMetaInfo();
	auto &texturePaths = metaInfo.texturePaths;
	auto &texturePath = texturePaths.front();
	auto *texInfo = mat ? mat->GetTextureInfo("skybox") : nullptr;
	if(texInfo) {
		// Skybox is valid; Skip the material
		if(texInfo->texture) {
			auto &tex = *static_cast<Texture *>(texInfo->texture.get());
			auto &vkTex = tex.GetVkTexture();
			if(vkTex) {
				auto &img = vkTex->GetImage();
				if(img.IsCubemap() == false) {
					// Equirectangular skybox; We need to convert it to a skybox
					auto *shader = static_cast<pragma::ShaderEquirectangularToCubemap *>(c_engine->GetShader("equirectangular_to_cubemap").get());
					if(shader) {
						constexpr uint32_t resolution = 1024;
						auto tex = shader->EquirectangularTextureToCubemap(*vkTex, resolution);
						if(tex) {
							static_cast<CMaterial *>(mat)->SetTexture("skybox", *tex);
							static_cast<CMaterial *>(mat)->UpdateTextures();
						}
					}
				}
			}
		}
		return;
	}
	if(!metaInfo.textures.empty()) {
		// Attempt to use HDR textures, otherwise LDR
		auto texture = ufile::get_file_from_filename(metaInfo.textures.front());
		ufile::remove_extension_from_filename(texture, pragma::asset::get_supported_extensions(pragma::asset::Type::Material));
		if(CreateCubemapFromIndividualTextures(texturePath + texture + ".pmat", "_hdr") || CreateCubemapFromIndividualTextures(texturePath + texture + ".pmat"))
			mdl->LoadMaterials();
	}
}
void CSkyboxComponent::SetSkyMaterial(Material *mat)
{
	auto &ent = GetEntity();
	auto mdlC = ent.GetComponent<CModelComponent>();
	if(mdlC.expired())
		return;
	if(mat) {
		auto overrideC = ent.AddComponent<CMaterialOverrideComponent>();
		overrideC->SetMaterialOverride(0, static_cast<CMaterial &>(*mat));
	}
	else {
		auto overrideC = ent.GetComponent<CMaterialOverrideComponent>();
		if(overrideC.valid())
			overrideC->ClearMaterialOverride(0);
	}
	ValidateMaterials();
	mdlC->UpdateRenderMeshes();
}

void CSkybox::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSkyboxComponent>();
}

static void sky_override(NetworkState *, const ConVar &, std::string, std::string skyMat)
{
	if(c_game == nullptr)
		return;
	CMaterial *matSky = nullptr;
	if(skyMat.empty() == false)
		matSky = static_cast<CMaterial *>(client->LoadMaterial(skyMat, nullptr, false, true));
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CSkyboxComponent>>();
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CModelComponent>>();
	for(auto *ent : entIt)
		ent->GetComponent<CSkyboxComponent>()->SetSkyMaterial(matSky);
}
REGISTER_CONVAR_CALLBACK_CL(sky_override, sky_override);

#include "pragma/console/c_cvar_global_functions.h"
#include <cmaterial_manager2.hpp>
#include <texturemanager/texture_manager2.hpp>
#include <util_image.hpp>
#include <fsys/ifile.hpp>

enum class ConversionMode : uint8_t { CubemapToEquirectangular = 0, EquirectangularToCubemap };
#include <wgui/types/wirect.h>
static void util_convert_cubemap_equirect(std::vector<std::string> &argv, ConversionMode conversionMode)
{
	if(argv.empty()) {
		Con::cwar << "No image path has been specified!" << Con::endl;
		return;
	}
	auto &fileName = argv.front();
	auto assetFileName = pragma::asset::find_file(argv.front(), pragma::asset::Type::Texture);
	if(!assetFileName.has_value()) {
		Con::cwar << "Failed to locate texture file for '" << *assetFileName << "'!" << Con::endl;
		return;
	}

	std::string absPath;
	if(!FileManager::FindLocalPath("materials/" + *assetFileName, absPath)) {
		Con::cwar << "Failed to determine absolute path for texture file '" << *assetFileName << "'!" << Con::endl;
		return;
	}

	auto &matMan = static_cast<msys::CMaterialManager &>(client->GetMaterialManager());
	auto tex = matMan.GetTextureManager().LoadAsset(fileName, util::AssetLoadFlags::DontCache | util::AssetLoadFlags::IgnoreCache);
	if(!tex) {
		Con::cwar << "Texture '" << fileName << "' could not be loaded!" << Con::endl;
		return;
	}

	auto &vkTex = tex->GetVkTexture();
	{
		//auto el = WGUI::GetInstance().Create<WITexturedRect>();
		//el->SetTexture(*vkTex,0);
		//el->SetSize(512,512);
		//return;
		//uimg::TextureInfo texInfo {};
		//texInfo.containerFormat = uimg::TextureInfo::ContainerFormat::DDS;
		//prosper::util::save_texture("test.dds",vkTex->GetImage(),texInfo);
	}

	if(!vkTex) {
		Con::cwar << "Invalid texture!" << Con::endl;
		return;
	}

	auto outputFileName = util::Path::CreateFile(absPath);
	outputFileName.RemoveFileExtension(pragma::asset::get_supported_extensions(pragma::asset::Type::Texture));
	if(conversionMode == ConversionMode::EquirectangularToCubemap) {
		auto *shader = static_cast<pragma::ShaderEquirectangularToCubemap *>(c_engine->GetShader("equirectangular_to_cubemap").get());
		if(!shader) {
			Con::cwar << "Invalid shader!" << Con::endl;
			return;
		}
		auto texCubemap = shader->EquirectangularTextureToCubemap(*vkTex, 1'024);
		if(!texCubemap) {
			Con::cwar << "Conversion failed!" << Con::endl;
			return;
		}
		uimg::TextureInfo texInfo {};
		texInfo.containerFormat = uimg::TextureInfo::ContainerFormat::DDS;
		outputFileName += "_cubemap.dds";
		auto res = prosper::util::save_texture(outputFileName.GetString(), texCubemap->GetImage(), texInfo);
		if(!res) {
			Con::cwar << "Failed to save output texture '" << outputFileName.GetString() << "'!" << Con::endl;
			return;
		}
		Con::cout << "Successfully saved output file '" << outputFileName.GetString() << "'!" << Con::endl;
		return;
	}

	auto *shader = static_cast<pragma::ShaderCubemapToEquirectangular *>(c_engine->GetShader("cubemap_to_equirectangular").get());
	if(!shader) {
		Con::cwar << "Invalid shader!" << Con::endl;
		return;
	}
	auto texEqui = shader->CubemapToEquirectangularTexture(*vkTex);
	if(!texEqui) {
		Con::cwar << "Conversion failed!" << Con::endl;
		return;
	}

	// TODO: We should save it as HDR, but that causes weird artifacts. Investigate! Maybe add support for exr?
	auto saveAsHdr = false;
	auto imgBuf = texEqui->GetImage().ToHostImageBuffer(uimg::Format::RGBA16, prosper::ImageLayout::ShaderReadOnlyOptimal);
	if(!imgBuf) {
		Con::cwar << "Failed to generate host image buffer!" << Con::endl;
		return;
	}
	if(saveAsHdr)
		outputFileName += "_equirect.hdr";
	else
		outputFileName += "_equirect.png";
	auto f = filemanager::open_file(outputFileName.GetString(), filemanager::FileMode::Write | filemanager::FileMode::Binary);
	if(!f) {
		Con::cwar << "Failed to open output file '" << outputFileName.GetString() << "'!" << Con::endl;
		return;
	}
	fsys::File fp {f};
	auto res = uimg::save_image(fp, *imgBuf, saveAsHdr ? uimg::ImageFormat::HDR : uimg::ImageFormat::PNG);
	if(!res) {
		Con::cwar << "Failed to save output file '" << outputFileName.GetString() << "'!" << Con::endl;
		return;
	}
	Con::cout << "Successfully saved output file '" << outputFileName.GetString() << "'!" << Con::endl;
}
void Console::commands::util_convert_cubemap_to_equirectangular_image(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv) { util_convert_cubemap_equirect(argv, ConversionMode::CubemapToEquirectangular); }
void Console::commands::util_convert_equirectangular_image_to_cubemap(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv) { util_convert_cubemap_equirect(argv, ConversionMode::EquirectangularToCubemap); }
