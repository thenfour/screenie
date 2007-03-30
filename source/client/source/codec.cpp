//
// codec.cpp - implementation for the image codec enumerator
// Copyright (c) 2003 Carl Corcoran
// Copyright (c) 2005 Roger Clark
//

#include "stdafx.hpp"

#include "codec.hpp"

ImageCodecsEnum::ImageCodecsEnum()
	: m_imageCodecs(0), m_numCodecs(0)
{
	UINT arraySize = 0;
	UINT numCodecs = 0;

	if (Gdiplus::GetImageEncodersSize(&numCodecs, &arraySize) == Gdiplus::Ok)
	{
		Gdiplus::ImageCodecInfo* imageCodecs = (Gdiplus::ImageCodecInfo*)::HeapAlloc(::GetProcessHeap(),
			HEAP_ZERO_MEMORY, arraySize);

		if (imageCodecs != NULL)
		{
			if (Gdiplus::GetImageEncoders(numCodecs, arraySize, imageCodecs) == Gdiplus::Ok)
			{
				m_imageCodecs = imageCodecs;
				m_numCodecs = numCodecs;
			}
			else
			{
				::HeapFree(::GetProcessHeap(), 0, imageCodecs);
			}
		}
	}
}

ImageCodecsEnum::~ImageCodecsEnum()
{
	if (m_imageCodecs != 0)
		::HeapFree(::GetProcessHeap(), 0, m_imageCodecs);
}

Gdiplus::ImageCodecInfo* ImageCodecsEnum::GetCodec(UINT codec) const
{
	if ((codec >= m_numCodecs) || (m_imageCodecs == 0))
		return 0;

	return &m_imageCodecs[codec];
}

Gdiplus::ImageCodecInfo* ImageCodecsEnum::GetCodecByMimeType(PCTSTR mimeType) const
{
	std::wstring wideMimeType = LibCC::ToUnicode(tstd::tstring(mimeType));

	for (UINT i = 0; i < GetNumCodecs(); ++i)
	{
		if (wideMimeType == m_imageCodecs[i].MimeType)
			return &m_imageCodecs[i];
	}

	return NULL;
}

Gdiplus::ImageCodecInfo* ImageCodecsEnum::GetCodecByDescription(PCTSTR description) const
{
	std::wstring wideDescription = LibCC::ToUnicode(tstd::tstring(description));

	for (UINT i = 0; i < GetNumCodecs(); ++i)
	{
		if (wideDescription == m_imageCodecs[i].FormatDescription)
			return &m_imageCodecs[i];
	}

	return NULL;
}


Gdiplus::ImageCodecInfo* ImageCodecsEnum::GetCodecByExtension(PCTSTR extension) const
{
	std::wstring wideExtension = LibCC::ToUnicode(extension);

	for (UINT i = 0; i < GetNumCodecs(); ++i)
	{
		if (::PathMatchSpecW(wideExtension.c_str(), m_imageCodecs[i].FilenameExtension))
			return &m_imageCodecs[i];
	}

	return NULL;
}
