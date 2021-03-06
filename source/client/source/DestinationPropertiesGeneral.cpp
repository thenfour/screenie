//
// DestinationPropertiesGeneral.cpp - destination properties "General" page
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//

#include "stdafx.hpp"
#include "resource.h"
#include "libcc/winapi.hpp"

#include "codec.hpp"
#include "image.hpp"
#include "path.hpp"
#include "utility.hpp"

#include "DestinationProperties.hpp"
#include "DestinationPropertiesGeneral.hpp"

CDestinationPropertiesGeneral::CDestinationPropertiesGeneral()
{
}

CDestinationPropertiesGeneral::~CDestinationPropertiesGeneral()
{
}

ScreenshotDestination::Type CDestinationPropertiesGeneral::GetType()
{
	ATLASSERT(IsWindow());

	tstd::tstring typeName = GetComboSelectionString(GetDlgItem(IDC_GENERAL_TYPE));
	return ScreenshotDestination::StringToType(typeName);
}

tstd::tstring CDestinationPropertiesGeneral::GetImageFormat()
{
	ATLASSERT(IsWindow());

	// the format description string is in the combo box, not the mime type.
	// since the mime type is what we want, we'll get the description string
	// and find the matching codec -- then we'll return that mime type.

	tstd::tstring formatDescription = GetComboSelectionString(GetDlgItem(IDC_GENERAL_FORMAT));

	// is this performant enough to do so often? if not, maybe it should be
	// some kind of global API, rather than an object that gets instantiated
	// every time we want to look up an image codec. it probably doesn't help
	// that memory is allocated for the entire array of codecs every time.

	ImageCodecsEnum codecs;
	Gdiplus::ImageCodecInfo* codecInfo = codecs.GetCodecByDescription(formatDescription.c_str());

	// if the correct descriptions were added to the combo box,
	// there is no excuse for this to fail.
	tstd::tstring ret;

	if (codecInfo != 0)
	{
		ret = ToTstring(codecInfo->MimeType);
	}

	return ret;
}

void CDestinationPropertiesGeneral::SetImageFormat(const tstd::tstring& imageFormat)
{
	ATLASSERT(IsWindow());

	// this function is the inverse of the above GetImageFormat(); we need to obtain the
	// corresponding description for this mime type, and then use that to set the selection.

	ImageCodecsEnum codecs;
	Gdiplus::ImageCodecInfo* codecInfo = codecs.GetCodecByMimeType(imageFormat.c_str());

	if (codecInfo != 0)
	{
		tstd::tstring description = ToTstring(codecInfo->FormatDescription);

		CComboBox fileFormat(GetDlgItem(IDC_GENERAL_FORMAT));
		fileFormat.SelectString(-1, description.c_str());
	}
}

void CDestinationPropertiesGeneral::PopulateFormatList()
{
	ATLASSERT(IsWindow());

	ImageCodecsEnum imageCodecs;
	CComboBox fileFormats(GetDlgItem(IDC_GENERAL_FORMAT));

	for (UINT i = 0; i < imageCodecs.GetNumCodecs(); ++i)
	{
		Gdiplus::ImageCodecInfo* codecInfo = imageCodecs.GetCodec(i);

		std::wstring wideCodecDescription = codecInfo->FormatDescription;
		tstd::tstring codecDescription = ToTstring(wideCodecDescription);

		fileFormats.AddString(codecDescription.c_str());
	}

	AutoSetComboBoxHeight(fileFormats);
}

void CDestinationPropertiesGeneral::UpdateQualityLabel()
{
	std::wstring desc;
	int quality = CTrackBarCtrl(GetDlgItem(IDC_QUALITY)).GetPos();
	if(quality < 25)
	{
		desc = L"Very low";
	}
	if(quality < 50)
	{
		desc = L"Low";
	}
	else if(quality < 75)
	{
		desc = L"Medium";
	}
	else if(quality < 90)
	{
		desc = L"High";
	}
	else
	{
		desc = L"Highest";
	}
	CStatic l(GetDlgItem(IDC_QUALITYLABEL));
	l.SetWindowTextW(LibCC::Format(L"%^% (%)").i(quality).s(desc).CStr());
}

