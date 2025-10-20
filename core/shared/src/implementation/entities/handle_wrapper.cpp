// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <string>

module pragma.shared;

import :entities.handle_wrapper;

pragma::EntityHandleWrapper::EntityHandleWrapper(pragma::ecs::BaseEntity *ent) {
	handle = new EntityHandle {ent->GetHandle()};
}

pragma::EntityHandleWrapper::~EntityHandleWrapper() {
	delete handle;
}

pragma::EntityHandleWrapper::operator EntityHandle&() {return *handle;}
pragma::EntityHandleWrapper::operator const EntityHandle&() const {return *static_cast<EntityHandle*>(handle);}
