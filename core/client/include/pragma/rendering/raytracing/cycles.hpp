#ifndef __RAYTRACING_CYCLES_HPP__
#define __RAYTRACING_CYCLES_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/environment/env_camera.h>
#include <sharedutils/util_weak_handle.hpp>
#include <sharedutils/functioncallback.h>
#include <memory>
#include <functional>

class ClientState;
class BaseEntity;
namespace pragma::rendering::cycles
{
	class DLLCLIENT Scene
	{
	public:
		enum class Type : uint8_t
		{
			RenderImage = 0u,
			BakeAmbientOcclusion,
			BakeNormals,
			BakeDiffuseLighting
		};
		struct DLLCLIENT CreateInfo
		{
			uint32_t width = 1'024;
			uint32_t height = 768;
			uint32_t samples = 1'024;
			Type type = Type::RenderImage;
			bool denoise = false;
			bool hdrOutput = false;

			Vector3 cameraPosition = {};
			Quat cameraRotation = {};
			float nearZ = pragma::BaseEnvCameraComponent::DEFAULT_NEAR_Z;
			float farZ = pragma::BaseEnvCameraComponent::DEFAULT_FAR_Z;
			umath::Degree fov = BaseEnvCameraComponent::DEFAULT_FOV;

			std::function<bool(BaseEntity&)> entityFilter = nullptr;
			std::function<void(const uint8_t*,int,int)> outputHandler = nullptr;
		};
		static util::WeakHandle<Scene> Create(ClientState &client,const CreateInfo &createInfo);
		~Scene();
		float GetProgress() const;
		bool IsComplete() const;
		bool IsCancelled() const;
		void Cancel();
		bool Update();
		void SetProgressCallback(const std::function<void(float)> &progressCallback);
	private:
		Scene()=default;
		void OnProgressChanged(float progress);
		std::shared_ptr<void> m_moduleScene = nullptr;
		CallbackHandle m_cbThink = {};
		CallbackHandle m_cbOnClose = {};
		bool m_bComplete = false;
		bool m_bCancelled = false;
		std::shared_ptr<Scene> m_scene = nullptr;
		std::function<void(float)> m_progressCallback = nullptr;

		void(*f_calc_raytraced_scene)(
			uint32_t,uint32_t,uint32_t,bool,bool,const Vector3&,const Quat&,float,float,umath::Degree,
			const std::function<bool(BaseEntity&)>&,const std::function<void(const uint8_t*,int,int,int)>&,std::shared_ptr<void>&
		) = nullptr;

		void(*f_cycles_bake_lighting)(
			uint32_t,uint32_t,uint32_t,bool,
			float,float,umath::Degree,
			const std::function<void(const uint8_t*,int,int,int)>&,std::shared_ptr<void>&
		) = nullptr;

		float(*f_cycles_get_scene_process)(const std::shared_ptr<void>&) = nullptr;
		bool(*f_cycles_is_scene_complete)(const std::shared_ptr<void>&) = nullptr;
		void(*f_cycles_cancel_scene)(const std::shared_ptr<void>&) = nullptr;
		void(*f_cycles_set_scene_progress_callback)(const std::shared_ptr<void>&,const std::function<void(float)>&) = nullptr;
		bool(*f_cycles_is_scene_cancelled)(const std::shared_ptr<void>&) = nullptr;
		void(*f_cycles_wait_for_scene_completion)(const std::shared_ptr<void>&) = nullptr;
	};
};

#endif
