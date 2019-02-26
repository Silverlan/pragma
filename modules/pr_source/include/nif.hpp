#ifndef __NIF_HPP__
#define __NIF_HPP__

#include <memory>
#include <string>

class NetworkState;
class Model;
namespace import
{
	bool load_nif(NetworkState *nw,std::shared_ptr<::Model> &mdl,const std::string &fpath);
};

#endif
