//
// DestinationPropertiesGeneral.cpp - destination properties "General" page
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#include "stdafx.hpp"
#include "resource.h"

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

	if (codecInfo != 0)
		return tstd::convert<tstd::tchar_t>(codecInfo->MimeType);

	return tstd::tstring();
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
		tstd::tstring description = tstd::convert<tstd::tchar_t>(codecInfo->FormatDescription);

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
		tstd::tstring codecDescription = tstd::convert<tstd::tchar_t>(wideCodecDescription);

		fileFormats.AddString(codecDescription.c_str());
	}
}

void CDestinationPropertiesGeneral::ShowSettings()
{
	SetDlgItemText(IDC_FILE_FOLDER, m_settings.path.c_str());

	CComboBox destinationType(GetDlgItem(IDC_GENERAL_TYPE));
	destinationType.SelectString(0, 
		ScreenshotDestination::TypeToString(m_settings.type).c_str());

	SetDlgItemText(IDC_GENERAL_NAME, m_settings.name.c_str());
	SetImageFormat(m_settings.imageFormat);

	SetDlgItemText(IDC_FILENAME_FORMAT, m_settings.filenameFormat.c_str());

  CheckDlgButton(m_settings.localTime ? IDC_FILENAME_LOCAL : IDC_FILENAME_UTC, BST_CHECKED);
}

LRESULT CDestinationPropertiesGeneral::OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	CComboBox destinationType(GetDlgItem(IDC_GENERAL_TYPE));

	destinationType.AddString(ScreenshotDestination::TypeToString(ScreenshotDestination::TYPE_FILE).c_str());
	destinationType.AddString(ScreenshotDestination::TypeToString(ScreenshotDestination::TYPE_FTP).c_str());
	destinationType.AddString(ScreenshotDestination::TypeToString(ScreenshotDestination::TYPE_CLIPBOARD).c_str());
//	destinationType.AddString(ScreenshotDestination::TypeToString(ScreenshotDestination::TYPE_EMAIL).c_str());
	destinationType.AddString(ScreenshotDestination::TypeToString(ScreenshotDestination::TYPE_SCREENIENET).c_str());

	PopulateFormatList();

	tstd::tstring formatDescription;
	if (GetStringResource(_Module.GetResourceInstance(), IDS_FORMATDESCRIPTION, formatDescription))
		SetDlgItemText(IDC_FILENAME_FORMATDESC, formatDescription.c_str());

	ShowSettings();

	m_parentSheet->Update(m_settings.type);

	return 0;
}

LRESULT CDestinationPropertiesGeneral::OnFolderBrowse(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	CFolderDialog dialog(m_hWnd);

	if (dialog.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_FILE_FOLDER, dialog.GetFolderPath());

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

LRESULT CDestinationPropertiesGeneral::OnImageFormatChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	// using "format" for both of these settings is pretty confusing. i should probably change that.
	tstd::tstring filenameFormat = GetWindowString(GetDlgItem(IDC_FILENAME_FORMAT));
	tstd::tstring formatDescription = GetComboSelectionString(GetDlgItem(IDC_GENERAL_FORMAT));

	ImageCodecsEnum codecs;
	Gdiplus::ImageCodecInfo* codecInfo = codecs.GetCodecByDescription(formatDescription.c_str());

	Path pathFilenameFormat(filenameFormat);

	// get the filename extension for this codec, without the dot prefix
	tstd::tstring extension = tstring_tolower(GetImageCodecExtension(codecInfo, false));

	SetDlgItemText(IDC_FILENAME_FORMAT, LibCC::Format(TEXT("%.%")).s(pathFilenameFormat.filename).s(extension).CStr());

	return 0;
}

LRESULT CDestinationPropertiesGeneral::OnFilenameFormatChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	tstd::tstring formatString = GetWindowString(GetDlgItem(IDC_FILENAME_FORMAT));

	SYSTEMTIME systemTime = { 0 };
  if(BST_CHECKED == IsDlgButtonChecked(IDC_FILENAME_LOCAL))
  {
	  ::GetLocalTime(&systemTime);
  }
  else
  {
	  ::GetSystemTime(&systemTime);
  }

	tstd::tstring formattedOutput = FormatFilename(systemTime, formatString);
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

void CDestinationPropertiesGeneral::SetSettings(const ScreenshotDestination& destination)
{
	m_settings = destination.general;

	if (IsWindow())
		ShowSettings();
}

void CDestinationPropertiesGeneral::GetSettings(ScreenshotDestination& destination)
{
	if (IsWindow())
	{
		destination.general.type = GetType();
		destination.general.name = GetWindowString(GetDlgItem(IDC_GENERAL_NAME));
		destination.general.imageFormat = GetImageFormat();
		destination.general.filenameFormat = GetWindowString(GetDlgItem(IDC_GENERAL_FILENAME));
		destination.general.path = GetWindowString(GetDlgItem(IDC_FILE_FOLDER));

    destination.general.localTime = (BST_CHECKED == IsDlgButtonChecked(IDC_FILENAME_LOCAL));
	}
}

void CDestinationPropertiesGeneral::SetDestinationType(ScreenshotDestination::Type type)
{
	// enable all windows to ensure a predefined state
	EnableChildWindows(m_hWnd, TRUE);

	if (type != ScreenshotDestination::TYPE_FILE)
	{
		::EnableWindow(GetDlgItem(IDC_FILE_FOLDER_BROWSE), FALSE);
		::EnableWindow(GetDlgItem(IDC_FILE_FOLDER), FALSE);
	}
	
  if (type == ScreenshotDestination::TYPE_CLIPBOARD)
	{
		// the clipboard doesn't have 'files.' the filename formatting and related
		// settings are pretty irrelevant when that's the destination type.

		BOOL enableControls = FALSE;

		::EnableWindow(GetDlgItem(IDC_FILE_FOLDER_BROWSE), enableControls);
		::EnableWindow(GetDlgItem(IDC_FILE_FOLDER), enableControls);
		::EnableWindow(GetDlgItem(IDC_GENERAL_FORMAT), enableControls);
		::EnableWindow(GetDlgItem(IDC_FILENAME_FORMAT), enableControls);
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