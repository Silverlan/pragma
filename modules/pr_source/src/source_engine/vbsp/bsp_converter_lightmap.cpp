#include <iostream>
#include "source_engine/vbsp/bsp_converter.hpp"
#include <pragma/asset_types/world.hpp>
#include <pragma/game/game_resources.hpp>
#include <util_image.hpp>
#include <util_texture_info.hpp>
#include <util_image_buffer.hpp>
#include <GuillotineBinPack.h>

bool pragma::asset::vbsp::BSPConverter::GenerateLightMapAtlas(LightmapData &lightmapInfo,const std::string &mapName)
{
	auto widthLightmapAtlas = lightmapInfo.atlasSize.x;
	auto heightLightmapAtlas = lightmapInfo.atlasSize.y;
	auto borderSize = lightmapInfo.borderSize;
	auto &lightMapData = lightmapInfo.luxelData;
	if(lightMapData.empty())
		return false;
	auto lightMapAtlas = uimg::ImageBuffer::Create(widthLightmapAtlas,heightLightmapAtlas,uimg::ImageBuffer::Format::RGBA16);
	auto &rects = lightmapInfo.lightmapAtlas;
	if(rects.size() != lightmapInfo.faceInfos.size())
		; // TODO: Print Warning: LIGHT MAP ATLAS TO SMALL TO ENCOMPASS ALL LIGHTMAPS
	auto rectIdx = 0u;
	for(auto lmIdx=decltype(lightmapInfo.faceInfos.size()){0u};lmIdx<lightmapInfo.faceInfos.size();++lmIdx)
	{
		auto &info = lightmapInfo.faceInfos.at(lmIdx);
		if(info.valid() == false)
			continue;
		auto &rect = rects.at(rectIdx++);
		info.x = rect.x;
		info.y = rect.y;
		if(info.lightMapSize.at(0) +borderSize *2u != rect.w)
		{
			// This shouldn't happen (The bounds should already have been put in the proper rotation before the map-file was created)
			throw std::runtime_error("Illegal light map bounds!");
			info.flags |= FaceLightMapInfo::Flags::Rotated;
		}
		info.lightMapSize.at(0) = rect.w -borderSize *2u;
		info.lightMapSize.at(1) = rect.h -borderSize *2u;
	}

	for(auto faceIndex=decltype(lightmapInfo.faceInfos.size()){0u};faceIndex<lightmapInfo.faceInfos.size();++faceIndex)
	{
		auto &lmInfo = lightmapInfo.faceInfos.at(faceIndex);
		if(lmInfo.valid() == false)
			continue;
		auto &face = lightmapInfo.faceInfos.at(faceIndex);
		if(face.luxelDataOffset == std::numeric_limits<uint32_t>::max())
			continue; // No light map available for this face (e.g. nodraw or skybox)
		auto widthLightmap = face.lightMapSize.at(0);
		auto heightLightmap = face.lightMapSize.at(1);
		auto bRotated = (lmInfo.flags &FaceLightMapInfo::Flags::Rotated) != FaceLightMapInfo::Flags::None;
		const auto fCalcPixelIndex = [&lmInfo,widthLightmapAtlas,heightLightmap,bRotated,borderSize](int x,int y,int offsetX,int offsetY) {
			x += lmInfo.x +borderSize +offsetX;
			y += lmInfo.y +borderSize +offsetY;
			return y *widthLightmapAtlas +x;
		};
		auto *lightmapData = lightMapData.data() +face.luxelDataOffset;
		for(auto y=decltype(heightLightmap){0u};y<heightLightmap;++y)
		{
			for(auto x=decltype(widthLightmap){0u};x<widthLightmap;++x)
			{
				auto xLuxel = x;
				auto yLuxel = y;
				if(bRotated)
				{
					auto tmp = yLuxel;
					yLuxel = widthLightmap -xLuxel -1;
					xLuxel = tmp;
				}
				auto &luxel = *reinterpret_cast<const bsp::ColorRGBExp32*>(lightmapData +(yLuxel *(bRotated ? heightLightmap : widthLightmap) +xLuxel) *sizeof(bsp::ColorRGBExp32));

				auto exp = umath::pow(2.0,static_cast<double>(luxel.exponent));
				auto rgbR = luxel.r *exp /255.f;
				auto rgbG = luxel.g *exp /255.f;
				auto rgbB = luxel.b *exp /255.f;

				if(lmInfo.lightMapSize.at(1) != 0)
				{
					auto pxIdx = fCalcPixelIndex(x,y,0,0);

					std::array<uint16_t,4> hdrPxCol = {
						umath::float32_to_float16_glm(rgbR),
						umath::float32_to_float16_glm(rgbG),
						umath::float32_to_float16_glm(rgbB),
						umath::float32_to_float16_glm(1.f)
					};
					lightMapAtlas->SetPixelColor(pxIdx,hdrPxCol);

					auto borderCol = hdrPxCol;
					//borderCol = bRotated ? Vector4{255.f,0.f,255.f,255.f} : Vector4{0.f,255.f,255.f,255.f};
					// Vertical border
					if(x == 0u)
					{
						for(auto i=decltype(borderSize){1u};i<=borderSize;++i)
							lightMapAtlas->SetPixelColor(fCalcPixelIndex(x,y,-i,0),borderCol);
						if(y == 0u)
						{
							// Fill out top left corner
							for(auto xOff=decltype(borderSize){1u};xOff<=borderSize;++xOff)
							{
								for(auto yOff=decltype(borderSize){1u};yOff<=borderSize;++yOff)
									lightMapAtlas->SetPixelColor(fCalcPixelIndex(x,y,-xOff,-yOff),borderCol);
							}
						}
					}
					else if(x == (widthLightmap -1u))
					{
						for(auto i=decltype(borderSize){1u};i<=borderSize;++i)
							lightMapAtlas->SetPixelColor(fCalcPixelIndex(x,y,i,0),borderCol);
						if(y == (heightLightmap -1u))
						{
							// Fill out bottom right corner
							for(auto xOff=decltype(borderSize){1u};xOff<=borderSize;++xOff)
							{
								for(auto yOff=decltype(borderSize){1u};yOff<=borderSize;++yOff)
									lightMapAtlas->SetPixelColor(fCalcPixelIndex(x,y,xOff,yOff),borderCol);
							}
						}
					}

					// Horizontal border
					if(y == 0u)
					{
						for(auto i=decltype(borderSize){1u};i<=borderSize;++i)
							lightMapAtlas->SetPixelColor(fCalcPixelIndex(x,y,0,-i),borderCol);
						if(x == (widthLightmap -1u))
						{
							// Fill out top right corner
							for(auto xOff=decltype(borderSize){1u};xOff<=borderSize;++xOff)
							{
								for(auto yOff=decltype(borderSize){1u};yOff<=borderSize;++yOff)
									lightMapAtlas->SetPixelColor(fCalcPixelIndex(x,y,xOff,-yOff),borderCol);
							}
						}
					}
					else if(y == (heightLightmap -1u))
					{
						for(auto i=decltype(borderSize){1u};i<=borderSize;++i)
							lightMapAtlas->SetPixelColor(fCalcPixelIndex(x,y,0,i),borderCol);
						if(x == 0u)
						{
							// Fill out bottom left corner
							for(auto xOff=decltype(borderSize){1u};xOff<=borderSize;++xOff)
							{
								for(auto yOff=decltype(borderSize){1u};yOff<=borderSize;++yOff)
									lightMapAtlas->SetPixelColor(fCalcPixelIndex(x,y,-xOff,yOff),borderCol);
							}
						}
					}
				}
			}
		}
	}
	m_outputWorldData->SetLightMapAtlas(*lightMapAtlas);
	return true;
}

