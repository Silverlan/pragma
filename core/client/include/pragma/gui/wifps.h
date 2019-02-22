#ifndef __WIFPS_H__
#define __WIFPS_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>

class DLLCLIENT WIFPS
	: public WIBase
{
private:
	WIHandle m_text;
	uint32_t m_fpsLast;
	double m_tLastUpdate;
public:
	WIFPS();
	virtual ~WIFPS() override;
	virtual void Initialize() override;
	virtual void Think() override;
};

#endif