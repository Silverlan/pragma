#ifndef __BASE_FLAMMABLE_COMPONENT_HPP__
#define __BASE_FLAMMABLE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>

namespace pragma
{
	class DLLNETWORK BaseFlammableComponent
		: public BaseEntityComponent
	{
	public:
		virtual ~BaseFlammableComponent() override;
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;

		const util::PBoolProperty &GetOnFireProperty() const;
		const util::PBoolProperty &GetIgnitableProperty() const;
		bool IsOnFire() const;
		bool IsIgnitable() const;
		virtual void Ignite(float duration,BaseEntity *attacker=nullptr,BaseEntity *inflictor=nullptr);
		virtual void Extinguish();
		virtual void SetIgnitable(bool b);

		virtual void Save(DataStream &ds) override;
		virtual void Load(DataStream &ds,uint32_t version) override;
	protected:
		BaseFlammableComponent(BaseEntity &ent);
		virtual void OnThink(double dt);
		pragma::NetEventId m_netEvIgnite = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvExtinguish = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSetIgnitable = pragma::INVALID_NET_EVENT;
		util::PBoolProperty m_bIsOnFire;
		util::PBoolProperty m_bIgnitable;
		float m_tExtinguishTime = 0.f;
	};
};

#endif
