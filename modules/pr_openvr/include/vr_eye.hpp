#ifndef __VR_EYE_HPP__
#define __VR_EYE_HPP__

#include <openvr.h>
#include <mathutil/umat.h>
#include <pragma/iscene.h>
#include <sharedutils/util_weak_handle.hpp>

namespace prosper {class RenderTarget; class IImage; class IPrimaryCommandBuffer; class IFence;};
namespace pragma {class CCameraComponent;};
namespace openvr
{
	class Instance;
	struct Eye
	{
	private:
		Instance *m_instance;
	public:
		Eye(Instance *instance,vr::EVREye eye);
		~Eye();
		bool Initialize(uint32_t w,uint32_t h);
		vr::EVREye eye;
		std::shared_ptr<pragma::rendering::BaseRenderer> renderer = nullptr;
		util::WeakHandle<pragma::CCameraComponent> camera;
#ifdef USE_VULKAN
		std::shared_ptr<prosper::RenderTarget> vkRenderTarget = nullptr;
		vr::VRVulkanTextureData_t vrTextureData {};
#elif USE_OPENGL_OFFSCREEN_CONTEXT
		GLuint fbo;
		GLuint texture;
#endif
		uint32_t width;
		uint32_t height;
		vr::Texture_t vrTexture;
		void UpdateImage(const prosper::IImage &src);
		Mat4 GetEyeViewMatrix(pragma::CCameraComponent &cam) const;
		Mat4 GetEyeProjectionMatrix(float nearZ,float farZ) const;
	};
};

#endif
