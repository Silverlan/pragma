/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_enginedefinitions.h"
#include "pragma/clientstate/clientstate.h"
#include <vorbis/vorbisfile.h>
#include <fsys/filesystem.h>
#include "pragma/audio/c_alsound.h"
#include <pragma/console/conout.h>
#include "pragma/audio/sound_ogg.h"
#include "pragma/audio/c_sound_load.h"



template<class T, class S>
static unsigned int stereo_to_mono(char *src, char **ptgt, unsigned int datalen, unsigned int fragmentSize = 2)
{
	*ptgt = new char[datalen];
	char *monoData = *ptgt;
	for(unsigned int i = 0; i < datalen; i += (fragmentSize * 2)) {
		T left = *(T *)&src[i];
		T right = *(T *)&src[i + fragmentSize];
#pragma message("TODO: This leads to information loss / Worse quality. Dividing by two leads to lower volume however. Find a better way to convert from stereo to mono!")
		S monoSample = (S(left) + S(right)); // /2;
		if(monoSample > std::numeric_limits<T>::max())
			monoSample = std::numeric_limits<T>::max();
		else if(monoSample < std::numeric_limits<T>::lowest())
			monoSample = std::numeric_limits<T>::lowest();
		T rsMonoSample = static_cast<T>(monoSample);
		memcpy(&monoData[i / 2], &rsMonoSample, sizeof(T));
	}
	return datalen / 2;
}

/*
static void fill_resource(ALResource &res,ALuint rate,ALenum channels,ALenum type,ALenum format)
{
	res.rate = rate;
	res.channels = channels;
	res.type = type;
	res.format = format;
}

static bool initialize_albuffer(std::string &path,ALenum format,ALuint rate,int freq,ALenum channels,void *buffer,ALsizei bufferSize,ALChannel mode,ALAudio &audio)
{
	//int err = alGetError();
	if(format == AL_FORMAT_MONO16)
	{
		if(audio.mono == nullptr)
			audio.mono = std::make_shared<ALResource>();
		if(audio.mono->buffer != -1)
			return true;
		fill_resource(*audio.mono,rate,channels,AL_UNSIGNED_BYTE_SOFT,format);
		unsigned int alBuffer;
		alGenBuffers(1,&alBuffer);
		audio.mono->buffer = alBuffer;
		alBufferData(audio.mono->buffer,audio.mono->format,buffer,bufferSize,static_cast<ALsizei>(freq));
		return al_check_error(path,alBuffer,*audio.mono);
	}
	if(mode != ALChannel::Mono)
	{
		if(audio.stereo == NULL)
			audio.stereo = std::make_shared<ALResource>();
		if(mode == ALChannel::Both)
		{
			if(audio.stereo == NULL)
				audio.stereo = std::make_shared<ALResource>();
			fill_resource(*audio.stereo,rate,channels,AL_UNSIGNED_BYTE_SOFT,format);
			audio.stereo->buffer = 0;
		}
		if(audio.stereo->buffer == -1)
		{
			fill_resource(*audio.stereo,rate,channels,AL_UNSIGNED_BYTE_SOFT,format);
			unsigned int alBuffer;
			alGenBuffers(1,&alBuffer);
			audio.stereo->buffer = alBuffer;
			alBufferData(audio.stereo->buffer,audio.stereo->format,buffer,bufferSize,static_cast<ALsizei>(freq));
			if(al_check_error(path,alBuffer,*audio.stereo) == false)
				return false;
		}
		if(mode == ALChannel::Auto || (audio.mono != NULL && audio.mono->buffer > 0))
			return true;
	}
	if(audio.mono == NULL)
		audio.mono = std::make_shared<ALResource>();
	char *monoData;
	unsigned int datalen = CUInt32(bufferSize);
	datalen = stereo_to_mono<short,int>(static_cast<char*>(buffer),&monoData,datalen);
	fill_resource(*audio.mono,rate,channels,AL_UNSIGNED_BYTE_SOFT,format);
	audio.mono->format = AL_MONO16_SOFT;
	audio.mono->channels = AL_MONO_SOFT;
	unsigned int alBuffer;
	alGenBuffers(1,&alBuffer);
	audio.mono->buffer = alBuffer;
	alBufferData(audio.mono->buffer,audio.mono->format,&monoData[0],datalen,freq);
	delete[] monoData;
	return al_check_error(path,alBuffer,*audio.mono);
}*/

