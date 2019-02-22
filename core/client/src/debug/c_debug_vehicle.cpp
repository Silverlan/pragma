#include "stdafx_client.h"
#include "pragma/debug/c_debug_vehicle.hpp"
#include <pragma/physics/physenvironment.h>
#include <pragma/entities/components/base_player_component.hpp>

extern DLLCLIENT PhysEnv *c_physEnv;

#define CUBE_HALF_EXTENTS 1

btVector3 Vehicle_Car::TractiveForce(btScalar force, btVector3 direction) {
	return direction * force;
}
btVector3 Vehicle_Car::ResistanceForce(const btScalar constDrag, btVector3 velocity) {
	//printf("\t%f - %f - %f\n", velocity.x(), velocity.y(), velocity.z());
	return -constDrag * velocity * btScalar(velocity.length2());
}
btVector3 Vehicle_Car::RollResistance(const btScalar constRoadResist, btVector3 velocity) {
	return -constRoadResist * velocity;
}
btVector3 Vehicle_Car::LongitudinalForce(btVector3 p1, btVector3 p2, btVector3 p3) {
	return p1 + p2 + p3;
}
btVector3 Vehicle_Car::NewtonSecondLaw(btVector3 netForce, btScalar mass) {
	return netForce / mass;
}

btVector3 Vehicle_Car::ForwardApproach(btVector3 dir, btVector3 velo) {
	btVector3 tractiveForce = TractiveForce(1000.0, dir);
	btVector3 resistanceForce = ResistanceForce(0.4257f, velo);
	btVector3 rollResistance = RollResistance(12.8f, velo);

	return NewtonSecondLaw(
		LongitudinalForce(
			tractiveForce,
			resistanceForce,
			rollResistance
		),
		10
	);
}

void Vehicle_Car::RunWheelRayCasting(PhysEnv * phys)
{
	auto *world = phys->GetWorld();
	for (uint8_t i = 0; i < 4; ++i) {
		btRigidBody* selfRigid = WheelRigidBody[i];
		btHinge2Constraint* selfConstraint = WheelConstraints[i];
		btTransform selfTrans = Chassis->getWorldTransform();
				
		btVector3 direction = btVector3(0.0f, 1.0f, 0.0f) * btMatrix3x3(selfTrans.getRotation());
		//printf("%f\t%f\t%f\n", (float)direction[0], (float)direction[1], (float)direction[2]);
		btCollisionWorld::ClosestRayResultCallback callback(selfTrans.getOrigin(), selfTrans.getOrigin() + direction * 5);

		world->rayTest(selfTrans.getOrigin(), selfTrans.getOrigin() + direction*5, callback);
	}
}

void Vehicle_Car::Mount_Object(
	int indexVehicle,
	btRigidBody* pObject,

	btVector3 pivotInA,
	btVector3 pivotInB
)
{
	//	printf("CALLED?N\n");
	//	//if (!GLOBAL_Physics_Class->CarPool.size() < indexVehicle) return;
	//	//if (!GLOBAL_Physics_Class->CarPool[indexVehicle]) return;
	//	Vehicle_Car* CurrentVehicle = GLOBAL_Physics_Class->CarPool[indexVehicle];
	//	//if (!CurrentVehicle || CurrentVehicle == nullptr) return;
	//	//printf("CALLED2?N\n");
	//	
	//	(CurrentVehicle->Chassis)->setActivationState(DISABLE_DEACTIVATION);
	//	
	//	btPoint2PointConstraint* constraint = new btPoint2PointConstraint(*Chassis, *GLOBAL_Physics_Class->LocalPlayer, pivotInA, pivotInB);
	//	
	//	GLOBAL_Physics_Class->dynamicsWorld->addConstraint(constraint, true);
	//	
/*	//	
	//	btFixedConstraint test(, , , );
	//	lpSetPos- aPos + offSet;
	//	
	//	Finally Execute the constraint creation.
	//	Attribute Fixed Constraint to table.
	//	
	//	Finally, drive where you press E....
*/
}

void Vehicle_Car::AccelerationForce(uint8_t dirForce, uint32_t maxForce) {
	_IsVehicleStalled = false;

	for (int i = 0; i < 4; i++) { // All Tires.
		btHinge2Constraint* pHinge2 = WheelConstraints[i];
		{
			int motorAxis = 3;
			pHinge2->enableMotor(motorAxis, true);
			pHinge2->setMaxMotorForce(motorAxis, maxForce);
			pHinge2->setTargetVelocity(motorAxis, -dirForce);
		}

		{
			int motorAxis = 5;
			pHinge2->enableMotor(motorAxis, true);
			pHinge2->setMaxMotorForce(motorAxis, maxForce);
			pHinge2->setTargetVelocity(motorAxis, dirForce);
		}
	}
}

