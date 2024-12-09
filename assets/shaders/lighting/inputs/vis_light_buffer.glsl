#ifndef F_SH_VIS_LIGHT_BUFFER_GLS
#define F_SH_VIS_LIGHT_BUFFER_GLS

struct VisibleIndex {
	int index;
};
layout(std430, LAYOUT_ID(RENDERER, VISIBLE_LIGHT_TILE_INDEX_BUFFER)) buffer VisibleLightTileIndicesBuffer { VisibleIndex data[]; }
visibleLightTileIndicesBuffer;

#endif
