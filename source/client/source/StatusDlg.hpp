#ifndef SCREENIE_STATUSDLG_HPP
#define SCREENIE_STATUSDLG_HPP

#include "ScreenshotOptions.hpp"
#include <vector>
#include "polarlut.h"
#include "animbitmap.h"


class ProgressImages
{
public:
  void InitializeProgressImages(CImageList& img, RgbPixel32 background, RgbPixel32 filled, RgbPixel32 unfilled);
  int GetImageFromProgress(int pos, int total);
private:
  /*
    this will hold linear from 0-perimiter.
    assume that the image list indices will not change.
  */
  std::vector<int> m_images;
  AngleLut<float> m_angles;

  void DrawHLine(long x1, long x2, long y);
  void DrawAlphaPixel(long cx, long cy, long x, long y, long f, long fmax);
  RgbPixel32 PositionToColor(long x, long y);

  // stuff that we persist between drawing calls
  AnimBitmap<32>* m_bmp;
  int m_i;

  float m_pieBlurringSize;
  int m_perimeter;
  int m_diameter;
  int m_radius;
  RgbPixel32 m_background;
  RgbPixel32 m_unfilled;
  RgbPixel32 m_filled;
};


struct AsyncStatusWindow
{
	enum MessageIcon
	{
		MSG_INFO,
    MSG_WARNING,
		MSG_ERROR,
    MSG_CHECK,
    MSG_PROGRESS
	};
  enum MessageType
  {
    ITEM_GENERAL,
    ITEM_FTP,
    ITEM_FILE
  };

  virtual LPARAM AsyncCreateMessage(const MessageIcon icon, const MessageType type, const tstd::tstring& destination, const tstd::tstring& message, const tstd::tstring& url = _T("")) = 0;
  virtual void AsyncMessageSetIcon(LPARAM msgID, const MessageIcon icon) = 0;
  virtual void AsyncMessageSetProgress(LPARAM msgID, int pos, int total) = 0;
  virtual void AsyncMessageSetText(LPARAM msgID, const tstd::tstring& msg) = 0;
  virtual void AsyncMessageSetURL(LPARAM msgID, const tstd::tstring& url) = 0;
};

class CStatusDlg :
	public AsyncStatusWindow,
	public CDialogImpl<CStatusDlg>,
	public CMessageFilter,
	public CDialogResize<CStatusDlg>
{
private:
	CImageList m_imageList;
  ProgressImages m_progress;
	CListViewCtrl m_listView;
  ScreenshotOptions& m_options;

  static const short ID_COPYURL = 4000;
  static const short ID_COPYMESSAGE = 4001;
  static const short ID_CLEAR = 4002;
  static const short ID_EXPLORE = 4003;
  static const short ID_OPENFILE = 4004;
  static const short ID_OPENURL = 4005;

  CriticalSection m_cs;
public:
	enum { IDD = IDD_STATUS };

  CStatusDlg(ScreenshotOptions& options) :
    m_options(options),
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
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		NOTIFY_HANDLER(IDC_MESSAGES, NM_RCLICK, OnRightClick)

    NOTIFY_HANDLER(IDC_MESSAGES, LVN_DELETEALLITEMS, OnDeleteAllItems)
    NOTIFY_HANDLER(IDC_MESSAGES, LVN_DELETEITEM, OnDeleteItem)

    COMMAND_ID_HANDLER(ID_COPYURL, OnCopyURL)
    COMMAND_ID_HANDLER(ID_COPYMESSAGE, OnCopyMessage)
    COMMAND_ID_HANDLER(ID_CLEAR, OnClear)
    COMMAND_ID_HANDLER(ID_EXPLORE, OnExplore)
    COMMAND_ID_HANDLER(ID_OPENFILE, OnOpenFile)
    COMMAND_ID_HANDLER(ID_OPENURL, OnOpenURL)

		COMMAND_ID_HANDLER(IDOK, OnOK)
		CHAIN_MSG_MAP(CDialogResize<CStatusDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CStatusDlg)
		DLGRESIZE_CONTROL(IDC_MESSAGES, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y | DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	//
	// AsyncStatusWindow implementation
	//
  LPARAM AsyncCreateMessage(const MessageIcon icon, const MessageType type, const tstd::tstring& destination, const tstd::tstring& message, const tstd::tstring& url = _T(""));
  void AsyncMessageSetIcon(LPARAM msgID, const MessageIcon icon);
  void AsyncMessageSetProgress(LPARAM msgID, int pos, int total);
  void AsyncMessageSetText(LPARAM msgID, const tstd::tstring& msg);
  void AsyncMessageSetURL(LPARAM msgID, const tstd::tstring& url);

	//
	// message handlers and whatnot
	//

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

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
  int m_iconInfo;
  int m_iconWarning;
  int m_iconError;
  int m_iconCheck;

  /*
    Stuff for dealing with the LPARAM for each item.
  */
  struct ItemSpec
  {
    MessageType type;
    tstd::tstring url;
  };

  int MessageIDToItemID(LPARAM msgID);
  ItemSpec* MessageIDToItemSpec(LPARAM msgID);// returns 0 if not found.
  ItemSpec* ItemToItemSpec(int id);// returns 0 if not found.
  ItemSpec* GetSelectedItemSpec();// returns 0 if not found.

  int MessageIconToIconIndex(const MessageIcon& t)
  {
    switch(t)
    {
    case MSG_INFO:
      return m_iconInfo;
    case MSG_WARNING:
      return m_iconWarning;
    case MSG_ERROR:
      return m_iconError;
    case MSG_CHECK:
      return m_iconCheck;
    case MSG_PROGRESS:
      return m_progress.GetImageFromProgress(0,1);// just return 0%
    }
    return m_iconError;
  }
};

#endif