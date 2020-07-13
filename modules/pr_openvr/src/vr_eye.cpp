#include "stdafx_openvr.h"
#include "vr_eye.hpp"
#include "vr_instance.hpp"
#include "wvmodule.h"
#include <pragma/c_engine.h>
#include <pragma/game/c_game.h>
#include <pragma/entities/environment/c_env_camera.h>
#include <pragma/rendering/renderers/rasterization_renderer.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/components/c_light_map_component.hpp>
#include <pragma/entities/c_world.h>
#include <prosper_context.hpp>
#include <image/prosper_image.hpp>
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_fence.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

openvr::Eye::Eye(Instance *instance,vr::EVREye eye)
	: 
#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
	fbo(0),texture(0),
#endif
	width(0),height(0),eye(eye),m_instance(instance)
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
	auto &context = const_cast<prosper::IPrContext&>(IState::get_render_context());
	auto format = prosper::Format::R8G8B8A8_UNorm;
#if LOPENVR_VERBOSE == 1
		std::cout<<"[VR] Creating render target..."<<std::endl;
#endif
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.width = w;
	imgCreateInfo.height = h;
	imgCreateInfo.format = format;
	imgCreateInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferDstBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::SampledBit;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
	imgCreateInfo.tiling = vk::ImageTiling::eLinear;
	imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::CPUToGPU;
#else
	imgCreateInfo.tiling = prosper::ImageTiling::Optimal;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
#endif
	auto img = context.CreateImage(imgCreateInfo);
	auto tex = context.CreateTexture({},*img,prosper::util::ImageViewCreateInfo{},prosper::util::SamplerCreateInfo{});
	prosper::util::RenderPassCreateInfo rpCreateInfo {
		{{format,prosper::ImageLayout::ColorAttachmentOptimal,prosper::AttachmentLoadOp::DontCare,prosper::AttachmentStoreOp::Store,prosper::SampleCountFlags::e1Bit,prosper::ImageLayout::ShaderReadOnlyOptimal}}
	};
	auto rp = context.CreateRenderPass(rpCreateInfo);
	vkRenderTarget = context.CreateRenderTarget({tex},rp,{});
	if(vkRenderTarget == nullptr)
		return false;
	auto mainScene = IState::get_main_scene();
#if LOPENVR_VERBOSE == 1
		std::cout<<"[VR] Creating render scene..."<<std::endl;
#endif

	renderer = pragma::rendering::RasterizationRenderer::Create<pragma::rendering::RasterizationRenderer>(mainScene.GetInternalScene());

	auto *cam = c_game->GetPrimaryCamera();
	auto *camEye = cam ? c_game->CreateCamera(width,height,cam->GetFOV(),cam->GetNearZ(),cam->GetFarZ()) : nullptr;
	if(camEye)
	{
		camera = camEye->GetHandle<pragma::CCameraComponent>();
		auto *vrInterface = m_instance->GetSystemInterface();
		auto mProj = vrInterface->GetProjectionMatrix(eye,camEye->GetNearZ(),camEye->GetFarZ());
		auto m = glm::transpose(reinterpret_cast<Mat4&>(mProj.m));
		m = glm::scale(m,Vector3(1.f,-1.f,1.f));
		camEye->SetProjectionMatrix(m);
	}

#ifdef USE_VULKAN
	auto extents = img->GetExtents();
	auto queueFamily = 0u;

	//auto *queue = dev.get_universal_queue(0u);
	auto &renderContext = c_engine->GetRenderContext();
	vrTextureData.m_nImage = reinterpret_cast<uint64_t>(img->GetInternalHandle());
	vrTextureData.m_pDevice = static_cast<VkDevice_T*>(renderContext.GetInternalDevice());
	vrTextureData.m_pPhysicalDevice = static_cast<VkPhysicalDevice_T*>(renderContext.GetInternalPhysicalDevice());
	vrTextureData.m_pInstance = static_cast<VkInstance_T*>(renderContext.GetInternalInstance());
	vrTextureData.m_pQueue = static_cast<VkQueue_T*>(renderContext.GetInternalUniversalQueue());
	vrTextureData.m_nQueueFamilyIndex = renderContext.GetUniversalQueueFamilyIndex();
	vrTextureData.m_nWidth = extents.width;
	vrTextureData.m_nHeight = extents.height;
	vrTextureData.m_nFormat = umath::to_integral(img->GetFormat());
	vrTextureData.m_nSampleCount = umath::to_integral(img->GetSampleCount());
	vrTexture = {&vrTextureData /* handle */,vr::TextureType_Vulkan,vr::EColorSpace::ColorSpace_Auto};
	//TextureType_OpenGL = 1,  // Handle is an OpenGL texture name or an OpenGL render buffer name, depending on submit flags
#endif
#if LOPENVR_VERBOSE == 1
		std::cout<<"[VR] Done!"<<std::endl;
#endif
	return true;
}
void openvr::Eye::UpdateImage(const prosper::IImage &src)
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

Mat4 openvr::Eye::GetEyeViewMatrix(pragma::CCameraComponent &cam) const
{
	auto *vrInterface = m_instance->GetSystemInterface();
	auto &matView = cam.GetViewMatrix();

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

	// Note: The documentation recommends using GetProjectionMatrix instead of
	// GetProjectionRaw, but I have no idea what format it's in and couldn't get it
	// to work with glm.
	// auto eyeProj = vrInterface->GetProjectionMatrix(eye,nearZ,farZ);
	// return Mat4{
	//	eyeProj.m[0][0],eyeProj.m[1][0],eyeProj.m[2][0],eyeProj.m[3][0],
	//	eyeProj.m[0][1],eyeProj.m[1][1],eyeProj.m[2][1],eyeProj.m[3][1],
	//	eyeProj.m[0][2],eyeProj.m[1][2],eyeProj.m[2][2],eyeProj.m[3][2],
	//	eyeProj.m[0][3],eyeProj.m[1][3],eyeProj.m[2][3],eyeProj.m[3][3]
	// };

	float left,right,top,bottom;
	vrInterface->GetProjectionRaw(eye,&left,&right,&top,&bottom);

	return glm::frustumRH(left,right,bottom,top,nearZ,farZ);
}
