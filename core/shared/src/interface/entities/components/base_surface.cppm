// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_surface;

export import :entities.components.base;
export import pragma.materialsystem;

export {
	namespace pragma {
		namespace geometry {
			class ModelMesh;
			class ModelSubMesh;
		}
		namespace baseSurfaceComponent {
			REGISTER_COMPONENT_EVENT(EVENT_ON_SURFACE_PLANE_CHANGED)
			REGISTER_COMPONENT_EVENT(EVENT_ON_SURFACE_MESH_CHANGED)
		}
		class DLLNETWORK BaseSurfaceComponent : public BaseEntityComponent {
		  public:
			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
			virtual void Initialize() override;

			virtual void SetPlane(const math::Plane &plane);
			const math::Plane &GetPlane() const;
			math::Plane GetPlaneWs() const;

			void SetPlane(const Vector3 &n, float d);
			void GetPlane(Vector3 &n, float &d) const;
			void GetPlaneWs(Vector3 &n, float &d) const;

			const Vector3 &GetPlaneNormal() const;
			float GetPlaneDistance() const;

			void SetPlaneNormal(const Vector3 &n);
			void SetPlaneDistance(float d);

			void Clear();

			Vector3 ProjectToSurface(const Vector3 &pos) const;
			geometry::ModelSubMesh *GetMesh();
			const geometry::ModelSubMesh *GetMesh() const { return const_cast<BaseSurfaceComponent *>(this)->GetMesh(); }

			struct DLLNETWORK MeshInfo {
				geometry::ModelMesh *mesh;
				geometry::ModelSubMesh *subMesh;
				material::Material *material;
			};
			std::optional<MeshInfo> FindAndAssignMesh(const std::function<int32_t(geometry::ModelMesh &, geometry::ModelSubMesh &, material::Material &, const std::string &)> &filter = nullptr);
			bool CalcLineSurfaceIntersection(const Vector3 &lineOrigin, const Vector3 &lineDir, double *outT = nullptr) const;

			bool IsPointBelowSurface(const Vector3 &p) const;
			Quat GetPlaneRotation() const;

			virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		  protected:
			virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
			BaseSurfaceComponent(ecs::BaseEntity &ent);
			math::Plane m_plane = {{0.f, 1.f, 0.f}, 0.f};
			std::weak_ptr<geometry::ModelSubMesh> m_mesh = {};
			NetEventId m_netEvSetPlane = INVALID_NET_EVENT;
			std::string m_kvSurfaceMaterial;
		};

		struct DLLNETWORK CEOnSurfaceMeshChanged : public ComponentEvent {
			CEOnSurfaceMeshChanged(const BaseSurfaceComponent::MeshInfo &meshInfo);
			virtual void PushArguments(lua::State *l) override;
			BaseSurfaceComponent::MeshInfo meshInfo;
		};
	}
};