void Vehicle_Car::StallEngine() {
	if (!_IsVehicleStalled) {
		for (int i = 0; i < 4; i++) { // All Tires.
			btHinge2Constraint* pHinge2 = WheelConstraints[i];
			{
				int motorAxis = 3;
				pHinge2->enableMotor(motorAxis, false);
			}
			{
				int motorAxis = 5;
				pHinge2->enableMotor(motorAxis, false);
			}
		}

		_IsVehicleStalled = true;
	}
}

void Vehicle_Car::DeaccelerationForce(uint8_t dirForce, uint32_t maxForce) {
	_IsVehicleStalled = false;
		
	for (int i = 0; i < 4; i++) { // All Tires.
		btHinge2Constraint* pHinge2 = WheelConstraints[i];
		{
			int motorAxis = 3;
			pHinge2->enableMotor(motorAxis, true);
			pHinge2->setMaxMotorForce(motorAxis, maxForce);
			pHinge2->setTargetVelocity(motorAxis, dirForce);
		}

		{
			int motorAxis = 5;
			pHinge2->enableMotor(motorAxis, true);
			pHinge2->setMaxMotorForce(motorAxis, maxForce);
			pHinge2->setTargetVelocity(motorAxis, dirForce);
		}
	}
}

void Vehicle_Car::SteerTurn(float directAngle) {
	for (int i = 0; i < 2; i++) { // All Tires.
		btHinge2Constraint* pHinge2 = WheelConstraints[i];

		pHinge2->setLowerLimit(SIMD_HALF_PI * directAngle);
		pHinge2->setUpperLimit(SIMD_HALF_PI * directAngle);
	}
}

void Vehicle_Car::ControlInput(int input) {
	std::vector<BaseEntity*> players;
	GLOBAL_Physics_Class->GetNetworkState()->GetGameState()->GetPlayers(&players);
	auto pl = players.front()->GetPlayerComponent();

	if(pl->GetActionInput(Action::MoveLeft) == false && pl->GetActionInput(Action::MoveRight) == false)
		SteerTurn(0.0f);
	const uint8_t Accel_Force = 1000;
	const uint32_t MaxForce = 3500;
	const float Turn_Degree = 0.13f;
	if(pl->GetActionInput(Action::MoveForward) == false && pl->GetActionInput(Action::MoveBackward) == false)
		StallEngine();

	//	if ((input & CInputCMD::Forward) && (input & CInputCMD::MoveLeft)) {
	//		SteerTurn(-Turn_Degree);
	//		AccelerationForce(Accel_Force, MaxForce);
	//	}
	//	if ((input & CInputCMD::Forward) && (input & CInputCMD::MoveLeft)) {
	//		SteerTurn(-Turn_Degree);
	//		AccelerationForce(Accel_Force, MaxForce);
	//	}
	//	if ((input & CInputCMD::Forward) && (input & CInputCMD::MoveRight)) {
	//		SteerTurn(-Turn_Degree);
	//		AccelerationForce(Accel_Force, MaxForce);
	//	}

	if(pl->GetActionInput(Action::MoveForward)) {
		//DeaccelerationForce(0, 0);
		AccelerationForce(Accel_Force, MaxForce);
	}
	if(pl->GetActionInput(Action::MoveBackward)) {
		DeaccelerationForce(-Accel_Force*3, MaxForce);
		//AccelerationForce(0, 0);
	}
	if(pl->GetActionInput(Action::MoveLeft)) {
		SteerTurn(-Turn_Degree);
	}
	if(pl->GetActionInput(Action::MoveRight)) {
		SteerTurn(Turn_Degree);
	}

	if(pl->GetActionInput(Action::Jump)) {
		for (int i = 0; i < 4; i++) {
			WheelRigidBody[i]->setLinearVelocity(WheelRigidBody[i]->getLinearVelocity() * 0.9875);
		}
	}
}

