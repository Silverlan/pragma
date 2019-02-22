#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_loading.hpp"

using namespace pragma;

extern DLLCLIENT ClientState *client;

ShaderLoading::ShaderLoading(prosper::Context &context,const std::string &identifier)
	: ShaderTextured3DBase(context,identifier,"world/vs_textured","world/fs_loading")
{
	SetBaseShader<ShaderTextured3D>();
}

bool ShaderLoading::BindMaterial(CMaterial&)
{
	auto *mat = client->GetMaterialManager().GetErrorMaterial();
	if(mat == nullptr)
		return false;
	return ShaderTextured3DBase::BindMaterial(*static_cast<CMaterial*>(mat));
}

 // prosper TODO
#if 0
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "c_shader_loading.h"
#include "pragma/model/c_side.h"
#include <texturemanager/texturemanager.h>
#include "textureinfo.h"
#include "pragma/model/c_modelmesh.h"
#include <cmaterialmanager.h>
#include <cmaterial.h>

using namespace Shader;

LINK_SHADER_TO_CLASS(Loading,loading);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

Loading::Loading()
	: Textured3D("loading","world/vs_textured","world/fs_loading")
{}

void Loading::InitializeMaterialBindings(std::vector<Vulkan::DescriptorSetLayout::Binding> &bindings)
{
	bindings = {
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // We don't actually sample any images, but we need to specify at least one dummy image
	};
}

void Loading::BindMaterialParameters(Material*)
{
	auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	auto &layout = GetPipeline()->GetPipelineLayout();

	int32_t maps = 0;
	float parallaxHeightScale = DEFAULT_PARALLAX_HEIGHT_SCALE;
	float phongScale = 0.f;
	int32_t glowMode = 0;
	float glowScale = 0.f;
	Vulkan::Std140LayoutBlockData material(5);
	material<<maps<<glowMode<<glowScale<<phongScale<<parallaxHeightScale;
	drawCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT,static_cast<uint32_t>(material.GetCount()),material.GetData());
}

void Loading::InitializeMaterial(Material *mat,bool bReload)
{
	auto &descSet = InitializeDescriptorSet(mat,bReload);

	auto *diffuseMap = mat->GetDiffuseMap();
	if(diffuseMap != nullptr && diffuseMap->texture != nullptr)
	{
		auto texture = std::static_pointer_cast<Texture>(diffuseMap->texture);
		descSet->Update(static_cast<uint32_t>(Binding::DiffuseMap),texture->vkTexture);
	}
}

void Loading::BindMaterial(Material *mat)
{
	BindMaterialParameters(mat);
}
#endif