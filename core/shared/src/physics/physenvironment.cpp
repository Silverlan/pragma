#include "stdafx_shared.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/constraint.hpp"
#include "pragma/physics/collision_object.hpp"
#include "pragma/physics/shape.hpp"
#include "pragma/physics/controller.hpp"
#include <pragma/console/conout.h>
#include "pragma/entities/baseentity.h"
#include "pragma/physics/physobj.h"
#include "pragma/physics/physcontact.h"
#include "pragma/physics/raytraces.h"
#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include "pragma/audio/alsound_type.h"
#include "pragma/model/modelmesh.h"
#include "pragma/util/util_game.hpp"
#include "pragma/physics/phys_water_buoyancy_simulator.hpp"
#include "pragma/physics/physsoftbodyinfo.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/trigger/base_trigger_touch.hpp"

pragma::physics::IEnvironment::IEnvironment(NetworkState &state)
	: m_nwState{state}
{
	Con::cout<<"Initializing physics environment..."<<Con::endl;
	for(auto i=0;i<umath::to_integral(Event::Count);i++)
		m_callbacks.insert(std::unordered_map<Event,std::vector<CallbackHandle>>::value_type(static_cast<Event>(i),std::vector<CallbackHandle>()));

	m_buoyancySim = std::make_shared<physics::WaterBuoyancySimulator>();
}
pragma::physics::IEnvironment::~IEnvironment()
{
	ClearConstraints();
	ClearCollisionObjects();
	ClearControllers();
}
NetworkState &pragma::physics::IEnvironment::GetNetworkState() const {return m_nwState;}
double pragma::physics::IEnvironment::GetTimeScale() const
{
	auto *game = m_nwState.GetGameState();
	if(game == nullptr)
		return 0.0;
	return game->GetTimeScale();
}
pragma::physics::IMaterial &pragma::physics::IEnvironment::GetGenericMaterial() const {return *m_genericMaterial;}
float pragma::physics::IEnvironment::GetWorldScale() const {return 1.f;}
pragma::physics::IVisualDebugger *pragma::physics::IEnvironment::GetVisualDebugger() const {return m_visualDebugger.get();}
void pragma::physics::IEnvironment::AddEventCallback(Event eventid,const CallbackHandle &hCb)
{
	if(umath::to_integral(eventid) < 0 || umath::to_integral(eventid) >= umath::to_integral(Event::Count))
		return;
	m_callbacks[eventid].push_back(hCb);
}
void pragma::physics::IEnvironment::ClearConstraints()
{
	for(auto &hConstraint : m_constraints)
	{
		if(hConstraint.IsExpired())
			continue;
		hConstraint.Remove();
	}
	m_constraints.clear();
}
void pragma::physics::IEnvironment::ClearCollisionObjects()
{
	for(auto &hColObj : m_collisionObjects)
	{
		if(hColObj.IsExpired())
			continue;
		hColObj.Remove();
	}
	m_collisionObjects.clear();
}
void pragma::physics::IEnvironment::ClearControllers()
{
	for(auto &hController : m_controllers)
	{
		if(hController.IsExpired())
			continue;
		hController.Remove();
	}
	m_controllers.clear();
}

const pragma::physics::WaterBuoyancySimulator &pragma::physics::IEnvironment::GetWaterBuoyancySimulator() const {return *m_buoyancySim;}
void pragma::physics::IEnvironment::AddConstraint(IConstraint &constraint)
{
	auto hConstraint = constraint.GetHandle();
	m_constraints.push_back(util::shared_handle_cast<IBase,IConstraint>(hConstraint));
	CallCallbacks<IConstraint>(Event::OnConstraintCreated,constraint);
}
void pragma::physics::IEnvironment::AddCollisionObject(ICollisionObject &obj)
{
	auto hCollisionObject = obj.GetHandle();
	m_collisionObjects.push_back(util::shared_handle_cast<IBase,ICollisionObject>(hCollisionObject));
	CallCallbacks<ICollisionObject>(Event::OnCollisionObjectCreated,obj);
}
void pragma::physics::IEnvironment::AddController(IController &controller)
{
	auto hController = controller.GetHandle();
	m_controllers.push_back(util::shared_handle_cast<IBase,IController>(hController));
	CallCallbacks<IController>(Event::OnControllerCreated,controller);
}
void pragma::physics::IEnvironment::RemoveConstraint(IConstraint &constraint)
{
	auto it = std::find_if(m_constraints.begin(),m_constraints.end(),[&constraint](const util::TSharedHandle<IConstraint> &hConstraint) {
		return hConstraint.Get() == &constraint;
	});
	if(it == m_constraints.end())
		return;
	auto pConstraint = *it; // Keep a handle to make sure the reference is still valid
	m_constraints.erase(it);
	if(pConstraint.IsValid() == false)
		return;
	CallCallbacks<IConstraint>(Event::OnConstraintRemoved,constraint);
	pConstraint.Remove();
}
void pragma::physics::IEnvironment::RemoveCollisionObject(ICollisionObject &obj)
{
	auto it = std::find_if(m_collisionObjects.begin(),m_collisionObjects.end(),[&obj](const util::TSharedHandle<ICollisionObject> &hColObj) {
		return hColObj.Get() == &obj;
	});
	if(it == m_collisionObjects.end())
		return;
	auto pCollisionObject = *it; // Keep a handle to make sure the reference is still valid
	m_collisionObjects.erase(it);
	if(pCollisionObject.IsValid() == false)
		return;
	CallCallbacks<ICollisionObject>(Event::OnCollisionObjectRemoved,obj);
	pCollisionObject.Remove();
}
void pragma::physics::IEnvironment::RemoveController(IController &controller)
{
	auto it = std::find_if(m_controllers.begin(),m_controllers.end(),[&controller](const util::TSharedHandle<IController> &hConstraint) {
		return hConstraint.Get() == &controller;
		});
	if(it == m_controllers.end())
		return;
	auto pController = *it; // Keep a handle to make sure the reference is still valid
	m_controllers.erase(it);
	if(pController.IsValid() == false)
		return;
	CallCallbacks<IController>(Event::OnControllerRemoved,controller);
	pController.Remove();
}

const std::vector<util::TSharedHandle<pragma::physics::IConstraint>> &pragma::physics::IEnvironment::GetConstraints() const {return const_cast<IEnvironment*>(this)->GetConstraints();}
std::vector<util::TSharedHandle<pragma::physics::IConstraint>> &pragma::physics::IEnvironment::GetConstraints() {return m_constraints;}
const std::vector<util::TSharedHandle<pragma::physics::ICollisionObject>> &pragma::physics::IEnvironment::GetCollisionObjects() const {return const_cast<IEnvironment*>(this)->GetCollisionObjects();}
std::vector<util::TSharedHandle<pragma::physics::ICollisionObject>> &pragma::physics::IEnvironment::GetCollisionObjects() {return m_collisionObjects;}
const std::vector<util::TSharedHandle<pragma::physics::IController>> &pragma::physics::IEnvironment::GetControllers() const {return const_cast<IEnvironment*>(this)->GetControllers();}
std::vector<util::TSharedHandle<pragma::physics::IController>> &pragma::physics::IEnvironment::GetControllers() {return m_controllers;}