struct SBPRect
{
	uint32_t index = 0u;
	uint32_t w = 0u;
	uint32_t h = 0u;
};

struct SBPSpace
{
	uint32_t x = 0u;
	uint32_t y = 0u;
	uint32_t w = 0u;
	uint32_t h = 0u;
};

struct SBPPacked
{
	uint32_t rectIdx = 0u;
	SBPRect packedRect = {};
};

static std::vector<rbp::Rect> simple_binpacking(std::vector<SBPRect> &rects,uint32_t &outAtlasWidth,uint32_t &outAtlasHeight)
{
	// Source: https://observablehq.com/@mourner/simple-rectangle-packing
	uint32_t area = 0u;
	uint32_t maxWidth = 0u;
	for(auto &rect : rects)
	{
		area += rect.w *rect.h;
		maxWidth = umath::max(maxWidth,rect.w);
	}

	std::sort(rects.begin(),rects.end(),[](const SBPRect &a,const SBPRect &b) {
		return (static_cast<int32_t>(b.h) -static_cast<int32_t>(a.h)) < 0;
		});

	auto startWidth = umath::max(static_cast<uint32_t>(umath::ceil(umath::sqrt(area /0.95f))),maxWidth);
	std::vector<SBPSpace> spaces {
		{0u,0u,static_cast<uint32_t>(startWidth),std::numeric_limits<uint32_t>::max()}
	};
	spaces.reserve(100);
	std::vector<rbp::Rect> packed {};
	packed.resize(rects.size());
	for(auto &rect : rects)
	{
		for(auto i=static_cast<int32_t>(spaces.size()) -1;i>=0;--i)
		{
			auto &space = spaces.at(i);
			if((rect.w > space.w || rect.h > space.h))
				continue;
			packed.at(rect.index) = {
				static_cast<int32_t>(space.x),static_cast<int32_t>(space.y),
				static_cast<int32_t>(rect.w),static_cast<int32_t>(rect.h)
			};
			if(rect.w == space.w && rect.h == space.h)
			{
				auto last = spaces.back();
				spaces.erase(spaces.end() -1);
				if(i < spaces.size())
					spaces.at(i) = last;
			}
			else if(rect.h == space.h)
			{
				space.x += rect.w;
				space.w -= rect.w;
			}
			else if(rect.w == space.w)
			{
				space.y += rect.h;
				space.h -= rect.h;
			}
			else
			{
				if(spaces.size() == spaces.capacity())
					spaces.reserve(spaces.size() *1.5f); // Increase by 50%
				auto &space = spaces.at(i); // We need a new reference, because the reserve call above may have invalidated the old one
				spaces.push_back({
					space.x +rect.w,
					space.y,
					space.w -rect.w,
					rect.h
					});
				space.y += rect.h;
				space.h -= rect.h;
			}
			break;
		}
	}

	outAtlasWidth = 0u;
	outAtlasHeight = 0u;
	for(auto &rect : packed)
	{
		outAtlasWidth = umath::max(outAtlasWidth,static_cast<uint32_t>(rect.x +rect.width));
		outAtlasHeight = umath::max(outAtlasHeight,static_cast<uint32_t>(rect.y +rect.height));
	}

	constexpr auto validateResult = false;
	if constexpr(validateResult)
	{
		// Validate (Make sure no boxes are overlapping or going out of bounds)
		std::vector<uint32_t> pixels {};
		pixels.resize(outAtlasWidth *outAtlasHeight,std::numeric_limits<uint32_t>::max());
		for(auto i=decltype(packed.size()){0u};i<packed.size();++i)
		{
			auto &rect = packed.at(i);
			if((rect.x +rect.width) > outAtlasWidth || (rect.y +rect.height) > outAtlasHeight)
				throw std::logic_error{"Lightmap resolutions out of bounds!"};
			for(auto x=rect.x;x<(rect.x +rect.width);++x)
			{
				for(auto y=rect.y;y<(rect.y +rect.height);++y)
				{
					auto pxOffset = (y *outAtlasWidth) +x;
					if(pixels.at(pxOffset) != std::numeric_limits<uint32_t>::max())
						throw std::logic_error{"Rectangle " +std::to_string(i) +" and " +std::to_string(pixels.at(pxOffset)) +" are overlapping!"};
					pixels.at(pxOffset) = i;
				}
			}
		}
	}
	return packed;
}

