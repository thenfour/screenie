#ifndef BITLYURL_H
#define BITLYURL_H

struct BitlyShortenInfo
{
	int nErrorCode;
	tstd::tstring errorMessage;
	tstd::tstring hash;
	tstd::tstring shortURL;
	tstd::tstring userHash;
};

bool BitlyShortenURL(DestinationArgs& args, const tstd::tstring& strLongURL, BitlyShortenInfo& info);

#endif
