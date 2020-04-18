#ifndef __C_FLEX_CONTROLLER_HPP__
#define __C_FLEX_CONTROLLER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_flex_component.hpp>

namespace pragma
{
	class DLLCLIENT CFlexComponent final
		: public BaseFlexComponent
	{
	public:
		CFlexComponent(BaseEntity &ent) : BaseFlexComponent(ent) {}
		virtual void Initialize() override;
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
		void UpdateFlexWeights();
		virtual luabind::object InitializeLuaObject(lua_State *l) override;

		void SetFlexWeightOverride(uint32_t flexId,float weight);
		void ClearFlexWeightOverride(uint32_t flexId);
		bool HasFlexWeightOverride(uint32_t flexId) const;
	protected:
		bool UpdateFlexWeight(uint32_t flexId,float &val,bool storeInCache=true);
		void UpdateEyeFlexes();
		void UpdateEyeFlexes(Eyeball &eyeball,uint32_t eyeballIdx);
		void UpdateFlexControllers();
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

		std::vector<std::optional<float>> m_flexOverrides {};
	};
};

#endif
