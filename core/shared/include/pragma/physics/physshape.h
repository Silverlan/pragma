#ifndef __PHYSSHAPE_H__
#define __PHYSSHAPE_H__

#include "pragma/networkdefinitions.h"
#include <memory>
#include <sharedutils/def_handle.h>
#include <pragma/physics/physapi.h>
#include <mathutil/glmutil.h>
#include <vector>
#include <BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.h>

class PhysShape;
class PhysConvexHullShape;
class PhysHeightfield;
DECLARE_BASE_HANDLE(DLLNETWORK,PhysShape,PhysShape);
DECLARE_DERIVED_HANDLE(DLLNETWORK,PhysShape,PhysShape,PhysConvexHullShape,PhysConvexHullShape);
DECLARE_DERIVED_HANDLE(DLLNETWORK,PhysShape,PhysShape,PhysHeightfield,PhysHeightfield);

class PhysEnv;
class DLLNETWORK PhysShape
	: public std::enable_shared_from_this<PhysShape>
{
public:
	friend PhysEnv;
protected:
	PhysShape(btCollisionShape *shape,bool bOwns=true);
	btCollisionShape *m_externalShape;
	std::shared_ptr<btCollisionShape> m_shape;
	PhysShapeHandle m_handle;
public:
	virtual ~PhysShape();
	btCollisionShape *GetShape();
	PhysShapeHandle GetHandle() const;
	virtual luabind::object GetLuaObject(lua_State *l);
	virtual void CalculateLocalInertia(float mass,Vector3 *localInertia);
	void Remove();
	void GetAABB(Vector3 &min,Vector3 &max) const;

	virtual bool IsConvex() const;
	virtual bool IsConvexHull() const;
	virtual bool IsCompoundShape() const;
	virtual bool IsHeightfield() const;
	virtual bool IsTriangleShape() const;
};

class CollisionMesh;
class DLLNETWORK PhysConvexShape
	: public PhysShape
{
public:
	friend PhysEnv;
protected:
	PhysConvexShape(btConvexShape *shape);
	btConvexShape *m_convexShape;
	std::shared_ptr<CollisionMesh> m_collisionMesh = nullptr;
public:
	btConvexShape *GetConvexShape();
	void SetLocalScaling(const Vector3 &scale);
	virtual luabind::object GetLuaObject(lua_State *l) override;

	void SetCollisionMesh(CollisionMesh &collisionMesh);
	void SetCollisionMesh();
	const CollisionMesh *GetCollisionMesh() const;
	CollisionMesh *GetCollisionMesh();

	virtual bool IsConvex() const override;
};

class DLLNETWORK PhysConvexHullShape
	: public PhysConvexShape
{
public:
	friend PhysEnv;
protected:
	PhysConvexHullShape(btConvexHullShape *shape);
	btConvexHullShape *m_convexHullShape;
	int m_surfaceMaterial;
public:
	PhysConvexHullShape();
	void AddPoint(const Vector3 &point);
	void SetSurfaceMaterial(int id);
	int GetSurfaceMaterial() const;
	virtual luabind::object GetLuaObject(lua_State *l) override;

	virtual bool IsConvexHull() const override;
};

class DLLNETWORK PhysCompoundShape
	: public PhysShape
{
public:
	friend PhysEnv;
protected:
	PhysCompoundShape(btCompoundShape *sh);
	std::vector<std::shared_ptr<PhysShape>> m_shapes;
public:
	PhysCompoundShape();
	PhysCompoundShape(std::shared_ptr<PhysShape> &shape);
	PhysCompoundShape(std::vector<std::shared_ptr<PhysShape>> &shapes);
	void AddShape(std::shared_ptr<PhysShape> &shape,const Vector3 &origin={},const Quat &rot={});

	virtual bool IsCompoundShape() const override;
};

class DLLNETWORK PhysHeightfield
	: public PhysShape
{
public:
	friend PhysEnv;
protected:
	std::weak_ptr<btHeightfieldTerrainShape> m_heightField;
	std::vector<float> m_heightFieldData {};
	uint32_t m_width = 0;
	uint32_t m_length = 0;
	btScalar m_maxHeight = 0.0;
	uint8_t m_upAxis = 0;
public:
	PhysHeightfield(uint32_t width,uint32_t length,btScalar maxHeight,uint8_t upAxis);
	virtual luabind::object GetLuaObject(lua_State *l) override;
	float GetHeight(uint32_t x,uint32_t y) const;
	void SetHeight(uint32_t x,uint32_t y,float height);
	uint32_t GetWidth() const;
	uint32_t GetLength() const;
	btScalar GetMaxHeight() const;
	uint8_t GetUpAxis() const;
	virtual bool IsHeightfield() const override;
};

class DLLNETWORK PhysTriangleShape
	: public PhysShape
{
public:
	friend PhysEnv;
protected:
	std::vector<btVector3> m_vertices;
	std::vector<int> m_triangles; // Index offsets into m_vertices
	std::vector<int> m_faceMaterials; // Surface material index for each vertex
	Bool m_bBuilt;
	std::unique_ptr<btTriangleIndexVertexArray> m_triangleArray;
	//std::unique_ptr<btTriangleMesh> m_triangleMesh;
	btTriangleInfoMap *m_infoMap;
public:
	PhysTriangleShape();
	virtual ~PhysTriangleShape();
	virtual void CalculateLocalInertia(float mass,Vector3 *localInertia) override;
	void AddTriangle(const Vector3 &a,const Vector3 &b,const Vector3 &c,const SurfaceMaterial *mat=nullptr);
	void Build(const std::vector<SurfaceMaterial> *materials=nullptr);
	void ReserveTriangles(std::size_t count);
	virtual luabind::object GetLuaObject(lua_State *l) override;
	btTriangleIndexVertexArray *GetBtIndexVertexArray();
	void GenerateInternalEdgeInfo();
	std::vector<btVector3> &GetVertices();
	const std::vector<btVector3> &GetVertices() const;
	std::vector<int32_t> &GetTriangles();
	const std::vector<int32_t> &GetTriangles() const;
	std::vector<int32_t> &GetSurfaceMaterials();
	const std::vector<int32_t> &GetSurfaceMaterials() const;

	virtual bool IsTriangleShape() const override;
};

#endif