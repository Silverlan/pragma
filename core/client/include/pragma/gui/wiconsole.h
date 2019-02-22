#ifndef __WICONSOLE_H__
#define __WICONSOLE_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>
#include <queue>

class WITextEntry;
class DLLCLIENT WIConsole
	: public WIBase
{
protected:
	WIHandle m_hBg;
	WIHandle m_hTextArea;
	WIHandle m_hTextEntry;
	void OnTextEntered();
public:
	WIConsole();
	virtual ~WIConsole() override;
	virtual void Initialize() override;
	virtual void SetSize(int x,int y) override;
	void Print(const char *str);
	void Print(const std::string &str);
};

#endif