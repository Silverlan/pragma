#ifndef __VR_CONTROLLER_STATE_HPP__
#define __VR_CONTROLLER_STATE_HPP__

#include <openvr.h>
#include <iglfw/glfw_keys.h>

namespace openvr
{
	struct ControllerState
	{
		ControllerState()=default;
		void UpdateState(const vr::VRControllerState_t &state);
		void OnStateChanged(uint32_t key,GLFW::KeyState state);
		void SetStateChangeCallback(const std::function<void(uint32_t,GLFW::KeyState)> &f);
	private:
		vr::VRControllerState_t m_vrState = {};
		std::function<void(uint32_t,GLFW::KeyState)> m_stateChangeCallback = nullptr;
	};
};

#endif
