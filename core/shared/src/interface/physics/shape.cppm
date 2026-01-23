// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.shape;

export import :physics.base;
export import :physics.material;

export {
	namespace pragma::physics {
		class CollisionMesh;
		class IEnvironment;
		class IConvexShape;
		class IConvexHullShape;
		class ICompoundShape;
		class IHeightfield;
		class ITriangleShape;
		class DLLNETWORK IShape : public IBase {
		  public:
			virtual ~IShape();
			virtual void CalculateLocalInertia(float mass, Vector3 *localInertia) const = 0;
			virtual void GetAABB(Vector3 &min, Vector3 &max) const = 0;

			virtual bool IsValid() const;

			virtual bool IsConvex() const;
			virtual bool IsConvexHull() const;
			virtual bool IsCompoundShape() const;
			virtual bool IsHeightfield() const;
			virtual bool IsTriangleShape() const;

			virtual IConvexShape *GetConvexShape();
			const IConvexShape *GetConvexShape() const;
			virtual IConvexHullShape *GetConvexHullShape();
			const IConvexHullShape *GetConvexHullShape() const;
			virtual ICompoundShape *GetCompoundShape();
			const ICompoundShape *GetCompoundShape() const;
			virtual IHeightfield *GetHeightfield();
			const IHeightfield *GetHeightfield() const;
			virtual ITriangleShape *GetTriangleShape();
			const ITriangleShape *GetTriangleShape() const;

			virtual void GetBoundingSphere(Vector3 &outCenter, float &outRadius) const = 0;

			virtual void ApplySurfaceMaterial(IMaterial &mat) = 0;
			void SetSurfaceMaterial(int32_t surfMatIdx); // TODO: Apply density from surface material
			int32_t GetSurfaceMaterialIndex() const;
			SurfaceMaterial *GetSurfaceMaterial() const;
			void SetMaterial(const IMaterial &mat);
			IMaterial *GetMaterial() const;

			virtual void SetLocalPose(const math::Transform &localPose) = 0;
			virtual math::Transform GetLocalPose() const = 0;

			virtual void SetDensity(float density);
			float GetDensity() const;
			virtual void SetMass(float mass) = 0;
			virtual float GetMass() const = 0;
			virtual void InitializeLuaObject(lua::State *lua) override;

			void *userData = nullptr;
		  protected:
			IShape(IEnvironment &env);
			float m_density = 1.f;
			util::WeakHandle<IMaterial> m_material = {};
		};

		class DLLNETWORK IConvexShape : virtual public IShape {
		  public:
			virtual ~IConvexShape() override;

			virtual void SetLocalScaling(const Vector3 &scale) = 0;
			virtual void InitializeLuaObject(lua::State *lua) override;

			void SetCollisionMesh(CollisionMesh &collisionMesh);
			void SetCollisionMesh();
			const CollisionMesh *GetCollisionMesh() const;
			CollisionMesh *GetCollisionMesh();

			virtual bool IsConvex() const override;
			virtual IConvexShape *GetConvexShape() override;
		  protected:
			IConvexShape(IEnvironment &env);
		  private:
			std::shared_ptr<CollisionMesh> m_collisionMesh = nullptr;
		};

		class DLLNETWORK ICapsuleShape : virtual public IConvexShape {
		  public:
			virtual ~ICapsuleShape() override;
			virtual void InitializeLuaObject(lua::State *lua) override;

			virtual float GetRadius() const = 0;
			virtual float GetHalfHeight() const = 0;
		  protected:
			ICapsuleShape(IEnvironment &env);
		};

		class DLLNETWORK IBoxShape : virtual public IConvexShape {
		  public:
			virtual ~IBoxShape() override;
			virtual void InitializeLuaObject(lua::State *lua) override;

			virtual Vector3 GetHalfExtents() const = 0;
		  protected:
			IBoxShape(IEnvironment &env);
		};

		class DLLNETWORK IConvexHullShape : virtual public IConvexShape {
		  public:
			virtual void InitializeLuaObject(lua::State *lua) override;
			virtual bool IsConvexHull() const override;
			virtual IConvexHullShape *GetConvexHullShape() override;
			virtual void AddPoint(const Vector3 &point) = 0;

			virtual void AddTriangle(uint32_t idx0, uint32_t idx1, uint32_t idx2) = 0;
			virtual void ReservePoints(uint32_t numPoints) = 0;
			virtual void ReserveTriangles(uint32_t numTris) = 0;
			void Build();
		  protected:
			IConvexHullShape(IEnvironment &env);
			virtual void DoBuild() = 0;
			bool m_bBuilt = false;
		};

		class DLLNETWORK ICompoundShape : virtual public IShape {
		  public:
			struct DLLNETWORK ShapeInfo {
				std::shared_ptr<IShape> shape;
				math::Transform localPose;
			};
			virtual void InitializeLuaObject(lua::State *lua) override;
			void AddShape(IShape &shape, const math::Transform &localPose = {});
			virtual void GetAABB(Vector3 &min, Vector3 &max) const override;

			virtual void SetMass(float mass) override;
			virtual float GetMass() const override;

			virtual bool IsCompoundShape() const override;
			virtual ICompoundShape *GetCompoundShape() override;
			const std::vector<ShapeInfo> &GetShapes() const;
		  protected:
			ICompoundShape(IEnvironment &env);
			ICompoundShape(IEnvironment &env, IShape &shape, const Vector3 &origin);
			std::vector<ShapeInfo> m_shapes;
			Vector3 m_min = {};
			Vector3 m_max = {};
		};

		class DLLNETWORK IHeightfield : virtual public IShape {
		  public:
			virtual void InitializeLuaObject(lua::State *lua) override;
			virtual bool IsHeightfield() const override;
			virtual IHeightfield *GetHeightfield() override;
			virtual float GetHeight(uint32_t x, uint32_t y) const = 0;
			virtual void SetHeight(uint32_t x, uint32_t y, float height) = 0;
			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetLength() const = 0;
			virtual float GetMaxHeight() const = 0;
			virtual uint8_t GetUpAxis() const = 0;
		  protected:
			IHeightfield(IEnvironment &env, uint32_t width, uint32_t length, float maxHeight, uint8_t upAxis);

			std::vector<float> m_heightFieldData {};
			uint32_t m_width = 0;
			uint32_t m_length = 0;
			float m_maxHeight = 0.0;
			uint8_t m_upAxis = 0;
		};

		class DLLNETWORK ITriangleShape : virtual public IShape {
		  public:
			virtual void InitializeLuaObject(lua::State *lua) override;
			virtual bool IsTriangleShape() const override;
			virtual ITriangleShape *GetTriangleShape() override;

			virtual void CalculateLocalInertia(float mass, Vector3 *localInertia) const override;
			virtual void AddTriangle(const Vector3 &a, const Vector3 &b, const Vector3 &c, const SurfaceMaterial *mat = nullptr);
			void Build(const std::vector<SurfaceMaterial> *materials = nullptr);
			void ReserveTriangles(std::size_t count);

			size_t GetVertexCount() const;
			Vector3 *GetVertex(size_t idx);
			const Vector3 *GetVertex(size_t idx) const;
			std::vector<Vector3> &GetVertices();
			const std::vector<Vector3> &GetVertices() const;

			std::vector<uint32_t> &GetTriangles();
			const std::vector<uint32_t> &GetTriangles() const;
			std::vector<int32_t> &GetSurfaceMaterials();
			const std::vector<int32_t> &GetSurfaceMaterials() const;
		  protected:
			ITriangleShape(IEnvironment &env);
			virtual void DoBuild(const std::vector<SurfaceMaterial> *materials = nullptr) = 0;

			std::vector<Vector3> m_vertices;
			std::vector<uint32_t> m_triangles; // Index offsets into m_vertices
			std::vector<int> m_faceMaterials;  // Surface material index for each vertex
			bool m_bBuilt = false;
		};
	};
}
