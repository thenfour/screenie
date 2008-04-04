#include "stdafx.hpp"
#include "CurlRequest.h"

namespace Curl
{
	CurlRequest::CurlRequest()
		: m_cancelstate(CancelStateNone), m_performstate(PerformStateNone), m_errorcode(CURLE_OK)
	{

	}

	bool CurlRequest::Perform()
	{
		CURL* curl = curl_easy_init();
		if (!this->OnPrePerform(curl))
		{
			this->OnProgress(0.0, 0.0, 0.0, 0.0, true);

			m_performstate = PerformStateFailed;
			m_cancelstate = CancelStateCanceled;

			m_errortext.clear();
			m_errorcode = CURLE_ABORTED_BY_CALLBACK;

			return false;
		}

		bool success = false;


		char errorbuf[CURL_ERROR_SIZE] = { 0 };
		curl_easy_setopt(curl, CURLOPT_URL, m_url.c_str());

		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorbuf);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, static_cast<void*>(this));
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void*>(this));
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadCallback);
		curl_easy_setopt(curl, CURLOPT_READDATA, static_cast<void*>(this));

		std::string userpwd = m_username;
		if (!userpwd.empty())
		{
			if (!m_password.empty())
			{
				userpwd += ":";
				userpwd += m_password;
			}
			curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str());
		}

		switch (m_errorcode = curl_easy_perform(curl))
		{
		case CURLE_OK:
			success = true;

			m_performstate = PerformStateSuccess;
			m_cancelstate = CancelStateNone;

			m_errortext.clear();

			//////////////////////////////////////////////////////////////////////////
			double uploaded;
			curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD, &uploaded);
			m_uploadSize = static_cast<size_t>(uploaded);

			double uploadspeed;
			curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &uploadspeed);
			m_uploadSpeed = uploadspeed;
			//////////////////////////////////////////////////////////////////////////

			break;
		case CURLE_ABORTED_BY_CALLBACK:

			success = false;

			m_performstate = PerformStateFailed;
			m_cancelstate = CancelStateCanceled;

			m_errortext.clear();

			break;
		default:
			success = false;

			m_performstate = PerformStateFailed;
			m_cancelstate = CancelStateNone;					

			m_errortext = errorbuf;

			break;
		}

		this->OnPostPerform(curl, success);
		curl_easy_cleanup(curl);

		return success;
	}

	size_t CurlRequest::ReadCallback(void* ptr, size_t size, size_t nmemb, void* userdata)
	{
		CurlRequest* pRequest = static_cast<CurlRequest*>(userdata);

		size_t datasize = nmemb * size;
		return pRequest->OnReadData(reinterpret_cast<unsigned char*>(ptr), datasize);
	}

	size_t CurlRequest::WriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata)
	{
		CurlRequest* pRequest = static_cast<CurlRequest*>(userdata);

		size_t datasize = nmemb * size;
		pRequest->OnWriteData(reinterpret_cast<unsigned char*>(ptr), datasize);

		return datasize;
	}

	int CurlRequest::ProgressCallback(void* userdata, double dltotal, double dlnow, double ultotal, double ulnow)
	{
		CurlRequest* pRequest = static_cast<CurlRequest*>(userdata);

		int retval = 0;
		if (pRequest->OnProgress(dltotal, dlnow, ultotal, ulnow, pRequest->m_cancelstate == CancelStateCanceling))
		{
			switch (pRequest->m_cancelstate)
			{
			case CancelStateNone:
				pRequest->m_cancelstate = CancelStateCanceling;
				retval = 1;
				break;
			case CancelStateCanceling:
				retval = 1;
				break;
			}
		}
		else
		{
			if (pRequest->m_cancelstate == CancelStatePending)
			{
				pRequest->m_cancelstate = CancelStateCanceling;
				retval = 1;
			}
		}

		return retval;
	}

}