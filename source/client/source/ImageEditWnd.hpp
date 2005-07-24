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
#include <map>


class IImageEditWindowEvents
{
public:
  virtual void OnSelectionChanged() = 0;
  virtual void OnCursorPositionChanged(int x, int y) = 0;
  virtual void OnZoomFactorChanged() = 0;
};


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
  public IToolOperations,
  public ISelectionToolCallback
{
  // global const settings
  static const int patternFrequency = 8;

public:
  // ATL shit
  static ATL::CWndClassInfo& GetWndClassInfo();

  // IImageEditWindowEvents methods
  void OnCursorPositionChanged(int x, int y) { }
  void OnSelectionChanged() { };
  void OnZoomFactorChanged() { };

  // ISelectionToolCallback methods
  void OnSelectionToolSelectionChanged();

  // IToolOperations methods
  void Pan(int x, int y, bool updateNow);
  PointF GetCursorPosition();
  int GetImageHeight();
  int GetImageWidth();
  void ClampToImage(PointI& p);
  void ClampToImage(PointF& p);
  void Refresh(bool now);
  void Refresh(const RECT& imageCoords, bool now);
  UINT_PTR CreateTimer(UINT elapse, ToolTimerProc, void* userData);
  void DeleteTimer(UINT_PTR cookie);

  // Our own shit
	CImageEditWindow(util::shared_ptr<Gdiplus::Bitmap> bitmap, IImageEditWindowEvents* pNotify);

  // selection
	bool HasSelection() const;
	void ClearSelection();
	bool GetVirtualSelection(RECT& selectionRect) const;

  // zoom
  void SetZoomFactor(float n);
  float GetZoomFactor() const;

  void CenterImage();

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

  void ResetOffscreenBitmaps();

  enum CursorMoveType
  {
    CM_NORMAL,
    CM_SLOW,
    CM_SUPERSLOW
  };

  // cursor stuff
	PointF m_lastCursorVirtual;// virtual coords.
  CPoint m_lastCursor;// in client coords

  // members
	util::shared_ptr<Gdiplus::Bitmap> m_bitmap;// the incoming bitmap.
  AnimBitmap<32> m_dibOriginal;
  AnimBitmap<32> m_dibStretched;
  AnimBitmap<32> m_dibOffscreen;
  Viewport m_view;
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
  bool m_bIsPanning;
  HCURSOR m_hPreviousCursor;
  CPoint m_panningStart;// in client coords
  PointF m_panningStartVirtual;// virtual coords.
  bool m_haveCapture;

  // for panning when the cursor is off the display & we have mouse capture
  PanningSpec m_panningSpec;
  UINT_PTR m_panningTimer;
  PanningSpec CImageEditWindow::GetPanningSpec();
  void Pan(const PanningSpec& ps, bool updateNow);
  static void PanningTimerProc(void* pUser)
  {
    // pan based on current velocity / direction
    CImageEditWindow* pThis = reinterpret_cast<CImageEditWindow*>(pUser);
    pThis->Pan(pThis->m_panningSpec, true);
  }
};

#endif