bool openal::load_alure(std::string &path, VFilePtr f, ALChannel mode, ALAudio &audio)
{
	/*alGetError(); // Clear Last Error
	ALenum format;
	ALuint freq;
	ALubyte *data;
	ALsizei dataSize;
	if(alureRawBufferDataFromFile(path.c_str(),&data,&dataSize,&format,&freq) == AL_FALSE)
		return false;
	auto r = initialize_albuffer(path,format,freq,freq,1,&data[0],dataSize,mode,audio);
	delete[] data;
	return r;*/
	return false;
}

bool openal::load_ogg(std::string &path, VFilePtr f, ALChannel mode, ALAudio &audio)
{
	/*alGetError(); // Clear Last Error
	vorbis_info *pInfo;
	OggVorbis_File oggFile;
	GetOGGFileData(f,&pInfo,oggFile);
	ALuint rate = pInfo->rate;
	ALenum channels = pInfo->channels;
	ALenum format;
	if(pInfo->channels == 1)
		format = AL_FORMAT_MONO16;
	else
		format = AL_FORMAT_STEREO16;
	int freq = pInfo->rate;
	int bytes;
	char buffer[OGG_BUFFER_SIZE];
	int endian = 0;
	int bitStream;
	std::vector<char> data;
	do {
		bytes = ov_read(&oggFile,buffer,OGG_BUFFER_SIZE,endian,2,1,&bitStream);
		data.insert(data.end(),buffer,buffer +bytes);
	}
	while(bytes > 0);
	ov_clear(&oggFile);
	return initialize_albuffer(path,format,rate,freq,channels,&data[0],static_cast<ALsizei>(data.size()),mode,audio);*/
	return false;
}
/*
static int ffmpeg_read_packet(void *userData,uint8_t *buf,int buf_size)
{
	VFilePtr &f = *static_cast<VFilePtr*>(userData);
	return CInt32(f->Read(&buf[0],buf_size));
}

static int64_t ffmpeg_seek(void *userData,int64_t offset,int whence)
{
	if(whence > SEEK_END)
		whence = SEEK_CUR;
	VFilePtr &f = *static_cast<VFilePtr*>(userData);
	f->Seek(offset,whence);
	return 0;
}
*/
#ifdef WEAVE_MP3_SUPPORT_ENABLED
extern DLLCLIENT LPALBUFFERSAMPLESSOFT alBufferSamplesSOFT;
extern DLLCLIENT LPALISBUFFERFORMATSUPPORTEDSOFT alIsBufferFormatSupportedSOFT;
bool openal::load_ffmpeg(std::string &path, VFilePtr f, ALChannel mode, ALAudio *audio)
{
	int err = alGetError();
	size_t datalen;
	void *data;
	FilePtr audiofile;
	StreamPtr sound;

	audiofile = openAVCustom(path.c_str(), &f, &ffmpeg_read_packet, NULL, &ffmpeg_seek);
	sound = getAVAudioStream(audiofile, 0);
	if(!sound) {
		Con::cwar << "Unable to load sound file '" << path << "'!" << Con::endl;
		return false;
	}
	ALuint rate;
	ALenum channels;
	ALenum type;
	if(getAVAudioInfo(sound, &rate, &channels, &type) != 0) {
		Con::cwar << "Unable to get audio info for '" << path << "'!" << Con::endl;
		closeAVFile(audiofile);
		return false;
	}

	ALenum format = GetFormat(channels, type, alIsBufferFormatSupportedSOFT);
	if(format == AL_NONE) {
		Con::cwar << "Unsupported format (" << ChannelsName(channels) << "," << TypeName(type) << ") for '" << path << "'!" << Con::endl;
		closeAVFile(audiofile);
		return false;
	}

	data = decodeAVAudioStream(sound, &datalen);
	if(!data) {
		Con::cwar << "Failed to read audio from '" << path << "'!" << Con::endl;
		closeAVFile(audiofile);
		return false;
	}

	bool bIsMonoFormat = (format == AL_MONO8_SOFT || format == AL_MONO16_SOFT || format == AL_MONO32F_SOFT) ? true : false;
	if(bIsMonoFormat) {
		if(audio->mono == NULL)
			audio->mono = new ALResource;
		if(mode == AL_CHANNEL_BOTH) {
			if(audio->stereo == NULL)
				audio->stereo = new ALResource;
			FillResource(audio->stereo, rate, channels, type, format);
			audio->stereo->buffer = 0;
		}
		if(audio->mono->buffer != -1) {
			free(data);
			closeAVFile(audiofile);
			return true;
		}
		FillResource(audio->mono, rate, channels, type, format);
		unsigned int alBuffer;
		alGenBuffers(1, &alBuffer);
		audio->mono->buffer = alBuffer;
		alBufferSamplesSOFT(audio->mono->buffer, audio->mono->rate, audio->mono->format, BytesToFrames(static_cast<ALsizei>(datalen), audio->mono->channels, audio->mono->type), audio->mono->channels, audio->mono->type, data);
		free(data);
		closeAVFile(audiofile);
		AL_CHECK_ERROR(audio->mono);
		return true;
	}
	if(mode != AL_CHANNEL_MONO) {
		if(audio->stereo == NULL)
			audio->stereo = new ALResource;
		if(audio->stereo->buffer == -1) {
			FillResource(audio->stereo, rate, channels, type, format);
			unsigned int alBuffer;
			alGenBuffers(1, &alBuffer);
			audio->stereo->buffer = alBuffer;
			alBufferSamplesSOFT(audio->stereo->buffer, audio->stereo->rate, audio->stereo->format, BytesToFrames(static_cast<ALsizei>(datalen), audio->stereo->channels, audio->stereo->type), audio->stereo->channels, audio->stereo->type, data);
			AL_CHECK_ERROR(audio->stereo);
		}
		if(mode == AL_CHANNEL_AUTO || (audio->mono != NULL && audio->mono->buffer > 0)) {
			free(data);
			closeAVFile(audiofile);
			return true;
		}
	}
	unsigned int fragmentSize;
	ALenum newFormat;
	if(audio->mono == NULL)
		audio->mono = new ALResource;
	char *monoData;
	if(format == AL_STEREO8_SOFT) {
		fragmentSize = 1;
		newFormat = AL_MONO8_SOFT;
		datalen = StereoToMono<char, short>(&((char *)data)[0], &monoData, CUInt32(datalen), fragmentSize);
	}
	else if(format == AL_STEREO16_SOFT) {
		fragmentSize = 2;
		newFormat = AL_MONO16_SOFT;
		datalen = StereoToMono<short, int>(&((char *)data)[0], &monoData, CUInt32(datalen), fragmentSize);
	}
	else if(format == AL_STEREO32F_SOFT) {
		fragmentSize = 4;
		newFormat = AL_MONO32F_SOFT;
		datalen = StereoToMono<int, long long>(&((char *)data)[0], &monoData, CUInt32(datalen), fragmentSize);
	}
	else // Attempt to convert it anyway
	{
		fragmentSize = 2;
		newFormat = AL_MONO16_SOFT;
		datalen = StereoToMono<short, int>(&((char *)data)[0], &monoData, CUInt32(datalen), fragmentSize);
	}

	FillResource(audio->mono, rate, channels, type, format);
	audio->mono->format = newFormat;
	audio->mono->channels = AL_MONO_SOFT;
	unsigned int alBuffer;
	alGenBuffers(1, &alBuffer);
	audio->mono->buffer = alBuffer;
	alBufferSamplesSOFT(audio->mono->buffer, audio->mono->rate, audio->mono->format, BytesToFrames(static_cast<ALsizei>(datalen), audio->mono->channels, audio->mono->type), audio->mono->channels, audio->mono->type, monoData);
	delete[] monoData;
	free(data);
	closeAVFile(audiofile);
	AL_CHECK_ERROR(audio->mono);
	return true;
}
#endif
