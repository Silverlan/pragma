// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.renderer_beam;

export import :particle_system.modifier;

export namespace pragma::ecs {
	class CParticleSystemComponent;
}
export namespace pragma::pts {
	class DLLCLIENT CParticleRendererBeam : public CParticleRenderer {
	  public:
		CParticleRendererBeam() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void RecordRender(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ParticleRenderFlags renderFlags) override;
		virtual void RecordRenderShadow(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, CLightComponent &light, uint32_t layerId = 0) override;
		virtual void OnParticleSystemStopped() override;
		virtual void PostSimulate(double tDelta) override;
		virtual std::pair<Vector3, Vector3> GetRenderBounds() const override;
		virtual ShaderParticleBase *GetShader() const override;
	  private:
#pragma pack(push, 1)
		struct DLLCLIENT Node {
			Node(const Vector3 &o, const Color &c);
			Vector3 origin = {};
			Vector4 color = {};
		};
#pragma pack(pop)
		std::shared_ptr<prosper::IBuffer> m_vertexBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_indexBuffer = nullptr;
		uint32_t m_startNode = 0u;
		uint32_t m_endNode = 0u;
		uint32_t m_nodeCount = 0u;
		std::vector<Node> m_nodeOrigins;
		uint32_t m_indexCount = 0u;
		util::WeakHandle<prosper::Shader> m_shader = {};
		float m_curvature = 1.f;
		void UpdateNodes();
	};
}
