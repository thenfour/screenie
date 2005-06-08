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


// interface for passing to things that d
struct StatusWindow
{
	enum MessageType
	{
		MSG_INFO,
    MSG_WARNING,
		MSG_ERROR,
    MSG_CHECK
	};

	virtual ~StatusWindow() { }

	virtual void ClearMessages() = 0;
	virtual void PrintMessage(const MessageType type, const tstd::tstring& destination, const tstd::tstring& message) = 0;
  virtual LPARAM CreateProgressMessage(const tstd::tstring& destination, const tstd::tstring& message) = 0;
  virtual void MessageSetIcon(LPARAM msgID, const MessageType type) = 0;
  virtual void MessageSetProgress(LPARAM msgID, int pos, int total) = 0;
  virtual void MessageSetText(LPARAM msgID, const tstd::tstring& msg) = 0;
};

class CStatusDlg :
	public StatusWindow,
	public CDialogImpl<CStatusDlg>,
	public CMessageFilter,
	public CDialogResize<CStatusDlg>
{
private:
	CImageList m_imageList;
  ProgressImages m_progress;
	CListViewCtrl m_listView;
  ScreenshotOptions& m_options;

  CriticalSection m_cs;
public:
	enum { IDD = IDD_STATUS };

  CStatusDlg(ScreenshotOptions& options) :
    m_options(options),
    m_hIconSmall(0),
    m_hIcon(0),
    m_nextMessageID(1)
  {
  }
	virtual ~CStatusDlg()
  {
  }

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	void CloseDialog(int nVal);
	
	BEGIN_MSG_MAP(CStatusDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		NOTIFY_HANDLER(IDC_MESSAGES, NM_RCLICK, OnRightClick)
		COMMAND_ID_HANDLER(ID_STATUSCONTEXTMENU_COPYTOCLIPBOARD, OnCopy)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		CHAIN_MSG_MAP(CDialogResize<CStatusDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CStatusDlg)
		DLGRESIZE_CONTROL(IDC_MESSAGES, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y | DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	//
	// StatusWindow implementation
	//

	void ClearMessages();
	void PrintMessage(const MessageType type, const tstd::tstring& destination,
		const tstd::tstring& message);

  // you don't have to create a Message for every item in the listview.  only ones that you need to deal with later.
  LPARAM CreateProgressMessage(const tstd::tstring& destination, const tstd::tstring& message);

	//
	// message handlers and whatnot
	//

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnRightClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

  HICON m_hIcon;
  HICON m_hIconSmall;

  void MessageSetIcon(LPARAM msgID, const MessageType type)
  {
    int item;
    if(-1 != (item = MessageIDToItemID(msgID)))
    {
      m_listView.SetItem(item, 0, LVIF_IMAGE, 0, MessageTypeToIconIndex(type), 0, 0, 0);
    }
  }

  void MessageSetProgress(LPARAM msgID, int pos, int total)
  {
    int item;
    if(-1 != (item = MessageIDToItemID(msgID)))
    {
      int iimage = m_progress.GetImageFromProgress(pos, total);
      if(pos >= total)
      {
        // 100% - use a special image.
        iimage = MessageTypeToIconIndex(MSG_CHECK);
      }
      m_listView.SetItem(item, 0, LVIF_IMAGE, 0, iimage, 0, 0, 0);
    }
  }

  void MessageSetText(LPARAM msgID, const tstd::tstring& msg)
  {
    int item;
    if(-1 != (item = MessageIDToItemID(msgID)))
    {
      m_listView.SetItemText(item, 1, msg.c_str());
    }
  }

private:
  LPARAM m_nextMessageID;
  int m_iconInfo;
  int m_iconWarning;
  int m_iconError;
  int m_iconCheck;

  int MessageIDToItemID(LPARAM msgID)
  {
    LVFINDINFO fi = {0};
    fi.flags = LVFI_PARAM;
    fi.lParam = msgID;
    return m_listView.FindItem(&fi, -1);
  }

  int MessageTypeToIconIndex(const MessageType& t)
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
    }
    return m_iconError;
  }
};

#endif