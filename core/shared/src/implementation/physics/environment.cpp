// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.environment;

std::vector<std::string> pragma::physics::IEnvironment::GetAvailablePhysicsEngines()
{
	std::vector<std::string> dirs {};
	fs::find_files("modules/physics_engines/*", nullptr, &dirs,
	  fs::SearchFlags::Local // Binary modules can only be loaded from actual files
	);
	return dirs;
}
std::string pragma::physics::IEnvironment::GetPhysicsEngineModuleLocation(const std::string &physEngine) { return "physics_engines/" + physEngine + "/pr_" + physEngine; }
pragma::physics::IEnvironment::IEnvironment(NetworkState &state) : m_nwState {state}
{
	Con::COUT << "Initializing physics environment..." << Con::endl;
	for(auto i = 0; i < math::to_integral(Event::Count); i++)
		m_callbacks.insert(std::unordered_map<Event, std::vector<CallbackHandle>>::value_type(static_cast<Event>(i), std::vector<CallbackHandle>()));

	m_buoyancySim = pragma::util::make_shared<WaterBuoyancySimulator>();
	m_surfTypeManager.RegisterType("generic");
}
void pragma::physics::IEnvironment::OnRemove()
{
	ClearConstraints();
	ClearCollisionObjects();
	ClearControllers();
	ClearVehicles();
}
bool pragma::physics::IEnvironment::Initialize()
{
	m_genericMaterial = CreateMaterial(0.5f, 0.5f, 0.6f);
	return m_genericMaterial != nullptr;
}
pragma::physics::IEnvironment::~IEnvironment() {}
pragma::NetworkState &pragma::physics::IEnvironment::GetNetworkState() const { return m_nwState; }
double pragma::physics::IEnvironment::GetTimeScale() const
{
	auto *game = m_nwState.GetGameState();
	if(game == nullptr)
		return 0.0;
	return game->GetTimeScale();
}
pragma::physics::IMaterial &pragma::physics::IEnvironment::GetGenericMaterial() const { return *m_genericMaterial; }
float pragma::physics::IEnvironment::GetWorldScale() const { return 1.f; }
void pragma::physics::IEnvironment::SetVisualDebugger(std::unique_ptr<IVisualDebugger> debugger)
{
	m_visualDebugger = std::move(debugger);
	OnVisualDebuggerChanged(m_visualDebugger.get());
}
pragma::physics::IVisualDebugger *pragma::physics::IEnvironment::GetVisualDebugger() const { return m_visualDebugger.get(); }
void pragma::physics::IEnvironment::AddEventCallback(Event eventid, const CallbackHandle &hCb)
{
	if(math::to_integral(eventid) < 0 || math::to_integral(eventid) >= math::to_integral(Event::Count))
		return;
	m_callbacks[eventid].push_back(hCb);
}
void pragma::physics::IEnvironment::ClearConstraints()
{
	for(auto &hConstraint : m_constraints) {
		if(hConstraint.IsExpired())
			continue;
		hConstraint.Remove();
	}
	m_constraints.clear();
}
void pragma::physics::IEnvironment::ClearCollisionObjects()
{
	for(auto &hColObj : m_collisionObjects) {
		if(hColObj.IsExpired())
			continue;
		hColObj.Remove();
	}
	m_collisionObjects.clear();
}
void pragma::physics::IEnvironment::ClearControllers()
{
	for(auto &hController : m_controllers) {
		if(hController.IsExpired())
			continue;
		hController.Remove();
	}
	m_controllers.clear();
}
void pragma::physics::IEnvironment::ClearVehicles()
{
	for(auto &hVehicle : m_vehicles) {
		if(hVehicle.IsExpired())
			continue;
		hVehicle.Remove();
	}
	m_vehicles.clear();
}

