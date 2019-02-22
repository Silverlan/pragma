#ifndef __WISILKICON_H__
#define __WISILKICON_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/wiicon.h"

class DLLCLIENT WISilkIcon
	: public WIIcon
{
public:
	WISilkIcon();
	virtual ~WISilkIcon() override;
	virtual void Initialize() override;
	void SetIcon(std::string icon);
};

#endif