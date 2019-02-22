#ifndef __C_OBSERVABLE_COMPONENT_HPP__
#define __C_OBSERVABLE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_observable_component.hpp>

namespace pragma
{
	class DLLCLIENT CObservableComponent final
		: public BaseObservableComponent,
		public CBaseNetComponent
	{
	public:
		CObservableComponent(BaseEntity &ent) : BaseObservableComponent(ent) {}
		using BaseObservableComponent::SetFirstPersonObserverOffset;
		using BaseObservableComponent::SetThirdPersonObserverOffset;
		virtual void SetFirstPersonObserverOffset(const Vector3 &offset) override;
		virtual void SetThirdPersonObserverOffset(const Vector3 &offset) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
};

#endif