const pragma::physics::WaterBuoyancySimulator &pragma::physics::IEnvironment::GetWaterBuoyancySimulator() const { return *m_buoyancySim; }
void pragma::physics::IEnvironment::AddConstraint(IConstraint &constraint)
{
	auto hConstraint = constraint.ClaimOwnership();
	m_constraints.push_back(pragma::util::shared_handle_cast<IBase, IConstraint>(hConstraint));
	CallCallbacks<IConstraint>(Event::OnConstraintCreated, constraint);
}
void pragma::physics::IEnvironment::AddCollisionObject(ICollisionObject &obj)
{
	auto hCollisionObject = obj.ClaimOwnership();
	m_collisionObjects.push_back(pragma::util::shared_handle_cast<IBase, ICollisionObject>(hCollisionObject));
	CallCallbacks<ICollisionObject>(Event::OnCollisionObjectCreated, obj);
}
void pragma::physics::IEnvironment::AddController(IController &controller)
{
	auto hController = controller.ClaimOwnership();
	m_controllers.push_back(pragma::util::shared_handle_cast<IBase, IController>(hController));
	CallCallbacks<IController>(Event::OnControllerCreated, controller);
}
void pragma::physics::IEnvironment::AddVehicle(IVehicle &vehicle)
{
	auto hVehicle = vehicle.ClaimOwnership();
	m_vehicles.push_back(pragma::util::shared_handle_cast<IBase, IVehicle>(hVehicle));
	CallCallbacks<IVehicle>(Event::OnVehicleCreated, vehicle);
}
void pragma::physics::IEnvironment::SetEventCallback(std::unique_ptr<IEventCallback> evCallback) { m_eventCallback = std::move(evCallback); }
void pragma::physics::IEnvironment::OnContact(const ContactInfo &contactInfo)
{
	if(m_eventCallback == nullptr)
		return;
	m_eventCallback->OnContact(contactInfo);
}
void pragma::physics::IEnvironment::OnStartTouch(ICollisionObject &a, ICollisionObject &b)
{
	if(m_eventCallback == nullptr)
		return;
	m_eventCallback->OnStartTouch(a, b);
}
void pragma::physics::IEnvironment::OnEndTouch(ICollisionObject &a, ICollisionObject &b)
{
	if(m_eventCallback == nullptr)
		return;
	m_eventCallback->OnEndTouch(a, b);
}
void pragma::physics::IEnvironment::OnWake(ICollisionObject &o)
{
	if(m_eventCallback == nullptr)
		return;
	m_eventCallback->OnWake(o);
}
void pragma::physics::IEnvironment::OnSleep(ICollisionObject &o)
{
	if(m_eventCallback == nullptr)
		return;
	m_eventCallback->OnSleep(o);
}
void pragma::physics::IEnvironment::OnConstraintBroken(IConstraint &constraint)
{
	if(m_eventCallback == nullptr)
		return;
	m_eventCallback->OnConstraintBroken(constraint);
}
void pragma::physics::IEnvironment::RemoveConstraint(IConstraint &constraint)
{
	auto it = std::find_if(m_constraints.begin(), m_constraints.end(), [&constraint](const util::TSharedHandle<IConstraint> &hConstraint) { return hConstraint.GetRawPtr() == &constraint; });
	if(it == m_constraints.end())
		return;
	auto pConstraint = *it; // Keep a handle to make sure the reference is still valid
	m_constraints.erase(it);
	if(pConstraint.IsValid() == false)
		return;
	CallCallbacks<IConstraint>(Event::OnConstraintRemoved, constraint);
	pConstraint.Remove();
}
void pragma::physics::IEnvironment::RemoveCollisionObject(ICollisionObject &obj)
{
	auto it = std::find_if(m_collisionObjects.begin(), m_collisionObjects.end(), [&obj](const util::TSharedHandle<ICollisionObject> &hColObj) { return hColObj.GetRawPtr() == &obj; });
	if(it == m_collisionObjects.end())
		return;
	auto pCollisionObject = *it; // Keep a handle to make sure the reference is still valid
	m_collisionObjects.erase(it);
	if(pCollisionObject.IsValid() == false)
		return;
	CallCallbacks<ICollisionObject>(Event::OnCollisionObjectRemoved, obj);
	pCollisionObject.Remove();
}
void pragma::physics::IEnvironment::RemoveController(IController &controller)
{
	auto it = std::find_if(m_controllers.begin(), m_controllers.end(), [&controller](const util::TSharedHandle<IController> &hConstraint) { return hConstraint.GetRawPtr() == &controller; });
	if(it == m_controllers.end())
		return;
	auto pController = *it; // Keep a handle to make sure the reference is still valid
	m_controllers.erase(it);
	if(pController.IsValid() == false)
		return;
	CallCallbacks<IController>(Event::OnControllerRemoved, controller);
	pController.Remove();
}
void pragma::physics::IEnvironment::RemoveVehicle(IVehicle &vehicle)
{
	auto it = std::find_if(m_vehicles.begin(), m_vehicles.end(), [&vehicle](const util::TSharedHandle<IVehicle> &hVehicle) { return hVehicle.GetRawPtr() == &vehicle; });
	if(it == m_vehicles.end())
		return;
	auto pVehicle = *it; // Keep a handle to make sure the reference is still valid
	m_vehicles.erase(it);
	if(pVehicle.IsValid() == false)
		return;
	CallCallbacks<IVehicle>(Event::OnVehicleRemoved, vehicle);
	pVehicle.Remove();
}

