#ifndef CURL_HTTPREQUEST_H
#define CURL_HTTPREQUEST_H

#include "CurlRequest.h"
#include <vector>

namespace Curl
{

	class HttpRequest : public CurlRequest
	{
	public:
		enum RequestMethod
		{
			MethodPOST,
			MethodGET
		};

		HttpRequest(RequestMethod method);

		void AddFile(const std::string& name, const std::string& localfilename,
			const std::string& contenttype = "application/octet-stream", const std::string& filename = "");
		void AddPostArgument(const std::string& name, const std::string& value);
		void AddHeader(const std::string& headertext);
	protected:
		bool OnPrePerform(CURL* curl);

		void OnPostPerform(CURL* curl, bool success);
	private:
		RequestMethod m_method;
		bool m_upload;
		curl_httppost* m_postargs;
		curl_httppost* m_postargslast;
		curl_slist* m_headers;
	};

	template<bool bNullTerminate = true>
	class StreamHttpRequest : public HttpRequest
	{
	public:
		StreamHttpRequest(HttpRequest::RequestMethod method) : HttpRequest(method) { }

		const unsigned char* GetData() const { return &m_buffer[0]; }
	protected:
		void OnPostPerform(CURL* curl, bool success)
		{
			HttpRequest::OnPostPerform(curl, success);

			if (success && bNullTerminate)
				m_buffer.push_back(0);
		}

		void OnWriteData(unsigned char* data, size_t length)
		{
			m_buffer.insert(m_buffer.end(), data, data + length);
		}

		long OnReadData(unsigned char* data, size_t length)
		{
			// blah
			assert(false);
			return 0;
		}
	private:
		std::vector<unsigned char> m_buffer;
	};

} // namespace Curl

#endif