//
// DestinationPropertiesFtp.cpp - "FTP" page for the destination properties dialog
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#include "stdafx.hpp"
#include "resource.h"

#include "utility.hpp"

#include "DestinationProperties.hpp"
#include "DestinationPropertiesFtp.hpp"

CDestinationPropertiesFTP::CDestinationPropertiesFTP()
{
}

CDestinationPropertiesFTP::~CDestinationPropertiesFTP()
{
}

void CDestinationPropertiesFTP::ShowSettings()
{
	if (IsWindow())
	{
		::EnableChildWindows(m_hWnd,
			(m_parentSheet->GetCurrentType() == ScreenshotDestination::TYPE_FTP));

		SetDlgItemText(IDC_FTP_HOSTNAME, m_settings.hostname.c_str());
		SetDlgItemInt(IDC_FTP_PORT, m_settings.port, FALSE);
		SetDlgItemText(IDC_FTP_USERNAME, m_settings.username.c_str());
		SetDlgItemText(IDC_FTP_PASSWORD, m_settings.DecryptPassword().c_str());
		SetDlgItemText(IDC_FTP_REMOTEPATH, m_settings.remotePath.c_str());
		SetDlgItemText(IDC_FTP_HTTPURL, m_settings.resultURL.c_str());
		CheckDlgButton(IDC_FTP_COPYURL, m_settings.copyURL ? BST_CHECKED : BST_UNCHECKED);
	}
}

LRESULT CDestinationPropertiesFTP::OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	ShowSettings();

	return 0;
}

//
// DestinationPropertyPage implementation
//

HPROPSHEETPAGE CDestinationPropertiesFTP::CreatePropertyPage()
{
	return CPropertyPageImpl<CDestinationPropertiesFTP>::Create();
}

void CDestinationPropertiesFTP::SetSettings(const ScreenshotDestination& destination)
{
	m_settings = destination.ftp;

	if (IsWindow())
		ShowSettings();
}

void CDestinationPropertiesFTP::GetSettings(ScreenshotDestination& destination)
{
	if (IsWindow())
	{
		m_settings.hostname = GetWindowString(GetDlgItem(IDC_FTP_HOSTNAME));
		m_settings.port = GetDlgItemInt(IDC_FTP_PORT, NULL, FALSE);
		m_settings.username = GetWindowString(GetDlgItem(IDC_FTP_USERNAME));
    m_settings.SetPassword(GetWindowString(GetDlgItem(IDC_FTP_PASSWORD)));
		m_settings.remotePath = GetWindowString(GetDlgItem(IDC_FTP_REMOTEPATH));
		m_settings.resultURL = GetWindowString(GetDlgItem(IDC_FTP_HTTPURL));
		m_settings.copyURL = (IsDlgButtonChecked(IDC_FTP_COPYURL) == TRUE);
	}

	destination.ftp = m_settings;
}

void CDestinationPropertiesFTP::SetDestinationType(const ScreenshotDestination::Type type)
{
	if (IsWindow())
	{
		::EnableChildWindows(m_hWnd, (type == ScreenshotDestination::TYPE_FTP));
	}
}

void CDestinationPropertiesFTP::SetParentSheet(DestinationPropertySheet* parentSheet)
{
	m_parentSheet = parentSheet;
}
