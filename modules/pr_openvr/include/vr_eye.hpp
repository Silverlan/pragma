#ifndef __VR_EYE_HPP__
#define __VR_EYE_HPP__

#include <openvr.h>
#include <mathutil/umat.h>
#include <pragma/iscene.h>

namespace prosper {class RenderTarget; class Image; class PrimaryCommandBuffer; class Fence;};
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
		IScene scene;
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
		void UpdateImage(const prosper::Image &src);
		Mat4 GetEyeViewMatrix() const;
		Mat4 GetEyeProjectionMatrix(float nearZ,float farZ) const;
	};
};

#endif
