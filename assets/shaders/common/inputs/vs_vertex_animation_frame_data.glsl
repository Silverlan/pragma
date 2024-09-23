#ifndef F_VS_VERTEX_ANIMATION_FRAME_DATA_GLS
#define F_VS_VERTEX_ANIMATION_FRAME_DATA_GLS

#ifndef DESCRIPTOR_SET_INSTANCE
#define DESCRIPTOR_SET_INSTANCE 0
#define DESCRIPTOR_SET_INSTANCE_BINDING_VERTEX_ANIMATIONS 0
#define DESCRIPTOR_SET_INSTANCE_BINDING_VERTEX_ANIMATION_FRAME_DATA (DESCRIPTOR_SET_INSTANCE_BINDING_VERTEX_ANIMATIONS + 1)
#endif

struct VertexAnimationFrameData {
	uint srcFrameOffset;
	uint dstFrameOffset;
	float blend;
	float padding0;
	float padding1;
	float padding2;
	float padding3;
	float padding4;
};

layout(std430, LAYOUT_ID(INSTANCE, VERTEX_ANIMATION_FRAME_DATA)) buffer VAFrameData { VertexAnimationFrameData data[]; }
vaFrameData;

#endif
