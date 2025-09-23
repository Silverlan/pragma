// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "mathutil/umath.h"

export module pragma.client:rendering.entity_instance_data;

export namespace pragma::rendering {
#pragma pack(push, 1)
	struct InstanceData {
		enum class RenderFlags : uint32_t { None = 0u, Weighted = 1u };
		Mat4 modelMatrix;
		Vector4 color;
		RenderFlags renderFlags;
		uint32_t entityIndex;
		Vector2 padding;
	};
#pragma pack(pop)
};
