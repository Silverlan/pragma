// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_soft_body;

export import :entities.components.base;

export {
	namespace pragma {
		namespace geometry {
			class ModelMesh;
		}
		class DLLNETWORK BaseSoftBodyComponent : public BaseEntityComponent {
		  public:
			// Soft-body physics
			struct SoftBodyData {
				// Copies of the model's meshes
				std::vector<std::shared_ptr<geometry::ModelMesh>> meshes = {};
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
}
