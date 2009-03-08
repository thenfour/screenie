//
// Copyright (c) 2003-2009 Roger Clark & Carl Corcoran
//

#ifndef SCREENIE_IMAGEEDITWND_HPP
#define SCREENIE_IMAGEEDITWND_HPP


#include "animbitmap.h"
#include "viewport.h"
#include "ToolBase.h"
#include "SelectionTool.h"
#include "HighlightTool.hpp"
#include "ImageEditRenderer.h"
#include <map>
#include <stack>
#include "resource.h"


class IImageEditWindowEvents
{
public:
  virtual void OnSelectionChanged() = 0;
  virtual void OnCursorPositionChanged(const PointF&) = 0;
  virtual void OnZoomFactorChanged() = 0;
  virtual void OnPaste(util::shared_ptr<Gdiplus::Bitmap> n) = 0;
	virtual void OnToolChanging(ToolBase* tool) = 0;
};


// simply specifies an amount to pan, in image coords
class PanningSpec
{
public:
  PanningSpec() :
    m_x(0),
    m_y(0)
  {
  }
  PanningSpec(int x, int y) :
    m_x(x),
    m_y(y)
  {
  }

  bool IsNotNull()
  {
    return (m_x != 0) || (m_y != 0);
  }

  int m_x;
  int m_y;
};


