//
// DestinationPropertiesImage.hpp - image resizing page for destination properties dialog
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#include "stdafx.hpp"
#include "resource.h"

#include "utility.hpp"

#include "DestinationProperties.hpp"
#include "DestinationPropertiesImage.hpp"

CDestinationPropertiesImage::CDestinationPropertiesImage()
{
	//
}

CDestinationPropertiesImage::~CDestinationPropertiesImage()
{
	//
}

void CDestinationPropertiesImage::ShowSettings()
{
	BOOL doScaling = (m_settings.scaleType != ScreenshotDestination::SCALE_NONE);

	CheckDlgButton(IDC_RESIZE, doScaling ? BST_CHECKED : BST_UNCHECKED);
	EnableSizingControls(doScaling);

	if (doScaling)
	{
		CheckDlgButton(IDC_RESIZE_SCALE,
			(m_settings.scaleType == ScreenshotDestination::SCALE_SCALETOPERCENT) ?
			BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(IDC_RESIZE_LIMIT,
			(m_settings.scaleType == ScreenshotDestination::SCALE_LIMITDIMENSIONS) ?
			BST_CHECKED : BST_UNCHECKED);
	}

	SetDlgItemInt(IDC_RESIZE_SCALE_VALUE, m_settings.scalePercent, FALSE);
	SetDlgItemInt(IDC_RESIZE_LIMIT_VALUE, m_settings.maxDimension, FALSE);

	ScreenshotDestination::Type type = m_parentSheet->GetCurrentType();

	CheckDlgButton(IDC_THUMB_GENERATE, m_settings.createThumbnail ? BST_CHECKED : BST_UNCHECKED);
	EnableThumbnailControls(m_settings.createThumbnail);

	CheckDlgButton(IDC_THUMB_FILEFMT_USE, m_settings.useFilenameFormat ? BST_CHECKED : BST_UNCHECKED);
	EnableFilenameFormatControls(m_settings.useFilenameFormat);
	SetDlgItemText(IDC_THUMB_FILEFMT, m_settings.filenameFormat.c_str());

	CheckDlgButton(IDC_THUMB_SIZING_SCALE,
		(m_settings.thumbScaleType == ScreenshotDestination::SCALE_SCALETOPERCENT) ?
		BST_CHECKED : BST_UNCHECKED);
	SetDlgItemInt(IDC_THUMB_SIZING_SCALE_VALUE, m_settings.thumbScalePercent, FALSE);

	CheckDlgButton(IDC_THUMB_SIZING_LIMIT,
		(m_settings.thumbScaleType == ScreenshotDestination::SCALE_LIMITDIMENSIONS) ?
		BST_CHECKED : BST_UNCHECKED);
	SetDlgItemInt(IDC_THUMB_SIZING_LIMIT_VALUE, m_settings.thumbMaxDimension, FALSE);

}

void CDestinationPropertiesImage::EnableSizingControls(BOOL enable)
{
	::EnableWindow(GetDlgItem(IDC_RESIZE_SCALE), enable);
	::EnableWindow(GetDlgItem(IDC_RESIZE_SCALE_VALUE), enable);
	::EnableWindow(GetDlgItem(IDC_RESIZE_LIMIT), enable);
	::EnableWindow(GetDlgItem(IDC_RESIZE_LIMIT_VALUE), enable);
	::EnableWindow(GetDlgItem(IDC_RESIZE_LIMIT_LABEL), enable);
}

void CDestinationPropertiesImage::EnableThumbnailControls(BOOL enable)
{
	::EnableWindow(GetDlgItem(IDC_THUMB_GROUP), enable);
	::EnableWindow(GetDlgItem(IDC_THUMB_FILEFMT_USE), enable);
	::EnableWindow(GetDlgItem(IDC_THUMB_FILEFMT), enable);
	::EnableWindow(GetDlgItem(IDC_THUMB_FILEFMT_PREVIEW), enable);
	::EnableWindow(GetDlgItem(IDC_THUMB_FILEFMT_LABEL), enable);
	::EnableWindow(GetDlgItem(IDC_THUMB_FILEFMT_PREVIEW_LABEL), enable);
	::EnableWindow(GetDlgItem(IDC_THUMB_SIZINGOPTIONS), enable);
	::EnableWindow(GetDlgItem(IDC_THUMB_SIZING_SCALE), enable);
	::EnableWindow(GetDlgItem(IDC_THUMB_SIZING_SCALE_VALUE), enable);
	::EnableWindow(GetDlgItem(IDC_THUMB_SIZING_LIMIT), enable);
	::EnableWindow(GetDlgItem(IDC_THUMB_SIZING_LIMIT_VALUE), enable);
	::EnableWindow(GetDlgItem(IDC_THUMB_PIXELS), enable);

	if (enable)
	{
		if (!IsDlgButtonChecked(IDC_THUMB_FILEFMT_USE))
		{
			EnableFilenameFormatControls(FALSE);
		}
	}
}

void CDestinationPropertiesImage::EnableFilenameFormatControls(BOOL enable)
{
	// if the state of these controls has been changed by EnableThumbnailControls(),
	// these calls will have no effect. (it won't toggle the check state or anything.)

	::EnableWindow(GetDlgItem(IDC_THUMB_FILEFMT), enable);
	::EnableWindow(GetDlgItem(IDC_THUMB_FILEFMT_PREVIEW), enable);
	::EnableWindow(GetDlgItem(IDC_THUMB_FILEFMT_LABEL), enable);
	::EnableWindow(GetDlgItem(IDC_THUMB_FILEFMT_PREVIEW_LABEL), enable);
}

LRESULT CDestinationPropertiesImage::OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	ShowSettings();

	return 0;
}

