#include "stdafx_shared.h"
// TODO: Remove this file
#if 0
#include "pragma/entities/components/base_game_object_component.hpp"

using namespace pragma;

BaseGameObjectComponent::BaseGameObjectComponent(GameObject &obj)
	: m_gameObject(obj)
{}
const GameObject &BaseGameObjectComponent::GetGameObject() const {return const_cast<BaseGameObjectComponent&>(*this).GetGameObject();}
GameObject &BaseGameObjectComponent::GetGameObject() {return m_gameObject;}
const GameObject &BaseGameObjectComponent::operator->() const {return GetGameObject();}
GameObject &BaseGameObjectComponent::operator->() {return GetGameObject();}
#endif
