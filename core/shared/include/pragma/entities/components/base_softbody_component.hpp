/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_SOFTBODY_COMPONENT_HPP__
#define __BASE_SOFTBODY_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

class BrushMesh;
struct BulletInfo;
namespace pragma
{
	class DLLNETWORK BaseSoftBodyComponent
		: public BaseEntityComponent
	{
	public:
		// Soft-body physics
		struct SoftBodyData
		{
			// Copies of the model's meshes
			std::vector<std::shared_ptr<ModelMesh>> meshes = {};
		};
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;

		virtual bool InitializeSoftBodyData();
		virtual void ReleaseSoftBodyData();

		const SoftBodyData *GetSoftBodyData() const;
		SoftBodyData *GetSoftBodyData();
	protected:
		std::unique_ptr<SoftBodyData> m_softBodyData = nullptr;
	};
};

#endif