void CDestinationPropertiesGeneral::ShowSettings()
{
	SetDlgItemText(IDC_FILE_PATH, m_settings->general.pathFormat.c_str());

	CComboBox destinationType(GetDlgItem(IDC_GENERAL_TYPE));
	destinationType.SelectString(0, 
		ScreenshotDestination::TypeToString(m_settings->general.type).c_str());

	SetDlgItemText(IDC_GENERAL_NAME, m_settings->general.name.c_str());
	SetImageFormat(m_settings->general.imageFormat);

	// enable / disable quality settings
	CTrackBarCtrl quality(GetDlgItem(IDC_QUALITY));
	quality.SetRange(0, 100, FALSE);
	quality.SetPos(m_settings->general.imageQuality);
	UpdateQualityLabel();

  CheckDlgButton(m_settings->general.localTime ? IDC_FILENAME_LOCAL : IDC_FILENAME_UTC, BST_CHECKED);
}

LRESULT CDestinationPropertiesGeneral::OnHScroll(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	if((HWND)lParam != GetDlgItem(IDC_QUALITY))
		return 0;
	handled = TRUE;
	UpdateQualityLabel();
	return 0;
}

LRESULT CDestinationPropertiesGeneral::OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	CComboBox destinationType(GetDlgItem(IDC_GENERAL_TYPE));

	destinationType.AddString(ScreenshotDestination::TypeToString(ScreenshotDestination::TYPE_FILE).c_str());
	destinationType.AddString(ScreenshotDestination::TypeToString(ScreenshotDestination::TYPE_FTP).c_str());
	destinationType.AddString(ScreenshotDestination::TypeToString(ScreenshotDestination::TYPE_CLIPBOARD).c_str());
	destinationType.AddString(ScreenshotDestination::TypeToString(ScreenshotDestination::TYPE_IMAGESHACK).c_str());

	AutoSetComboBoxHeight(destinationType);

	PopulateFormatList();

	tstd::tstring formatStr;
	LibCC::LoadStringX(_Module.GetResourceInstance(), IDS_FORMATDESCRIPTION, formatStr);
	SetDlgItemText(IDC_FILENAME_FORMATDESC, formatStr.c_str());

	ShowSettings();

	m_parentSheet->Update(m_settings->general.type);

	return 0;
}

LRESULT CDestinationPropertiesGeneral::OnFolderBrowse(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	CFolderDialog dialog(m_hWnd);
	dialog.m_bi.ulFlags |= BIF_NEWDIALOGSTYLE;

	if (dialog.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_FILE_PATH, dialog.GetFolderPath());

		// it would be nice to show the user the end of the string,
		// but unfortunately, EM_SCROLLCARET is only supported in rich edits.
		//
		// m_fileFolder.SetSel(-1, -1);
		// m_fileFolder.SendMessage(EM_SCROLLCARET);
	}

	return 0;
}

LRESULT CDestinationPropertiesGeneral::OnTypeChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	// if the type of the destination changes, the parent property sheet
	// and the rest of the property pages will need to know about it.
	// some options only make sense for certain types of destinations.

	if (m_parentSheet != 0)
		m_parentSheet->Update(GetType());

	return 0;
}

std::wstring ChangeExtension(const std::wstring& org, const std::wstring& newExtension)
{
	// must work for URLs and paths and freeform crap.
	// if we don't understand the input then return it unchanged.
	
	// find the last backslash or slash
	std::wstring::size_type lastslash = org.find_last_of(L"\\/");
	std::wstring::size_type lastdot = org.find_last_of('.');
	if(lastdot == std::wstring::npos)// there is no dot; forget the extension
		return org;
	if(lastslash != std::wstring::npos)// there is a slash; make sure the last dot is AFTER it.
	{
		if(lastslash > lastdot)
			return org;// for example http://blah.org/mypic
	}

	// we know we are good to go.
	std::wstring ret = org.substr(0, lastdot + 1);
	ret.append(newExtension);
	return ret;
}

LRESULT CDestinationPropertiesGeneral::OnImageFormatChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	// using "format" for both of these settings is pretty confusing. i should probably change that.
	tstd::tstring filenameFormat = GetWindowString(GetDlgItem(IDC_FILE_PATH));
	tstd::tstring formatDescription = GetComboSelectionString(GetDlgItem(IDC_GENERAL_FORMAT));

	ImageCodecsEnum codecs;
	Gdiplus::ImageCodecInfo* codecInfo = codecs.GetCodecByDescription(formatDescription.c_str());

	// get the filename extension for this codec, without the dot prefix
	tstd::tstring extension = tstring_tolower(GetImageCodecExtension(codecInfo, false));

	SetDlgItemText(IDC_FILE_PATH, ChangeExtension(filenameFormat, extension).c_str());

	SetDestinationType(GetType());

	// set the FTP info now...
	m_settings->ftp.remotePathFormat = ChangeExtension(m_settings->ftp.remotePathFormat, extension);
	m_settings->ftp.resultURLFormat = ChangeExtension(m_settings->ftp.resultURLFormat, extension);

	return 0;
}

