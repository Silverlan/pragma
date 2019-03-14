#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_depth_to_rgb.h"
#include <prosper_context.hpp>
#include <prosper_util.hpp>
#include <prosper_util_square_shape.hpp>
#include <buffers/prosper_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

decltype(ShaderDepthToRGB::VERTEX_BINDING_VERTEX) ShaderDepthToRGB::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderDepthToRGB::VERTEX_ATTRIBUTE_POSITION) ShaderDepthToRGB::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,prosper::util::get_square_vertex_format()};
decltype(ShaderDepthToRGB::VERTEX_ATTRIBUTE_UV) ShaderDepthToRGB::VERTEX_ATTRIBUTE_UV = {VERTEX_BINDING_VERTEX,prosper::util::get_square_uv_format()};

decltype(ShaderDepthToRGB::DESCRIPTOR_SET) ShaderDepthToRGB::DESCRIPTOR_SET = {
	{
		prosper::Shader::DescriptorSetInfo::Binding {
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
ShaderDepthToRGB::ShaderDepthToRGB(prosper::Context &context,const std::string &identifier,const std::string &fsShader)
	: ShaderGraphics(context,identifier,"screen/vs_screen_uv",fsShader)
{}
ShaderDepthToRGB::ShaderDepthToRGB(prosper::Context &context,const std::string &identifier)
	: ShaderDepthToRGB(context,identifier,"debug/fs_depth_to_rgb")
{}

void ShaderDepthToRGB::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	ToggleDynamicScissorState(pipelineInfo,true);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_UV);

	AttachPushConstantRange(pipelineInfo,0u,GetPushConstantSize(),Anvil::ShaderStageFlagBits::FRAGMENT_BIT);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET);
}

template<class TPushConstants>
	bool ShaderDepthToRGB::Draw(Anvil::DescriptorSet &descSetDepthTex,const TPushConstants &pushConstants)
{
	auto &dev = c_engine->GetDevice();
	return RecordBindVertexBuffers({&prosper::util::get_square_vertex_uv_buffer(dev)->GetAnvilBuffer()}) == true &&
		RecordBindDescriptorSet(descSetDepthTex) == true &&
		RecordPushConstants(pushConstants) == true &&
		RecordDraw(prosper::util::get_square_vertex_count()) == true;
}

bool ShaderDepthToRGB::Draw(Anvil::DescriptorSet &descSetDepthTex,float nearZ,float farZ,float contrastFactor)
{
	return Draw(descSetDepthTex,PushConstants{
		nearZ,farZ,contrastFactor
	});
}

uint32_t ShaderDepthToRGB::GetPushConstantSize() const {return sizeof(PushConstants);}

//////////////////////

ShaderCubeDepthToRGB::ShaderCubeDepthToRGB(prosper::Context &context,const std::string &identifier)
	: ShaderDepthToRGB(context,identifier,"debug/fs_cube_depth_to_rgb")
{}

bool ShaderCubeDepthToRGB::Draw(Anvil::DescriptorSet &descSetDepthTex,float nearZ,float farZ,uint32_t cubeSide,float contrastFactor)
{
	return ShaderDepthToRGB::Draw(descSetDepthTex,PushConstants{
		{nearZ,farZ,contrastFactor},static_cast<int32_t>(cubeSide)
	});
}

uint32_t ShaderCubeDepthToRGB::GetPushConstantSize() const {return sizeof(PushConstants);}

//////////////////////

ShaderCSMDepthToRGB::ShaderCSMDepthToRGB(prosper::Context &context,const std::string &identifier)
	: ShaderDepthToRGB(context,identifier,"debug/fs_csm_depth_to_rgb")
{}

bool ShaderCSMDepthToRGB::Draw(Anvil::DescriptorSet &descSetDepthTex,float nearZ,float farZ,uint32_t layer,float contrastFactor)
{
	return ShaderDepthToRGB::Draw(descSetDepthTex,PushConstants{
		{nearZ,farZ,contrastFactor},static_cast<int32_t>(layer)
	});
}

uint32_t ShaderCSMDepthToRGB::GetPushConstantSize() const {return sizeof(PushConstants);}

#if 0

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

using namespace Shader;

LINK_SHADER_TO_CLASS(DepthToRGB,depthtorgb);
LINK_SHADER_TO_CLASS(CubeDepthToRGB,cubedepthtorgb);
LINK_SHADER_TO_CLASS(CSMDepthToRGB,csmdepthtorgb);

DepthToRGB::DepthToRGB(const std::string &fs)
	: Screen("depthtorgb","screen/vs_screen_uv",fs)
{}

DepthToRGB::DepthToRGB()
	: DepthToRGB("debug/fs_depth_to_rgb")
{}

void DepthToRGB::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Base::InitializePipelineLayout(context,setLayouts,pushConstants);

	pushConstants.push_back({Anvil::ShaderStageFlagBits::FRAGMENT_BIT,2});

	setLayouts.push_back(Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT}
	}));
}

bool DepthToRGB::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline,float nearZ,float farZ)
{
	if(Screen::BeginDraw(cmdBuffer,shaderPipeline) == false)
		return false;
	Vector2 plane{nearZ,farZ};
	cmdBuffer->PushConstants(GetPipeline()->GetPipelineLayout(),Anvil::ShaderStageFlagBits::FRAGMENT_BIT,2,&plane);
	return true;
}
bool DepthToRGB::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline)
{
	auto &scene = c_game->GetScene();
	return BeginDraw(cmdBuffer,shaderPipeline,scene->GetZNear(),scene->GetZFar());
}

////////////////////////////////

CubeDepthToRGB::CubeDepthToRGB()
	: DepthToRGB("debug/fs_cube_depth_to_rgb")
{}
void CubeDepthToRGB::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	DepthToRGB::InitializePipelineLayout(context,setLayouts,pushConstants);
	pushConstants = {
		{Anvil::ShaderStageFlagBits::FRAGMENT_BIT,3}
	};
}
bool CubeDepthToRGB::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline,float nearZ,float farZ,int32_t cubeSide)
{
	if(Screen::BeginDraw(cmdBuffer,shaderPipeline) == false)
		return false;
	Vulkan::Std140LayoutBlockData pushConstants(3);
	pushConstants<<nearZ<<farZ<<cubeSide;
	cmdBuffer->PushConstants(GetPipeline()->GetPipelineLayout(),Anvil::ShaderStageFlagBits::FRAGMENT_BIT,static_cast<uint32_t>(pushConstants.GetCount()),pushConstants.GetData());
	return true;
}

////////////////////////////////

CSMDepthToRGB::CSMDepthToRGB()
	: DepthToRGB("debug/fs_csm_depth_to_rgb")
{}
void CSMDepthToRGB::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	DepthToRGB::InitializePipelineLayout(context,setLayouts,pushConstants);
	pushConstants = {
		{Anvil::ShaderStageFlagBits::FRAGMENT_BIT,3}
	};
}
bool CSMDepthToRGB::BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline,float nearZ,float farZ,int32_t layer)
{
	if(Screen::BeginDraw(cmdBuffer,shaderPipeline) == false)
		return false;
	Vulkan::Std140LayoutBlockData pushConstants(3);
	pushConstants<<nearZ<<farZ<<layer;
	cmdBuffer->PushConstants(GetPipeline()->GetPipelineLayout(),Anvil::ShaderStageFlagBits::FRAGMENT_BIT,static_cast<uint32_t>(pushConstants.GetCount()),pushConstants.GetData());
	return true;
}
#endif
// prosper TODO