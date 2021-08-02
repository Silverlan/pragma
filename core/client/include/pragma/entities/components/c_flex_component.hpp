/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_FLEX_CONTROLLER_HPP__
#define __C_FLEX_CONTROLLER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/util/lookup_identifier.hpp>
#include <pragma/entities/components/base_flex_component.hpp>

struct Eyeball;
namespace pragma
{
	class DLLCLIENT CFlexComponent final
		: public BaseFlexComponent
	{
	public:
		static ComponentEventId EVENT_ON_FLEX_CONTROLLERS_UPDATED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		struct FlexAnimationData
		{
			uint32_t flexAnimationId = std::numeric_limits<uint32_t>::max();
			float t = 0.f;
			bool loop = false;
			float playbackRate = 1.f;
		};
		CFlexComponent(BaseEntity &ent) : BaseFlexComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnTick(double dt) override;
		// Vertex animations
		virtual void SetFlexController(uint32_t flexId,float val,float duration=0.f,bool clampToLimits=true) override;
		using BaseFlexComponent::SetFlexController;
		virtual bool GetFlexController(uint32_t flexId,float &val) const override;
		bool CalcFlexValue(uint32_t flexId,float &val) const;
		const std::vector<float> &GetFlexWeights() const;
		float GetFlexWeight(uint32_t flexId) const;
		bool GetFlexWeight(uint32_t flexId,float &outWeight) const;
		void SetFlexWeight(uint32_t flexId,float weight);
		void UpdateSoundPhonemes(CALSound &snd);
		void UpdateFlexWeightsMT();
		virtual void InitializeLuaObject(lua_State *l) override;

		void SetFlexWeightOverride(uint32_t flexId,float weight);
		void ClearFlexWeightOverride(uint32_t flexId);
		bool HasFlexWeightOverride(uint32_t flexId) const;

		void SetFlexAnimationCycle(const LookupIdentifier &id,float cycle);
		float GetFlexAnimationCycle(const LookupIdentifier &id) const;
		void PlayFlexAnimation(const LookupIdentifier &id,bool loop=true,bool reset=false);
		void StopFlexAnimation(const LookupIdentifier &id);
		void SetFlexAnimationPlaybackRate(const LookupIdentifier &id,float playbackRate);
		const std::vector<FlexAnimationData> &GetFlexAnimations() const;
	protected:
		FlexAnimationData *FindFlexAnimationData(uint32_t flexAnimId);
		const FlexAnimationData *FindFlexAnimationData(uint32_t flexAnimId) const {return const_cast<CFlexComponent*>(this)->FindFlexAnimationData(flexAnimId);}
		void ResolveFlexAnimation(const LookupIdentifier &lookupId) const;
		void MaintainFlexAnimations(float dt);
		bool UpdateFlexWeight(uint32_t flexId,float &val,bool storeInCache=true);
		void UpdateEyeFlexes();
		void UpdateEyeFlexes(Eyeball &eyeball,uint32_t eyeballIdx);
		void UpdateFlexControllers(float dt);
		void OnModelChanged(const std::shared_ptr<Model> &mdl);

		// Flex controllers
		struct FlexControllerInfo
		{
			float value = 0.f;
			float targetValue = 0.f;
			float endTime = 0.f;
		};
		std::unordered_map<uint32_t,FlexControllerInfo> m_flexControllers = {};
		std::vector<float> m_flexWeights = {};
		std::vector<bool> m_updatedFlexWeights = {};
		bool m_flexDataUpdateRequired = false;
		std::vector<std::optional<float>> m_flexOverrides {};

		std::vector<FlexAnimationData> m_flexAnimations;
	};
};

#endif
