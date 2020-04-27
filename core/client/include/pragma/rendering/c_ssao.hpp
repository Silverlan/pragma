/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SSAO_HPP__
#define __C_SSAO_HPP__

#include "pragma/clientdefinitions.h"
#include <memory>
#include <image/prosper_texture.hpp>

namespace prosper {class Shader; class Context; class RenderTarget; class DescriptorSetGroup;};

#pragma warning(push)
#pragma warning(disable : 4251)
struct DLLCLIENT SSAOInfo
{
	bool Initialize(
		prosper::Context &context,uint32_t width,uint32_t height,prosper::SampleCountFlags samples,
		const std::shared_ptr<prosper::Texture> &texNorm,const std::shared_ptr<prosper::Texture> &texDepth
	);
	void Clear();
	std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
	std::shared_ptr<prosper::RenderTarget> renderTargetBlur = nullptr;
	std::shared_ptr<prosper::IDescriptorSetGroup> descSetGroupPrepass = nullptr;
	std::shared_ptr<prosper::IDescriptorSetGroup> descSetGroupOcclusion = nullptr;

	::util::WeakHandle<prosper::Shader> shader = {};
	::util::WeakHandle<prosper::Shader> shaderBlur = {};
	prosper::Shader *GetSSAOShader() const;
	prosper::Shader *GetSSAOBlurShader() const;
};
#pragma warning(pop)

#endif
