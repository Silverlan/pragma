/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "pr_video_recorder.hpp"
#include <util_video_recorder.hpp>
#include <util_video_player.hpp>
#include <luainterface.hpp>
#include <luasystem.h>
#include <pragma/lua/ldefinitions.h>
#include <pragma/lua/libraries/lfile.h>
#include <mathutil/umath.h>
#include <image/prosper_image.hpp>
#include <util_image_buffer.hpp>
#include <wrappers/memory_block.h>

struct VideoRecorderFileInterface
	: public media::ICustomFile
{
	VideoRecorderFileInterface(lua_State *l)
		: m_luaState{l}
	{}
	virtual bool open(const std::string &fileName) override
	{
		auto realPath = fileName;
		if(Lua::file::validate_write_operation(m_luaState,realPath) == false)
			return false;
		m_file = FileManager::OpenFile<VFilePtrReal>(realPath.c_str(),"wb");
		return m_file != nullptr;
	}
	virtual void close() override
	{
		m_file = nullptr;
	}
	virtual std::optional<uint64_t> write(const uint8_t *data, size_t size) override
	{
		if(m_file == nullptr)
			return {};
		return m_file->Write(data,size);
	}
	virtual std::optional<uint64_t> read(uint8_t *data, size_t size) override
	{
		if(m_file == nullptr)
			return {};
		return m_file->Read(data,size);
	}
	virtual std::optional<uint64_t> seek(int64_t offset, int whence) override
	{
		if(m_file == nullptr)
			return {};
		m_file->Seek(offset,whence);
		return m_file->Tell();
	}
private:
	VFilePtrReal m_file = nullptr;
	lua_State *m_luaState = nullptr;
};

