#ifndef __WAD_H__
#define __WAD_H__
#include "pragma/networkdefinitions.h"
#include "pragma/file_formats/wdf.h"
#include "pragma/model/animation/animation.h"

// Currently unused
//#define WAD_VERSION 0x0001

class DLLNETWORK FWAD
	: FWDF
{
private:
	
public:
	std::shared_ptr<Animation> ReadData(unsigned short version,VFilePtr f);
	std::shared_ptr<Animation> Load(unsigned short version,const char *animation);
};

#endif