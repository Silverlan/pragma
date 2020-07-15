/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_UNLIT_HPP__
#define __C_SHADER_UNLIT_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include <mathutil/uvec.h>

namespace pragma
{
	class DLLCLIENT ShaderUnlit
		: public ShaderTextured3DBase
	{
	public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;

		enum class MaterialBinding : uint32_t
		{
			MaterialSettings = umath::to_integral(ShaderTextured3DBase::MaterialBinding::MaterialSettings),
			AlbedoMap,

			Count
		};

		ShaderUnlit(prosper::IPrContext &context,const std::string &identifier);

		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual bool BindLights(prosper::IDescriptorSet &dsLights) override;
	protected:
		virtual prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const override;
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::DescriptorSetInfo &descSetInfo);
	};
};

#endif
