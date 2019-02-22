#ifndef __WILUASKIN_H__
#define __WILUASKIN_H__

#include "pragma/clientdefinitions.h"
#include "wgui/wiskin.h"
#include "luasystem.h"

struct DLLCLIENT WISkinClass
{
	WISkinClass(lua_State *l);
	~WISkinClass();
	std::unordered_map<std::string,WISkinClass*> classes;
	int referenceInitialize;
	int referenceRelease;
	lua_State *lua;
	WISkinClass *Copy();
};

class DLLCLIENT WILuaSkin
	: public WISkin
{
public:
	struct Settings
	{
		Settings()
			: vars(-1),skin(-1),base(nullptr)
		{}
		int vars;
		int skin;
		WILuaSkin *base;
	};
protected:
	lua_State *m_lua;
	WISkinClass m_rootClass;
	int m_refVars;
	void InitializeClasses(WISkinClass &cl);
	void InitializeClasses();
	void FindSkinClasses(WIBase *el,std::unordered_map<std::string,WISkinClass*> &classes,std::vector<WISkinClass*> &outClasses);
	WISkinClass *FindSkinClass(const std::string &className,std::unordered_map<std::string,WISkinClass*> &classes);
	
	void InitializeBase(WILuaSkin *base);
	void InitializeBaseClass(WISkinClass &base,WISkinClass &cl);
public:
	WILuaSkin(std::string id);
	virtual ~WILuaSkin() override;
	virtual void Release(WIBase *el) override;
	virtual void Initialize(WIBase *el) override;
	void Initialize(lua_State *l,Settings &settings);
};

#endif