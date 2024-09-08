#ifndef F_SH_LIGHT_SOURCE_GLS
#define F_SH_LIGHT_SOURCE_GLS

#define FLIGHT_TURNED_ON 1
#define FLIGHT_TYPE_SPOT (FLIGHT_TURNED_ON << 1)
#define FLIGHT_TYPE_POINT (FLIGHT_TYPE_SPOT << 1)
#define FLIGHT_TYPE_DIRECTIONAL (FLIGHT_TYPE_POINT << 1)
#define FLIGHT_BAKED_LIGHT_SOURCE (FLIGHT_TYPE_DIRECTIONAL << 1)

struct LightSourceData {
	vec4 position;  // position.w = distance
	vec4 color;     // color.a = light intensity in candela
	vec4 direction; // direction.w = start offset for spotlights

	uint sceneFlags;

	// If light type is directional, this is a boolean specifying whether csm shadows are enabled,
	// otherwise this is an index into the shadowmap or shadowcubemap array.
	uint shadowIndex;

	// Spotlights
	float cutoffOuter;
	float cutoffInner;
	float attenuation;

	uint flags;
	uint shadowMapIndexStatic;
	uint shadowMapIndexDynamic;

	float falloffExponent;

	// Alignment to vec4!
	float padding0;
	float padding1;
	float padding2;
};

#endif
