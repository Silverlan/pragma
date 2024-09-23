#ifndef F_FS_PBR_LIGHTING_ATTENUATION_GLS
#define F_FS_PBR_LIGHTING_ATTENUATION_GLS

// https://github.com/KhronosGroup/glTF/blob/master/extensions/2.0/Khronos/KHR_lights_punctual/README.md#range-property
float get_range_attenuation(float range, float distance)
{
	if(range <= 0.0) {
		// negative range means unlimited
		return 1.0;
	}
	return max(min(1.0 - pow(distance / range, 4.0), 1.0), 0.0) / pow(distance, 2.0);
}

#endif
