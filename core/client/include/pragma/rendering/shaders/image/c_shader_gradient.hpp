/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_GRADIENT_HPP__
#define __C_SHADER_GRADIENT_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>
#include <array>

namespace pragma
{
	class DLLCLIENT ShaderGradient
		: public prosper::ShaderBaseImageProcessing
	{
	public:
		static const uint32_t MAX_GRADIENT_NODES = 3u;
#pragma pack(push,1)
		struct Node
		{
			Node(const Vector4 &color={},float offset=0.f)
				: color(color),offset(offset)
			{}
			Vector4 color = {};
			float offset = 0.f;
			std::array<uint8_t,12> padding;
		};
		struct PushConstants
		{
			Vector2i textureSize;
			Vector2 boxIntersection;
			int32_t nodeCount;
			std::array<uint8_t,12> padding;
			std::array<Node,MAX_GRADIENT_NODES> nodes;
		};
#pragma pack(pop)

		ShaderGradient(prosper::Context &context,const std::string &identifier);
		virtual ~ShaderGradient() override;
		bool Draw(const PushConstants &pushConstants);
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};

	namespace util
	{
		DLLCLIENT bool record_draw_gradient(prosper::Context &context,const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,prosper::RenderTarget &rt,const Vector2 &dir,const std::vector<ShaderGradient::Node> &nodes);
	};
};

#endif
