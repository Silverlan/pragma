/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_SURFACE_COMPONENT_HPP__
#define __BASE_SURFACE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <mathutil/plane.hpp>

namespace pragma {
	class DLLNETWORK BaseSurfaceComponent : public BaseEntityComponent {
	  public:
		static pragma::ComponentEventId EVENT_ON_SURFACE_PLANE_CHANGED;
		static pragma::ComponentEventId EVENT_ON_SURFACE_MESH_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;

		virtual void SetPlane(const umath::Plane &plane);
		const umath::Plane &GetPlane() const;
		umath::Plane GetPlaneWs() const;

		void SetPlane(const Vector3 &n, float d);
		void GetPlane(Vector3 &n, float &d) const;
		void GetPlaneWs(Vector3 &n, float &d) const;

		const Vector3 &GetPlaneNormal() const;
		float GetPlaneDistance() const;

		void SetPlaneNormal(const Vector3 &n);
		void SetPlaneDistance(float d);

		void Clear();

		Vector3 ProjectToSurface(const Vector3 &pos) const;
		ModelSubMesh *GetMesh();
		const ModelSubMesh *GetMesh() const { return const_cast<BaseSurfaceComponent *>(this)->GetMesh(); }

		struct DLLNETWORK MeshInfo {
			ModelMesh *mesh;
			ModelSubMesh *subMesh;
			Material *material;
		};
		std::optional<MeshInfo> FindAndAssignMesh(const std::function<int32_t(ModelMesh &, ModelSubMesh &, Material &, const std::string &)> &filter = nullptr);
		bool CalcLineSurfaceIntersection(const Vector3 &lineOrigin, const Vector3 &lineDir, double *outT = nullptr) const;

		bool IsPointBelowSurface(const Vector3 &p) const;
		Quat GetPlaneRotation() const;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
	  protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		BaseSurfaceComponent(BaseEntity &ent);
		umath::Plane m_plane = {{0.f, 1.f, 0.f}, 0.f};
		std::weak_ptr<ModelSubMesh> m_mesh = {};
		pragma::NetEventId m_netEvSetPlane = pragma::INVALID_NET_EVENT;
		std::string m_kvSurfaceMaterial;
	};

	struct DLLNETWORK CEOnSurfaceMeshChanged : public ComponentEvent {
		CEOnSurfaceMeshChanged(const BaseSurfaceComponent::MeshInfo &meshInfo);
		virtual void PushArguments(lua_State *l) override;
		BaseSurfaceComponent::MeshInfo meshInfo;
	};
};

#endif
