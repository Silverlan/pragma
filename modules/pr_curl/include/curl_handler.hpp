#ifndef __CURL_HANDLER_HPP__
#define __CURL_HANDLER_HPP__

#include <cinttypes>
#include <string>
#include <functional>
#include <memory>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>

__declspec(dllexport) class CurlHandler
{
public:
	enum class ResultCode : uint32_t
	{
		Ok = 0,
		BadHandle,
		BadEasyHandle,
		OutOfMemory,
		InternalError,
		BadSocket,
		UnknownOption,
		AddedAlready
	};
	CurlHandler();
	~CurlHandler();
	void AddResource(const std::string &fname,const std::function<size_t(void*,size_t,size_t)> &callback,const std::shared_ptr<void> &userData=nullptr,const std::function<void(int32_t)> &onComplete=nullptr);
	void AddRequest(
		const std::string &url,
		const std::unordered_map<std::string,std::string> &postValues,
		const std::function<void(int32_t,const std::string&)> &onComplete=nullptr,
		const std::function<void(int64_t,int64_t,int64_t,int64_t)> &progressCallback=nullptr
	);
	void StartDownload();
	void CancelDownload();
	bool IsComplete() const;
	void SetErrorHandler(const std::function<void(ResultCode)> &f);
private:
	void *m_multiCurl = nullptr;

	struct Request
	{
		struct Header
		{
			std::string fileName;
		} header;
		std::string url;
		std::function<size_t(void*,size_t,size_t)> writeCallback = nullptr; // Write Callback
		std::function<void(Request&,int32_t)> completeCallback = nullptr;
		std::function<void(int64_t,int64_t,int64_t,int64_t)> progressCallback = nullptr;
		std::shared_ptr<void> userData = nullptr;
		CurlHandler *curl = nullptr;
		void *handle = nullptr;
		std::function<void(void*)> request = nullptr;
	};

	std::queue<std::shared_ptr<Request>> m_queuedRequests;
	std::vector<std::shared_ptr<Request>> m_activeRequests;
	std::mutex m_resourceMutex;
	std::mutex m_requestMutex;
	std::unique_ptr<std::thread> m_thread = nullptr;
	std::atomic<bool> m_bComplete = true;
	std::atomic<bool> m_bCancel = false;
	std::atomic<bool> m_bThreadActive = false;
	std::function<void(ResultCode)> m_errorHandler = nullptr;

	void ProcessNextRequest();
	uint32_t ProcessAllRequests();

	void AddRequest(
		const std::string &url,
		const std::function<size_t(void*,size_t,size_t)> &writeCallback,
		const std::function<void(int32_t,const std::string&)> &onComplete,
		const std::function<void(int64_t,int64_t,int64_t,int64_t)> &progressCallback,
		const std::function<void(Request*,void*)> &fRequest
	);
	static size_t ReceiveHeader(char *buffer,size_t size,size_t nitems,void *userdata);
	static void InitializeCurl(void *curl,Request *request);
	static size_t WriteData(void *ptr,size_t size,size_t nmemb,void *userData);
	static int32_t UpdateProgress(void *clientp,int64_t dltotal,int64_t dlnow,int64_t ultotal,int64_t ulnow);
	static std::string ResultCodeToString(ResultCode ec);
};

#endif
