// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.flex;

import :audio;

export namespace pragma {
	namespace cFlexComponent {
		using namespace baseFlexComponent;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_FLEX_CONTROLLERS_UPDATED;
	}
	class DLLCLIENT CFlexComponent final : public BaseFlexComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		struct FlexAnimationData {
			uint32_t flexAnimationId = std::numeric_limits<uint32_t>::max();
			float t = 0.f;
			bool loop = false;
			float playbackRate = 1.f;
		};
		CFlexComponent(ecs::BaseEntity &ent) : BaseFlexComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnTick(double dt) override;
		// Vertex animations
		virtual void SetFlexController(uint32_t flexId, float val, float duration = 0.f, bool clampToLimits = true) override;
		using BaseFlexComponent::SetFlexController;
		virtual bool GetFlexController(uint32_t flexId, float &val) const override;
		bool CalcFlexValue(uint32_t flexId, float &val) const;
		const std::vector<float> &GetFlexWeights() const;
		float GetFlexWeight(uint32_t flexId) const;
		bool GetFlexWeight(uint32_t flexId, float &outWeight) const;
		void SetFlexWeight(uint32_t flexId, float weight);
		void UpdateSoundPhonemes(audio::CALSound &snd);
		void UpdateFlexWeightsMT();
		virtual void InitializeLuaObject(lua::State *l) override;

		void SetFlexWeightOverride(uint32_t flexId, float weight);
		void ClearFlexWeightOverride(uint32_t flexId);
		bool HasFlexWeightOverride(uint32_t flexId) const;

		void SetFlexAnimationCycle(const LookupIdentifier &id, float cycle);
		float GetFlexAnimationCycle(const LookupIdentifier &id) const;
		void PlayFlexAnimation(const LookupIdentifier &id, bool loop = true, bool reset = false);
		void StopFlexAnimation(const LookupIdentifier &id);
		void SetFlexAnimationPlaybackRate(const LookupIdentifier &id, float playbackRate);
		const std::vector<FlexAnimationData> &GetFlexAnimations() const;
	  protected:
		FlexAnimationData *FindFlexAnimationData(uint32_t flexAnimId);
		const FlexAnimationData *FindFlexAnimationData(uint32_t flexAnimId) const { return const_cast<CFlexComponent *>(this)->FindFlexAnimationData(flexAnimId); }
		void ResolveFlexAnimation(const LookupIdentifier &lookupId) const;
		void MaintainFlexAnimations(float dt);
		bool UpdateFlexWeight(uint32_t flexId, float &val, bool storeInCache = true);
		void UpdateEyeFlexes();
		void UpdateEyeFlexes(asset::Eyeball &eyeball, uint32_t eyeballIdx);
		void UpdateFlexControllers(float dt);
		void OnModelChanged(const std::shared_ptr<asset::Model> &mdl);

		// Flex controllers
		struct FlexControllerInfo {
			float value = 0.f;
			float targetValue = 0.f;
			float endTime = 0.f;
		};
		std::unordered_map<uint32_t, FlexControllerInfo> m_flexControllers = {};
		std::vector<float> m_flexWeights = {};
		std::vector<bool> m_updatedFlexWeights = {};
		bool m_flexDataUpdateRequired = false;
		std::vector<std::optional<float>> m_flexOverrides {};

		std::vector<FlexAnimationData> m_flexAnimations;
	};
};
