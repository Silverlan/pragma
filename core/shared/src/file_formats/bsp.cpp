#include "stdafx_shared.h"
#include "pragma/level/bsp.h"
#include <pragma/console/conout.h>
#include <iostream>
#include <cstring>
#include <fsys/filesystem.h>
#include <sharedutils/util.h>
#include <mathutil/umath.h>

BSP::BSP(const char *map)
{
	std::string path = "maps\\";
	path += map;
	const char *cPath = path.c_str();
	Con::cout<<"Loading map '"<<map<<"'..."<<Con::endl;
	auto f = FileManager::OpenFile(cPath,"rb");
	if(f == NULL)
	{
		Con::cout<<"WARNING: Unable to open map '"<<map<<"'!"<<Con::endl;
		return;
	}
	char cHeader[sizeof(dheader_t)];
	f->Read(cHeader,sizeof(dheader_t));
	m_header = *(dheader_t*)&(cHeader[0]);

	char ident[4];
	ident[0] = CChar(m_header.ident);
	ident[1] = CChar(m_header.ident>>8);
	ident[2] = CChar(m_header.ident>>16);
	ident[3] = CChar(m_header.ident>>24);
	if(strncmp(ident,"VBSP",4) != 0)
	{
		Con::cout<<"WARNING: '"<<map<<"' is not a valid BSP file!"<<Con::endl;
		return;
	}

	lump_t lBrushes = m_header.lumps[LUMP_BRUSHES];
	f->Seek(lBrushes.fileofs);
	unsigned int numBrushes = lBrushes.filelen /LUMP_BRUSHES_SIZE;
	for(unsigned int i=0;i<numBrushes;i++)
	{
		char cBrush[LUMP_BRUSHES_SIZE];
		f->Read(cBrush,LUMP_BRUSHES_SIZE);
		m_brushes.push_back(*(dbrush_t*)&(cBrush[0]));
	}
	
	lump_t lBrushSides = m_header.lumps[LUMP_BRUSHSIDES];
	f->Seek(lBrushSides.fileofs);
	unsigned int numBrushSides = lBrushSides.filelen /LUMP_BRUSHSIDES_SIZE;
	for(unsigned int i=0;i<numBrushSides;i++)
	{
		char cBrushSide[LUMP_BRUSHSIDES_SIZE];
		f->Read(cBrushSide,LUMP_BRUSHSIDES_SIZE);
		m_brushSides.push_back(*(dbrushside_t*)&(cBrushSide[0]));
	}

	lump_t lPlanes = m_header.lumps[LUMP_PLANES];
	f->Seek(lPlanes.fileofs);
	unsigned int numPlanes = lPlanes.filelen /LUMP_PLANES_SIZE;
	for(unsigned int i=0;i<numPlanes;i++)
	{
		char cPlane[LUMP_PLANES_SIZE];
		f->Read(cPlane,LUMP_PLANES_SIZE);
		m_planes.push_back(*(dplane_t*)&(cPlane[0]));
	}

	lump_t lTexinfo = m_header.lumps[LUMP_TEXINFO];
	f->Seek(lTexinfo.fileofs);
	unsigned int numTexInfo = lTexinfo.filelen /LUMP_TEXINFO_SIZE;
	for(unsigned int i=0;i<numTexInfo;i++)
	{
		char cTexInfo[LUMP_TEXINFO_SIZE];
		f->Read(cTexInfo,LUMP_TEXINFO_SIZE);
		m_texInfo.push_back(*(texinfo_t*)&(cTexInfo[0]));
	}

	lump_t lTexdata = m_header.lumps[LUMP_TEXDATA];
	f->Seek(lTexdata.fileofs);
	unsigned int numTexData = lTexdata.filelen /LUMP_TEXDATA_SIZE;
	for(unsigned int i=0;i<numTexData;i++)
	{
		char cTexData[LUMP_TEXDATA_SIZE];
		f->Read(cTexData,LUMP_TEXDATA_SIZE);
		m_texData.push_back(*(dtexdata_t*)&(cTexData[0]));
	}

	lump_t lTexdataStringTable = m_header.lumps[LUMP_TEXDATA_STRING_TABLE];
	f->Seek(lTexdataStringTable.fileofs);
	unsigned int numTexDataStringTable = lTexdataStringTable.filelen /LUMP_TEXDATA_STRING_TABLE_SIZE;
	for(unsigned int i=0;i<numTexDataStringTable;i++)
	{
		char cTexDataStringTable[LUMP_TEXDATA_STRING_TABLE_SIZE];
		f->Read(cTexDataStringTable,LUMP_TEXDATA_STRING_TABLE_SIZE);
		m_texDataStringTable.push_back(*(int*)&(cTexDataStringTable[0]));
	}

	lump_t lTexdataStringData = m_header.lumps[LUMP_TEXDATA_STRING_DATA];
	f->Seek(lTexdataStringData.fileofs);
	int sz = lTexdataStringData.filelen;
	while(sz > 0)
	{
		std::string s("");
		for(int i=0;i<LUMP_TEXDATA_STRING_DATA_SIZE;i++)
		{
			char c = CChar(f->ReadChar());
			sz--;
			if(c == '\0')
			{
				m_texDataStringData.push_back(s);
				break;
			}
			else s += c;
		}
	}

	lump_t lFaces = m_header.lumps[LUMP_FACES];
	f->Seek(lFaces.fileofs);
	unsigned int numFaces = lFaces.filelen /LUMP_FACE_SIZE;
	for(unsigned int i=0;i<numFaces;i++)
	{
		char cSide[LUMP_FACE_SIZE];
		f->Read(cSide,LUMP_FACE_SIZE);
		m_faces.push_back(*(dface_t*)&(cSide[0]));
	}
	return;
}

lump_t *BSP::GetLump(const short lump)
{
	return &m_header.lumps[lump];
}

void BSP::GetBrushes(std::vector<dbrush_t> **brushes)
{
	*brushes = &m_brushes;
}

void BSP::GetBrushSides(std::vector<dbrushside_t> **brushSides)
{
	*brushSides = &m_brushSides;
}

void BSP::GetPlanes(std::vector<dplane_t> **planes)
{
	*planes = &m_planes;
}

void BSP::GetTexInfo(std::vector<texinfo_t> **texinfo)
{
	*texinfo = &m_texInfo;
}

void BSP::GetTexData(std::vector<dtexdata_t> **texdata)
{
	*texdata = &m_texData;	
}

void BSP::GetTexDataStringData(std::vector<std::string> **texdatastringdata)
{
	*texdatastringdata = &m_texDataStringData;
}

void BSP::GetFaces(std::vector<dface_t> **faces)
{
	*faces = &m_faces;
}