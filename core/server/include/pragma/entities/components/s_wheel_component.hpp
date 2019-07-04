#ifndef __S_WHEEL_COMPONENT_HPP__
#define __S_WHEEL_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_wheel_component.hpp>

namespace pragma
{
	class DLLSERVER SWheelComponent final
		: public BaseWheelComponent,
		public SBaseSnapshotComponent
	{
	public:
		SWheelComponent(BaseEntity &ent) : BaseWheelComponent(ent) {}
		virtual ~SWheelComponent() override;
		virtual void Initialize() override;
		virtual void Attach(BaseEntity *ent,UChar wheelId) override;
		virtual void Detach() override;
		virtual void SetFrontWheel(Bool b) override;
		virtual void SetChassisConnectionPoint(const Vector3 &p) override;
		virtual void SetWheelAxle(const Vector3 &axis) override;
		virtual void SetWheelDirection(const Vector3 &dir) override;
		virtual void SetMaxSuspensionLength(Float len) override;
		virtual void SetMaxSuspensionCompression(Float cmp) override;
		virtual void SetMaxDampingRelaxation(Float damping) override;
		virtual void SetWheelRadius(Float radius) override;
		virtual void SetSuspensionStiffness(Float stiffness) override;
		virtual void SetWheelDampingCompression(Float cmp) override;
		virtual void SetFrictionSlip(Float slip) override;
		virtual void SetSteeringAngle(Float ang) override;
		virtual void SetWheelRotation(Float rot) override;
		virtual void SetRollInfluence(Float influence) override;
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual void SendSnapshotData(NetPacket &packet,pragma::BasePlayerComponent &pl) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual bool ShouldTransmitSnapshotData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};
class DLLSERVER SWheel
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif