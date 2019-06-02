#include "stdafx_openvr.h"
#include "vr_eye.hpp"
#include "vr_instance.hpp"
#include "wvmodule.h"
#include <prosper_context.hpp>
#include <image/prosper_image.hpp>
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_fence.hpp>
#include <prosper_command_buffer.hpp>

openvr::Eye::Eye(Instance *instance,vr::EVREye _eye)
	: 
#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
	fbo(0),texture(0),
#endif
	width(0),height(0),eye(_eye),scene(nullptr),m_instance(instance)
{}

openvr::Eye::~Eye()
{
#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
	if(fbo != 0)
		glDeleteFramebuffers(1,&fbo);
	if(texture != 0)
		glDeleteTextures(1,&texture);
#endif
}

bool openvr::Eye::Initialize(uint32_t w,uint32_t h)
{
#if LOPENVR_VERBOSE == 1
		std::cout<<"[VR] Initializing eye..."<<std::endl;
#endif
	width = w;
	height = h;

#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
	glGetError();
	glGenTextures(1,&texture);
	if(texture == 0)
		return false;
	glGenFramebuffers(1,&fbo);
	if(fbo == 0)
		return false; // Resources are freed through the destructor, no need to do it here!

	glBindFramebuffer(GL_FRAMEBUFFER,fbo);
	glBindTexture(GL_TEXTURE_2D,texture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0);
	auto err = glGetError();
	if(err != GL_NO_ERROR)
		return false;
	auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
		return false;
	vrTexture = {reinterpret_cast<void*>(texture),vr::TextureType_OpenGL,vr::EColorSpace::ColorSpace_Gamma};

	glDrawBuffer(GL_FRONT);
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
#ifdef _DEBUG
	glClearColor(1.f,0.f,0.f,1.f);
#else
	glClearColor(0.f,0.f,0.f,1.f);
#endif
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_TEXTURE_2D,0);
	glBindTexture(GL_FRAMEBUFFER,0);
#endif

	auto bHostVisible = false;
#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
	bHostVisible = true;
#endif
	auto &context = const_cast<prosper::Context&>(IState::get_render_context());
	auto format = Anvil::Format::R8G8B8A8_UNORM;
#if LOPENVR_VERBOSE == 1
		std::cout<<"[VR] Creating render target..."<<std::endl;
#endif
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.width = w;
	imgCreateInfo.height = h;
	imgCreateInfo.format = format;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;
	imgCreateInfo.postCreateLayout = Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
	imgCreateInfo.tiling = vk::ImageTiling::eLinear;
	imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::CPUToGPU;
#else
	imgCreateInfo.tiling = Anvil::ImageTiling::OPTIMAL;
	imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
