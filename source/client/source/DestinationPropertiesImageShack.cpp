//
// DestinationPropertiesImageShack.cpp - "ImageShack" page for the destination properties dialog
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//

#include "stdafx.hpp"
#include "resource.h"

#include "utility.hpp"

#include "DestinationProperties.hpp"
#include "DestinationPropertiesImageShack.hpp"

CDestinationPropertiesImageShack::CDestinationPropertiesImageShack()
{
}

CDestinationPropertiesImageShack::~CDestinationPropertiesImageShack()
{
}

void CDestinationPropertiesImageShack::ShowSettings()
{
	if (IsWindow())
	{
		bool enableAny = (m_parentSheet->GetCurrentType() == ScreenshotDestination::TYPE_IMAGESHACK);
		::EnableChildWindows(m_hWnd, enableAny);

		CheckDlgButton(IDC_IMAGESHACK_COPYURL, m_settings->imageshack.copyURL ? BST_CHECKED : BST_UNCHECKED);

		GetDlgItem(IDC_IMAGESHACK_SHORTENURL).EnableWindow(enableAny && (IsDlgButtonChecked(IDC_IMAGESHACK_COPYURL) == TRUE));
		CheckDlgButton(IDC_IMAGESHACK_SHORTENURL, m_settings->imageshack.shortenURL ? BST_CHECKED : BST_UNCHECKED);

	}
}

LRESULT CDestinationPropertiesImageShack::OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	ShowSettings();

	return 0;
}

LRESULT CDestinationPropertiesImageShack::OnCopyURLClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	GetDlgItem(IDC_IMAGESHACK_SHORTENURL).EnableWindow(IsDlgButtonChecked(IDC_IMAGESHACK_COPYURL) == TRUE);
	return 0;
}

//
// DestinationPropertyPage implementation
//

HPROPSHEETPAGE CDestinationPropertiesImageShack::CreatePropertyPage()
{
	return CPropertyPageImpl<CDestinationPropertiesImageShack>::Create();
}

void CDestinationPropertiesImageShack::SetSettings(ScreenshotDestination* destination)
{
	m_settings = destination;

	if (IsWindow())
		ShowSettings();
}

void CDestinationPropertiesImageShack::GetSettings()
{
	if (IsWindow())
	{
		m_settings->imageshack.copyURL = (IsDlgButtonChecked(IDC_IMAGESHACK_COPYURL) == TRUE);
		m_settings->imageshack.shortenURL = (IsDlgButtonChecked(IDC_IMAGESHACK_SHORTENURL) == TRUE);
	}
}

void CDestinationPropertiesImageShack::SetDestinationType(const ScreenshotDestination::Type type)
{
}

void CDestinationPropertiesImageShack::SetParentSheet(DestinationPropertySheet* parentSheet)
{
	m_parentSheet = parentSheet;
}
