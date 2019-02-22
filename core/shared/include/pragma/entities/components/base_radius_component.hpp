#ifndef __BASE_RADIUS_COMPONENT_HPP__
#define __BASE_RADIUS_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>

namespace pragma
{
	class DLLNETWORK BaseRadiusComponent
		: public BaseEntityComponent
	{
	public:
		static ComponentEventId EVENT_ON_RADIUS_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);
		virtual void Initialize() override;
		float GetRadius() const;
		const util::PFloatProperty &GetRadiusProperty() const;
		void SetRadius(float radius);

		virtual void Save(DataStream &ds) override;
		virtual void Load(DataStream &ds,uint32_t version) override;
	protected:
		BaseRadiusComponent(BaseEntity &ent);
		pragma::NetEventId m_netEvSetRadius = pragma::INVALID_NET_EVENT;
		util::PFloatProperty m_radius;
	};
	struct DLLNETWORK CEOnRadiusChanged
		: public ComponentEvent
	{
		CEOnRadiusChanged(float oldRadius,float newRadius);
		virtual void PushArguments(lua_State *l) override;
		float oldRadius;
		float newRadius;
	};
};

#endif
