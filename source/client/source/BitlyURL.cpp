//
// BitlyURL.cpp - actual processing of FTP destinations
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//

#include "stdafx.hpp"
#include "resource.h"

#include "libcc/winapi.hpp"

// ui
#include "destination.hpp"
#include "StatusDlg.hpp"

// general
#include "clipboard.hpp"
#include "codec.hpp"
#include "image.hpp"
#include "internet.hpp"
#include "path.hpp"
#include "utility.hpp"

#include <curl/curl.h>
#include <curl/easy.h>
#include "curlutil.h"

#include "jsonxx/jsonxx.h"
#include "BitlyURL.h"

const TCHAR SCREENIE_BITLY_API_KEY[] = _T("R_93be59f96ebf75654a2cf1956e0566af");

using namespace jsonxx;

bool BitlyShortenURL(DestinationArgs& args, const tstd::tstring& strLongURL, BitlyShortenInfo& info)
{
	std::string strLongURL8 = LibCC::ToUTF8(strLongURL);

	EventID msgid = args.statusDlg.RegisterEvent(args.screenshotID, EI_PROGRESS, ET_SHORTENURL, args.dest.general.name, _T("Initiating bit.ly URL shortening"));
	args.statusDlg.EventSetProgress(msgid, 0, 1);// set it to 0%

	ScreenieHttpRequest request(&args.statusDlg, msgid);
	request.AddHeader("Expect: ");

	std::string strURL = LibCC::FormatA("http://api.bit.ly/shorten?version=2.0.1&longUrl=%&login=screenie&apiKey=%")
		.s(strLongURL8).s(LibCC::ToUTF8(SCREENIE_BITLY_API_KEY)).Str();

	request.SetURL(strURL);

	if (request.Perform())
	{
		std::string strJSON = (const char*)request.GetData();
		std::istringstream stream(strJSON);

		Object o;
		if (o.parse(stream))
		{
			if (o.has<Object>("results"))
			{
				Object& results = o.get<Object>("results");
				if (results.has<Object>(strLongURL8))
				{
					Object& url = results.get<Object>(strLongURL8);
					if (url.has<std::string>("shortUrl"))
					{
						std::string strShortURL = url.get<std::string>("shortUrl");
						info.shortURL = LibCC::ToUTF16(strShortURL);
						return true;
					}
				}
			}
		}

		return true;
	}

	return false;
}
