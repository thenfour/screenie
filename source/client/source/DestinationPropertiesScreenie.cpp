//
// DestinationPropertiesFtp.cpp - "FTP" page for the destination properties dialog
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#include "stdafx.hpp"
#include "resource.h"

#include "utility.hpp"

#include "DestinationProperties.hpp"
#include "DestinationPropertiesScreenie.hpp"

CDestinationPropertiesScreenie::CDestinationPropertiesScreenie()
{
}

CDestinationPropertiesScreenie::~CDestinationPropertiesScreenie()
{
}

void CDestinationPropertiesScreenie::ShowSettings()
{
	if (IsWindow())
	{
		::EnableChildWindows(m_hWnd, (m_parentSheet->GetCurrentType() == ScreenshotDestination::TYPE_SCREENIENET));

		SetDlgItemText(IDC_SCREENIE_URL, m_settings.url.c_str());
		SetDlgItemText(IDC_SCREENIE_USERNAME, m_settings.username.c_str());
		SetDlgItemText(IDC_SCREENIE_PASSWORD, m_settings.password.c_str());

		CheckDlgButton(IDC_SCREENIE_COPYURL, m_settings.copyURL ? BST_CHECKED : BST_UNCHECKED);
	}
}

LRESULT CDestinationPropertiesScreenie::OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	ShowSettings();

	return 0;
}

//
// DestinationPropertyPage implementation
//

HPROPSHEETPAGE CDestinationPropertiesScreenie::CreatePropertyPage()
{
	return CPropertyPageImpl<CDestinationPropertiesScreenie>::Create();
}

void CDestinationPropertiesScreenie::SetSettings(const ScreenshotDestination& destination)
{
	m_settings = destination.screenie;

	if (IsWindow())
		ShowSettings();
}

void CDestinationPropertiesScreenie::GetSettings(ScreenshotDestination& destination)
{
	if (IsWindow())
	{
		m_settings.url = GetWindowString(GetDlgItem(IDC_SCREENIE_URL));
		m_settings.username = GetWindowString(GetDlgItem(IDC_SCREENIE_USERNAME));
		m_settings.password = GetWindowString(GetDlgItem(IDC_SCREENIE_PASSWORD));
		m_settings.copyURL = (IsDlgButtonChecked(IDC_SCREENIE_COPYURL) == TRUE);
	}

	destination.screenie = m_settings;
}

void CDestinationPropertiesScreenie::SetDestinationType(const ScreenshotDestination::Type type)
{
	if (IsWindow())
	{
		::EnableChildWindows(m_hWnd, (type == ScreenshotDestination::TYPE_SCREENIENET));
	}
}

void CDestinationPropertiesScreenie::SetParentSheet(DestinationPropertySheet* parentSheet)
{
	m_parentSheet = parentSheet;
}