class CImageEditWindow :
  public CWindowImpl<CImageEditWindow>,
  public IImageEditWindowEvents,
  public IToolOperations
{
  // global const settings
  static const int patternFrequency = 8;

public:
  // ATL shit
  static ATL::CWndClassInfo& GetWndClassInfo();

  // IImageEditWindowEvents methods
  void OnCursorPositionChanged(const PointF&) { }
  void OnSelectionChanged() { }
  void OnZoomFactorChanged() { }
	void OnPaste(util::shared_ptr<Gdiplus::Bitmap> n) { }
	virtual void OnToolChanging(ToolBase* tool) { }

  // IToolOperations methods
  void Pan(int x, int y);
  PointF GetCursorPosition();
  int GetImageHeight() const;
  int GetImageWidth() const;
  void ClampToImage(PointF& p);
  UINT_PTR CreateTimer(UINT elapse, ToolTimerProc, void* userData);
  void DeleteTimer(UINT_PTR cookie);
	const Viewport& GetViewport() const { return m_display.GetViewport(); }
	void SetCursorPosition(const PointF& img);
	void ClearSelection()
	{
		m_display.ClearSelection();
		if(m_slave != 0)
			m_slave->ClearSelection();
		m_notify->OnSelectionChanged();
	}
	void SetSelection(const RECT& rc)
	{
		m_display.SetSelectionRect(rc);
		if(m_slave != 0)
			m_slave->SetSelection(rc);
		m_notify->OnSelectionChanged();
	}
	bool HasSelection() const { return m_display.HasSelection(); }
	CRect GetSelection() const { return m_display.GetSelection(); }
	void PushCursor(PCWSTR newcursor);
	void PopCursor(bool set);

	virtual AnimBitmap<32>* GetDocument()
	{
		return &m_dibDocument;
	}
	virtual void SetDocumentDirty()
	{
		m_display.SetOriginalImage(m_dibRenderSource);
	}
	virtual void Redraw()
	{
		m_display.Invalidate();
	}
	virtual void SetTemporarySurfaceDirty()
	{
		m_display.Invalidate();
	}

  // Our own shit
	CImageEditWindow(util::shared_ptr<Gdiplus::Bitmap> bitmap, IImageEditWindowEvents* pNotify);

	void Render(HDC target, CRect rc);
	void RenderOffscreen();

	void SetMaster(CImageEditWindow* s)
	{
		m_slave = 0;
		m_master = s;
		s->m_slave = this;
		s->m_master = 0;
		s->m_display.SetSlaveHWND(*this);
		m_display.SetOriginalImage(s->m_dibRenderSource);
	}

	void CenterOnImage(const PointF&);// centers the display on the given image coords

	ToolBase* GetCurrentTool() const
	{
		return m_currentTool;
	}

	void SwitchToTool(UINT resourceID)
	{
		ToolBase* newTool = 0;
		switch(resourceID)
		{
		case IDC_CROPPINGTOOL:
			newTool = &m_selectionTool;
			break;
		case IDC_HIGHLIGHTTOOL:
			newTool = &m_highlightTool;
			break;
		}

		if(newTool)
		{
			m_notify->OnToolChanging(newTool);
		}
		m_currentTool = newTool;

	}

	void ResetImage()
	{
		CopyImage(m_dibDocument, *m_bitmap.get());
		m_display.Invalidate();
	}

  // zoom
  void SetZoomFactor(float n);
  ViewPortSubPixel GetZoomFactor() const;

  void CenterImage();

	//void SetShowCursor(bool b) { m_showCursor = b; }
	//void SetEnablePanning(bool b) { m_enablePanning = b; }
	//void SetEnableTools(bool b) { m_enableTools = b; }

  // gets the specified section of the original image.
	util::shared_ptr<Gdiplus::Bitmap> GetBitmapRect(const RECT& rectToCopy);

	void SetBitmap(util::shared_ptr<Gdiplus::Bitmap> n);

	RgbPixel32 GetPixel_(CPoint p)
	{
		RgbPixel32 ret;
		if(!m_dibDocument.GetPixelSafe(ret, p.x, p.y))
		{
			return 0;
		}
		return ret;
	}

protected:

	BEGIN_MSG_MAP(CImageEditWindow)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
    MESSAGE_HANDLER(WM_CAPTURECHANGED, OnLoseCapture)
    MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)

		COMMAND_HANDLER(ID_EDIT_COPY, 0, OnCopy)
		COMMAND_HANDLER(ID_EDIT_PASTE, 0, OnPaste)
  END_MSG_MAP()

	LRESULT OnCommand(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		LibCC::LogScopeMessage l(LibCC::Format(L"OnCommand (loword:%, hiword:%)")(LOWORD(wParam))(HIWORD(wParam)).Str());
		return 0;
	}
	LRESULT OnContextMenu(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
  LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnRButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnLoseCapture(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
public:
	LRESULT OnPaste(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
protected:

  enum CursorMoveType
  {
    CM_NORMAL,
    CM_SLOW,
    CM_SUPERSLOW
  };

  // cursor stuff
	PointF m_lastCursorImage;// in image coords.
  PointF m_lastCursor;// in client coords
	bool m_isLeftClickDragging;

  // members
	util::shared_ptr<Gdiplus::Bitmap> m_bitmap;// the incoming bitmap. never modified.
	ImageEditRenderer m_display;
  AnimBitmap<32> m_dibDocument;// a cached image of the original bitmap. this is the "working" bitmap that tools can draw onto.
  AnimBitmap<32> m_dibRenderSource;// a cached image of the original bitmap. this is the "working" bitmap that tools can draw onto.
	AnimBitmap<32> m_offscreen;// backbuffer (m_display renders to this).

	CImageEditWindow* m_master;// the zoom window needs a pointer to the main one to be able to synchronize stuff.
	CImageEditWindow* m_slave;// the zoom window needs a pointer to the main one to be able to synchronize stuff.

	//bool m_showCursor;
	//bool m_enablePanning;
	//bool m_enableTools;
	bool EnableTools()
	{
		return m_master == 0;
	}

	void ClampImageOrigin(PointF& org)
	{
		RECT rcClient;
		GetClientRect(&rcClient);
		InflateRect(&rcClient, -max(10, rcClient.right / 10), -max(10, rcClient.bottom / 10));

		PointF v = m_display.GetViewport().GetViewOrigin();
		PointF a(rcClient.right - v.x, rcClient.bottom - v.y);// size of the rect from view_origin to the BR corner of the screen.
		a = m_display.GetViewport().ViewToImageSize(a);// convert that to image coords. violà!

		PointF b(v.x - rcClient.left, v.y - rcClient.top);
		b = m_display.GetViewport().ViewToImageSize(b);// for this one, need to also add the image size
		b.x += m_dibDocument.GetWidth();
		b.y += m_dibDocument.GetHeight();		

		if(org.x < -a.x) org.x = -a.x;
		if(org.y < -a.y) org.y = -a.y;
		if(org.x > b.x) org.x = b.x;
		if(org.y > b.y) org.y = b.y;
	}

	IImageEditWindowEvents* m_notify;

  // tool selection stuff
  ToolBase* m_currentTool;
  SelectionTool m_selectionTool;
	HighlightTool m_highlightTool;

  // timer crap (ugh all this could be avoided if SetTimer() had a userdata arg.
  std::map<UINT_PTR, std::pair<ToolTimerProc, void*> > m_timerMap;// maps 'this' pointer to proc
  UINT_PTR m_nextTimerID;

  bool MouseEnter()
  {
    if(m_mouseEntrancy) return false;
    return m_mouseEntrancy = true;
  }
  LRESULT MouseLeave()
  {
    m_mouseEntrancy = false;
    return 0;
  }
  bool m_mouseEntrancy;

  // panning.
  bool m_bIsPanning;// true during right-mouse button dragging
	bool m_actuallyDidPanning;

	// Capture
	void AddCapture();
	void ReleaseCapture();
	inline bool HaveCapture() { return 0 != m_captureRefs; }
	int m_captureRefs;

  // for panning when the cursor is off the display & we have mouse capture
  PanningSpec m_panningSpec;
  UINT_PTR m_panningTimer;
  PanningSpec CImageEditWindow::GetPanningSpec();
  void Pan(const PanningSpec& ps);
	void KillPanningTimer();
  static void PanningTimerProc(void* pUser)
  {
    // pan based on current velocity / direction
    CImageEditWindow* pThis = reinterpret_cast<CImageEditWindow*>(pUser);
    pThis->Pan(pThis->m_panningSpec);
  }

	std::stack<HCURSOR> m_cursorStack;
};

#endif

