#include "stdafx_client.h"
#include "pragma/rendering/c_msaa.h"
#include <mathutil/umath.h>

extern DLLCENGINE CEngine *c_engine;

int GetMaxMSAASampleCount()
{
	auto &dev = c_engine->GetDevice();
	Anvil::ImageFormatProperties imgFormatProperties {};
	if(dev.get_physical_device_image_format_properties(
		Anvil::ImageFormatPropertiesQuery{
			Anvil::Format::R16G16B16A16_SFLOAT,Anvil::ImageType::_2D,Anvil::ImageTiling::OPTIMAL,
			Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT,
			{}
		},&imgFormatProperties
		) == false
	)
	{
		Con::cwar<<"WARNING: Unable to retrieve max MSAA sample count! Setting sample count to 1..."<<Con::endl;
		return 1;
	}
	auto sampleMask = static_cast<uint32_t>(imgFormatProperties.sample_counts.get_vk());
	return umath::get_highest_bit(sampleMask);
}
unsigned char ClampMSAASampleCount(unsigned int *samples)
{
	auto maxSamples = CUInt32(GetMaxMSAASampleCount());
	if(*samples > maxSamples)
	{
		*samples = maxSamples;
		return 1;
	}
	else if(*samples == 1 || (*samples &(*samples -1)) != 0)
	{
		if(*samples == 1)
			*samples = 1;
		else
		{
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
