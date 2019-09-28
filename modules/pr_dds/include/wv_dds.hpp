#ifndef __WV_DDS_HPP__
#define __WV_DDS_HPP__

#include <string>
#include <functional>

namespace prosper {class Image;};
namespace util {class ImageBuffer;};
struct ImageWriteInfo;
namespace Lua
{
	class Interface;
	namespace dds
	{
		void register_lua_library(Lua::Interface &l);
		bool save_image(prosper::Image &image,const std::string &fileName,const ImageWriteInfo &ktxCreateInfo,const std::function<void(const std::string&)> &errorHandler=nullptr);
		bool save_image(
			const std::vector<std::vector<const void*>> &imgLayerMipmapData,uint32_t width,uint32_t height,const std::string &fileName,
			const ImageWriteInfo &ktxCreateInfo,const std::function<void(const std::string&)> &errorHandler=nullptr
		);
		bool save_image(
			util::ImageBuffer &imgBuffer,const std::string &fileName,const ImageWriteInfo &ktxCreateInfo,const std::function<void(const std::string&)> &errorHandler=nullptr
		);
	};
};

#endif
