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
	//CImageList m_imageList;
  //ProgressImages m_progress;
	//CListViewCtrl m_listView;
	ActivityList m_activity;
  ScreenshotOptions& m_options;
  ScreenshotArchive& m_archive;

  //static const short ID_COPYURL = 4000;
  //static const short ID_COPYMESSAGE = 4001;
  //static const short ID_CLEAR = 4002;
  //static const short ID_EXPLORE = 4003;
  //static const short ID_OPENFILE = 4004;
  //static const short ID_OPENURL = 4005;

  //CriticalSection m_cs;
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
		
		//MESSAGE_HANDLER(WM_CHAR, OnChar)
		//NOTIFY_HANDLER(IDC_MESSAGES, NM_RCLICK, OnRightClick)

  //  NOTIFY_HANDLER(IDC_MESSAGES, LVN_DELETEALLITEMS, OnDeleteAllItems)
  //  NOTIFY_HANDLER(IDC_MESSAGES, LVN_DELETEITEM, OnDeleteItem)

  //  COMMAND_ID_HANDLER(ID_COPYURL, OnCopyURL)
  //  COMMAND_ID_HANDLER(ID_COPYMESSAGE, OnCopyMessage)
  //  COMMAND_ID_HANDLER(ID_CLEAR, OnClear)
  //  COMMAND_ID_HANDLER(ID_EXPLORE, OnExplore)
  //  COMMAND_ID_HANDLER(ID_OPENFILE, OnOpenFile)
  //  COMMAND_ID_HANDLER(ID_OPENURL, OnOpenURL)

		COMMAND_ID_HANDLER(IDOK, OnOK)
		CHAIN_MSG_MAP(CDialogResize<CStatusDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CStatusDlg)
		//DLGRESIZE_CONTROL(IDC_MESSAGES, DLSZ_SIZE_X)
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

  HICON m_hIcon;
  HICON m_hIconSmall;

private:
  //int m_iconInfo;
  //int m_iconWarning;
  //int m_iconError;
  //int m_iconCheck;

  /*
    Stuff for dealing with the LPARAM for each item.
  */
  //struct ItemSpec
  //{
		//ScreenshotID screenshotID;
  //  EventType type;
  //  tstd::tstring url;
		//EventID archiveID;
		//EventID activityListID;
  //};

  //int EventIDToItemID(EventID msgID);
  //ItemSpec* EventIDToItemSpec(EventID msgID);// returns 0 if not found.
  //ItemSpec* ItemToItemSpec(int id);// returns 0 if not found.
  //ItemSpec* GetSelectedItemSpec();// returns 0 if not found.

  //int EventIconToIconIndex(const EventIcon& t)
  //{
  //  switch(t)
  //  {
  //  case EI_INFO:
  //    return m_iconInfo;
  //  case EI_WARNING:
  //    return m_iconWarning;
  //  case EI_ERROR:
  //    return m_iconError;
  //  case EI_CHECK:
  //    return m_iconCheck;
  //  case EI_PROGRESS:
  //    return m_progress.GetImageFromProgress(0,1);// just return 0%
  //  }
  //  return m_iconError;
  //}

	// for THIS class's return ScreenshotID, well just use the Archive's returned ID.
	// but i need a way to map that to activity's ScreenshotIDs.
	//typedef std::map<ScreenshotID, ScreenshotID> ScreenshotIDMap;// maps archive's IDs to activity list's
	//ScreenshotIDMap m_screenshotIDMap;
	// NOTE that THIS class's returned EventIDs are pointers to ItemSpec, not any sequential ID.
};

#endif