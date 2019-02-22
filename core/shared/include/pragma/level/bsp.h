#ifndef __BSP_H__
#define __BSP_H__
#include <string>
#include <vector>

const short LUMP_PLANES = 1;
const short LUMP_TEXDATA = 2;
const short LUMP_TEXINFO = 6;
const short LUMP_FACES = 7;
const short LUMP_BRUSHES = 18;
const short LUMP_BRUSHSIDES = 19;
const short LUMP_TEXDATA_STRING_DATA = 43;
const short LUMP_TEXDATA_STRING_TABLE = 44;


const short LUMP_PLANES_SIZE = 20;
const short LUMP_TEXDATA_SIZE = 32;
const short LUMP_TEXINFO_SIZE = 72;
const short LUMP_FACE_SIZE = 56;
const short LUMP_BRUSHES_SIZE = 12;
const short LUMP_BRUSHSIDES_SIZE = 8;
const short LUMP_TEXDATA_STRING_DATA_SIZE = 128;
const short LUMP_TEXDATA_STRING_TABLE_SIZE = 4;

const int MAX_MAP_BRUSHES = 8192;

struct lump_t
{
	int	fileofs;	// offset into file (bytes)
	int	filelen;	// length of lump (bytes)
	int	version;	// lump format version
	char	fourCC[4];	// lump ident code
};
const unsigned short HEADER_LUMPS = 64;

struct dheader_t
{
	int	ident;                // BSP file identifier
	int	version;              // BSP file version
	lump_t	lumps[HEADER_LUMPS];  // lump directory array
	int	mapRevision;          // the map's revision (iteration, version) number
};


struct dbrush_t
{
	int	firstside;	// first brushside
	int	numsides;	// number of brushsides
	int	contents;	// contents flags
};

struct dbrushside_t
{
	unsigned short	planenum;	// facing out of the leaf
	short		texinfo;	// texture info
	short		dispinfo;	// displacement info
	short		bevel;		// is the side a bevel plane?
};

struct texinfo_t
{
	float	textureVecs[2][4];	// [s/t][xyz offset]
	float	lightmapVecs[2][4];	// [s/t][xyz offset] - length is in units of texels/area
	int	flags;			// miptex flags	overrides
	int	texdata;		// Pointer to texture name, size, etc.
};

struct Vector
{
	float x;
	float y;
	float z;
};

struct dtexdata_t
{
	Vector	reflectivity;		// RGB reflectivity
	int	nameStringTableID;	// index into TexdataStringTable
	int	width, height;		// source image
	int	view_width, view_height;
};

struct dplane_t
{
	Vector	normal;	// normal vector
	float	dist;	// distance from origin
	int	type;	// plane axis identifier
};

struct dface_t
{
	unsigned short	planenum;		// the plane number
	char		side;			// faces opposite to the node's plane direction
	char		onNode;			// 1 of on node, 0 if in leaf
	int		firstedge;		// index into surfedges
	short		numedges;		// number of surfedges
	short		texinfo;		// texture info
	short		dispinfo;		// displacement info
	short		surfaceFogVolumeID;	// ?
	char		styles[4];		// switchable lighting info
	int		lightofs;		// offset into lightmap lump
	float		area;			// face area in units^2
	int		LightmapTextureMinsInLuxels[2];	// texture lighting info
	int		LightmapTextureSizeInLuxels[2];	// texture lighting info
	int		origFace;		// original face this was split from
	unsigned short	numPrims;		// primitives
	unsigned short	firstPrimID;
	unsigned int	smoothingGroups;	// lightmap smoothing group
};

#include <vector>
class BSP
{
public:
	BSP(const char *map);
private:
	dheader_t m_header;
	std::vector<dbrush_t> m_brushes;
	std::vector<dbrushside_t> m_brushSides;
	std::vector<texinfo_t> m_texInfo;
	std::vector<dtexdata_t> m_texData;
	std::vector<dplane_t> m_planes;
	std::vector<int> m_texDataStringTable;
	std::vector<std::string> m_texDataStringData;
	std::vector<dface_t> m_faces;
	lump_t *GetLump(const short lump);
public:
	void GetBrushes(std::vector<dbrush_t> **brushes);
	void GetBrushSides(std::vector<dbrushside_t> **brushSides);
	void GetPlanes(std::vector<dplane_t> **planes);
	void GetTexInfo(std::vector<texinfo_t> **texinfo);
	void GetTexData(std::vector<dtexdata_t> **texdata);
	void GetFaces(std::vector<dface_t> **faces);
	void GetTexDataStringData(std::vector<std::string> **texdatastringdata);
};
#endif // __BSP_H__