void video_recorder::register_lua_library(Lua::Interface &l)
{
	Lua::RegisterLibrary(l.GetState(),"media",{
		{"create_video_recorder",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			auto videoRecorder = media::VideoRecorder::Create(std::make_unique<VideoRecorderFileInterface>(l));
			if(videoRecorder == nullptr)
				return 0;
			Lua::Push<std::shared_ptr<media::VideoRecorder>>(l,std::move(videoRecorder));
			return 1;
		})},
		{"create_video_decoder",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			auto f = Lua::Check<LFile>(l,1).GetHandle();
			if(f == nullptr)
				return 0;
			auto videoPlayer = media::VideoPlayer::Create(f);
			if(videoPlayer == nullptr)
				return 0;
			Lua::Push<std::shared_ptr<media::VideoPlayer>>(l,std::move(videoPlayer));
			return 1;
		})},
		{"calc_bitrate",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			auto w = Lua::CheckInt(l,1);
			auto h = Lua::CheckInt(l,2);
			auto frameRate = Lua::CheckInt(l,3);
			auto bpp = Lua::CheckInt(l,4);
			auto bitrate = w *h *frameRate *bpp;
			Lua::PushInt(l,bitrate);
			return 1;
		})}
	});

	auto &modMedia = l.RegisterLibrary("media");
	auto classDefDec = luabind::class_<media::VideoPlayer>("VideoDecoder");
	classDefDec.def("ReadFrame",static_cast<void(*)(lua_State*,media::VideoPlayer&)>(
		[](lua_State *l,media::VideoPlayer &decoder) {
			double pts;
			auto frame = decoder.ReadFrame(pts);
			if(frame == nullptr)
				return;
			Lua::Push(l,frame);
			Lua::PushNumber(l,pts);
	}));
	classDefDec.def("GetVideoFrameRate",static_cast<void(*)(lua_State*,media::VideoPlayer&)>(
		[](lua_State *l,media::VideoPlayer &decoder) {
			Lua::PushNumber(l,decoder.GetVideoFrameRate());
	}));
	classDefDec.def("GetAudioFrameRate",static_cast<void(*)(lua_State*,media::VideoPlayer&)>(
		[](lua_State *l,media::VideoPlayer &decoder) {
			Lua::PushNumber(l,decoder.GetAudioFrameRate());
	}));
	classDefDec.def("GetAspectRatio",static_cast<void(*)(lua_State*,media::VideoPlayer&)>(
		[](lua_State *l,media::VideoPlayer &decoder) {
			Lua::PushNumber(l,decoder.GetAspectRatio());
	}));
	classDefDec.def("GetWidth",static_cast<void(*)(lua_State*,media::VideoPlayer&)>(
		[](lua_State *l,media::VideoPlayer &decoder) {
			Lua::PushInt(l,decoder.GetWidth());
	}));
	classDefDec.def("GetHeight",static_cast<void(*)(lua_State*,media::VideoPlayer&)>(
		[](lua_State *l,media::VideoPlayer &decoder) {
			Lua::PushInt(l,decoder.GetHeight());
	}));
	modMedia[classDefDec];

	auto classDefData = luabind::class_<media::VideoRecorder>("VideoRecorder");
	classDefData.def("StartRecording",static_cast<void(*)(lua_State*,media::VideoRecorder&,const std::string&,const media::VideoRecorder::EncodingSettings&)>(
		[](lua_State *l,media::VideoRecorder &videoRecorder,const std::string &fileName,const media::VideoRecorder::EncodingSettings &encodingSettings) {
		try
		{
			videoRecorder.StartRecording(fileName,encodingSettings);
			Lua::PushBool(l,true);
		}
		catch(const media::LogicError &err)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,err.what());
		}
		catch(const media::RuntimeError &err)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,err.what());
		}
	}));
	classDefData.def("EndRecording",static_cast<void(*)(lua_State*,media::VideoRecorder&)>([](lua_State *l,media::VideoRecorder &videoRecorder) {
		try
		{
			videoRecorder.EndRecording();
			Lua::PushBool(l,true);
		}
		catch(const media::LogicError &err)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,err.what());
		}
		catch(const media::RuntimeError &err)
		{
			Lua::PushBool(l,false);
			Lua::PushString(l,err.what());
		}
	}));
	classDefData.def("IsRecording",static_cast<void(*)(lua_State*,media::VideoRecorder&)>([](lua_State *l,media::VideoRecorder &videoRecorder) {
		Lua::PushBool(l,videoRecorder.IsRecording());
	}));
	classDefData.def("GetEncodingDuration",static_cast<void(*)(lua_State*,media::VideoRecorder&)>([](lua_State *l,media::VideoRecorder &videoRecorder) {
		auto duration = videoRecorder.GetEncodingDuration();
		Lua::PushInt(l,duration.count());
	}));
	classDefData.def("StartFrame",static_cast<void(*)(lua_State*,media::VideoRecorder&)>([](lua_State *l,media::VideoRecorder &videoRecorder) {
		Lua::PushInt(l,videoRecorder.StartFrame());
	}));
	classDefData.def("WriteFrame",static_cast<void(*)(lua_State*,media::VideoRecorder&,prosper::IImage&,double)>([](lua_State *l,media::VideoRecorder &videoRecorder,prosper::IImage &img,double frameTime) {
		auto extents = img.GetExtents();
		if(extents.width != videoRecorder.GetWidth() || extents.height != videoRecorder.GetHeight() || img.GetFormat() != prosper::Format::R8G8B8A8_UNorm)
		{
			Lua::PushBool(l,false);
			return;
		}
		void *imgData;
		auto size = extents.width *extents.height *sizeof(media::Color);
		if(img.Map(0,size,&imgData) == false)
		{
			Lua::PushBool(l,false);
			return;
		}
		auto imgBuf = uimg::ImageBuffer::Create(imgData,extents.width,extents.height,uimg::ImageBuffer::Format::RGBA8,true);
		Lua::PushInt(l,videoRecorder.WriteFrame(*imgBuf,frameTime));
		img.Unmap(); // memory doesn't actually get unmapped; imgData stays valid as long as the image is valid, as long as proper memory flags have been set!
		// TODO: Validate image memory flags to make sure this is actually the case
	}));
	classDefData.def("WriteFrame",static_cast<void(*)(lua_State*,media::VideoRecorder&,DataStream&,double)>([](lua_State *l,media::VideoRecorder &videoRecorder,DataStream &ds,double frameIndex) {
		auto szRequired = videoRecorder.GetWidth() *videoRecorder.GetHeight() *4;
		if(ds->GetSize() < szRequired)
		{
			Lua::PushBool(l,false);
			return;
		}
		auto imgBuf = uimg::ImageBuffer::Create(ds->GetData(),videoRecorder.GetWidth(),videoRecorder.GetHeight(),uimg::ImageBuffer::Format::RGBA8,true);
		Lua::PushInt(l,videoRecorder.WriteFrame(*imgBuf,frameIndex));
	}));
	classDefData.def("WriteFrame",static_cast<void(*)(lua_State*,media::VideoRecorder&,uimg::ImageBuffer&,double)>([](lua_State *l,media::VideoRecorder &videoRecorder,uimg::ImageBuffer &imgBuffer,double frameIndex) {
		auto tgtImgBuffer = imgBuffer.shared_from_this();
		if(tgtImgBuffer->GetWidth() != videoRecorder.GetWidth() || tgtImgBuffer->GetHeight() != videoRecorder.GetHeight())
			tgtImgBuffer->Resize(videoRecorder.GetWidth(),videoRecorder.GetHeight());
		Lua::PushInt(l,videoRecorder.WriteFrame(*tgtImgBuffer,frameIndex));
	}));

	auto classDefEncodingSettings = luabind::class_<media::VideoRecorder::EncodingSettings>("EncodingSettings");
	classDefEncodingSettings.def(luabind::constructor<>());
	classDefEncodingSettings.def_readwrite("width",&media::VideoRecorder::EncodingSettings::width);
	classDefEncodingSettings.def_readwrite("height",&media::VideoRecorder::EncodingSettings::height);
	classDefEncodingSettings.def_readwrite("frameRate",&media::VideoRecorder::EncodingSettings::frameRate);
	classDefEncodingSettings.def_readwrite("quality",
		reinterpret_cast<std::underlying_type_t<decltype(media::VideoRecorder::EncodingSettings::quality)> media::VideoRecorder::EncodingSettings::*>(&media::VideoRecorder::EncodingSettings::quality)
	);
	classDefEncodingSettings.def_readwrite("codec",
		reinterpret_cast<std::underlying_type_t<decltype(media::VideoRecorder::EncodingSettings::codec)> media::VideoRecorder::EncodingSettings::*>(&media::VideoRecorder::EncodingSettings::codec)
	);
	classDefEncodingSettings.def_readwrite("format",
		reinterpret_cast<std::underlying_type_t<decltype(media::VideoRecorder::EncodingSettings::format)> media::VideoRecorder::EncodingSettings::*>(&media::VideoRecorder::EncodingSettings::format)
	);
	classDefEncodingSettings.def("SetBitrate",static_cast<void(*)(lua_State*,media::VideoRecorder::EncodingSettings&,uint32_t)>([](lua_State *l,media::VideoRecorder::EncodingSettings &encodingSettings,uint32_t bitRate) {
		encodingSettings.bitRate = bitRate;
	}));
	classDefData.scope[classDefEncodingSettings];
	modMedia[classDefData];

	Lua::RegisterLibraryEnums(l.GetState(),"media",{
		{"VIDEO_CODEC_RAW",umath::to_integral(media::Codec::Raw)},
		{"VIDEO_CODEC_MPEG4",umath::to_integral(media::Codec::MPEG4)},
#ifdef VIDEO_RECORDER_ENABLE_H264_CODEC
		{"VIDEO_CODEC_H264",umath::to_integral(media::Codec::H264)},
#endif
		{"VIDEO_CODEC_OPENH264",umath::to_integral(media::Codec::OpenH264)},
		{"VIDEO_CODEC_VP8",umath::to_integral(media::Codec::VP8)},
		{"VIDEO_CODEC_VP9",umath::to_integral(media::Codec::VP9)},
		{"VIDEO_CODEC_DIRAC",umath::to_integral(media::Codec::Dirac)},
		{"VIDEO_CODEC_AV1",umath::to_integral(media::Codec::AV1)},
		{"VIDEO_CODEC_MOTION_JPEG",umath::to_integral(media::Codec::MotionJPEG)},
		{"VIDEO_CODEC_MPEG1",umath::to_integral(media::Codec::MPEG1)},
		{"VIDEO_CODEC_HEVC",umath::to_integral(media::Codec::HEVC)},
		{"VIDEO_CODEC_COUNT",umath::to_integral(media::Codec::Count)},
		
		{"VIDEO_FORMAT_RAW",umath::to_integral(media::Format::Raw)},
		{"VIDEO_FORMAT_WEBM",umath::to_integral(media::Format::WebM)},
		{"VIDEO_FORMAT_MATROSKA",umath::to_integral(media::Format::Matroska)},
		{"VIDEO_FORMAT_FLASH",umath::to_integral(media::Format::Flash)},
		{"VIDEO_FORMAT_F4V",umath::to_integral(media::Format::F4V)},
		{"VIDEO_FORMAT_SWF",umath::to_integral(media::Format::SWF)},
		{"VIDEO_FORMAT_VOB",umath::to_integral(media::Format::Vob)},
		{"VIDEO_FORMAT_OGG",umath::to_integral(media::Format::Ogg)},
		{"VIDEO_FORMAT_DIRAC",umath::to_integral(media::Format::Dirac)},
		{"VIDEO_FORMAT_GIF",umath::to_integral(media::Format::GIF)},
		{"VIDEO_FORMAT_AVI",umath::to_integral(media::Format::AVI)},
		{"VIDEO_FORMAT_QUICKTIME",umath::to_integral(media::Format::QuickTime)},
		{"VIDEO_FORMAT_REALMEDIA",umath::to_integral(media::Format::RealMedia)},
		{"VIDEO_FORMAT_MPEG4",umath::to_integral(media::Format::MPEG4)},
		{"VIDEO_FORMAT_MPEG1",umath::to_integral(media::Format::MPEG1)},
		{"VIDEO_FORMAT_MPEG2",umath::to_integral(media::Format::MPEG2)},
		{"VIDEO_FORMAT_M4V",umath::to_integral(media::Format::M4V)},
		{"VIDEO_FORMAT_3GPP",umath::to_integral(media::Format::ThreeGPP)},
		{"VIDEO_FORMAT_3GPP2",umath::to_integral(media::Format::ThreeGPP2)},

		{"VIDEO_FORMAT_COUNT",umath::to_integral(media::Format::Count)},
		
		{"QUALITY_VERY_LOW",umath::to_integral(media::Quality::VeryLow)},
		{"QUALITY_LOW",umath::to_integral(media::Quality::Low)},
		{"QUALITY_MEDIUM",umath::to_integral(media::Quality::Medium)},
		{"QUALITY_HIGH",umath::to_integral(media::Quality::High)},
		{"QUALITY_VERY_HIGH",umath::to_integral(media::Quality::VeryHigh)}
	});
}
