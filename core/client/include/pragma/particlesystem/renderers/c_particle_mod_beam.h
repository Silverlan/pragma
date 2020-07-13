/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_PARTICLE_MOD_BEAM_H__
#define __C_PARTICLE_MOD_BEAM_H__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

namespace pragma {class CLightComponent;};
class DLLCLIENT CParticleRendererBeam
	: public CParticleRenderer
{
public:
	CParticleRendererBeam()=default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual void Render(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::ParticleRenderFlags renderFlags) override;
	virtual void RenderShadow(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::CLightComponent &light,uint32_t layerId=0) override;
	virtual void OnParticleSystemStopped() override;
	virtual void PostSimulate(double tDelta) override;
	virtual std::pair<Vector3,Vector3> GetRenderBounds() const override;
	virtual pragma::ShaderParticleBase *GetShader() const override;
private:
#pragma pack(push,1)
	struct DLLCLIENT Node
	{
		Node(const Vector3 &o,const Color &c);
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

#endif