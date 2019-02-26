#include "curl_handler.hpp"
#include <curl/curl.h>
#include <pragma_module.hpp>

CurlHandler::CurlHandler()
{}

CurlHandler::~CurlHandler()
{
	CancelDownload();
	if(m_thread != nullptr)
		m_thread->join();
}

void CurlHandler::SetErrorHandler(const std::function<void(ResultCode)> &f) {m_errorHandler = f;}

size_t CurlHandler::WriteData(void *ptr,size_t size,size_t nmemb,void *clientp)
{
	auto &res = *static_cast<Request*>(clientp);
	return res.writeCallback(ptr,size,nmemb);
}

int32_t CurlHandler::UpdateProgress(void *clientp,int64_t dltotal,int64_t dlnow,int64_t ultotal,int64_t ulnow)
{
	auto &res = *static_cast<Request*>(clientp);
	if(res.progressCallback == nullptr)
		return 0;
	res.progressCallback(dltotal,dlnow,ultotal,ulnow);
	return 0;
}

std::string CurlHandler::ResultCodeToString(ResultCode ec) {return curl_easy_strerror(static_cast<CURLcode>(ec));}

void CurlHandler::ProcessNextRequest()
{
	auto &req = m_queuedRequests.front();
	m_activeRequests.push_back(req);
	req->request(m_multiCurl);
	m_queuedRequests.pop();
}

uint32_t CurlHandler::ProcessAllRequests()
{
	uint32_t count = 0;
	m_requestMutex.lock();
		m_activeRequests.reserve(m_activeRequests.size() +m_queuedRequests.size());
		while(m_queuedRequests.empty() == false)
		{
			ProcessNextRequest();
			++count;
		}
	m_requestMutex.unlock();
	return count;
}

void CurlHandler::StartDownload()
{
	if(m_bThreadActive == true)
	{
		if(m_bCancel == true)
		{
			if(m_thread != nullptr)
			{
				m_thread->join(); // Wait for thread to complete cancelling
				m_thread = nullptr;
			}
		}
		else
			return;
	}
	m_bCancel = false;
	m_bComplete = false;
	m_bThreadActive = true;
	if(m_thread != nullptr)
	{
		m_thread->join();
		m_thread = nullptr;
	}
	m_thread = std::make_unique<std::thread>([this]() {
		m_multiCurl = curl_multi_init();
		ProcessAllRequests();

		int32_t handleCount;
		auto code = curl_multi_perform(m_multiCurl,&handleCount);
		auto repeats = 0u;
		while(m_bComplete == false && m_bCancel == false)
		{
			auto count = ProcessAllRequests();
			if(handleCount == 0 && count > 0)
				code = curl_multi_perform(m_multiCurl,&handleCount);
			while(code == CURLE_OK && handleCount > 0 && m_bCancel == false)
			{
				int32_t numfds;
				auto mc = curl_multi_wait(m_multiCurl,nullptr,0,1'000,&numfds);
				if(mc != CURLM_OK)
				{
					if(m_errorHandler != nullptr)
						m_errorHandler(static_cast<ResultCode>(mc));
					break;
				}
 
				if(numfds == 0)
				{
					repeats++;
					if(repeats > 1)
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				else
					repeats = 0;
				code = curl_multi_perform(m_multiCurl,&handleCount);

				CURLMsg *msg = nullptr;
				int32_t msgsLeft = 0;
				m_resourceMutex.lock();
				while((msg = curl_multi_info_read(m_multiCurl,&msgsLeft)) != nullptr)
				{
					if(msg->msg == CURLMSG_DONE)
					{
						auto *curl = msg->easy_handle;

						auto returnCode = msg->data.result;
						auto it = std::find_if(m_activeRequests.begin(),m_activeRequests.end(),[curl](const std::shared_ptr<CurlHandler::Request> &request) {
							return (request->handle == curl) ? true : false;
						});
						if(it != m_activeRequests.end())
						{
							auto &request = *it;
							if(request->completeCallback != nullptr)
								request->completeCallback(*request,returnCode);
							m_activeRequests.erase(it);
						}

						curl_multi_remove_handle(m_multiCurl,curl);
						curl_easy_cleanup(curl);
					}
				}
				m_resourceMutex.unlock();
			}
		}

		m_resourceMutex.lock();
			for(auto &req : m_activeRequests)
			{
				curl_multi_remove_handle(m_multiCurl,req->handle);
				curl_easy_cleanup(req->handle);
			}
			curl_multi_cleanup(m_multiCurl);
			m_activeRequests.clear();
		m_resourceMutex.unlock();

		m_bComplete = true;
		m_bThreadActive = false;
	});
}

void CurlHandler::CancelDownload() {m_bCancel = true;}

bool CurlHandler::IsComplete() const {return m_bComplete;}

void CurlHandler::AddRequest(
	const std::string &url,
	const std::function<size_t(void*,size_t,size_t)> &writeCallback,
	const std::function<void(int32_t,const std::string&)> &onComplete,
	const std::function<void(int64_t,int64_t,int64_t,int64_t)> &progressCallback,
	const std::function<void(Request*,void*)> &fRequest
)
{
	m_requestMutex.lock();
		m_queuedRequests.push(std::shared_ptr<Request>(new Request()));
		auto &req = m_queuedRequests.back();
		req->url = url;
		req->curl = this;
		req->writeCallback = writeCallback;
		req->completeCallback = [onComplete](Request &request,int32_t code) {
			if(onComplete == nullptr)
				return;
			onComplete(code,request.header.fileName);
		};
		req->progressCallback = progressCallback;
		req->request = std::bind(fRequest,req.get(),std::placeholders::_1);
	m_requestMutex.unlock();
}

size_t CurlHandler::ReceiveHeader(char *buffer,size_t size,size_t nitems,void *userdata)
{
	auto line = std::string(buffer,size *nitems);
	if(line.substr(0,20) == "Content-Disposition:")
	{
		auto fnameStart = line.find("filename=");
		if(fnameStart != std::string::npos)
		{
			fnameStart = line.find_first_of("\"'",fnameStart +1);
			if(fnameStart != std::string::npos)
			{
				auto fnameEnd = line.find_first_of("\"'",fnameStart +1);
				if(fnameEnd != std::string::npos)
				{
					auto &res = *static_cast<Request*>(userdata);
					res.header.fileName = line.substr(fnameStart +1,fnameEnd -fnameStart -1);
				}
			}
		}
	}
	return size *nitems;
}

void CurlHandler::InitializeCurl(void *curl,Request *request)
{
	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,false);

	curl_easy_setopt(curl,CURLOPT_HEADERFUNCTION,CurlHandler::ReceiveHeader);
	curl_easy_setopt(curl,CURLOPT_HEADERDATA,request);

	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,CurlHandler::WriteData);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,request);

	curl_easy_setopt(curl,CURLOPT_URL,request->url.c_str());

	if(request->progressCallback != nullptr)
	{
		curl_easy_setopt(curl,CURLOPT_NOPROGRESS,false);
		curl_easy_setopt(curl,CURLOPT_XFERINFOFUNCTION,CurlHandler::UpdateProgress);
		curl_easy_setopt(curl,CURLOPT_XFERINFODATA,request);
	}
}

