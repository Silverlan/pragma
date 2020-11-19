/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_EYE_COMPONENT_HPP__
#define __C_EYE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>
#include <mathutil/transform.hpp>
#include <mathutil/uvec.h>

struct Eyeball;
namespace pragma
{
	class CFlexComponent;
	class DLLCLIENT CEyeComponent final
		: public BaseEntityComponent
	{
	public:
		enum class StateFlags : uint8_t
		{
			None = 0u,
			BlinkingEnabled = 1u,
			PrevBlinkToggle = BlinkingEnabled<<1u,
			BlinkToggle = PrevBlinkToggle<<1u
		};

		struct EyeballState
		{
			Vector3 origin = {};
			Vector3 forward = {};
			Vector3 right = {};
			Vector3 up = {};
			Vector4 irisProjectionU = {};
			Vector4 irisProjectionV = {};
		};

		struct EyeballConfig
		{
			Vector3 eyeShift = {};
			bool eyeMove = false;
			Vector2 jitter = {};
			float eyeSize = 0.f;
			float dilation = 0.5f;
		};

		struct EyeballData
		{
			EyeballState state = {};
			EyeballConfig config = {};
		};

		// static ComponentEventId EVENT_ON_EYEBALLS_UPDATED;
		// static ComponentEventId EVENT_ON_BLINK;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		CEyeComponent(BaseEntity &ent);

		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;

		void Blink();

		const EyeballConfig *GetEyeballConfig(uint32_t eyeballIndex) const;
		EyeballConfig *GetEyeballConfig(uint32_t eyeballIndex);
		void SetEyeballConfig(const EyeballConfig &eyeballConfig);
		EyeballData *GetEyeballData(uint32_t eyeballIndex);
		const EyeballData *GetEyeballData(uint32_t eyeballIndex) const;
		bool GetEyeballProjectionVectors(uint32_t eyeballIndex,Vector4 &outProjU,Vector4 &outProjV) const;

		void ClearViewTarget();
		Vector3 GetViewTarget() const;
		void SetViewTarget(const Vector3 &viewTarget);

		void SetBlinkDuration(float dur);
		float GetBlinkDuration() const;

		void SetBlinkingEnabled(bool enabled);
		bool IsBlinkingEnabled() const;
		bool FindEyeballIndex(CModelSubMesh &subMesh,uint32_t &outEyeballIndex) const;
		bool FindEyeballIndex(uint32_t skinMatIdx,uint32_t &outEyeballIndex) const;

		umath::Transform CalcEyeballPose(uint32_t eyeballIndex,umath::Transform *optOutBonePose=nullptr) const;
		
		void UpdateEyeballsMT();
	protected:
		void UpdateBlinkMT();
		void OnModelChanged(const std::shared_ptr<Model> &mdl);
		Vector3 ClampViewTarget(const Vector3 &viewTarget) const;
		void UpdateEyeballMT(const Eyeball &eyeball,uint32_t eyeballIndex);
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
	private:
		EyeballConfig m_eyeballConfig = {};
		std::vector<EyeballData> m_eyeballData = {};
		std::optional<Vector3> m_viewTarget = {};
		StateFlags m_stateFlags;
		std::unordered_map<uint32_t,uint32_t> m_skinMaterialIndexToEyeballIndex = {};
		util::WeakHandle<CFlexComponent> m_flexC = {};
		util::WeakHandle<CAnimatedComponent> m_animC = {};
		uint32_t m_eyeUpDownFlexController = std::numeric_limits<uint32_t>::max();
		uint32_t m_eyeLeftRightFlexController = std::numeric_limits<uint32_t>::max();
		uint32_t m_eyeAttachmentIndex = std::numeric_limits<uint32_t>::max();

		// Blinking
		float m_curBlinkTime = 0.f;
		float m_blinkDuration = 0.2f;
		float m_tNextBlink = 0.f;
		uint32_t m_blinkFlexController = std::numeric_limits<uint32_t>::max();
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CEyeComponent::StateFlags)

#endif
