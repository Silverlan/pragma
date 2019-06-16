#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/model/vertex.h"
#include "pragma/util/util_bsp_tree.hpp"
#include "pragma/game/game_resources.hpp"
#include <util_bsp.hpp>
#include <GuillotineBinPack.h>

#pragma optimize("",off)
bool util::bsp::FaceLightMapInfo::valid() const {return (flags &Flags::Valid) != Flags::None;}

static util::bsp::LightMapInfo load_light_map_data(bsp::File &bsp)
{
	auto &faces = bsp.GetHDRFaces().empty() ? bsp.GetFaces() : bsp.GetHDRFaces();
	auto &texInfo = bsp.GetTexInfo();
	{
		/*auto &dispInfo = bsp.GetDispInfo().front();
		auto &face = faces.at(dispInfo.MapFace);

		auto &dispLightmapSamplePositions = bsp.GetDispLightmapSamplePositions();
		struct SamplePosition
		{
			uint32_t triangleIndex;
			Vector3 barycentricCoordinates;
		};
		std::vector<SamplePosition> decompressedSamplePositions {};
		decompressedSamplePositions.reserve(dispLightmapSamplePositions.size());

		for(auto i=decltype(dispLightmapSamplePositions.size()){0u};i<dispLightmapSamplePositions.size();)
		{
			decompressedSamplePositions.push_back({});
			auto &samplePosition = decompressedSamplePositions.back();
			auto v = dispLightmapSamplePositions.at(i++);
			if(v < 255)
				samplePosition.triangleIndex = v;
			else
				samplePosition.triangleIndex = dispLightmapSamplePositions.at(i++) +255;
			auto &flBarycentric = samplePosition.barycentricCoordinates;
			flBarycentric.x = dispLightmapSamplePositions.at(i++) /255.9f;
			flBarycentric.y = dispLightmapSamplePositions.at(i++) /255.9f;
			flBarycentric.z = dispLightmapSamplePositions.at(i++) /255.9f;

			auto it = test.find(samplePosition.triangleIndex);
			if(it != test.end())
				throw std::runtime_error("!!");
			test[samplePosition.triangleIndex] = samplePosition;
		}
		for(auto i=0;i<512;++i)
		{
			auto it = test.find(i);
			if(it == test.end())
				throw std::runtime_error("@@");
		}*/
	}

	util::bsp::LightMapInfo lightMapInfo {};
	lightMapInfo.faceInfos.reserve(faces.size());
	lightMapInfo.luxelData = bsp.GetHDRLightMapData();
	if(lightMapInfo.luxelData.empty())
		lightMapInfo.luxelData = bsp.GetLightMapData(); // Use LDR light map data if HDR is not available
	auto borderSize = lightMapInfo.borderSize;

	// Load lightmaps
	auto &planes = bsp.GetPlanes();
	auto numFacesWithLightMapInfo = 0u;
	for(auto &face : faces)
	{
		auto faceIndex = lightMapInfo.faceInfos.size();
		//if(face.styles.front() == 0 && face.lightofs >= -1)
		{
			auto widthLightmap = face.LightmapTextureSizeInLuxels.at(0) +1u;
			auto heightLightmap = face.LightmapTextureSizeInLuxels.at(1) +1u;

			auto &plane = planes.at(face.planenum);
			auto n = plane.normal;
			if(face.side)
				n = -n;
			lightMapInfo.faceInfos.push_back({
				0,0,
				{static_cast<int32_t>(widthLightmap),static_cast<int32_t>(heightLightmap)},
				{static_cast<int32_t>(face.LightmapTextureMinsInLuxels.at(0)),static_cast<int32_t>(face.LightmapTextureMinsInLuxels.at(1))},
				util::bsp::FaceLightMapInfo::Flags::Valid,
				faceIndex,
				static_cast<uint32_t>(face.lightofs),
				face.texinfo,
				face.dispinfo,
				static_cast<uint32_t>(face.firstedge),
				static_cast<uint16_t>(face.numedges),
				n
			});
			++numFacesWithLightMapInfo;
		}
		//else
		//	lightMapInfo.faceInfos.push_back({0,0,{0,0},{0,0},util::bsp::FaceLightMapInfo::Flags::None,faceIndex,0u,-1,-1,0u,0u,Vector3{}});
	}

	// Attempt to find smallest texture required to fit all of the
	// lightmaps. This may take several attempts.
	const auto maxSizeLightMapAtlas = 16'384;
	auto szLightMapAtlas = 8u;
	rbp::GuillotineBinPack binPack {};
	do
	{
		szLightMapAtlas <<= 1u;
		binPack.Init(szLightMapAtlas,szLightMapAtlas);

		for(auto &info : lightMapInfo.faceInfos)
		{
			if(info.valid() == false)
				continue;
			binPack.Insert(info.lightMapSize.at(0) +borderSize *2u,info.lightMapSize.at(1) +borderSize *2u,true,rbp::GuillotineBinPack::FreeRectChoiceHeuristic::RectBestAreaFit,rbp::GuillotineBinPack::GuillotineSplitHeuristic::SplitShorterLeftoverAxis);
		}

		// Note: Passing the entire container to the Insert-call
		// like below would be more efficient, but causes
		// discrepancies between the width and height of the original
		// rect and the inserted one in some cases.
		/*
		// This vector will be cleared by the Insert-call, but we may need it for
		// another iteration if the size doesn't fit all rects,
		// so we need to make a copy.
		auto tmpLightMapRects = lightMapRects;
		binPack.Insert(tmpLightMapRects,true,rbp::GuillotineBinPack::FreeRectChoiceHeuristic::RectBestAreaFit,rbp::GuillotineBinPack::GuillotineSplitHeuristic::SplitShorterLeftoverAxis);*/
	}
	while(binPack.GetUsedRectangles().size() < numFacesWithLightMapInfo && szLightMapAtlas < maxSizeLightMapAtlas);

	lightMapInfo.atlasSize = szLightMapAtlas;
	auto &packedRects = binPack.GetUsedRectangles();
	auto &atlasRects = lightMapInfo.lightmapAtlas;
	atlasRects.reserve(packedRects.size());
	for(auto &rect : packedRects)
		atlasRects.push_back({static_cast<uint16_t>(rect.x),static_cast<uint16_t>(rect.y),static_cast<uint16_t>(rect.width),static_cast<uint16_t>(rect.height)});

	auto rectIdx = 0u;
	for(auto lmIdx=decltype(lightMapInfo.faceInfos.size()){0u};lmIdx<lightMapInfo.faceInfos.size();++lmIdx)
	{
		auto &info = lightMapInfo.faceInfos.at(lmIdx);
		if(info.valid() == false)
			continue;
		auto &rect = packedRects.at(rectIdx++);
		info.x = rect.x;
		info.y = rect.y;
		if(info.lightMapSize.at(0) +borderSize *2u != rect.width)
			info.flags |= util::bsp::FaceLightMapInfo::Flags::Rotated;
		info.lightMapSize.at(0) = rect.width -borderSize *2u;
		info.lightMapSize.at(1) = rect.height -borderSize *2u;
	}
	return lightMapInfo;
}

util::bsp::GeometryData util::bsp::load_bsp_geometry(NetworkState &nw,::bsp::File &bsp)
{
	util::bsp::GeometryData geometryData {};
	geometryData.lightMapData = load_light_map_data(bsp);
	return geometryData;
}
#pragma optimize("",on)
