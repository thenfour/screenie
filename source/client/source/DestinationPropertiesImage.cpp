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
}

void CDestinationPropertiesImage::EnableSizingControls(BOOL enable)
{
	::EnableWindow(GetDlgItem(IDC_RESIZE_SCALE), enable);
	::EnableWindow(GetDlgItem(IDC_RESIZE_SCALE_VALUE), enable);
	::EnableWindow(GetDlgItem(IDC_RESIZE_LIMIT), enable);
	::EnableWindow(GetDlgItem(IDC_RESIZE_LIMIT_VALUE), enable);
	::EnableWindow(GetDlgItem(IDC_RESIZE_LIMIT_LABEL), enable);
}

LRESULT CDestinationPropertiesImage::OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	ShowSettings();

	return 0;
}

LRESULT CDestinationPropertiesImage::OnCheckboxChecked(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	switch (wID)
	{
		case IDC_RESIZE:
			EnableSizingControls(IsDlgButtonChecked(IDC_RESIZE));
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
}

void CDestinationPropertiesImage::SetDestinationType(const ScreenshotDestination::Type type)
{
	if (IsWindow())
	{
		// enable all child windows to ensure a predefined state
		EnableChildWindows(m_hWnd, TRUE);

		// disable what needs to be disabled
		ShowSettings();
	}
}

void CDestinationPropertiesImage::SetParentSheet(DestinationPropertySheet* parentSheet)
{
	m_parentSheet = parentSheet;
}
