// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
#include "stdafx.hpp"

#include <curl/curl.h>
#include <curl/easy.h>

struct CurlInit
{
	CurlInit()
	{
		// only do this once per instance of the application
		curl_global_init(CURL_GLOBAL_ALL);
	}
};
static CurlInit g_curlinit;

namespace Curl
{

}