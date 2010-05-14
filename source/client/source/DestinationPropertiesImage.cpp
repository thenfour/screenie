//
// DestinationPropertiesImage.hpp - image resizing page for destination properties dialog
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
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
	BOOL doScaling = (m_settings->image.scaleType != ScreenshotDestination::SCALE_NONE);

	CheckDlgButton(IDC_RESIZE, doScaling ? BST_CHECKED : BST_UNCHECKED);
	EnableSizingControls(doScaling);

	if (doScaling)
	{
		CheckDlgButton(IDC_RESIZE_SCALE,
			(m_settings->image.scaleType == ScreenshotDestination::SCALE_SCALETOPERCENT) ?
			BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(IDC_RESIZE_LIMIT,
			(m_settings->image.scaleType == ScreenshotDestination::SCALE_LIMITDIMENSIONS) ?
			BST_CHECKED : BST_UNCHECKED);
	}

	SetDlgItemInt(IDC_RESIZE_SCALE_VALUE, m_settings->image.scalePercent, FALSE);
	SetDlgItemInt(IDC_RESIZE_LIMIT_VALUE, m_settings->image.maxDimension, FALSE);

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

void CDestinationPropertiesImage::SetSettings(ScreenshotDestination* destination)
{
	m_settings = destination;

	if (IsWindow())
		ShowSettings();
}

void CDestinationPropertiesImage::GetSettings()
{
	if (!IsWindow())
		return;

	if (IsDlgButtonChecked(IDC_RESIZE))
	{
		if (IsDlgButtonChecked(IDC_RESIZE_SCALE))
		{
			m_settings->image.scaleType = ScreenshotDestination::SCALE_SCALETOPERCENT;
			m_settings->image.scalePercent = GetDlgItemInt(IDC_RESIZE_SCALE_VALUE, NULL, FALSE);
		}
		else
		{
			m_settings->image.scaleType = ScreenshotDestination::SCALE_LIMITDIMENSIONS;
			m_settings->image.maxDimension = GetDlgItemInt(IDC_RESIZE_LIMIT_VALUE, NULL, FALSE);
		}
	}
	else
	{
		m_settings->image.scaleType = ScreenshotDestination::SCALE_NONE;
	}
}

void CDestinationPropertiesImage::SetDestinationType(const ScreenshotDestination::Type type)
{
}

void CDestinationPropertiesImage::SetParentSheet(DestinationPropertySheet* parentSheet)
{
	m_parentSheet = parentSheet;
}
