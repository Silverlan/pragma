#ifndef F_FS_RENDERER_GLS
#define F_FS_RENDERER_GLS

#define FRENDER_SSAO_ENABLED 1

layout(std140, LAYOUT_ID(RENDERER, RENDERER)) uniform Renderer
{
	uint flags;
	uint vpResolution;
	uint tileInfo; // First 16 bits = number of tiles (x-axis), second 16 bits = tile size
	float lightmapExposurePow;
	float bloomThreshold;
}
u_renderer;

bool is_ssao_enabled() { return (u_renderer.flags & FRENDER_SSAO_ENABLED) != 0; }
uint get_number_of_tiles_x() { return (u_renderer.tileInfo << 16) >> 16; }
uint get_tile_size() { return u_renderer.tileInfo >> 16; }

uint get_viewport_width() { return u_renderer.vpResolution >> 16; }
uint get_viewport_height() { return (u_renderer.vpResolution << 16) >> 16; }
float get_lightmap_exposure_pow() { return u_renderer.lightmapExposurePow; }

#endif
