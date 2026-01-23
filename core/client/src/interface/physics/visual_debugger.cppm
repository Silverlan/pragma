// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:physics.visual_debugger;

export import :entities.components.camera;
export import pragma.prosper;

#undef DrawText

export namespace pragma::physics {
	class DLLCLIENT CPhysVisualDebugger : public IVisualDebugger {
	  public:
		static constexpr uint32_t MAX_LINES = 250'000;
		static constexpr uint32_t LINE_SIZE = sizeof(Vector3) * 2;

		static constexpr uint32_t MAX_POINTS = 50'000;
		static constexpr uint32_t POINT_SIZE = sizeof(Vector3);

		static constexpr uint32_t MAX_TRIANGLES = 250'000;
		static constexpr uint32_t TRI_SIZE = sizeof(Vector3) * 3;

		CPhysVisualDebugger();
		void Render(std::shared_ptr<prosper::ICommandBuffer> &drawCmd, CCameraComponent &cam);

		virtual void DrawLine(const Vector3 &from, const Vector3 &to, const Color &fromColor, const Color &toColor) override;
		virtual void DrawPoint(const Vector3 &pos, const Color &color) override;
		virtual void DrawTriangle(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, const Color &c0, const Color &c1, const Color &c2) override;
		virtual void ReportErrorWarning(const std::string &str) override;
		virtual void DrawText(const std::string &str, const Vector3 &location, const Color &color, float size) override;
	  protected:
		void InitializeBuffers();
		virtual void Reset() override;
		virtual void Flush() override;
	  private:
		template<uint32_t TMAX_INSTANCES, uint32_t TVERTS_PER_INSTANCE>
		struct DataBuffer {
			static constexpr auto MAX_INSTANCES = TMAX_INSTANCES;
			static constexpr auto VERTS_PER_INSTANCE = TVERTS_PER_INSTANCE;
			static constexpr auto SIZE_PER_INSTANCE = VERTS_PER_INSTANCE * sizeof(Vector3);
			static constexpr auto BUFFER_SIZE = MAX_INSTANCES * SIZE_PER_INSTANCE;
			static constexpr auto COLOR_BUFFER_SIZE = MAX_INSTANCES * sizeof(Vector4) * VERTS_PER_INSTANCE;
			std::array<std::array<Vector3, VERTS_PER_INSTANCE>, MAX_INSTANCES> vertices = {};
			std::array<std::array<Vector4, VERTS_PER_INSTANCE>, MAX_INSTANCES> vertexColors = {};
			std::shared_ptr<prosper::IBuffer> buffer = nullptr;
			std::shared_ptr<prosper::IBuffer> colorBuffer = nullptr;
			uint32_t instanceCount = 0;

			size_t GetDataSize() const { return instanceCount * SIZE_PER_INSTANCE; }
			size_t GetColorDataSize() const { return instanceCount * sizeof(Vector4) * VERTS_PER_INSTANCE; }
			void AddInstance(const std::array<Vector3, VERTS_PER_INSTANCE> &instanceVerts, const std::array<Vector4, VERTS_PER_INSTANCE> &instanceColors)
			{
				if(instanceCount + instanceVerts.size() > MAX_INSTANCES)
					return;
				vertexColors.at(instanceCount) = instanceColors;
				vertices.at(instanceCount++) = instanceVerts;
			}
			void Reset() { instanceCount = 0; }
		};
		DataBuffer<MAX_LINES, 2> m_lineBuffer = {};
		DataBuffer<MAX_POINTS, 1> m_pointBuffer = {};
		DataBuffer<MAX_TRIANGLES, 3> m_triangleBuffer = {};
		std::shared_ptr<prosper::IBuffer> m_debugBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_colorBuffer = nullptr;
		std::vector<Vector4> m_colorData = {};
	};
};
