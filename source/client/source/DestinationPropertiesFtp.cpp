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
		bool destIsFTP = (m_parentSheet->GetCurrentType() == ScreenshotDestination::TYPE_FTP);
		::EnableChildWindows(m_hWnd, destIsFTP);

		CComboBox passwordOptions(GetDlgItem(IDC_PASSWORDOPTIONS));
		// indices must correspond directly to enum values.
		passwordOptions.Clear();
		passwordOptions.AddString(_T("Insecure; store as plain text"));
		passwordOptions.AddString(_T("Obscure; basic symmetric encryption"));
		passwordOptions.AddString(_T("Most secure, but only usable on this computer"));
		passwordOptions.SetCurSel((int)m_settings->ftp.passwordOptions);
		AutoSetComboBoxHeight(passwordOptions);

		SetDlgItemText(IDC_FTP_HOSTNAME, m_settings->ftp.hostname.c_str());
		SetDlgItemInt(IDC_FTP_PORT, m_settings->ftp.port, FALSE);
		SetDlgItemText(IDC_FTP_USERNAME, m_settings->ftp.username.c_str());
		SetDlgItemText(IDC_FTP_PASSWORD, m_settings->ftp.DecryptPassword().c_str());
		SetDlgItemText(IDC_FTP_REMOTEPATH, m_settings->ftp.remotePathFormat.c_str());
		SetDlgItemText(IDC_FTP_HTTPURL, m_settings->ftp.resultURLFormat.c_str());
		CheckDlgButton(IDC_FTP_COPYURL, m_settings->ftp.copyURL ? BST_CHECKED : BST_UNCHECKED);

		GetDlgItem(IDC_FTP_SHORTENURL).EnableWindow(destIsFTP && (IsDlgButtonChecked(IDC_FTP_COPYURL) == TRUE));
		CheckDlgButton(IDC_FTP_SHORTENURL, m_settings->ftp.shortenURL ? BST_CHECKED : BST_UNCHECKED);
	}
}

LRESULT CDestinationPropertiesFTP::OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	ShowSettings();

	return 0;
}

LRESULT CDestinationPropertiesFTP::OnCopyURLClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	GetDlgItem(IDC_FTP_SHORTENURL).EnableWindow(IsDlgButtonChecked(IDC_FTP_COPYURL) == TRUE);
	return 0;
}


//
// DestinationPropertyPage implementation
//

HPROPSHEETPAGE CDestinationPropertiesFTP::CreatePropertyPage()
{
	return CPropertyPageImpl<CDestinationPropertiesFTP>::Create();
}

void CDestinationPropertiesFTP::SetSettings(ScreenshotDestination* destination)
{
	m_settings = destination;

	if (IsWindow())
		ShowSettings();
}

void CDestinationPropertiesFTP::GetSettings()
{
	if (IsWindow())
	{
		m_settings->ftp.hostname = GetWindowString(GetDlgItem(IDC_FTP_HOSTNAME));
		m_settings->ftp.port = GetDlgItemInt(IDC_FTP_PORT, NULL, FALSE);
		m_settings->ftp.username = GetWindowString(GetDlgItem(IDC_FTP_USERNAME));
    m_settings->ftp.SetPassword(GetWindowString(GetDlgItem(IDC_FTP_PASSWORD)));

		m_settings->ftp.remotePathFormat = GetWindowString(GetDlgItem(IDC_FTP_REMOTEPATH));
		m_settings->ftp.resultURLFormat = GetWindowString(GetDlgItem(IDC_FTP_HTTPURL));

		m_settings->ftp.copyURL = (IsDlgButtonChecked(IDC_FTP_COPYURL) == TRUE);
		m_settings->ftp.shortenURL = (IsDlgButtonChecked(IDC_FTP_SHORTENURL) == TRUE);

		CComboBox passwordOptions(GetDlgItem(IDC_PASSWORDOPTIONS));
		m_settings->ftp.passwordOptions = (ScreenshotDestination::Ftp::PasswordOptions)passwordOptions.GetCurSel();
	}
}

void CDestinationPropertiesFTP::SetDestinationType(const ScreenshotDestination::Type type)
{
}

void CDestinationPropertiesFTP::SetParentSheet(DestinationPropertySheet* parentSheet)
{
	m_parentSheet = parentSheet;
}


void CDestinationPropertiesFTP::UpdatePreview(UINT src, UINT preview, UINT other)
{
	tstd::tstring formatString = GetWindowString(GetDlgItem(src));
	tstd::tstring otherFormatString = GetWindowString(GetDlgItem(other));

	SYSTEMTIME systemTime = { 0 };
  ::GetLocalTime(&systemTime);

	tstd::tstring formattedOutput = FormatFilename(systemTime, formatString, _T("Screenie"), true);
	SetDlgItemText(preview, formattedOutput.c_str());
}


LRESULT CDestinationPropertiesFTP::OnURLChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	UpdatePreview(IDC_FTP_HTTPURL, IDC_FTP_URLPREVIEW, IDC_FTP_REMOTEPATH);

	// ideally, we would see if you need to change the other one as well...
	// for example if you have
	//      url: http://lol/omg/wtf.png
	// and path: /blah/lol/omg/wtf.png
	// and you change URL to http://lol/omg/SHIT/wtf.png
	// we should update the path autamitacally to /blah/lol/omg/SHIT/wtf.png
	// but that's too complicated, and there may be cases where it needs to be turned off.

	UpdatePreview(IDC_FTP_REMOTEPATH, IDC_FTP_REMOTEPATHPREVIEW, IDC_FTP_HTTPURL);
	return 0;
}

LRESULT CDestinationPropertiesFTP::OnPathChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	UpdatePreview(IDC_FTP_REMOTEPATH, IDC_FTP_REMOTEPATHPREVIEW, IDC_FTP_HTTPURL);
	UpdatePreview(IDC_FTP_HTTPURL, IDC_FTP_URLPREVIEW, IDC_FTP_REMOTEPATH);
	return 0;
}

