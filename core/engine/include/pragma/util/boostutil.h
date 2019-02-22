#ifndef __BOOSTUTIL_H__
#define __BOOSTUTIL_H__
// Sockets
#include <deque>
#pragma warning(disable: 4005)
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#pragma warning(default: 4005)

using boost::asio::ip::tcp;
using boost::asio::ip::udp;
//
#endif