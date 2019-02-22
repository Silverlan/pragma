#ifndef __C_DEBUG_VEHICLE_HPP__
#define __C_DEBUG_VEHICLE_HPP__

namespace debug
{
	class Vehicle
	{
	public:
		Vehicle();
	protected:
	private:

	};
};

class Vehicle_Car  {
public:
	const float
		veh__defaultBreakingForce = 10.f,
		veh__gBreakingForce = 100.f,
		veh__maxEngineForce = 1000.f,//this should be engine/velocity dependent
		veh__maxBreakingForce = 100.f,
		veh__gVehicleSteering = 0.f,
		veh__steeringIncrement = 0.04f,
		veh__steeringClamp = 0.3f,
		veh__wheelRadius = 3.3f,
		veh__wheelWidth = 1.3f,
		veh__wheelFriction = 4000, //BT_LARGE_float;
		veh__suspensionStiffness = 100.f,
		veh__suspensionDamping = 0.19f,
		veh__suspensionCompression = 0.4f,
		veh__rollInfluence = 8.1f;

public:
	void InitVehicle(PhysEnv* phys, Vector3 pos);
	void AccelerationForce(uint8_t dirForce, uint32_t maxForce);
	void DeaccelerationForce(uint8_t dirForce, uint32_t maxForce);
	void SteerTurn(float directAngle);
	void ControlInput(int input);
	void RunWheelRayCasting(PhysEnv* phys);
	void Mount_Object(int indexVehicle, btRigidBody* rbB, btVector3 frameInA, btVector3 frameInB );

	std::array<btHinge2Constraint*, 4> WheelConstraints;
	std::array<btRigidBody*, 4> WheelRigidBody;

	btRigidBody* Chassis;

	btRaycastVehicle* m_vehicle;
	btDefaultVehicleRaycaster* m_vehicleRayCaster;
private:
	btVector3 TractiveForce(btScalar force, btVector3 direction);
	btVector3 ResistanceForce(const btScalar constDrag, btVector3 velocity);
	btVector3 RollResistance(const btScalar constRoadResist, btVector3 velocity);
	btVector3 LongitudinalForce(btVector3 p1, btVector3 p2, btVector3 p3);
	btVector3 NewtonSecondLaw(btVector3 netForce, btScalar mass);
	btVector3 ForwardApproach(btVector3 dir, btVector3 velo);
	
	void StallEngine(); bool _IsVehicleStalled = false;

	PhysEnv* GLOBAL_Physics_Class;
};

#endif
