// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.renderer_rotational_buffer;

export import pragma.prosper;
export import pragma.shared;

export namespace pragma::ecs {
	class CParticleSystemComponent;
}
export namespace pragma::pts {
	class DLLCLIENT CParticleRendererRotationalBuffer {
	  public:
		CParticleRendererRotationalBuffer() = default;
		void Initialize(BaseEnvParticleSystemComponent &pSystem);
		virtual bool Update();
		const std::shared_ptr<prosper::IBuffer> &GetBuffer() const;
		void SetRotationAlignVelocity(bool b);
		bool ShouldRotationAlignVelocity() const;
	  protected:
		uint64_t m_lastFrameUpdate = std::numeric_limits<uint64_t>::max();
		std::shared_ptr<prosper::IBuffer> m_rotBuffer = nullptr;
		std::vector<Quat> m_rotations;
		ComponentHandle<ecs::CParticleSystemComponent> m_hParticleSystem = {};
		bool m_bAlignVelocity = false;
	};
}