LRESULT CDestinationPropertiesGeneral::OnFilenameFormatChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	tstd::tstring formatString = GetWindowString(GetDlgItem(IDC_FILE_PATH));
	bool useLocalTime = (BST_CHECKED == IsDlgButtonChecked(IDC_FILENAME_LOCAL));

	ScreenshotNamingData namingData;
	namingData.windowTitle = L"Screenie";

	tstd::tstring formattedOutput = FormatFilename(namingData, useLocalTime, formatString, true);
	SetDlgItemText(IDC_FILENAME_FORMATPREVIEW, formattedOutput.c_str());

	return 0;
}

//
// DestinationPropertyPage implementation
//

HPROPSHEETPAGE CDestinationPropertiesGeneral::CreatePropertyPage()
{
	return CPropertyPageImpl<CDestinationPropertiesGeneral>::Create();
}

void CDestinationPropertiesGeneral::SetSettings(ScreenshotDestination* destination)
{
	m_settings = destination;

	if (IsWindow())
		ShowSettings();
}

void CDestinationPropertiesGeneral::GetSettings()
{
	if (IsWindow())
	{
		m_settings->general.type = GetType();
		m_settings->general.name = GetWindowString(GetDlgItem(IDC_GENERAL_NAME));
		m_settings->general.imageFormat = GetImageFormat();
		m_settings->general.imageQuality = CTrackBarCtrl(GetDlgItem(IDC_QUALITY)).GetPos();
		//destination.general.filenameFormat = GetWindowString(GetDlgItem(IDC_GENERAL_FILENAME));
		m_settings->general.pathFormat = GetWindowString(GetDlgItem(IDC_FILE_PATH));

    m_settings->general.localTime = (BST_CHECKED == IsDlgButtonChecked(IDC_FILENAME_LOCAL));
	}
}

void CDestinationPropertiesGeneral::SetDestinationType(ScreenshotDestination::Type type)
{
	// enable all windows to ensure a predefined state
	EnableChildWindows(m_hWnd, TRUE);

	// only let quality settings be enabled if the format is JPEG
	std::wstring formatDescription = LibCC::StringToLower(GetComboSelectionString(GetDlgItem(IDC_GENERAL_FORMAT)));
	bool enableQuality = ImageCodecsEnum::SupportsQualitySetting(formatDescription);

	::EnableWindow(GetDlgItem(IDC_QUALITY), enableQuality);
	::EnableWindow(GetDlgItem(IDC_QUALITYLABEL), enableQuality);

	if (type != ScreenshotDestination::TYPE_FILE)
	{
		::EnableWindow(GetDlgItem(IDC_FILE_FOLDER_BROWSE), FALSE);
		::EnableWindow(GetDlgItem(IDC_FILE_PATH), FALSE);
	}
	
	if ((type == ScreenshotDestination::TYPE_CLIPBOARD) || (type == ScreenshotDestination::TYPE_IMAGESHACK))
	{
		// the clipboard doesn't have 'files.' the filename formatting and related
		// settings are pretty irrelevant when that's the destination type.

		BOOL enableControls = FALSE;

		::EnableWindow(GetDlgItem(IDC_FILE_FOLDER_BROWSE), enableControls);
		::EnableWindow(GetDlgItem(IDC_FILE_PATH), enableControls);
		::EnableWindow(GetDlgItem(IDC_GENERAL_FORMAT), type == ScreenshotDestination::TYPE_IMAGESHACK);
		::EnableWindow(GetDlgItem(IDC_QUALITY), type == ScreenshotDestination::TYPE_IMAGESHACK && enableQuality);
		::EnableWindow(GetDlgItem(IDC_QUALITYLABEL), type == ScreenshotDestination::TYPE_IMAGESHACK && enableQuality);
		::EnableWindow(GetDlgItem(IDC_FILENAME_FORMATDESC), enableControls);
		::EnableWindow(GetDlgItem(IDC_FILENAME_FORMATPREVIEW), enableControls);
		::EnableWindow(GetDlgItem(IDC_FILENAME_LOCAL), enableControls);
		::EnableWindow(GetDlgItem(IDC_FILENAME_UTC), enableControls);
	}
}

void CDestinationPropertiesGeneral::SetParentSheet(DestinationPropertySheet* parentSheet)
{
	m_parentSheet = parentSheet;
}

