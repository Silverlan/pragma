/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/util/c_util.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/rendering/raytracing/cycles.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include <util_image.hpp>
#include <util_image_buffer.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_texture.hpp>
#include <image/prosper_render_target.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_window.hpp>
#include <fsys/ifile.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;

static std::string get_screenshot_name(Game *game, uimg::ImageFormat format)
{
	std::string map;
	if(game == nullptr)
		map = engine_info::get_identifier();
	else
		map = game->GetMapName();
	std::string path;
	int i = 1;
	do {
		path = "screenshots\\";
		path += map;
		path += ustring::fill_zeroes(std::to_string(i), 4);
		path += "." + uimg::get_image_output_format_extension(format);
		i++;
	} while(FileManager::Exists(path.c_str() /*,fsys::SearchFlags::Local*/));
	return path;
}

void util::rt_screenshot(CGame &game, uint32_t width, uint32_t height, const RtScreenshotSettings &settings, uimg::ImageFormat format)
{
	FileManager::CreateDirectory("screenshots");

	auto *pCam = game.GetRenderCamera();
	Con::cout << "Taking raytraced screenshot..." << Con::endl;
	Con::cout << "Preparing scene for raytracing..." << Con::endl;

	// A raytracing screenshot has been requested; We'll have to re-render the scene with raytracing enabled

	auto resolution = c_engine->GetRenderResolution();
	::pragma::rendering::cycles::SceneInfo sceneInfo {};
	sceneInfo.width = width;
	sceneInfo.height = height;
	sceneInfo.samples = settings.samples;
	sceneInfo.hdrOutput = false; //;//(format == pragma::image::ImageOutputFormat::HDR);
	::pragma::rendering::cycles::RenderImageInfo renderImgInfo {};
	if(pCam) {
		renderImgInfo.camPose = pCam->GetEntity().GetPose();
		renderImgInfo.farZ = pCam->GetFarZ();
		renderImgInfo.fov = pCam->GetFOV();
	}
	sceneInfo.denoise = settings.denoise;

	auto quality = settings.quality;
	auto toneMapping = settings.toneMapping;

	if(settings.sky)
		sceneInfo.sky = *settings.sky;

	sceneInfo.skyStrength = settings.skyStrength;
	sceneInfo.skyAngles = settings.skyAngles;

	Con::cout << "Executing raytracer... This may take a few minutes!" << Con::endl;
	auto job = ::pragma::rendering::cycles::render_image(*client, sceneInfo, renderImgInfo);
	if(job.IsValid()) {
		job.SetCompletionHandler([format, quality, toneMapping](util::ParallelWorker<uimg::ImageLayerSet> &worker) {
			if(worker.IsSuccessful() == false) {
				Con::cwar << "Raytraced screenshot failed: " << worker.GetResultMessage() << Con::endl;
				return;
			}

			auto path = get_screenshot_name(client ? client->GetGameState() : nullptr, format);
			Con::cout << "Raytracing complete! Saving screenshot as '" << path << "'..." << Con::endl;
			auto fp = FileManager::OpenFile<VFilePtrReal>(path.c_str(), "wb");
			if(fp == nullptr) {
				Con::cwar << "Unable to open file '" << path << "' for writing!" << Con::endl;
				return;
			}
			auto imgBuffer = worker.GetResult().images.begin()->second;
			if(imgBuffer->IsHDRFormat())
				imgBuffer = imgBuffer->ApplyToneMapping(toneMapping);
			fsys::File f {fp};
			if(uimg::save_image(f, *imgBuffer, format, quality) == false)
				Con::cwar << "Unable to save screenshot as '" << path << "'!" << Con::endl;

			// Obsolete
			// imgBuffer->Convert(util::ImageBuffer::Format::RGB8);
			// util::tga::write_tga(f,imgBuffer->GetWidth(),imgBuffer->GetHeight(),static_cast<uint8_t*>(imgBuffer->GetData()));
		});
		job.Start();
		c_engine->AddParallelJob(job, "Raytraced screenshot");
	}
}