void Vehicle_Car::InitVehicle(PhysEnv* phys, Vector3 pos) {
	auto *world = phys->GetWorld();
	GLOBAL_Physics_Class = phys;
	// Inits //
	btScalar
		mass = 500.f,
		wheelMass = 520.f,
		friction(0.20f),
		restitution(0.0f);
	btTransform localTrans;
	///rigidbody is dynamic if and only if mass is non zero, otherwise static

	btVector3
		global_position = btVector3(btScalar(pos.x), btScalar(pos.y), btScalar(pos.z)),
		obbmins = btVector3(1, 1, 1.0)*-1, obbmaxes = btVector3(1, 1, 1.0);
	/////


	// Object Creations //
	const float heightOfPhysBox = 4.45f;
	const float heightOfWheels = -4.45f;

	btConvexShape* chassisShape = new btBoxShape(btVector3(5.f, heightOfPhysBox, 10.0f));
	//phys->collisionShapes.push_back(chassisShape);
	// Entitiy Push_back
	/*phys->PhysicsObjectTable.push_back(
		{
			true, "vehicle_jeep",
			(uint32_t)phys->collisionShapes.size() - 1,
			mass, friction, restitution,  {
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
				0, 0, 0, 0,
			}, obbmins, obbmaxes,
			btVector3(1.0f, 1.0f, 1.0f),
			Collision_Group::Vehicle,
			1
		}
	);*/
	//
	//

	///*********************************************************** Begin Subobject Sequence
	btCompoundShape* compound = new btCompoundShape();
	//phys->collisionShapes.push_back(compound);
		
	localTrans.setIdentity();
	localTrans.setOrigin(btVector3(0, 5, 0));
	/// Adds current parent as the chassis, then adds the parent object for future bindings.
	compound->addChildShape(localTrans, chassisShape);

	localTrans.setOrigin(btVector3(0, 0.f, 0));


	/*---------------------------------------------------------------*/
	/// Create Dynamic Objects
	btVector3 localInertia(0, 0, 0);
	bool isDynamic = (mass != 0.f);
	if (isDynamic) chassisShape->calculateLocalInertia(mass, localInertia);

	btTransform startTransform;
	startTransform.setIdentity();

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, chassisShape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);
		body->setFriction(friction);
		body->setRestitution(restitution);
		body->activate(true);
		//body->setAngularFactor(0.075f);
		body->setRollingFriction(10);
		body->setDamping(0.08f, 0.1f);
		//body->setCcdMotionThreshold(0.0001f);
		body->setCcdSweptSphereRadius(0.5f);
		//body->setLinearFactor(btVector3(0.99f, 0.99f, 0.99f));
		
	world->addRigidBody(body, umath::to_integral(CollisionMask::Vehicle),
		umath::to_integral(CollisionMask::All));

	Chassis = body;


	/*---------------------------------------------------------------*/
	//body->setCcdMotionThreshold(1.0f);
	//body->setCcdSweptSphereRadius(0.2f);


	btSphereShape* m_wheelShape = new btSphereShape(veh__wheelRadius);
	//btCylinderShapeX* m_wheelShape = new btCylinderShapeX(btVector3(veh__wheelWidth, veh__wheelRadius, veh__wheelRadius));
	//btCapsuleShapeX* m_wheelShape = new btCapsuleShapeX(veh__wheelRadius, veh__wheelWidth);
	m_wheelShape->setMargin(veh__wheelRadius*0.25f);
	/////// Begin wheel creation offsets.
	btVector3 wheelPos[4] = {
		btVector3(btScalar(-8.), btScalar(heightOfWheels), btScalar(15.5)),
		btVector3(btScalar(	8.), btScalar(heightOfWheels), btScalar(15.5)),
		btVector3(btScalar(	8.), btScalar(heightOfWheels), btScalar(-15.5)),
		btVector3(btScalar(-8.), btScalar(heightOfWheels), btScalar(-15.5))
	};
	std::vector<btRigidBody*> wheelRigids;

	for (int i = 0; i < 4; i++)
	{
		//phys->collisionShapes.push_back(m_wheelShape);

		/*phys->PhysicsObjectTable.push_back(
			{
				true, "prop_wheel",
				(uint32_t)phys->collisionShapes.size() - 1,
				mass, friction, restitution, {
					0, 0, 0, 0,
					0, 0, 0, 0,
					0, 0, 0, 0,
					0, 0, 0, 0,
				}, obbmins, obbmaxes,
				btVector3(1.0f * veh__wheelRadius, 1.0f * veh__wheelRadius, 1.0f * veh__wheelRadius),
				umath::to_integral(CollisionMask::Vehicle),//Collision_Group::Wheel,
				1
			}
		);*/

		///////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////

		btRigidBody* pBodyA = body;
			pBodyA->setActivationState(DISABLE_DEACTIVATION);

			WheelRigidBody[i] = pBodyA;
		btTransform tr;
			tr.setIdentity();
			tr.setOrigin(wheelPos[i]);

		/*---------------------------------------------------------------*/ /// Generate Rigid Body (Physical Object) Wheel Created....
					btVector3 localInertia(0, 0, 0);
					bool isDynamic = (wheelMass != 0.f);
					if (isDynamic) m_wheelShape->calculateLocalInertia(wheelMass, localInertia);

					btDefaultMotionState* myMotionState = new btDefaultMotionState(tr);
					btRigidBody::btRigidBodyConstructionInfo rbInfo(wheelMass, myMotionState, m_wheelShape, localInertia);
					btRigidBody* pBodyB = new btRigidBody(rbInfo);
					//phys->collisionShapes.push_back(m_wheelShape);
					pBodyB->setRestitution(0.0f);
					world->addRigidBody(pBodyB, umath::to_integral(CollisionMask::Vehicle),//Collision_Group::Wheel,
						umath::to_integral(CollisionMask::All));//Collision_Group::All);
		/*---------------------------------------------------------------*/
			
		//pBodyB->setCcdMotionThreshold(0.0001f);
		//pBodyB->setCcdSweptSphereRadius(veh__wheelRadius);
		//pBodyB->setAngularFactor(0.2f);
		//pBodyB->setRestitution(1.0f);
		pBodyB->setDamping(0.08f, 0.15f);
		pBodyB->setActivationState(DISABLE_DEACTIVATION);
		pBodyB->setFriction(0.99f);
		pBodyB->setRollingFriction(1);
		pBodyB->setContactProcessingThreshold(0);
		wheelRigids.push_back(pBodyB);
		//pBodyB->setSpinningFriction(0.1);
		pBodyB->setAnisotropicFriction(m_wheelShape->getAnisotropicRollingFrictionDirection(), btCollisionObject::CF_ANISOTROPIC_ROLLING_FRICTION);

		// add some data to build constraint frames
		btVector3 parentAxis(0.f, 1.f, 0.f);
		btVector3 childAxis(1.f, 0.f, 0.f);
		btVector3 anchor = tr.getOrigin();//(0.f, 0.f, 0.f);
		WheelConstraints[i] = new btHinge2Constraint(*pBodyA, *pBodyB, anchor, parentAxis, childAxis);
		btHinge2Constraint* pHinge2 = WheelConstraints[i];
		//pHinge2->enableSpring(3, true);
		//pHinge2->enableSpring(5, true);
		pHinge2->setLowerLimit(0.0);
		pHinge2->setUpperLimit(0.0);
		//pHinge2->setLinearLowerLimit(btVector3(0.0, 0.0, 0.0));
		//pHinge2->setAngularLowerLimit(btVector3(0.0, 0.0, 0.0));


		//pHinge2->setAngularLowerLimitReversed(btVector3(0.0, 0.0, 0.0));

		//pHinge2->setLinearUpperLimit(btVector3(0.0, 0.0, 0.0));
		world->addConstraint(pHinge2, true);
		{
			int motorAxis = 3;
			pHinge2->enableMotor(motorAxis, false);
			pHinge2->enableSpring(motorAxis, false);
			pHinge2->setStiffness(motorAxis, veh__suspensionStiffness, true);
			pHinge2->setDamping(motorAxis, veh__suspensionDamping, true);
			pHinge2->setBounce(motorAxis, 8.0f);
			pHinge2->setTargetVelocity(motorAxis, -1);
		}

		{
			int motorAxis = 5;
			pHinge2->enableMotor(motorAxis, false);
			pHinge2->enableSpring(motorAxis, false);
			pHinge2->setStiffness(motorAxis, veh__suspensionStiffness, true);
			pHinge2->setDamping(motorAxis, veh__suspensionDamping, true);
			pHinge2->setBounce(motorAxis, 8.0f);
			pHinge2->setTargetVelocity(motorAxis, 0);
		}

		///////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////

		//pHinge2->setDbgDrawSize(btScalar(5.f));
	}

	startTransform.setIdentity();
	startTransform.setOrigin(global_position);

	body->setWorldTransform(startTransform);
	body->setLinearVelocity(btVector3(0, 0, 0));
	body->setAngularVelocity(btVector3(0, 0, 0));

	for (int w = 0; w < wheelRigids.size(); w++) {
		startTransform.setOrigin(global_position + wheelPos[w]);
		auto v = wheelRigids[w];

		v->setWorldTransform(startTransform);
		body->setLinearVelocity(btVector3(0, 0, 0));
		body->setAngularVelocity(btVector3(0, 0, 0));
	}
	wheelRigids.clear();

	{
		btRaycastVehicle::btVehicleTuning	m_tuning;
		m_vehicleRayCaster = new btDefaultVehicleRaycaster(world);
		m_vehicle = new btRaycastVehicle(m_tuning, body, m_vehicleRayCaster);

		float connectionHeight = 1.2f;
		btVector3 wheelDirectionCS0(0, -1, 0);
		btVector3 wheelAxleCS(-1, 0, 0);

		m_vehicle->setCoordinateSystem(0, 1, 2);

		btVector3
			connectionPointCS0(CUBE_HALF_EXTENTS - (0.3*veh__wheelWidth), connectionHeight, 2 * CUBE_HALF_EXTENTS - veh__wheelRadius);
		m_vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, veh__suspensionDamping, veh__wheelRadius, m_tuning, true);
		connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS + (0.3*veh__wheelWidth), connectionHeight, 2 * CUBE_HALF_EXTENTS - veh__wheelRadius);
		m_vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, veh__suspensionDamping, veh__wheelRadius, m_tuning, true);
		connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS + (0.3*veh__wheelWidth), connectionHeight, -2 * CUBE_HALF_EXTENTS + veh__wheelRadius);
		/// First Two Wheels
		m_vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, veh__suspensionDamping, veh__wheelRadius, m_tuning, false);
		connectionPointCS0 = btVector3(CUBE_HALF_EXTENTS - (0.3*veh__wheelWidth), connectionHeight, -2 * CUBE_HALF_EXTENTS + veh__wheelRadius);
		m_vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, veh__suspensionDamping, veh__wheelRadius, m_tuning, false);
		m_vehicle->setSteeringValue(-1.0f, 0);
		m_vehicle->setSteeringValue(-1.0f, 1);
		m_vehicle->setSteeringValue(-1.0f, 2);
		m_vehicle->setSteeringValue(-1.0f, 3);
		for (int i = 0; i < m_vehicle->getNumWheels(); i++) {
			btWheelInfo& wheel = m_vehicle->getWheelInfo(i);
			wheel.m_suspensionStiffness = veh__suspensionStiffness;
			wheel.m_wheelsDampingRelaxation = veh__suspensionDamping;
			wheel.m_wheelsDampingCompression = veh__suspensionCompression;
			wheel.m_frictionSlip = veh__wheelFriction;
			wheel.m_rollInfluence = veh__rollInfluence;
		}
	}
	//
}

