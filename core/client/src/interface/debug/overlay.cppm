// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:debug.overlay;
import :client_state;
import :engine;

#undef DrawText

export namespace pragma::debug {
	namespace DebugRenderer {
		enum class DLLCLIENT Type : uint32_t {
			Triangles = 0,
			Lines,
			LinesStrip,
			Points,
			PointsVertex,
			Other,

			Count
		};
		enum class DLLCLIENT ObjectType : uint32_t { World, Text, Collection };
#pragma pack(push, 1)
		struct DLLCLIENT InstanceData {
			Vector4 color = {};
			bool useVertexColor = false;
		};
#pragma pack(pop)
		class DLLCLIENT BaseObject {
		  public:
			enum class Flags : uint32_t {
				None = 0,
				Valid = 1,
				Visible = Valid << 1,
				IgnoreDepth = Visible << 1,
			};
			virtual ~BaseObject() = default;
			const math::ScaledTransform &GetPose() const;
			math::ScaledTransform &GetPose();
			void SetPose(const math::ScaledTransform &pose);
			const Vector3 &GetPos() const;
			virtual void SetPos(const Vector3 &pos);
			const Quat &GetRotation() const;
			virtual void SetRotation(const Quat &rot);
			EulerAngles GetAngles() const;
			virtual void SetAngles(const EulerAngles &ang);
			const Vector3 &GetScale() const;
			virtual void SetScale(const Vector3 &scale);
			const Mat4 &GetModelMatrix() const;
			bool IsVisible() const;
			void SetVisible(bool b);
			bool ShouldIgnoreDepth() const;
			void SetIgnoreDepth(bool b);
			virtual void Remove();
			bool IsValid() const;
			virtual ObjectType GetType() const = 0;
		  protected:
			BaseObject();
			math::ScaledTransform m_pose {};
			Mat4 m_modelMatrix = umat::identity();
			Flags m_flags = Flags::Visible;
			void UpdateModelMatrix();
		};
		class DLLCLIENT CollectionObject : public BaseObject {
		  protected:
			std::vector<std::shared_ptr<BaseObject>> m_objects;
		  public:
			CollectionObject(const std::vector<std::shared_ptr<BaseObject>> &objs = {});
			void AddObject(const std::shared_ptr<BaseObject> &o);
			virtual void SetPos(const Vector3 &pos) override;
			virtual void SetRotation(const Quat &rot) override;
			virtual void SetAngles(const EulerAngles &ang) override;
			virtual void SetScale(const Vector3 &scale) override;
			virtual ObjectType GetType() const override;
			virtual void Remove() override;
		};
		class DLLCLIENT WorldObject : public BaseObject {
		  public:
			enum class BufferType : uint32_t {
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
			const std::shared_ptr<prosper::IBuffer> &GetColorBuffer() const;
			const std::shared_ptr<prosper::IBuffer> &GetVertexBuffer() const;
			uint32_t GetVertexCount() const;
			void SetVertexPosition(uint32_t idx, const Vector3 &pos);
			std::optional<Vector3> GetVertexPosition(uint32_t idx) const;
			void AddVertex(const Vector3 &v);
			std::vector<Vector3> &GetVertices();
			std::vector<Vector4> &GetColors();
			bool InitializeBuffers(const std::shared_ptr<prosper::IBuffer> &vertexBuffer, uint32_t vertexCount);
			bool InitializeBuffers();
			void UpdateVertexBuffer();
			void UpdateColorBuffer();
			bool HasOutline() const;
			virtual ObjectType GetType() const override;
		  protected:
			std::shared_ptr<prosper::IBuffer> m_vertexBuffer = nullptr;
			std::shared_ptr<prosper::IBuffer> m_colorBuffer = nullptr;
			Vector4 m_color = {};
			Vector4 m_outlineColor = {};
			std::vector<Vector3> m_vertices;
			std::vector<Vector4> m_colors;
			bool m_bOutline = false;
			uint32_t m_vertexCount = std::numeric_limits<uint32_t>::max();
		};
		class DLLCLIENT TextObject : public BaseObject {
		  protected:
			mutable gui::WIHandle m_hText;
			mutable std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupText = nullptr;
			CallbackHandle m_hCbRender;
		  public:
			TextObject(gui::types::WIText *elText);
			virtual ~TextObject() override;
			prosper::IDescriptorSet *GetTextDescriptorSet() const;
			virtual ObjectType GetType() const override;
			void Initialize(CallbackHandle &hCallback);
			gui::types::WIText *GetTextElement() const;
		};
		std::shared_ptr<BaseObject> DrawPoints(const std::shared_ptr<prosper::IBuffer> &vertexBuffer, uint32_t vertexCount, const DebugRenderInfo &renderInfo = {});
		std::shared_ptr<BaseObject> DrawPoints(const std::vector<Vector3> &points, const DebugRenderInfo &renderInfo = {});
		std::shared_ptr<BaseObject> DrawPoint(const DebugRenderInfo &renderInfo);
		std::shared_ptr<BaseObject> DrawLines(const std::vector<Vector3> &lines, const DebugRenderInfo &renderInfo = {});
		std::shared_ptr<BaseObject> DrawLine(const Vector3 &start, const Vector3 &end, const DebugRenderInfo &renderInfo = {});
		std::shared_ptr<BaseObject> DrawBox(const Vector3 &start, const Vector3 &end, const DebugRenderInfo &renderInfo = {});
		std::shared_ptr<BaseObject> DrawText(const DebugRenderInfo &renderInfo, const std::string &text, const Vector2 &worldSize);
		std::shared_ptr<BaseObject> DrawText(const DebugRenderInfo &renderInfo, const std::string &text, float sizeScale);
		std::shared_ptr<BaseObject> DrawText(const DebugRenderInfo &renderInfo, const std::string &text, const Vector2 &worldSize);
		std::shared_ptr<BaseObject> DrawMesh(const std::vector<Vector3> &verts, const DebugRenderInfo &renderInfo = {});
		std::shared_ptr<BaseObject> DrawSphere(const DebugRenderInfo &renderInfo, float radius, uint32_t recursionLevel = 1);
		std::shared_ptr<BaseObject> DrawTruncatedCone(const DebugRenderInfo &renderInfo, float startRadius, const Vector3 &dir, float dist, float endRadius, uint32_t segmentCount = 12);
		std::shared_ptr<BaseObject> DrawCylinder(const DebugRenderInfo &renderInfo, const Vector3 &dir, float dist, float radius, uint32_t segmentCount = 12);
		std::shared_ptr<BaseObject> DrawCylinder(const DebugRenderInfo &renderInfo, float dist, float radius, uint32_t segmentCount = 12);
		std::shared_ptr<BaseObject> DrawCone(const DebugRenderInfo &renderInfo, const Vector3 &dir, float dist, float angle, uint32_t segmentCount = 12);
		std::shared_ptr<BaseObject> DrawPath(const std::vector<Vector3> &path, const DebugRenderInfo &renderInfo = {});
		std::shared_ptr<BaseObject> DrawSpline(const std::vector<Vector3> &path, uint32_t segmentCount, float curvature = 1.f, const DebugRenderInfo &renderInfo = {});
		std::shared_ptr<BaseObject> DrawPlane(const Vector3 &n, float dist, const DebugRenderInfo &renderInfo = {});
		std::shared_ptr<BaseObject> DrawPlane(const math::Plane &plane, const DebugRenderInfo &renderInfo = {});
		std::shared_ptr<BaseObject> DrawFrustum(const std::vector<Vector3> &points, const DebugRenderInfo &renderInfo = {});
		std::array<std::shared_ptr<BaseObject>, 3> DrawAxis(const DebugRenderInfo &renderInfo, const Vector3 &x, const Vector3 &y, const Vector3 &z);
		std::array<std::shared_ptr<BaseObject>, 3> DrawAxis(const DebugRenderInfo &renderInfo);
		void ClearObjects();
		void Render(std::shared_ptr<prosper::ICommandBuffer> &drawCmd, CCameraComponent &cam);

		using namespace pragma::math::scoped_enum::bitwise;
	};
}
export {
	REGISTER_ENUM_FLAGS(pragma::debug::DebugRenderer::BaseObject::Flags)
}
