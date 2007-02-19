//
//
//
//
//

#ifndef SCREENIE_TEXTPROMPTDLG_H
#define SCREENIE_TEXTPROMPTDLG_H

#include "utility.hpp"

class CTextPromptDlg : public CDialogImpl<CTextPromptDlg>
{
public:
	enum { IDD = IDD_PROMPT };

	CTextPromptDlg(const tstd::tstring& title, const tstd::tstring& label, const tstd::tstring& text)
		: m_title(title), m_label(label), m_text(text)
	{
	}

	~CTextPromptDlg()
	{
	}

	tstd::tstring GetText() const
	{
		return m_text;
	}
private:
	BEGIN_MSG_MAP(CTextPromptDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		SetWindowText(title.c_str());
		SetDlgItemText(IDC_PROMPT_LABEL, m_label.c_str());

		return TRUE;
	}

	LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (wID == IDOK)
			m_text = GetWindowString(GetDlgItem(IDC_PROMPT_TEXT));

		EndDialog(wID);

		return 0;
	}

	tstd::tstring m_title;
	tstd::tstring m_label;
	tstd::tstring m_text;
};

#endif
