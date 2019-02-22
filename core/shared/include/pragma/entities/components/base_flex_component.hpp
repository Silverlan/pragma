#ifndef __BASE_FLEX_COMPONENT_HPP__
#define __BASE_FLEX_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma
{
	class DLLNETWORK BaseFlexComponent
		: public BaseEntityComponent
	{
	public:
		virtual void Initialize() override;

		// Flex Controllers
		void SetFlexController(const std::string &name,float val,float duration=0.f);
		virtual void SetFlexController(uint32_t flexId,float val,float duration=0.f) {}; // TODO /* =0 */
		float GetFlexController(uint32_t flexId) const;
		float GetFlexController(const std::string &flexController) const;
		virtual bool GetFlexController(uint32_t flexId,float &val) const {val = 0.f; return true;}; // TODO /* =0 */
	protected:
		BaseFlexComponent(BaseEntity &ent);
	};
};

#endif
