#ifndef __SOUND_OGG_H__
#define __SOUND_OGG_H__

#include "pragma/networkdefinitions.h"

static const unsigned int OGG_BUFFER_SIZE = 32768;

class VFilePtrInternal;
struct vorbis_info;
struct OggVorbis_File;
DLLNETWORK void GetOGGFileData(std::shared_ptr<VFilePtrInternal> f,vorbis_info **pInfo,OggVorbis_File &oggFile);
DLLNETWORK float GetOGGSoundDuration(std::shared_ptr<VFilePtrInternal> f);

#endif