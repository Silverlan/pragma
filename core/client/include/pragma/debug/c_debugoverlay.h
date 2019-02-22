#ifndef __C_DEBUGOVERLAY_H__
#define __C_DEBUGOVERLAY_H__

#include "pragma/clientdefinitions.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/c_engine.h"
#include <pragma/math/angle/wvangle.h>
#include <mathutil/color.h>
#include <mathutil/glmutil.h>

namespace prosper
{
	class Buffer;
};

class WIText;
namespace DebugRenderer
{
	enum class DLLCLIENT Type : uint32_t
	{
		Triangles = 0,
		Lines,
		LinesStrip,
		Points,
		PointsVertex,
		Other,

		Count
	};
	enum class DLLCLIENT ObjectType : uint32_t
	{
		World,
		Text,
		Collection
	};
#pragma pack(push,1)
	struct DLLCLIENT InstanceData
	{
		Vector4 color = {};
		bool useVertexColor = false;
	};
#pragma pack(pop)
	class DLLCLIENT BaseObject
	{
	protected:
		BaseObject();
		Vector3 m_position = {};
		Quat m_rotation = uquat::identity();
		Mat4 m_modelMatrix = umat::identity();
		bool m_bValid = false;
		bool m_bVisible = true;
		void UpdateModelMatrix();
	public:
		virtual ~BaseObject()=default;
		const Vector3 &GetPos() const;
		virtual void SetPos(const Vector3 &pos);
		const Quat &GetRotation() const;
		virtual void SetRotation(const Quat &rot);
		EulerAngles GetAngles() const;
		virtual void SetAngles(const EulerAngles &ang);
		const Mat4 &GetModelMatrix() const;
		bool IsVisible() const;
		void SetVisible(bool b);
		void Remove();
		bool IsValid() const;
		virtual ObjectType GetType() const=0;
	};
	class DLLCLIENT CollectionObject
		: public BaseObject
	{
	protected:
		std::vector<std::shared_ptr<BaseObject>> m_objects;
	public:
		CollectionObject(const std::vector<std::shared_ptr<BaseObject>> &objs={});
		void AddObject(const std::shared_ptr<BaseObject> &o);
		virtual void SetPos(const Vector3 &pos) override;
		virtual void SetRotation(const Quat &rot) override;
		virtual void SetAngles(const EulerAngles &ang) override;
		virtual ObjectType GetType() const override;
	};
	class DLLCLIENT WorldObject
		: public BaseObject
	{
	public:
		enum class BufferType : uint32_t
		{
			Geometry = 0u,
			Outline,

			Count
		};

		WorldObject(const Vector4 &color);
		WorldObject();
		virtual ~WorldObject() override;
		void SetOutlineColor(const Vector4 &outlineColor);
		void SetColor(const Vector4 &col);
		const Vector4 &GetColor() const;
		const Vector4 &GetOutlineColor() const;
		const std::shared_ptr<prosper::Buffer> &GetColorBuffer() const;
		const std::shared_ptr<prosper::Buffer> &GetVertexBuffer() const;
		uint32_t GetVertexCount() const;
		void AddVertex(const Vector3 &v);
		std::vector<Vector3> &GetVertices();
		std::vector<Vector4> &GetColors();
		void InitializeBuffers(const std::shared_ptr<prosper::Buffer> &vertexBuffer,uint32_t vertexCount);
		void InitializeBuffers();
		void UpdateVertexBuffer();
		void UpdateColorBuffer();
		bool HasOutline() const;
		virtual ObjectType GetType() const override;
	protected:
		std::shared_ptr<prosper::Buffer> m_vertexBuffer = nullptr;
		std::shared_ptr<prosper::Buffer> m_colorBuffer = nullptr;
		Vector4 m_color = {};
		Vector4 m_outlineColor = {};
		std::vector<Vector3> m_vertices;
		std::vector<Vector4> m_colors;
		bool m_bOutline = false;
		uint32_t m_vertexCount = std::numeric_limits<uint32_t>::max();
	};
	class DLLCLIENT TextObject
		: public BaseObject
	{
	protected:
		mutable WIHandle m_hText;
		mutable std::shared_ptr<prosper::DescriptorSetGroup> m_descSetGroupText = nullptr;
		CallbackHandle m_hCbRender;
	public:
		TextObject(WIText *elText);
		virtual ~TextObject() override;
		Anvil::DescriptorSet *GetTextDescriptorSet() const;
		virtual ObjectType GetType() const override;
		void Initialize(CallbackHandle &hCallback);
		WIText *GetTextElement() const;
	};
	std::shared_ptr<DebugRenderer::BaseObject> DrawPoints(const std::shared_ptr<prosper::Buffer> &vertexBuffer,uint32_t vertexCount,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawPoints(const std::vector<Vector3> &points,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawPoint(const Vector3 &pos,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawLines(const std::vector<Vector3> &lines,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawLine(const Vector3 &start,const Vector3 &end,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawBox(const Vector3 &center,const Vector3 &min,const Vector3 &max,const EulerAngles &ang,const Color &color,const Color &outlineColor,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawBox(const Vector3 &center,const Vector3 &min,const Vector3 &max,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawBox(const Vector3 &start,const Vector3 &end,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawBox(const Vector3 &center,const Vector3 &min,const Vector3 &max,const EulerAngles &ang,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawBox(const Vector3 &start,const Vector3 &end,const EulerAngles &ang,const Color &color,const Color &outlineColor,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawBox(const Vector3 &start,const Vector3 &end,const EulerAngles &ang,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawText(const std::string &text,const Vector3 &pos,const Vector2 &worldSize,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawText(const std::string &text,const Vector3 &pos,float sizeScale,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawText(const std::string &text,const Vector3 &pos,const Vector2 &worldSize,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawText(const std::string &text,const Vector3 &pos,float sizeScale,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawMesh(const std::vector<Vector3> &verts,const Color &color,const Color &outlineColor,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawMesh(const std::vector<Vector3> &verts,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawSphere(const Vector3 &origin,float radius,const Color &color,const Color &outlineColor,float duration=0.f,uint32_t recursionLevel=1);
	std::shared_ptr<DebugRenderer::BaseObject> DrawSphere(const Vector3 &origin,float radius,const Color &color,float duration=0.f,uint32_t recursionLevel=1);
	std::shared_ptr<DebugRenderer::BaseObject> DrawTruncatedCone(const Vector3 &origin,float startRadius,const Vector3 &dir,float dist,float endRadius,const Color &color,const Color &outlineColor,float duration=0.f,uint32_t segmentCount=12);
	std::shared_ptr<DebugRenderer::BaseObject> DrawTruncatedCone(const Vector3 &origin,float startRadius,const Vector3 &dir,float dist,float endRadius,const Color &color,float duration=0.f,uint32_t segmentCount=12);
	std::shared_ptr<DebugRenderer::BaseObject> DrawCylinder(const Vector3 &origin,const Vector3 &dir,float dist,float radius,const Color &color,float duration=0.f,uint32_t segmentCount=12);
	std::shared_ptr<DebugRenderer::BaseObject> DrawCylinder(const Vector3 &origin,const Vector3 &dir,float dist,float radius,const Color &color,const Color &outlineColor,float duration=0.f,uint32_t segmentCount=12);
	std::shared_ptr<DebugRenderer::BaseObject> DrawCone(const Vector3 &origin,const Vector3 &dir,float dist,float angle,const Color &color,const Color &outlineColor,float duration=0.f,uint32_t segmentCount=12);
	std::shared_ptr<DebugRenderer::BaseObject> DrawCone(const Vector3 &origin,const Vector3 &dir,float dist,float angle,const Color &color,float duration=0.f,uint32_t segmentCount=12);
	std::shared_ptr<DebugRenderer::BaseObject> DrawPath(const std::vector<Vector3> &path,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawSpline(const std::vector<Vector3> &path,const Color &color,uint32_t segmentCount,float curvature=1.f,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawPlane(const Vector3 &n,float dist,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawPlane(const Plane &plane,const Color &color,float duration=0.f);
	std::shared_ptr<DebugRenderer::BaseObject> DrawFrustum(const std::vector<Vector3> &points,float duration=0.f);
	std::array<std::shared_ptr<DebugRenderer::BaseObject>,3> DrawAxis(const Vector3 &origin,const Vector3 &x,const Vector3 &y,const Vector3 &z,float duration=0.f);
	std::array<std::shared_ptr<DebugRenderer::BaseObject>,3> DrawAxis(const Vector3 &origin,const EulerAngles &ang,float duration=0.f);
	std::array<std::shared_ptr<DebugRenderer::BaseObject>,3> DrawAxis(const Vector3 &origin,float duration=0.f);
	void ClearObjects();
	void Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,Camera &cam);
};

#endif