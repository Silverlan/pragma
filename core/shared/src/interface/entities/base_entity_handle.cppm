// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:entities.base_entity_handle;

export import pragma.util;

export {
	namespace pragma::ecs {
		class BaseEntity;
	}
	using EntityHandle = pragma::util::TWeakSharedHandle<pragma::ecs::BaseEntity>;
};
