// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
#include "stdafx.hpp"
#include "FtpRequest.h"

namespace Curl
{

	bool FtpRequest::OnPrePerform(CURL* curl)
	{
		assert(m_file == 0);
		m_file = ::fopen(m_filename.c_str(),  m_type == RequestDownload ? "w+" : "rb");

		if (m_type == RequestUpload)
		{
			fseek(m_file, 0, SEEK_END);
			long size = ftell(m_file);
			fseek(m_file, 0, SEEK_SET);

			curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
			curl_easy_setopt(curl, CURLOPT_INFILESIZE, size);

			EnableRead(curl);
		}
		else
			EnableWrite(curl);

		return m_file != 0;
	}

	void FtpRequest::OnPostPerform(CURL* curl, bool success)
	{
		if (m_file != 0)
		{
			::fclose(m_file);
			m_file = 0;
		}
	}

	long FtpRequest::OnReadData(unsigned char* data, size_t length)
	{
		assert(m_type == RequestUpload);
		return fread(data, 1, length, m_file);
	}

	void FtpRequest::OnWriteData(unsigned char* data, size_t length)
	{
		assert(m_type == RequestDownload);
		fwrite(data, 1, length, m_file);
	}

}