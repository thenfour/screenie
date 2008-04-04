#include "stdafx.hpp"
#include "HttpRequest.h"

namespace Curl
{

	HttpRequest::HttpRequest(RequestMethod method)
		: m_method(method), m_upload(false), m_postargs(0), m_postargslast(0), m_headers(0)
	{

	}

	void HttpRequest::AddFile(const std::string& name, const std::string& localfilename,
		const std::string& contenttype, const std::string& filename)
	{
		curl_formadd(&m_postargs, &m_postargslast,
			CURLFORM_COPYNAME, name.c_str(),
			CURLFORM_FILE, localfilename.c_str(),
			CURLFORM_CONTENTTYPE, contenttype.c_str(),
			CURLFORM_END);

		m_upload = true;
	}

	void HttpRequest::AddPostArgument(const std::string& name, const std::string& value)
	{
		curl_formadd(&m_postargs, &m_postargslast, CURLFORM_COPYNAME, name.c_str(),
			CURLFORM_COPYCONTENTS, value.c_str(), CURLFORM_END);
	}

	void HttpRequest::AddHeader(const std::string& headertext)
	{
		m_headers = curl_slist_append(m_headers, headertext.c_str());
	}

	bool HttpRequest::OnPrePerform(CURL* curl)
	{
//		if (m_upload)
//			curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);

		if (m_method == MethodPOST)
		{
			if (m_postargs != 0)
				curl_easy_setopt(curl, CURLOPT_HTTPPOST, m_postargs);
		}

		if (m_headers != 0)
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, m_headers);

		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

		EnableWrite(curl);

		return true;
	}

	void HttpRequest::OnPostPerform(CURL* curl, bool success)
	{
		if (m_postargs != 0)
			curl_formfree(m_postargs);

		if (m_headers != 0)
			curl_slist_free_all(m_headers);
	}

}