LRESULT CDestinationPropertiesImage::OnFormatChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
	tstd::tstring formatString = GetWindowString(GetDlgItem(IDC_THUMB_FILEFMT));

	SYSTEMTIME systemTime = { 0 };
	::GetSystemTime(&systemTime);

	tstd::tstring formattedOutput = FormatFilename(systemTime, formatString);
	SetDlgItemText(IDC_THUMB_FILEFMT_PREVIEW, formattedOutput.c_str());

	return 0;
}

LRESULT CDestinationPropertiesImage::OnCheckboxChecked(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	switch (wID)
	{
		case IDC_RESIZE:
			EnableSizingControls(IsDlgButtonChecked(IDC_RESIZE));
			break;
		case IDC_THUMB_GENERATE:
			EnableThumbnailControls(IsDlgButtonChecked(IDC_THUMB_GENERATE));
			break;
		case IDC_THUMB_FILEFMT_USE:
			EnableFilenameFormatControls(IsDlgButtonChecked(IDC_THUMB_FILEFMT_USE));
			break;
	}

	return 0;
}
//
// DestinationPropertyPage implementation
//

HPROPSHEETPAGE CDestinationPropertiesImage::CreatePropertyPage()
{
	return CPropertyPageImpl<CDestinationPropertiesImage>::Create();
}

void CDestinationPropertiesImage::SetSettings(const ScreenshotDestination& destination)
{
	m_settings = destination.image;

	if (IsWindow())
		ShowSettings();
}

void CDestinationPropertiesImage::GetSettings(ScreenshotDestination& destination)
{
	if (!IsWindow())
		return;

	if (IsDlgButtonChecked(IDC_RESIZE))
	{
		if (IsDlgButtonChecked(IDC_RESIZE_SCALE))
		{
			destination.image.scaleType = ScreenshotDestination::SCALE_SCALETOPERCENT;
			destination.image.scalePercent = GetDlgItemInt(IDC_RESIZE_SCALE_VALUE, NULL, FALSE);
		}
		else
		{
			destination.image.scaleType = ScreenshotDestination::SCALE_LIMITDIMENSIONS;
			destination.image.maxDimension = GetDlgItemInt(IDC_RESIZE_LIMIT_VALUE, NULL, FALSE);
		}
	}
	else
	{
		destination.image.scaleType = ScreenshotDestination::SCALE_NONE;
	}

	destination.image.createThumbnail = (IsDlgButtonChecked(IDC_THUMB_GENERATE) == TRUE);
	destination.image.useFilenameFormat = (IsDlgButtonChecked(IDC_THUMB_FILEFMT_USE) == TRUE);
	destination.image.filenameFormat = GetWindowString(GetDlgItem(IDC_THUMB_FILEFMT));

	if (IsDlgButtonChecked(IDC_THUMB_SIZING_SCALE))
	{
		destination.image.thumbScaleType = ScreenshotDestination::SCALE_SCALETOPERCENT;
		destination.image.thumbScalePercent = GetDlgItemInt(IDC_THUMB_SIZING_SCALE_VALUE, NULL, FALSE);
	}
	else
	{
		destination.image.thumbScaleType = ScreenshotDestination::SCALE_LIMITDIMENSIONS;
		destination.image.thumbMaxDimension = GetDlgItemInt(IDC_THUMB_SIZING_LIMIT_VALUE, NULL, FALSE);
	}
}

void CDestinationPropertiesImage::SetDestinationType(const ScreenshotDestination::Type type)
{
	if (IsWindow())
	{
		// enable all child windows to ensure a predefined state
		EnableChildWindows(m_hWnd, TRUE);

		// disable what needs to be disabled
		ShowSettings();

		// disable the thumbnail controls for email/clipboard.
		if ((type == ScreenshotDestination::TYPE_EMAIL) ||
			(type == ScreenshotDestination::TYPE_CLIPBOARD))
		{
			EnableThumbnailControls(FALSE);
			::EnableWindow(GetDlgItem(IDC_THUMB_GENERATE), FALSE);
		}
	}
}

void CDestinationPropertiesImage::SetParentSheet(DestinationPropertySheet* parentSheet)
{
	m_parentSheet = parentSheet;
}
