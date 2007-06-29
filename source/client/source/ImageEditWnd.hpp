//
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//
/*
  Major features of the edit window:
  1) selection rectangle (left-mouse drags)
  2) panning (right-drag)
  3) zooming (mouse-wheel)

  Ok let's plan for the future here... tools.  Let's design 1 "tool" to add to the
  arsenal to kickstart the longterm design.  Selection tool is of course 1 tool.
  Let's do the Text edit tool also, because it's so 

  Some words about the tool base class.  OnPaint should be called even if the tool
  is not selected.  That way the tool can have the option of painting stuff at all times.
  Selection tool should be showing selection all the time.

  In the future the tools should be managed in an external toolcollection class or something
*/


#ifndef SCREENIE_IMAGEEDITWND_HPP
#define SCREENIE_IMAGEEDITWND_HPP


#include "animbitmap.h"
#include "viewport.h"
#include "ToolBase.h"
#include "SelectionTool.h"
#include "ImageEditRenderer.h"
#include <map>
#include <stack>


class IImageEditWindowEvents
{
public:
  virtual void OnSelectionChanged() = 0;
  virtual void OnCursorPositionChanged(const PointF&) = 0;
  virtual void OnZoomFactorChanged() = 0;
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
  void OnSelectionChanged() { };
  void OnZoomFactorChanged() { };

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
		m_notify->OnSelectionChanged();
	}
	void SetSelection(const RECT& rc)
	{
		m_display.SetSelectionRect(rc);
		m_notify->OnSelectionChanged();
	}
	bool HasSelection() const { return m_display.HasSelection(); }
	CRect GetSelection() const { return m_display.GetSelection(); }
	void PushCursor(PCWSTR newcursor);
	void PopCursor(bool set);

  // Our own shit
	CImageEditWindow(util::shared_ptr<Gdiplus::Bitmap> bitmap, IImageEditWindowEvents* pNotify);
	void CenterOnImage(const PointF&);// centers the display on the given image coords

  // zoom
  void SetZoomFactor(float n);
  ViewPortSubPixel GetZoomFactor() const;

  void CenterImage();

	void SetShowCursor(bool b) { m_showCursor = b; }
	void SetEnablePanning(bool b) { m_enablePanning = b; }
	void SetEnableTools(bool b) { m_enableTools = b; }

  // gets the specified section of the original image.
	util::shared_ptr<Gdiplus::Bitmap> GetBitmapRect(const RECT& rectToCopy);

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
  END_MSG_MAP()

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
	util::shared_ptr<Gdiplus::Bitmap> m_bitmap;// the incoming bitmap.
	ImageEditRenderer m_display;
  AnimBitmap<32> m_dibOriginal;// a cached image of the original bitmap.
	AnimBitmap<32> m_offscreen;// backbuffer.

	bool m_showCursor;
	bool m_enablePanning;
	bool m_enableTools;

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
		b.x += m_dibOriginal.GetWidth();
		b.y += m_dibOriginal.GetHeight();		

		if(org.x < -a.x) org.x = -a.x;
		if(org.y < -a.y) org.y = -a.y;
		if(org.x > b.x) org.x = b.x;
		if(org.y > b.y) org.y = b.y;
	}

	IImageEditWindowEvents* m_notify;

  // tool selection stuff
  ToolBase* m_currentTool;
  SelectionTool m_selectionTool;

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
  //HCURSOR m_hPreviousCursor;

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

