//
// codec.hpp - utilities for dealing with GDI+ image codecs
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//

#ifndef SCREENIE_CODEC_HPP
#define SCREENIE_CODEC_HPP

#include "tstdlib/tstring.hpp"

// this really needs to be redone as a pseudocontainer with iterators;
// the GDI+ mechanism for dealing with these codecs is just disgusting.

// TODO: optimize this with static/reference-counted member variables.
//       this gets instantiated quite often, and it probably shouldn't be.

class ImageCodecsEnum
{
public:
	ImageCodecsEnum();
	~ImageCodecsEnum();

	Gdiplus::ImageCodecInfo* GetCodec(UINT codec) const;
	Gdiplus::ImageCodecInfo* GetCodecByMimeType(PCTSTR mimeType) const;
	Gdiplus::ImageCodecInfo* GetCodecByDescription(PCTSTR description) const;
	Gdiplus::ImageCodecInfo* GetCodecByExtension(PCTSTR extension) const;

	UINT GetNumCodecs() const { return m_numCodecs; }

	static bool SupportsQualitySetting(Gdiplus::ImageCodecInfo* p);
	static bool SupportsQualitySetting(const std::wstring& description);
	static bool SupportsQualitySettingByMimeType(const std::wstring& m);

private:
	UINT m_numCodecs;
	Gdiplus::ImageCodecInfo* m_imageCodecs;
};

#endif