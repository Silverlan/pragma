#ifndef __BASE_POINT_AT_TARGET_COMPONENT_HPP__
#define __BASE_POINT_AT_TARGET_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/baseentity_net_event_manager.hpp"
#include "pragma/entities/entity_property.hpp"

namespace pragma
{
	class DLLNETWORK BasePointAtTargetComponent
		: public BaseEntityComponent
	{
	public:
		virtual void Initialize() override;
		
		const pragma::PEntityProperty &GetPointAtTargetProperty() const;
		void SetPointAtTarget(BaseEntity &ent);
		void ClearPointAtTarget();
		BaseEntity *GetPointAtTarget() const;
	protected:
		BasePointAtTargetComponent(BaseEntity &ent);
		virtual void OnEntitySpawn() override;
		virtual void SetPointAtTarget(BaseEntity *ent);

		std::string m_kvPointAtTargetName = "";
		pragma::PEntityProperty m_pointAtTarget;
		pragma::NetEventId m_netEvSetPointAtTarget = pragma::INVALID_NET_EVENT;
		CallbackHandle m_cbTick = {};
	};
};

#endif
