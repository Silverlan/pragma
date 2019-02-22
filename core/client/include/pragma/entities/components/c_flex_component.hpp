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
		virtual void SetFlexController(uint32_t flexId,float val,float duration=0.f) override;
		using BaseFlexComponent::SetFlexController;
		virtual bool GetFlexController(uint32_t flexId,float &val) const override;
		bool CalcFlexValue(uint32_t flexId,float &val) const;
		void UpdateSoundPhonemes(CALSound &snd);
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		void UpdateFlexControllers();

		// Flex controllers
		struct FlexControllerInfo
		{
			float value = 0.f;
			float targetValue = 0.f;
			float endTime = 0.f;
		};
		std::unordered_map<uint32_t,FlexControllerInfo> m_flexControllers = {};
	};
};

#endif