bool pragma::asset::vbsp::FaceLightMapInfo::valid() const {return (flags &Flags::Valid) != Flags::None;}

std::shared_ptr<pragma::asset::vbsp::LightmapData> pragma::asset::vbsp::BSPConverter::LoadLightmapData(NetworkState &nw,::bsp::File &bsp)
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

	auto lightMapInfo = std::make_shared<LightmapData>();
	lightMapInfo->faceInfos.reserve(faces.size());
	lightMapInfo->luxelData = bsp.GetHDRLightMapData();
	if(lightMapInfo->luxelData.empty())
		lightMapInfo->luxelData = bsp.GetLightMapData(); // Use LDR light map data if HDR is not available
	auto borderSize = lightMapInfo->borderSize;

	// Load lightmaps
	auto &planes = bsp.GetPlanes();
	auto numFacesWithLightMapInfo = 0u;
	for(auto &face : faces)
	{
		auto faceIndex = lightMapInfo->faceInfos.size();
		//if(face.styles.front() == 0 && face.lightofs >= -1)
		{
			auto widthLightmap = face.LightmapTextureSizeInLuxels.at(0) +1u;
			auto heightLightmap = face.LightmapTextureSizeInLuxels.at(1) +1u;

			auto &plane = planes.at(face.planenum);
			auto n = plane.normal;
			if(face.side)
				n = -n;
			lightMapInfo->faceInfos.push_back({
				0,0,
				{static_cast<int32_t>(widthLightmap),static_cast<int32_t>(heightLightmap)},
				{static_cast<int32_t>(face.LightmapTextureMinsInLuxels.at(0)),static_cast<int32_t>(face.LightmapTextureMinsInLuxels.at(1))},
				FaceLightMapInfo::Flags::Valid,
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

	enum class BinPackAlgorithm : uint8_t
	{
		Guillotine = 0u,
		Simple
	};
	constexpr auto algorithm = BinPackAlgorithm::Simple;
	std::vector<rbp::Rect> packedRects {};
	Vector2i atlasResolution {};
	switch(algorithm)
	{
	case BinPackAlgorithm::Guillotine:
	{
		// Attempt to find smallest texture required to fit all of the
		// lightmaps. This may take several attempts.
		const auto maxSizeLightMapAtlas = 16'384;
		auto szLightMapAtlas = 8u;
		rbp::GuillotineBinPack binPack {};
		do
		{
			szLightMapAtlas <<= 1u;
			binPack.Init(szLightMapAtlas,szLightMapAtlas);

			for(auto &info : lightMapInfo->faceInfos)
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
		packedRects = std::move(binPack.GetUsedRectangles());
		atlasResolution = {szLightMapAtlas,szLightMapAtlas};
		break;
	}
	case BinPackAlgorithm::Simple:
	{
		// Very simple and inaccurate algorithm, but much faster than the one above and
		// it's sufficient for our purposes.
		std::vector<SBPRect> rects {};
		rects.reserve(lightMapInfo->faceInfos.size());
		for(auto i=decltype(lightMapInfo->faceInfos.size()){0u};i<lightMapInfo->faceInfos.size();++i)
		{
			auto &info = lightMapInfo->faceInfos.at(i);
			if(info.valid() == false)
				continue;
			rects.push_back({});
			auto &rect = rects.back();
			rect.index = i;
			rect.w = info.lightMapSize.at(0) +borderSize *2u;
			rect.h = info.lightMapSize.at(1) +borderSize *2u;
		}
		uint32_t w,h;
		packedRects = simple_binpacking(rects,w,h);
		if((w & (w - 1)) != 0) // Check if not power of 2. TODO: Use std::ispow2 once Visual Studio has C++-20 support
			w = umath::next_power_of_2(w);
		if((h & (h - 1)) != 0)
			h = umath::next_power_of_2(h);
		atlasResolution = {w,h};
		break;
	}
	};

	lightMapInfo->atlasSize = atlasResolution;
	auto &atlasRects = lightMapInfo->lightmapAtlas;
	atlasRects.reserve(packedRects.size());
	for(auto &rect : packedRects)
		atlasRects.push_back({static_cast<uint16_t>(rect.x),static_cast<uint16_t>(rect.y),static_cast<uint16_t>(rect.width),static_cast<uint16_t>(rect.height)});

	auto rectIdx = 0u;
	for(auto lmIdx=decltype(lightMapInfo->faceInfos.size()){0u};lmIdx<lightMapInfo->faceInfos.size();++lmIdx)
	{
		auto &info = lightMapInfo->faceInfos.at(lmIdx);
		if(info.valid() == false)
			continue;
		auto &rect = packedRects.at(rectIdx++);
		info.x = rect.x;
		info.y = rect.y;
		if(info.lightMapSize.at(0) +borderSize *2u != rect.width)
			info.flags |= FaceLightMapInfo::Flags::Rotated;
		info.lightMapSize.at(0) = rect.width -borderSize *2u;
		info.lightMapSize.at(1) = rect.height -borderSize *2u;
	}
	return lightMapInfo;
}
