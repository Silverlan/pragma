/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

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
			static_cast<Anvil::Format>(prosper::Format::R16G16B16A16_SFloat),static_cast<Anvil::ImageType>(prosper::ImageType::e2D),static_cast<Anvil::ImageTiling>(prosper::ImageTiling::Optimal),
			static_cast<Anvil::ImageUsageFlagBits>(prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit),
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
