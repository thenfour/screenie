#ifndef SCREENIE_STATUSDLG_HPP
#define SCREENIE_STATUSDLG_HPP

#include "ScreenshotOptions.hpp"
#include <vector>
#include "ScreenshotArchive.hpp"
#include "ActivityList.hpp"


class CStatusDlg :
	public IActivity,
	public CDialogImpl<CStatusDlg>,
	public CMessageFilter,
	public CDialogResize<CStatusDlg>
{
private:
	ActivityList m_activity;
  ScreenshotOptions& m_options;
  ScreenshotArchive& m_archive;
public:
	enum { IDD = IDD_STATUS };

  CStatusDlg(ScreenshotOptions& options, ScreenshotArchive& archive) :
    m_options(options),
		m_archive(archive),
		m_activity(archive, options),
    m_hIconSmall(0),
    m_hIcon(0)
  {
  }

	virtual ~CStatusDlg()
  {
  }

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	void CloseDialog(int nVal);
  void ClearMessages();

	BEGIN_MSG_MAP(CStatusDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)

		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
		MESSAGE_HANDLER(WM_DELETEITEM, OnDeleteItem)

		COMMAND_ID_HANDLER(IDOK, OnOK)
		CHAIN_MSG_MAP(CDialogResize<CStatusDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CStatusDlg)
		DLGRESIZE_CONTROL(IDC_ACTIVITY, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CLEAR, DLSZ_MOVE_Y | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_REMOVE, DLSZ_MOVE_Y | DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	//
	// IActivity implementation
	//
	ScreenshotID RegisterScreenshot(Gdiplus::BitmapPtr image, Gdiplus::BitmapPtr thumbnail);
	EventID RegisterEvent(ScreenshotID screenshotID, EventIcon icon, EventType type, const std::wstring& destination, const std::wstring& message, const std::wstring& url = L"");
	void EventSetIcon(EventID eventID, EventIcon icon);
	void EventSetProgress(EventID eventID, int pos, int total);
	void EventSetText(EventID eventID, const std::wstring& msg);
	void EventSetURL(EventID eventID, const std::wstring& url);
	void DeleteEvent(EventID eventID);
	void DeleteScreenshot(ScreenshotID screenshotID);

	//
	// message handlers and whatnot
	//

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGrumbleNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	
	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMeasureItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnDeleteItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	LRESULT OnRightClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

  LRESULT OnOpenURL(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnCopyURL(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopyMessage(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnClear(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnExplore(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnOpenFile(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

  LRESULT OnDeleteAllItems(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
  LRESULT OnDeleteItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

  UINT m_uMsgGrumbleNotify;
  HICON m_hIcon;
  HICON m_hIconSmall;
};

#endif