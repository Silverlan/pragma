// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASE_SOFTBODY_COMPONENT_HPP__
#define __BASE_SOFTBODY_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

class BrushMesh;
struct BulletInfo;
namespace pragma {
	class DLLNETWORK BaseSoftBodyComponent : public BaseEntityComponent {
	  public:
		// Soft-body physics
		struct SoftBodyData {
			// Copies of the model's meshes
			std::vector<std::shared_ptr<ModelMesh>> meshes = {};
		};
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;

		virtual bool InitializeSoftBodyData();
		virtual void ReleaseSoftBodyData();

		const SoftBodyData *GetSoftBodyData() const;
		SoftBodyData *GetSoftBodyData();
	  protected:
		std::unique_ptr<SoftBodyData> m_softBodyData = nullptr;
	};
};

#endif
