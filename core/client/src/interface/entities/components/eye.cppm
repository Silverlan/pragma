// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:entities.components.eye;
import :entities.components.animated;
import :entities.components.flex;
export import :model.mesh;

export {
	namespace pragma {
		class DLLCLIENT CEyeComponent final : public BaseEntityComponent {
		  public:
			enum class StateFlags : uint8_t { None = 0u, BlinkingEnabled = 1u, PrevBlinkToggle = BlinkingEnabled << 1u, BlinkToggle = PrevBlinkToggle << 1u };

			struct EyeballState {
				Vector3 origin = {};
				Vector3 forward = {};
				Vector3 right = {};
				Vector3 up = {};
				Vector4 irisProjectionU = {};
				Vector4 irisProjectionV = {};
			};

			struct EyeballConfig {
				Vector3 eyeShift = {};
				bool eyeMove = false;
				Vector2 jitter = {};
				float eyeSize = 0.f;
				float dilation = 0.5f;
				float irisScale = 1.f;

				// This is a workaround for the legacy eye shader.
				// Usually the iris uv coordinates are clamped to the range [0,1] to prevent it
				// from repeating over the eyeball.
				// However, some models have improperly defined eyeballs, in which case the
				// uv coordinates exceed the normal range [0,1]. In these cases the
				// uv coordinates must *not* be clamped, or the iris will not be rendered properly.
				// To accomplish that, this value can be set to a really high range to effectively disable the clamping.
				// The material parameter 'iris_uv_clamp_range' can be used to change this value.
				// If this problem is ever fixed properly, this workaround can be removed.
				Vector2 irisUvClampRange = {0.f, 1.f};
			};

			struct EyeballData {
				EyeballState state = {};
				EyeballConfig config = {};
			};

			// static ComponentEventId EVENT_ON_EYEBALLS_UPDATED;
			// static ComponentEventId EVENT_ON_BLINK;
			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

			CEyeComponent(ecs::BaseEntity &ent);

			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua::State *l) override;

			void Blink();

			const EyeballConfig *GetEyeballConfig(uint32_t eyeballIndex) const;
			EyeballConfig *GetEyeballConfig(uint32_t eyeballIndex);
			void SetEyeballConfig(const EyeballConfig &eyeballConfig);
			EyeballData *GetEyeballData(uint32_t eyeballIndex);
			const EyeballData *GetEyeballData(uint32_t eyeballIndex) const;
			bool GetEyeballProjectionVectors(uint32_t eyeballIndex, Vector4 &outProjU, Vector4 &outProjV) const;

			void ClearViewTarget();
			Vector3 GetViewTarget() const;
			void SetViewTarget(const Vector3 &viewTarget);
			std::optional<math::Transform> GetEyePose() const;

			void SetBlinkDuration(float dur);
			float GetBlinkDuration() const;

			void SetBlinkingEnabled(bool enabled);
			bool IsBlinkingEnabled() const;
			bool FindEyeballIndex(geometry::CModelSubMesh &subMesh, uint32_t &outEyeballIndex) const;
			bool FindEyeballIndex(uint32_t skinMatIdx, uint32_t &outEyeballIndex) const;

			void SetLocalViewTargetFactor(float f);
			float GetLocalViewTargetFactor() const;

			math::Transform CalcEyeballPose(uint32_t eyeballIndex, math::Transform *optOutBonePose = nullptr) const;

			void UpdateEyeballsMT();
		  protected:
			void UpdateBlinkMT();
			void OnModelChanged(const std::shared_ptr<asset::Model> &mdl);
			Vector3 ClampViewTarget(const Vector3 &viewTarget) const;
			void UpdateEyeballMT(const asset::Eyeball &eyeball, uint32_t eyeballIndex);
			void UpdateEyeMaterialData();
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		  private:
			EyeballConfig m_eyeballConfig = {};
			std::vector<EyeballData> m_eyeballData = {};
			Vector3 m_viewTarget {};
			float m_localViewTargetFactor = 1.f;
			StateFlags m_stateFlags;
			std::unordered_map<uint32_t, uint32_t> m_skinMaterialIndexToEyeballIndex = {};
			ComponentHandle<CFlexComponent> m_flexC = {};
			ComponentHandle<CAnimatedComponent> m_animC = {};
			uint32_t m_eyeUpDownFlexController = std::numeric_limits<uint32_t>::max();
			uint32_t m_eyeLeftRightFlexController = std::numeric_limits<uint32_t>::max();
			uint32_t m_eyeAttachmentIndex = std::numeric_limits<uint32_t>::max();

			// Blinking
			float m_curBlinkTime = 0.f;
			float m_blinkDuration = 0.2f;
			float m_tNextBlink = 0.f;
			uint32_t m_blinkFlexController = std::numeric_limits<uint32_t>::max();
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::CEyeComponent::StateFlags)
};
