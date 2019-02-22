#include "stdafx_client.h"
 // prosper TODO
#if 0
#include "pragma/rendering/shaders/c_shader_screen_clip.hpp"

extern DLLCENGINE CEngine *c_engine;

using namespace Shader;

LINK_SHADER_TO_CLASS(ScreenClip,screen_clip);

ScreenClip::ScreenClip(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: Screen(identifier,vsShader,fsShader,gsShader)
{}

ScreenClip::ScreenClip()
	: ScreenClip("screen_clip","screen/vs_screen_clip","screen/fs_screen")
{}

void ScreenClip::BindScene(const Vulkan::CommandBufferObject *drawCmd,const Scene &scene)
{
	auto &cam = scene.GetCamera();
	std::array<float,3> pushConstants;
	cam->GetNearPlaneBounds(&pushConstants.at(0),&pushConstants.at(1));
	pushConstants.at(2) = cam->GetZNear();
	drawCmd->PushConstants(GetPipeline()->GetPipelineLayout(),Anvil::ShaderStageFlagBits::VERTEX_BIT,(sizeof(Mat4) +sizeof(Vector4)) /sizeof(float),(pushConstants.size() *sizeof(pushConstants.front())) /sizeof(float),pushConstants.data());
}

void ScreenClip::Draw(const Vulkan::DescriptorSet &descTexture,const Mat4 &mvp,const Vector4 &clipPlane)
{
	auto &context = c_engine->GetRenderContext();
	auto &drawCmd = context.GetDrawCmd();
	auto &layout = GetPipeline()->GetPipelineLayout();
	drawCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::VERTEX_BIT,sizeof(Mat4) /sizeof(float),&mvp);
	drawCmd->PushConstants(layout,Anvil::ShaderStageFlagBits::VERTEX_BIT,sizeof(Mat4) /sizeof(float),sizeof(Vector4) /sizeof(float),&clipPlane);
	Screen::Draw(descTexture);
}

void ScreenClip::InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants)
{
	Screen::InitializePipelineLayout(context,setLayouts,pushConstants);
	pushConstants.push_back({
		Anvil::ShaderStageFlagBits::VERTEX_BIT,0,(sizeof(Mat4) +sizeof(Vector4) +sizeof(float) *3) /sizeof(float)
	});
}
#endif