std::shared_ptr<pragma::physics::ICompoundShape> pragma::physics::IEnvironment::CreateCompoundShape()
{
	std::vector<IShape *> shapes {};
	return CreateCompoundShape(shapes);
}
std::shared_ptr<pragma::physics::ICompoundShape> pragma::physics::IEnvironment::CreateCompoundShape(IShape &shape)
{
	std::vector<IShape *> shapes {&shape};
	return CreateCompoundShape(shapes);
}

const std::vector<pragma::util::TSharedHandle<pragma::physics::IConstraint>> &pragma::physics::IEnvironment::GetConstraints() const { return const_cast<IEnvironment *>(this)->GetConstraints(); }
std::vector<pragma::util::TSharedHandle<pragma::physics::IConstraint>> &pragma::physics::IEnvironment::GetConstraints() { return m_constraints; }
const std::vector<pragma::util::TSharedHandle<pragma::physics::ICollisionObject>> &pragma::physics::IEnvironment::GetCollisionObjects() const { return const_cast<IEnvironment *>(this)->GetCollisionObjects(); }
std::vector<pragma::util::TSharedHandle<pragma::physics::ICollisionObject>> &pragma::physics::IEnvironment::GetCollisionObjects() { return m_collisionObjects; }
const std::vector<pragma::util::TSharedHandle<pragma::physics::IController>> &pragma::physics::IEnvironment::GetControllers() const { return const_cast<IEnvironment *>(this)->GetControllers(); }
std::vector<pragma::util::TSharedHandle<pragma::physics::IController>> &pragma::physics::IEnvironment::GetControllers() { return m_controllers; }
const std::vector<pragma::util::TSharedHandle<pragma::physics::IVehicle>> &pragma::physics::IEnvironment::GetVehicles() const { return const_cast<IEnvironment *>(this)->GetVehicles(); }
std::vector<pragma::util::TSharedHandle<pragma::physics::IVehicle>> &pragma::physics::IEnvironment::GetVehicles() { return m_vehicles; }

const pragma::physics::SurfaceTypeManager &pragma::physics::IEnvironment::GetSurfaceTypeManager() const { return const_cast<IEnvironment *>(this)->GetSurfaceTypeManager(); }
pragma::physics::SurfaceTypeManager &pragma::physics::IEnvironment::GetSurfaceTypeManager() { return m_surfTypeManager; }
const pragma::physics::TireTypeManager &pragma::physics::IEnvironment::GetTireTypeManager() const { return const_cast<IEnvironment *>(this)->GetTireTypeManager(); }
pragma::physics::TireTypeManager &pragma::physics::IEnvironment::GetTireTypeManager() { return m_tireTypeManager; }

void pragma::physics::IEnvironment::SetSurfaceTypesDirty() { math::set_flag(m_stateFlags, StateFlags::SurfacesDirty, true); }

pragma::physics::IEnvironment::RemainingDeltaTime pragma::physics::IEnvironment::StepSimulation(float timeStep, int maxSubSteps, float fixedTimeStep)
{
	if(math::is_flag_set(m_stateFlags, StateFlags::SurfacesDirty)) {
		math::set_flag(m_stateFlags, StateFlags::SurfacesDirty, false);
		UpdateSurfaceTypes();
	}
	return DoStepSimulation(timeStep, maxSubSteps, fixedTimeStep);
}

bool pragma::physics::PhysSoftBodyInfo::operator==(const PhysSoftBodyInfo &other) const
{
	return poseMatchingCoefficient == other.poseMatchingCoefficient && anchorsHardness == other.anchorsHardness && dragCoefficient == other.dragCoefficient && rigidContactsHardness == other.rigidContactsHardness && softContactsHardness == other.softContactsHardness
	  && liftCoefficient == other.liftCoefficient && kineticContactsHardness == other.kineticContactsHardness && dynamicFrictionCoefficient == other.dynamicFrictionCoefficient && dampingCoefficient == other.dampingCoefficient
	  && volumeConversationCoefficient == other.volumeConversationCoefficient && softVsRigidImpulseSplitK == other.softVsRigidImpulseSplitK && softVsRigidImpulseSplitR == other.softVsRigidImpulseSplitR && softVsRigidImpulseSplitS == other.softVsRigidImpulseSplitS
	  && softVsKineticHardness == other.softVsKineticHardness && softVsRigidHardness == other.softVsRigidHardness && softVsSoftHardness == other.softVsSoftHardness && pressureCoefficient == other.pressureCoefficient && velocitiesCorrectionFactor == other.velocitiesCorrectionFactor &&

	  bendingConstraintsDistance == other.bendingConstraintsDistance && clusterCount == other.clusterCount && maxClusterIterations == other.maxClusterIterations && materialStiffnessCoefficient == other.materialStiffnessCoefficient;
}