void CurlHandler::AddResource(const std::string &url,const std::function<size_t(void*,size_t,size_t)> &callback,const std::shared_ptr<void> &userData,const std::function<void(int32_t)> &onComplete)
{
	AddRequest(url,callback,std::bind(onComplete,std::placeholders::_1),[](int64_t dltotal,int64_t dlnow,int64_t ultotal,int64_t ulnow) {
		
	},[userData](Request *pReq,void *multiCurl) {
		pReq->userData = userData;
		auto *curl = curl_easy_init();
		if(curl != nullptr)
		{
			InitializeCurl(curl,pReq);

			curl_multi_add_handle(multiCurl,curl);
			pReq->handle = curl;
		}
	});
}

void CurlHandler::AddRequest(
	const std::string &url,
	const std::unordered_map<std::string,std::string> &postValues,
	const std::function<void(int32_t,const std::string&)> &onComplete,
	const std::function<void(int64_t,int64_t,int64_t,int64_t)> &progressCallback
)
{
	std::string post;
	auto bFirst = true;
	for(auto &pair : postValues)
	{
		if(bFirst == false)
			post += '&';
		else
			bFirst = false;
		post += pair.first +'=' +pair.second;
	}
	auto response = std::make_shared<std::string>();
	AddRequest(url,[response](void *ptr,size_t size,size_t nmemb) -> size_t {
		auto offset = response->size();
		response->resize(offset +size *nmemb);
		memcpy(&(*response)[offset],ptr,size *nmemb);
		return size *nmemb;
	},[response,onComplete](int32_t code,const std::string &fileName) {
		onComplete(code,*response);
	},[progressCallback](int64_t dltotal,int64_t dlnow,int64_t ultotal,int64_t ulnow) {
		if(progressCallback == nullptr)
			return;
		progressCallback(dltotal,dlnow,ultotal,ulnow);
	},[post](Request *pReq,void *multiCurl) {
		auto *curl = curl_easy_init();
		if(curl != nullptr)
		{
			InitializeCurl(curl,pReq);
			//curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER ,1);
			//curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST ,1);
			//curl_easy_setopt(curl,CURLOPT_CAINFO,"./ca.cert");

			curl_easy_setopt(curl,CURLOPT_POSTFIELDS,post.c_str());

			curl_multi_add_handle(multiCurl,curl);
			pReq->handle = curl;
		}
	});
}

extern "C" {
	PRAGMA_EXPORT void *mcd_create() {return new CurlHandler();}
	PRAGMA_EXPORT void mcd_release(void *cd) {delete static_cast<CurlHandler*>(cd);}
	PRAGMA_EXPORT void mcd_add_resource(void *cd,const std::string &fname,const std::function<size_t(void*,size_t,size_t)> &callback,const std::shared_ptr<void> &userData,const std::function<void(int32_t)> &onComplete) {static_cast<CurlHandler*>(cd)->AddResource(fname,callback,userData,onComplete);}
	PRAGMA_EXPORT void mcd_start_download(void *cd) {static_cast<CurlHandler*>(cd)->StartDownload();}
	PRAGMA_EXPORT void mcd_cancel_download(void *cd) {static_cast<CurlHandler*>(cd)->CancelDownload();}
	PRAGMA_EXPORT bool mcd_is_complete(void *cd) {return static_cast<CurlHandler*>(cd)->IsComplete();}
	PRAGMA_EXPORT void mcd_set_error_handler(void *cd,const std::function<void(uint32_t)> &f) {static_cast<CurlHandler*>(cd)->SetErrorHandler(reinterpret_cast<const std::function<void(CurlHandler::ResultCode)>&>(f));}
	PRAGMA_EXPORT void mcd_send_request(void *cd,const std::string &url,const std::unordered_map<std::string,std::string> &post,const std::function<void(int32_t,const std::string&)> &onComplete,const std::function<void(int64_t,int64_t,int64_t,int64_t)> &progressCallback) {static_cast<CurlHandler*>(cd)->AddRequest(url,post,onComplete,progressCallback);}
	PRAGMA_EXPORT std::string mcd_code_to_string(int32_t code) {return curl_easy_strerror(static_cast<CURLcode>(code));}
	PRAGMA_EXPORT bool mcd_is_error_code(int32_t code) {return code != CURLE_OK;}
};
