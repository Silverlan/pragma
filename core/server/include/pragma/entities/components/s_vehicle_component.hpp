#ifndef __S_VEHICLE_COMPONENT_HPP__
#define __S_VEHICLE_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_vehicle_component.hpp>

namespace pragma
{
	class DLLSERVER SVehicleComponent final
		: public BaseVehicleComponent,
		public SBaseNetComponent
	{
	public:
		static unsigned int GetVehicleCount();
		static const std::vector<SVehicleComponent*> &GetAll();
		SVehicleComponent(BaseEntity &ent);
		virtual ~SVehicleComponent() override;
		virtual void ClearDriver() override;
		virtual void SetDriver(BaseEntity *ent) override;
		BaseEntity *AddWheel(const std::string &mdl,const Vector3 &connectionPoint,const Vector3 &wheelAxle,Bool bIsFrontWheel,const Vector3 &mdlOffset={},const Quat &mdlRotOffset={});
		BaseEntity *AddWheel(const Vector3 &connectionPoint,const Vector3 &wheelAxle,Bool bIsFrontWheel,const Vector3 &mdlOffset={},const Quat &mdlRotOffset={});
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual void Initialize() override;

		virtual void SetMaxEngineForce(Float force) override;
		virtual void SetMaxReverseEngineForce(Float force) override;
		virtual void SetMaxBrakeForce(Float force) override;
		virtual void SetAcceleration(Float acc) override;
		virtual void SetTurnSpeed(Float speed) override;
		virtual void SetMaxTurnAngle(Float ang) override;
		virtual void SetSteeringWheelModel(const std::string &mdl) override;
		virtual void SetFirstPersonCameraEnabled(bool b) override;
		virtual void SetThirdPersonCameraEnabled(bool b) override;

		virtual bool ShouldTransmitNetData() const override {return true;};
	protected:
		void WriteWheelInfo(NetPacket &p,WheelData &data,btWheelInfo *info);
		CallbackHandle m_playerAction;
		std::string m_steeringWheelMdl;
		void OnActionInput(Action action, bool b);
		virtual void OnRemove() override;
		void OnPostSpawn();
		void OnUse(BaseEntity *pl);
		virtual Bool AddWheel(const Vector3 &connectionPoint,const Vector3 &wheelAxle,Bool bIsFrontWheel,UChar *wheelId,const Vector3 &mdlOffset={},const Quat &mdlRotOffset={}) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	private:
		static std::vector<SVehicleComponent*> s_vehicles;
	};
};

#endif
