// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :util.screenshot;
import :client_state;
import :engine;
import :entities.components;

static std::string get_screenshot_name(pragma::Game *game, pragma::image::ImageFormat format)
{
	std::string map;
	if(game == nullptr)
		map = pragma::engine_info::get_identifier();
	else
		map = game->GetMapName();
	std::string path;
	int i = 1;
	do {
		path = "screenshots\\";
		path += map;
		path += pragma::string::fill_zeroes(std::to_string(i), 4);
		path += "." + pragma::image::get_image_output_format_extension(format);
		i++;
	} while(pragma::fs::exists(path.c_str() /*,fs::SearchFlags::Local*/));
	return path;
}

void pragma::util::rt_screenshot(CGame &game, uint32_t width, uint32_t height, const RtScreenshotSettings &settings, image::ImageFormat format)
{
	fs::create_directory("screenshots");

	auto *pCam = game.GetRenderCamera<CCameraComponent>();
	Con::COUT << "Taking raytraced screenshot..." << Con::endl;
	Con::COUT << "Preparing scene for raytracing..." << Con::endl;

	// A raytracing screenshot has been requested; We'll have to re-render the scene with raytracing enabled

	auto resolution = get_cengine()->GetRenderResolution();
	rendering::cycles::SceneInfo sceneInfo {};
	sceneInfo.width = width;
	sceneInfo.height = height;
	sceneInfo.samples = settings.samples;
	sceneInfo.hdrOutput = false; //;//(format == pragma::image::ImageOutputFormat::HDR);
	rendering::cycles::RenderImageInfo renderImgInfo {};
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

	Con::COUT << "Executing raytracer... This may take a few minutes!" << Con::endl;
	auto job = ::pragma::rendering::cycles::render_image(*get_client_state(), sceneInfo, renderImgInfo);
	if(job.IsValid()) {
		job.SetCompletionHandler([format, quality, toneMapping](ParallelWorker<image::ImageLayerSet> &worker) {
			if(worker.IsSuccessful() == false) {
				Con::CWAR << "Raytraced screenshot failed: " << worker.GetResultMessage() << Con::endl;
				return;
			}

			auto *client = get_client_state();
			auto path = get_screenshot_name(client ? client->GetGameState() : nullptr, format);
			Con::COUT << "Raytracing complete! Saving screenshot as '" << path << "'..." << Con::endl;
			auto fp = fs::open_file<fs::VFilePtrReal>(path, fs::FileMode::Write | fs::FileMode::Binary);
			if(fp == nullptr) {
				Con::CWAR << "Unable to open file '" << path << "' for writing!" << Con::endl;
				return;
			}
			auto imgBuffer = worker.GetResult().images.begin()->second;
			if(imgBuffer->IsHDRFormat())
				imgBuffer = imgBuffer->ApplyToneMapping(toneMapping);
			fs::File f {fp};
			if(image::save_image(f, *imgBuffer, format, quality) == false)
				Con::CWAR << "Unable to save screenshot as '" << path << "'!" << Con::endl;

			// Obsolete
			// imgBuffer->Convert(pragma::util::ImageBuffer::Format::RGB8);
			// pragma::util::tga::write_tga(f,imgBuffer->GetWidth(),imgBuffer->GetHeight(),static_cast<uint8_t*>(imgBuffer->GetData()));
		});
		job.Start();
		get_cengine()->AddParallelJob(job, "Raytraced screenshot");
	}
}

std::optional<std::string> pragma::util::screenshot(CGame &game)
{
	fs::create_directory("screenshots");

	auto scene = game.GetScene<CSceneComponent>();
	std::shared_ptr<prosper::IImage> imgScreenshot = nullptr;
	std::shared_ptr<prosper::IBuffer> bufScreenshot = nullptr;
	{
		// Just use the last rendered image
		auto *renderer = scene ? dynamic_cast<CRendererComponent *>(scene->GetRenderer<CRendererComponent>()) : nullptr;
		if(renderer == nullptr) {
			Con::CWAR << "No scene renderer found!" << Con::endl;
			return {};
		}
		auto rasterC = renderer->GetEntity().GetComponent<CRasterizationRendererComponent>();
		if(rasterC.expired()) {
			Con::CWAR << "No rasterization renderer found!" << Con::endl;
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
			rt = get_cengine()->GetRenderContext().GetWindow().GetStagingRenderTarget();
			break;
		}
		if(rt == nullptr) {
			Con::CWAR << "Scene render target is invalid!" << Con::endl;
			return {};
		}
		get_cengine()->GetRenderContext().WaitIdle(); // Make sure rendering is complete

		auto &img = rt->GetTexture().GetImage();
		imgScreenshot = img.shared_from_this();

		auto bufSize = img.GetWidth() * img.GetHeight() * prosper::util::get_byte_size(img.GetFormat());
		auto extents = img.GetExtents();
		bufScreenshot = get_cengine()->GetRenderContext().AllocateTemporaryBuffer(bufSize);

		// TODO: Check if image formats are compatible (https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#features-formats-compatibility)
		// before issuing the copy command
		uint32_t queueFamilyIndex;
		auto cmdBuffer = get_cengine()->GetRenderContext().AllocatePrimaryLevelCommandBuffer(prosper::QueueFamilyType::Universal, queueFamilyIndex);
		cmdBuffer->StartRecording();
		cmdBuffer->RecordImageBarrier(img, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferSrcOptimal);
		cmdBuffer->RecordCopyImageToBuffer({}, img, prosper::ImageLayout::TransferDstOptimal, *bufScreenshot);
		cmdBuffer->RecordImageBarrier(img, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
		// Note: Blit can't be used because some Nvidia GPUs don't support blitting for images with linear tiling
		//.RecordBlitImage(**cmdBuffer,{},**img,**imgDst);
		cmdBuffer->StopRecording();
		get_cengine()->GetRenderContext().SubmitCommandBuffer(*cmdBuffer, true);
	}
	if(bufScreenshot == nullptr) {
		Con::CWAR << "Failed to create screenshot image buffer!" << Con::endl;
		return {};
	}
	auto imgFormat = image::ImageFormat::PNG;
	auto path = get_screenshot_name(&game, imgFormat);
	auto fp = fs::open_file<fs::VFilePtrReal>(path, fs::FileMode::Write | fs::FileMode::Binary);
	if(fp == nullptr) {
		Con::CWAR << "Failed to open image output file '" << path << "' for writing!" << Con::endl;
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
	auto imgBuf = image::ImageBuffer::Create(data, extents.width, extents.height, image::Format::RGBA8);
	fs::File f {fp};
	image::save_image(f, *imgBuf, imgFormat);
	bufScreenshot->Unmap();
	//util::tga::write_tga(f,extents.width,extents.height,pixels);
	return path;
}
