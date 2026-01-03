// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.camera;
import :game;

using namespace pragma;

CCameraComponent::~CCameraComponent() {}
void CCameraComponent::Save(udm::LinkedPropertyWrapperArg udm) { BaseEnvCameraComponent::Save(udm); }
void CCameraComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) { BaseEnvCameraComponent::Load(udm, version); }
void CCameraComponent::Initialize()
{
	BaseEnvCameraComponent::Initialize();

	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent != nullptr) {
		auto &trC = *pTrComponent;
		FlagCallbackForRemoval(pTrComponent->AddEventCallback(cTransformComponent::EVENT_ON_POSE_CHANGED,
		                         [this, &trC](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
			                         FlagViewMatrixAsDirty();
			                         return util::EventReply::Unhandled;
		                         }),
		  CallbackType::Entity);
	}
}
void CCameraComponent::OnEntitySpawn()
{
	BaseEnvCameraComponent::OnEntitySpawn();
	FlagViewMatrixAsDirty();
	UpdateState();
}
void CCameraComponent::UpdateState()
{
	if(GetEntity().IsSpawned() == false)
		return;
	auto toggleC = GetEntity().GetComponent<CToggleComponent>();
	if(toggleC.expired() || toggleC->IsTurnedOn()) {
		auto *renderScene = get_cgame()->GetRenderScene<CSceneComponent>();
		if(renderScene && static_cast<ecs::CBaseEntity &>(GetEntity()).IsInScene(*renderScene))
			renderScene->SetActiveCamera(*this);
		return;
	}

	auto *renderScene = get_cgame()->GetRenderScene<CSceneComponent>();
	if(renderScene) {
		if(renderScene->GetActiveCamera().get() == this) {
			ecs::EntityIterator entIt {*get_cgame()};
			entIt.AttachFilter<TEntityIteratorFilterComponent<CCameraComponent>>();
			for(auto *ent : entIt) {
				auto toggleC = ent->GetComponent<CToggleComponent>();
				if((toggleC.valid() && toggleC->IsTurnedOn() == false) || static_cast<ecs::CBaseEntity *>(ent)->IsInScene(*renderScene) == false)
					continue;
				auto camC = ent->GetComponent<CCameraComponent>();
				renderScene->SetActiveCamera(*camC);
				break;
			}
		}
	}
}
void CCameraComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEnvCameraComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CFieldAngleComponent))
		SetFieldAngleComponent(static_cast<CFieldAngleComponent &>(component));
}
util::EventReply CCameraComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEnvCameraComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == baseToggleComponent::EVENT_ON_TURN_ON || eventId == baseToggleComponent::EVENT_ON_TURN_OFF)
		UpdateState();
	return util::EventReply::Unhandled;
}
void CCameraComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////

void CEnvCamera::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CCameraComponent>();
}
