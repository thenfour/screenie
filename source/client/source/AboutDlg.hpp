//
//
//
//
//

#ifndef SCREENIE_ABOUTDLG_HPP
#define SCREENIE_ABOUTDLG_HPP

#include "xversion.h"
#include "utility.hpp"

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUTBOX };

  CAboutDlg() :
    m_hIconSmall(0),
    m_hIcon(0)
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
