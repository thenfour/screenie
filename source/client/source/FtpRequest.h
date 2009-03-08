// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
#ifndef CURL_FTPREQUEST_H
#define CURL_FTPREQUEST_H

#include "CurlRequest.h"
#include <stdio.h>

namespace Curl
{

	class FtpRequest : public CurlRequest
	{
	public:
		enum RequestType
		{
			RequestUpload,
			RequestDownload
		};

		FtpRequest(RequestType type) : m_type(type), m_file(0) { }

		void SetFilename(const std::string& filename) { m_filename = filename; }
		std::string GetFilename() const { return m_filename; }
	protected:
		bool OnPrePerform(CURL* curl);

		void OnPostPerform(CURL* curl, bool success);

		long OnReadData(unsigned char* data, size_t length);

		void OnWriteData(unsigned char* data, size_t length);
	private:
		RequestType m_type;
		std::string m_filename;
		FILE* m_file;
	};

} // namespace Curl

#endif