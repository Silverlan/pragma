#ifndef __WIKEYENTRY_H__
#define __WIKEYENTRY_H__

#include "pragma/clientdefinitions.h"
#include <wgui/types/witextentry.h>
#include <unordered_map>

class DLLCLIENT WIKeyEntry
	: public WITextEntryBase
{
protected:
	std::string m_previousKey;
	GLFW::Key m_key;
	bool m_bKeyPressed;
	WIHandle m_hMouseTrap;
	virtual void OnTextChanged(const std::string &text) override;
	void ApplyKey(GLFW::Key key);
public:
	WIKeyEntry();
	virtual ~WIKeyEntry() override;
	virtual void Initialize() override;
	virtual void SetSize(int x,int y) override;
	virtual util::EventReply KeyboardCallback(GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods) override;
	virtual util::EventReply ScrollCallback(Vector2 offset) override;
	virtual util::EventReply CharCallback(unsigned int c,GLFW::Modifier mods=GLFW::Modifier::None) override;
	virtual void OnFocusGained() override;
	virtual void OnFocusKilled() override;
	void SetKey(GLFW::Key key);
	GLFW::Key GetKey() const;
};

#endif