// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//
//
//
//
//

#ifndef SCREENIE_ABOUTDLG_HPP
#define SCREENIE_ABOUTDLG_HPP

#include "xversion.h"
#include "utility.hpp"
#include "ScreenshotOptions.hpp"
#include "ScreenshotArchive.hpp"

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUTBOX };
	const ScreenshotOptions& m_options;
	const ScreenshotArchive& m_archive;

	CAboutDlg(const ScreenshotOptions& opt, const ScreenshotArchive& archive) :
    m_hIconSmall(0),
    m_hIcon(0),
		m_options(opt),
		m_archive(archive)
  {
  }

  ~CAboutDlg()
  {
    if(m_hIcon) DestroyIcon(m_hIcon);
    if(m_hIconSmall) DestroyIcon(m_hIconSmall);
  }

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
	  // set icons
    if(m_hIcon) DestroyIcon(m_hIcon);
    if(m_hIconSmall) DestroyIcon(m_hIconSmall);
	  m_hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SCREENIE), 
		  IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	  SetIcon(m_hIcon, TRUE);
	  m_hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SCREENIE), 
		  IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	  SetIcon(m_hIconSmall, FALSE);

    // set version text based on resources
    Version v;
    v.FromFile(GetModuleFileNameX().c_str());
    SetDlgItemText(IDC_PRODUCTVERSION, LibCC::Format("Screenie v%.%.%").ui(v.GetA()).ui(v.GetB()).ui(v.GetC()).CStr());
    SetDlgItemText(IDC_COPYRIGHT, v.GetCopyright().c_str());
    SetDlgItemText(IDC_REGISTRANT, v.GetRegistrant().c_str());

		// other stuff
		std::wstring archiveSize = L"[error]";
		DWORD s;
		if(m_archive.GetDBFileSize(s))
		{
			archiveSize = BytesToString(s);
		}
		LibCC::Format infoText = LibCC::Format(
			"Application:|  %|"
			"Configuration:|  %|"
			"Archive:|  %|"
			"Archive size:|  %|"
			"%"
			)
			(GetModuleFileNameX())
			(m_options.GetConfigPath())
			(m_archive.GetDBFilename())
			(archiveSize)
			(v.GetFileDescription());

		SetDlgItemText(IDC_INFO, infoText.CStr());

    CenterWindow(GetParent());
		m_link.SubclassWindow(GetDlgItem(IDC_HYPERLINK));

		return TRUE;
	}

	LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);

		return 0;
	}

	CHyperLink m_link;

  HICON m_hIcon;
  HICON m_hIconSmall;
};

#endif
