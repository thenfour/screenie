// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
#ifndef CURLUTIL_H
#define CURLUTIL_H

#include "FtpRequest.h"
#include "HttpRequest.h"

class ScreenieFtpRequest : public Curl::FtpRequest
{
public:
	ScreenieFtpRequest(IActivity* status, EventID msgid)
		: m_status(status), m_msgid(msgid), Curl::FtpRequest(Curl::FtpRequest::RequestUpload)
	{
	}
private:
	IActivity* m_status;
	EventID m_msgid;
public:
	bool OnPrePerform(CURL* curl)
	{
		this->EnableProgress(curl);
		return Curl::FtpRequest::OnPrePerform(curl);
	}
	bool OnProgress(double dltotal, double dlnow, double ultotal, double ulnow, bool canceled)
	{
		m_status->EventSetProgress(m_msgid, static_cast<int>(ulnow), static_cast<int>(ultotal));
		m_status->EventSetText(m_msgid, LibCC::Format(L"Uploading % of % bytes").ul((unsigned long)ulnow).ul((unsigned long)ultotal).Str());

		return false;
	}
};

class ScreenieHttpRequest : public Curl::StreamHttpRequest<true>
{
public:
	ScreenieHttpRequest(IActivity* status, EventID msgid)
		: m_status(status), m_msgid(msgid), Curl::StreamHttpRequest<true>(Curl::StreamHttpRequest<true>::MethodPOST)
	{
	}
private:
	IActivity* m_status;
	EventID m_msgid;
public:
	bool OnPrePerform(CURL* curl)
	{
		this->EnableProgress(curl);
		return Curl::StreamHttpRequest<true>::OnPrePerform(curl);
	}
	bool OnProgress(double dltotal, double dlnow, double ultotal, double ulnow, bool canceled)
	{
		m_status->EventSetProgress(m_msgid, static_cast<int>(ulnow), static_cast<int>(ultotal));
		m_status->EventSetText(m_msgid, LibCC::Format(L"Uploading % of % bytes").ul((unsigned long)ulnow).ul((unsigned long)ultotal).Str());

		return false;
	}
};

#endif