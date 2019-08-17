#ifndef __WV_DDS_HPP__
#define __WV_DDS_HPP__

#include <string>
#include <functional>

namespace Lua
{
	class Interface;
	namespace dds
	{
		void register_lua_library(Lua::Interface &l);
	};
};

namespace prosper {class Image;};
struct ImageWriteInfo;
bool save_prosper_image_as_ktx(prosper::Image &image,const std::string &fileName,const ImageWriteInfo &ktxCreateInfo,const std::function<void(const std::string&)> &errorHandler=nullptr);

#endif