std::optional<std::string> util::screenshot(CGame &game)
{
	FileManager::CreateDirectory("screenshots");

	auto scene = game.GetScene();
	std::shared_ptr<prosper::IImage> imgScreenshot = nullptr;
	std::shared_ptr<prosper::IBuffer> bufScreenshot = nullptr;
	{
		// Just use the last rendered image
		auto *renderer = scene ? dynamic_cast<::pragma::CRendererComponent *>(scene->GetRenderer()) : nullptr;
		if(renderer == nullptr) {
			Con::cwar << "No scene renderer found!" << Con::endl;
			return {};
		}
		auto rasterC = renderer->GetEntity().GetComponent<::pragma::CRasterizationRendererComponent>();
		if(rasterC.expired()) {
			Con::cwar << "No rasterization renderer found!" << Con::endl;
			return {};
		}

		enum class ImageStage : uint8_t { GameScene = 0, ScreenOutput };
		auto stage = ImageStage::ScreenOutput;
		std::shared_ptr<prosper::RenderTarget> rt = nullptr;
		switch(stage) {
		case ImageStage::GameScene:
			rt = rasterC->GetHDRInfo().toneMappedRenderTarget;
			break;
		case ImageStage::ScreenOutput:
			rt = c_engine->GetRenderContext().GetWindow().GetStagingRenderTarget();
			break;
		}
		if(rt == nullptr) {
			Con::cwar << "Scene render target is invalid!" << Con::endl;
			return {};
		}
		c_engine->GetRenderContext().WaitIdle(); // Make sure rendering is complete

		auto &img = rt->GetTexture().GetImage();
		imgScreenshot = img.shared_from_this();

		auto bufSize = img.GetWidth() * img.GetHeight() * prosper::util::get_byte_size(img.GetFormat());
		auto extents = img.GetExtents();
		bufScreenshot = c_engine->GetRenderContext().AllocateTemporaryBuffer(bufSize);

		// TODO: Check if image formats are compatible (https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#features-formats-compatibility)
		// before issuing the copy command
		uint32_t queueFamilyIndex;
		auto cmdBuffer = c_engine->GetRenderContext().AllocatePrimaryLevelCommandBuffer(prosper::QueueFamilyType::Universal, queueFamilyIndex);
		cmdBuffer->StartRecording();
		cmdBuffer->RecordImageBarrier(img, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferSrcOptimal);
		cmdBuffer->RecordCopyImageToBuffer({}, img, prosper::ImageLayout::TransferDstOptimal, *bufScreenshot);
		cmdBuffer->RecordImageBarrier(img, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
		// Note: Blit can't be used because some Nvidia GPUs don't support blitting for images with linear tiling
		//.RecordBlitImage(**cmdBuffer,{},**img,**imgDst);
		cmdBuffer->StopRecording();
		c_engine->GetRenderContext().SubmitCommandBuffer(*cmdBuffer, true);
	}
	if(bufScreenshot == nullptr) {
		Con::cwar << "Failed to create screenshot image buffer!" << Con::endl;
		return {};
	}
	auto imgFormat = uimg::ImageFormat::PNG;
	auto path = get_screenshot_name(&game, imgFormat);
	auto fp = FileManager::OpenFile<VFilePtrReal>(path.c_str(), "wb");
	if(fp == nullptr) {
		Con::cwar << "Failed to open image output file '" << path << "' for writing!" << Con::endl;
		return {};
	}
	auto format = imgScreenshot->GetFormat();
	auto bSwapped = false;
	if(format == prosper::Format::B8G8R8A8_UNorm) {
		format = prosper::Format::R8G8B8A8_UNorm;
		bSwapped = true;
	}
	else if(format == prosper::Format::B8G8R8_UNorm_PoorCoverage) {
		format = prosper::Format::R8G8B8_UNorm_PoorCoverage;
		bSwapped = true;
	}

	void *data;
	auto byteSize = prosper::util::get_byte_size(format);
	auto extents = imgScreenshot->GetExtents();
	auto numBytes = extents.width * extents.height * byteSize;
	bufScreenshot->Map(0ull, numBytes, prosper::IBuffer::MapFlags::None, &data);
	auto imgBuf = uimg::ImageBuffer::Create(data, extents.width, extents.height, uimg::Format::RGBA8);
	fsys::File f {fp};
	uimg::save_image(f, *imgBuf, imgFormat);
	bufScreenshot->Unmap();
	//util::tga::write_tga(f,extents.width,extents.height,pixels);
	return path;
}
