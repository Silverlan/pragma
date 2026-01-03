// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.enums;
bool pragma::rendering::premultiply_alpha(Vector4 &color, ParticleAlphaMode alphaMode)
{
	switch(alphaMode) {
	case ParticleAlphaMode::Opaque:
		color = {color.r * color.a, color.g * color.a, color.b * color.a, 1.f};
		break;
	case ParticleAlphaMode::Translucent:
		color = {color.r * color.a, color.g * color.a, color.b * color.a, color.a};
		break;
	case ParticleAlphaMode::Additive:
	case ParticleAlphaMode::AdditiveByColor:
		color = {color.r * color.a, color.g * color.a, color.b * color.a, 0.f};
		break;
	case ParticleAlphaMode::Premultiplied:
		// No change
		break;
	default:
		return false;
	}
	return true;
}

int pragma::rendering::GetMaxMSAASampleCount()
{
	auto props = get_cengine()->GetRenderContext().GetPhysicalDeviceImageFormatProperties(
	  {prosper::ImageCreateFlags {}, prosper::Format::R16G16B16A16_SFloat, prosper::ImageType::e2D, prosper::ImageTiling::Optimal, prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit});
	if(props.has_value() == false) {
		Con::CWAR << "Unable to retrieve max MSAA sample count! Setting sample count to 1..." << Con::endl;
		return 1;
	}
	return math::get_highest_bit(math::to_integral(props->sampleCount));
}
unsigned char pragma::rendering::ClampMSAASampleCount(unsigned int *samples)
{
	auto maxSamples = CUInt32(GetMaxMSAASampleCount());
	if(*samples > maxSamples) {
		*samples = maxSamples;
		return 1;
	}
	else if(*samples == 1 || (*samples & (*samples - 1)) != 0) {
		if(*samples == 1)
			*samples = 1;
		else {
			unsigned int newSamples = 2;
			while(newSamples <= *samples)
				newSamples *= 2;
			newSamples /= 2;
			*samples = newSamples;
		}
		return 2;
	}
	if(*samples == 0)
		*samples = 1;
	return 0;
}
