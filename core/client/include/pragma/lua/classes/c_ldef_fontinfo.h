#ifndef __C_LDEF_FONTINFO_H__
#define __C_LDEF_FONTINFO_H__
#include <pragma/lua/ldefinitions.h>

lua_registercheck(Font,std::shared_ptr<const FontInfo>);
#endif