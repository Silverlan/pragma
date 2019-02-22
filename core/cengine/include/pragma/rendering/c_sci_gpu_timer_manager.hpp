#ifndef __C_SCI_GPU_TIMER_MANAGER_HPP__
#define __C_SCI_GPU_TIMER_MANAGER_HPP__

#include "pragma/c_enginedefinitions.h"
#include <array>

enum class DLLCENGINE GPUTimerEvent : uint32_t
{
	// Draw
	GUI = 0,
	WorldScene,
	Particles,
	UpdateExposure,
	Shadow,
	Skybox,
	World,
	WorldTranslucent,
	Water,
	Debug,
	View,
	ViewParticles,
	Glow,
	BlurHDRBloom,
	GlowAdditive,
	FinalAdditive,
	HDR,
	SceneToScreen,
	GameRender,
	Mesh0,
	Mesh1,
	Mesh2,
	Mesh3,
	Mesh4,
	Mesh5,
	Mesh6,
	Mesh7,
	Mesh8,
	DebugMesh,
	ShadowLayerBlit,

	// Compute
	ComputeStart,
	WaterSurface = ComputeStart,

	Count
};

namespace prosper
{
	class QueryPool;
	class TimerQuery;
};
class DLLCENGINE CSciGPUTimerManager
{
public:
	CSciGPUTimerManager();
	void StartTimer(GPUTimerEvent e);
	void StopTimer(GPUTimerEvent e);
	bool IsResultAvailable(GPUTimerEvent e) const;
	bool GetResult(GPUTimerEvent e,float &v) const;
	float GetResult(GPUTimerEvent e) const;
	bool IsTimerActive(GPUTimerEvent e) const;
	void GetResults(std::vector<float> &results) const;
	static std::string EventToString(GPUTimerEvent e);
private:
	enum class DLLCENGINE TimerState : uint32_t
	{
		Inactive = 0,
		Started,
		Stopped
	};
	struct DLLCENGINE Query
	{
		struct DLLCENGINE CmdInfo
		{
			std::shared_ptr<prosper::TimerQuery> query = nullptr;
			TimerState state = TimerState::Inactive;
			uint64_t resultFrame = std::numeric_limits<uint64_t>::max();
		};
		std::vector<std::shared_ptr<CmdInfo>> cmdQueries; // Data per swapchain command buffer
		long double result = 0.L;
	};
	uint32_t m_cmdCount = 0;
	std::shared_ptr<prosper::QueryPool> m_pool = nullptr;
	std::unordered_map<std::shared_ptr<prosper::CommandBuffer>,std::size_t> m_cmdBufferIndices;
	std::array<std::shared_ptr<Query>,umath::to_integral(GPUTimerEvent::Count)> m_timerQueries;
	TimerState GetTimerState(GPUTimerEvent e) const;
	void InitializeQueries();
	const prosper::CommandBuffer &GetCommandBuffer(GPUTimerEvent e) const;
	std::size_t GetCommandBufferIndex(GPUTimerEvent e) const;
};

#endif
