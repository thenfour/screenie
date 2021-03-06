//
// codec.cpp - implementation for the image codec enumerator
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
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
	std::wstring wideMimeType = LibCC::ToUTF16(mimeType);

	for (UINT i = 0; i < GetNumCodecs(); ++i)
	{
		if (wideMimeType == m_imageCodecs[i].MimeType)
			return &m_imageCodecs[i];
	}

	return NULL;
}

Gdiplus::ImageCodecInfo* ImageCodecsEnum::GetCodecByDescription(PCTSTR description) const
{
	std::wstring wideDescription = LibCC::StringToLower(LibCC::ToUTF16(description));

	for (UINT i = 0; i < GetNumCodecs(); ++i)
	{
		if (wideDescription == LibCC::StringToLower(m_imageCodecs[i].FormatDescription))
			return &m_imageCodecs[i];
	}

	return NULL; 
}


Gdiplus::ImageCodecInfo* ImageCodecsEnum::GetCodecByExtension(PCTSTR extension) const
{
	std::wstring wideExtension = LibCC::ToUTF16(extension);

	for (UINT i = 0; i < GetNumCodecs(); ++i)
	{
		if (::PathMatchSpecW(wideExtension.c_str(), m_imageCodecs[i].FilenameExtension))
			return &m_imageCodecs[i];
	}

	return NULL;
}

bool ImageCodecsEnum::SupportsQualitySetting(const std::wstring& description)
{
	ImageCodecsEnum x;
	Gdiplus::ImageCodecInfo* p = x.GetCodecByDescription(description.c_str());
	return SupportsQualitySetting(p);
}

bool ImageCodecsEnum::SupportsQualitySetting(Gdiplus::ImageCodecInfo* p)
{
	std::wstring desc = LibCC::StringToLower(p->FormatDescription);
	// only support it for jpeg
	if((std::string::npos != desc.find(L"jpg")) || (std::string::npos != desc.find(L"jpeg")))
	{
		return true;
	}
	return false;
}

bool ImageCodecsEnum::SupportsQualitySettingByMimeType(const std::wstring& m)
{
	ImageCodecsEnum x;
	Gdiplus::ImageCodecInfo* p = x.GetCodecByMimeType(m.c_str());
	return SupportsQualitySetting(p);
}