#endif
	auto &dev = context.GetDevice();
	auto img = prosper::util::create_image(dev,imgCreateInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto tex = prosper::util::create_texture(dev,{},img,&imgViewCreateInfo,&samplerCreateInfo);
	prosper::util::RenderPassCreateInfo rpCreateInfo {
		{{format,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::DONT_CARE,Anvil::AttachmentStoreOp::STORE,Anvil::SampleCountFlagBits::_1_BIT,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL}}
	};
	auto rp = prosper::util::create_render_pass(dev,rpCreateInfo);
	vkRenderTarget = prosper::util::create_render_target(dev,{tex},rp,{});
	if(vkRenderTarget == nullptr)
		return false;
	auto mainScene = IState::get_main_scene();
#if LOPENVR_VERBOSE == 1
		std::cout<<"[VR] Creating render scene..."<<std::endl;
#endif
	scene = IScene::Create(w,h,mainScene.GetFOV(),mainScene.GetViewFOV(),static_cast<float>(w) /static_cast<float>(h),mainScene.GetZNear(),mainScene.GetZFar());
	scene.InitializeRenderTarget();
	scene.SetWorldEnvironment(mainScene.GetWorldEnvironment());
	scene.SetLightSourceListInfo(mainScene.GetLightSourceListInfo());
	scene.LinkEntities(mainScene);
	auto *vrInterface = m_instance->GetSystemInterface();
	auto mProj = vrInterface->GetProjectionMatrix(eye,scene.GetZNear(),scene.GetZFar());
	auto m = glm::transpose(reinterpret_cast<Mat4&>(mProj.m));
	m = glm::scale(m,Vector3(1.f,-1.f,1.f));
	scene.SetProjectionMatrix(reinterpret_cast<IMat4&>(m));

#ifdef USE_VULKAN
	auto extents = img->GetExtents();
	auto queueFamily = 0u;

	auto *queue = dev.get_universal_queue(0u);
	vrTextureData.m_nImage = reinterpret_cast<uint64_t>((*img)->get_image());
	vrTextureData.m_pDevice = dev.get_device_vk();
	vrTextureData.m_pPhysicalDevice = dev.get_physical_device()->get_physical_device();
	vrTextureData.m_pInstance = dev.get_parent_instance()->get_instance_vk();
	vrTextureData.m_pQueue = queue->get_queue();
	vrTextureData.m_nQueueFamilyIndex = queue->get_queue_family_index();
	vrTextureData.m_nWidth = extents.width;
	vrTextureData.m_nHeight = extents.height;
	vrTextureData.m_nFormat = umath::to_integral(img->GetFormat());
	vrTextureData.m_nSampleCount = umath::to_integral(img->GetSampleCount());
	vrTexture = {&vrTextureData,vr::TextureType_Vulkan,vr::EColorSpace::ColorSpace_Auto};
#endif
#if LOPENVR_VERBOSE == 1
		std::cout<<"[VR] Done!"<<std::endl;
#endif
	return true;
}
void openvr::Eye::UpdateImage(const prosper::Image &src)
{
#if USE_OPENGL_OFFSCREEN_CONTEXT
	auto &mem = src->GetMemory();
	auto map = mem->MapMemory();
	auto *data = static_cast<uint8_t*>(map->GetData());
	glBindTexture(GL_TEXTURE_2D,texture);
		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,m_instance->GetWidth(),m_instance->GetHeight(),GL_RGBA,GL_UNSIGNED_BYTE,data);
	glBindTexture(GL_TEXTURE_2D,0);
#endif
}

Mat4 openvr::Eye::GetEyeViewMatrix() const
{
	auto *vrInterface = m_instance->GetSystemInterface();
	auto &matView = reinterpret_cast<const Mat4&>(scene.GetViewMatrix());

	auto eyeTransform = vrInterface->GetEyeToHeadTransform(eye);
	Mat4 m(
		eyeTransform.m[0][0],eyeTransform.m[1][0],eyeTransform.m[2][0],0.f,
		eyeTransform.m[0][1],eyeTransform.m[1][1],eyeTransform.m[2][1],0.f,
		eyeTransform.m[0][2],eyeTransform.m[1][2],eyeTransform.m[2][2],0.f,
		eyeTransform.m[0][3],eyeTransform.m[1][3],eyeTransform.m[2][3],1.f
	);
	return matView *glm::inverse(m);
}

Mat4 openvr::Eye::GetEyeProjectionMatrix(float nearZ,float farZ) const
{
	auto *vrInterface = m_instance->GetSystemInterface();
	auto eyeProj = vrInterface->GetProjectionMatrix(eye,nearZ,farZ);
	return Mat4{
		eyeProj.m[0][0],eyeProj.m[1][0],eyeProj.m[2][0],eyeProj.m[3][0],
		eyeProj.m[0][1],eyeProj.m[1][1],eyeProj.m[2][1],eyeProj.m[3][1],
		eyeProj.m[0][2],eyeProj.m[1][2],eyeProj.m[2][2],eyeProj.m[3][2],
		eyeProj.m[0][3],eyeProj.m[1][3],eyeProj.m[2][3],eyeProj.m[3][3]
	};
}
