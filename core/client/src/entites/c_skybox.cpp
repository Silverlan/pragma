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
#include <prosper_command_buffer.hpp>
#include <pr_dds.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

using namespace pragma;

LINK_ENTITY_TO_CLASS(skybox,CSkybox);

#pragma optimize("",off)
void CSkyboxComponent::Initialize()
{
	BaseSkyboxComponent::Initialize();
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto pRenderComponent = ent.GetComponent<pragma::CRenderComponent>();
	if(pRenderComponent.valid())
	{
		pRenderComponent->SetRenderMode(RenderMode::Skybox);
		pRenderComponent->SetCastShadows(false);
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
void CSkyboxComponent::OnRemove()
{
	BaseSkyboxComponent::OnRemove();
	if(m_cbOnModelMaterialsLoaded.IsValid())
		m_cbOnModelMaterialsLoaded.Remove();
}
luabind::object CSkyboxComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CSkyboxComponentHandleWrapper>(l);}
bool CSkyboxComponent::CreateCubemapFromIndividualTextures(Material &mat,const std::string &postfix) const
{
	// Check if this skybox is made of individual textures
	// (e.g. if it came from the source engine)
	auto matName = mat.GetName();
	ufile::remove_extension_from_filename(matName);

	auto containerFormat = ImageWriteInfo::ContainerFormat::KTX;
	auto ext = "ktx";
	if(FileManager::Exists("materials/" +matName +"." +ext))
		return true; // Skybox texture already exists; There's nothing for us to do

	// Load all sides as textures
	const std::array<std::string,6> sidePostfixes {
		"ft","bk","up","dn","rt","lf"
	};
	std::array<std::shared_ptr<prosper::Image>,6> cubemapImages {};
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
		if(texture->texture == nullptr)
			return false;
		auto &img = texture->texture->GetImage();
		cubemapImages.at(i) = img;
		auto extents = img->GetExtents();
		largestWidth = umath::max(largestWidth,extents.width);
		largestHeight = umath::max(largestHeight,extents.height);
	}
	Con::cout<<"Found individual skybox textures for skybox '"<<mat.GetName()<<"'! Generating cubemap texture..."<<Con::endl;

	// Merge textures into cubemap image
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.flags = prosper::util::ImageCreateInfo::Flags::Cubemap | prosper::util::ImageCreateInfo::Flags::FullMipmapChain;
	imgCreateInfo.format = Anvil::Format::R8G8B8A8_UNORM;
	imgCreateInfo.width = largestWidth;
	imgCreateInfo.height = largestHeight;
	imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::CPUToGPU;
	imgCreateInfo.postCreateLayout = Anvil::ImageLayout::TRANSFER_DST_OPTIMAL;
	imgCreateInfo.tiling = Anvil::ImageTiling::LINEAR;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT;
	auto imgCubemap = prosper::util::create_image(c_engine->GetDevice(),imgCreateInfo);
	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	for(auto iLayer=decltype(cubemapImages.size()){0u};iLayer<cubemapImages.size();++iLayer)
	{
		auto &img = cubemapImages.at(iLayer);

		prosper::util::record_image_barrier(**setupCmd,**img,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		prosper::util::BlitInfo blitInfo {};
		blitInfo.dstSubresourceLayer.base_array_layer = iLayer;
		prosper::util::record_blit_image(**setupCmd,blitInfo,**img,**imgCubemap);
		prosper::util::record_image_barrier(**setupCmd,**img,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	}
	c_engine->FlushSetupCommandBuffer();

	// Save the cubemap image on disk; It will automatically be reloaded
	ImageWriteInfo imgWriteInfo {};
	imgWriteInfo.containerFormat = ImageWriteInfo::ContainerFormat::KTX;;
	imgWriteInfo.inputFormat = ImageWriteInfo::InputFormat::R8G8B8A8_UInt;
	imgWriteInfo.outputFormat = ImageWriteInfo::OutputFormat::ColorMap;
	imgWriteInfo.wrapMode = ImageWriteInfo::WrapMode::Clamp;
	auto fullPath = "addons/converted/materials/" +matName;
	if(c_game->SaveImage(*imgCubemap,fullPath,imgWriteInfo))
	{
		Con::cout<<"Skybox cubemap texture saved as '"<<fullPath<<"'! Generating material..."<<Con::endl;
		auto *mat = client->CreateMaterial("skybox");
		mat->GetDataBlock()->AddValue("texture","skybox",matName);
		if(mat->Save(matName,"addons/converted/"))
		{
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
	auto mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	for(auto &hMat : mdl->GetMaterials())
	{
		if(hMat.IsValid() == false)
			continue;
		auto *skyboxTex = hMat->GetTextureInfo("skybox");
		if(skyboxTex != nullptr)
			continue; // Skybox is valid; Skip this material
		// Attempt to use HDR textures
		if(CreateCubemapFromIndividualTextures(*hMat.get(),"_hdr"))
			continue;
		// Try LDR textures instead
		CreateCubemapFromIndividualTextures(*hMat.get());
	}
}

void CSkybox::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSkyboxComponent>();
}
#pragma optimize("",on)
