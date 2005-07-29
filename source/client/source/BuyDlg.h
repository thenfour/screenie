//
//
//
//
//

#ifndef SCREENIE_BUYDLG_HPP
#define SCREENIE_BUYDLG_HPP


class CBuyDlg : public CDialogImpl<CBuyDlg>
{
public:
	enum { IDD = IDD_BUY };

  CBuyDlg() :
    m_hIconSmall(0),
    m_hIcon(0)
  {
  }

  ~CBuyDlg()
  {
    if(m_hIcon) DestroyIcon(m_hIcon);
    if(m_hIconSmall) DestroyIcon(m_hIconSmall);
  }

	BEGIN_MSG_MAP(CBuyDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_BUY, OnBuy)

		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

  tstd::tstring m_featureText;

  void DoModal(const tstd::tstring& feature)
  {
    m_featureText = feature;
    CDialogImpl<CBuyDlg>::DoModal();
  }

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
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
    CenterWindow(GetParent());

    SetDlgItemText(IDC_DESCRIPTION,
      LibCC::Format("The demo version of Screenie does not allow %. "
      "Please buy the full version from the URL below.").s(m_featureText).CStr());

    ::SetFocus(GetDlgItem(IDC_BUY));

    m_link.SubclassWindow(GetDlgItem(IDC_HYPERLINK));

		return FALSE;
	}

	LRESULT OnBuy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  {
    m_link.Navigate();
    return 0;
  }

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	CHyperLink m_link;

  HICON m_hIcon;
  HICON m_hIconSmall;
};

#endif