//debug::Vehicle::Vehicle()
//{
//	auto *dynamicsWorld = c_physEnv->GetWorld();
//	auto vehicleMass = 400.0;
//	auto wheelWidth = 0.3;
//	auto wheelRadius = 0.4;
//	auto vehicleTuning = btRaycastVehicle::btVehicleTuning{};
//
//	auto suspensionStiffness = 1.f;
//	auto suspensionDamping = 1.f;
//	auto suspensionCompression = 1.f;
//	auto wheelFriction = 1.f;
//	auto rollInfluence = 1.f;
//
//	auto CUBE_HALF_EXTENT = 1.f; // ??
//	auto wheelDirectionCS0 = Vector3{0.f,1.f,0.f};
//	auto wheelAxleCS = Vector3{1.f,0.f,0.f};
//	auto suspensionRestLength = 1.f;
//
//	//wheelDirectionCS0, wheelAxleCS, suspensionRestLength
//	//tuning. // TODO
//
//    auto *groundShape = new btBoxShape(btVector3(50, 3, 50));
//    auto *fallShape = new btBoxShape(btVector3(1, 1, 1));
//
//    // Orientation and Position of Ground
//    auto *groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -3, 0)));
//    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
//    auto *groundRigidBody = new btRigidBody(groundRigidBodyCI);
//    dynamicsWorld->addRigidBody(groundRigidBody);
//
//    ///////////////////////////////////////////////////////////////////////
//    //              Vehicle Setup
//    ///////////////////////////////////////////////////////////////////////
//    auto *vehicleChassisShape = new btBoxShape(btVector3(1.f, 0.5f, 2.f));
//    auto *vehicleBody = new btCompoundShape();
//	
//	btTransform localTrans {};
//    localTrans.setIdentity();
//    localTrans.setOrigin(btVector3(0, 1, 0));
//    vehicleBody->addChildShape(localTrans, vehicleChassisShape);
//
//    localTrans.setOrigin(btVector3(3, 0.f, 0));
//    auto *vehicleMotionState = new btDefaultMotionState(localTrans);
//    //vehicleMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(3, 0, 0)));
//    btVector3 vehicleInertia(0, 0, 0);
//    vehicleBody->calculateLocalInertia(vehicleMass, vehicleInertia);
//    btRigidBody::btRigidBodyConstructionInfo vehicleRigidBodyCI(vehicleMass, vehicleMotionState, vehicleBody, vehicleInertia);
//
//    auto *vehicleRigidBody = new btRigidBody(vehicleRigidBodyCI);
//    dynamicsWorld->addRigidBody(vehicleRigidBody);
//
//    auto *wheelShape = new btCylinderShapeX(btVector3(wheelWidth, wheelRadius, wheelRadius));
//    {
//        auto *vehicleRayCaster = new btDefaultVehicleRaycaster(dynamicsWorld);
//        auto *vehicle = new btRaycastVehicle(vehicleTuning, vehicleRigidBody, vehicleRayCaster);
//
//        // never deactivate vehicle
//        vehicleRigidBody->setActivationState(DISABLE_DEACTIVATION);
//        dynamicsWorld->addVehicle(vehicle);
//
//        float connectionHeight = 1.2f;
//        bool isFrontWheel = true;
//
//        vehicle->setCoordinateSystem(0,1,2);//rightIndex, upIndex, forwardIndex); // 0, 1, 2
//
//        // add wheels
//        // front left
//        btVector3 connectionPointCS0(CUBE_HALF_EXTENT-(0.3*wheelWidth), connectionHeight, 2*CUBE_HALF_EXTENT-wheelRadius);
//        vehicle->addWheel(connectionPointCS0, uvec::create_bt(wheelDirectionCS0), uvec::create_bt(wheelAxleCS), suspensionRestLength, wheelRadius, vehicleTuning, isFrontWheel);
//        // front right
//        connectionPointCS0 = btVector3(-CUBE_HALF_EXTENT+(0.3*wheelWidth), connectionHeight, 2*CUBE_HALF_EXTENT-wheelRadius);
//        vehicle->addWheel(connectionPointCS0, uvec::create_bt(wheelDirectionCS0), uvec::create_bt(wheelAxleCS), suspensionRestLength, wheelRadius, vehicleTuning, isFrontWheel);
//        isFrontWheel = false;
//        // rear right
//        connectionPointCS0 = btVector3(-CUBE_HALF_EXTENT+(0.3*wheelWidth), connectionHeight, -2*CUBE_HALF_EXTENT+wheelRadius);
//        vehicle->addWheel(connectionPointCS0, uvec::create_bt(wheelDirectionCS0), uvec::create_bt(wheelAxleCS), suspensionRestLength, wheelRadius, vehicleTuning, isFrontWheel);
//        // rear left
//        connectionPointCS0 = btVector3(CUBE_HALF_EXTENT-(0.3*wheelWidth), connectionHeight, -2*CUBE_HALF_EXTENT+wheelRadius);
//        vehicle->addWheel(connectionPointCS0, uvec::create_bt(wheelDirectionCS0), uvec::create_bt(wheelAxleCS), suspensionRestLength, wheelRadius, vehicleTuning, isFrontWheel);
//
//        for (int i = 0; i < vehicle->getNumWheels(); i++)
//        {
//            btWheelInfo& wheel = vehicle->getWheelInfo(i);
//            wheel.m_suspensionStiffness = suspensionStiffness;
//            wheel.m_wheelsDampingRelaxation = suspensionDamping;
//            wheel.m_wheelsDampingCompression = suspensionCompression;
//            wheel.m_frictionSlip = wheelFriction;
//            wheel.m_rollInfluence = rollInfluence;
//        }
//
//
//    }
//
//    ///////////////////////////////////////////////////////////////////////
//
//    // Orientation and Position of Falling body
//    auto *fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(-1, 5, 0)));
//    btScalar mass = 1;
//    btVector3 fallInertia(0, 0, 0);
//    fallShape->calculateLocalInertia(mass, fallInertia);
//    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
//    auto *fallRigidBody = new btRigidBody(fallRigidBodyCI);
//    dynamicsWorld->addRigidBody(fallRigidBody);
//}
