//
// DestinationPropertiesFtp.cpp - "FTP" page for the destination properties dialog
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
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

		CComboBox passwordOptions(GetDlgItem(IDC_PASSWORDOPTIONS));
		// indices must correspond directly to enum values.
		passwordOptions.AddString(_T("Insecure; store as plain text"));
		passwordOptions.AddString(_T("Obscure; basic symmetric encryption"));
		passwordOptions.AddString(_T("Most secure, but only usable on this computer"));
		passwordOptions.SetCurSel((int)m_settings.passwordOptions);
		AutoSetComboBoxHeight(passwordOptions);

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
		if (m_settings.remotePath[m_settings.remotePath.length() - 1] != _T('/'))
			m_settings.remotePath += _T('/');

		m_settings.resultURL = GetWindowString(GetDlgItem(IDC_FTP_HTTPURL));
		if (m_settings.resultURL[m_settings.resultURL.length() - 1] != _T('/'))
			m_settings.resultURL += _T('/');

		m_settings.copyURL = (IsDlgButtonChecked(IDC_FTP_COPYURL) == TRUE);
		CComboBox passwordOptions(GetDlgItem(IDC_PASSWORDOPTIONS));
		m_settings.passwordOptions = (ScreenshotDestination::Ftp::PasswordOptions)passwordOptions.GetCurSel();
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
