//
// CroppingWnd.hpp - cropping functionality
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


class ToolBase
{
public:
  virtual void OnSelectTool() = 0;
  virtual void OnDeselectTool() = 0;
  virtual void OnCursorMove(CPoint p) = 0;
  virtual void OnLeftClick(CPoint p) = 0;
  virtual void OnRightClick(CPoint p) = 0;
  virtual void OnPaint(AnimBitmap<32>& img, const Viewport<int>& view) = 0;
};

class PencilTool : public ToolBase
{
public:
  void OnSelectTool() { }
  void OnDeselectTool() { }
  void OnCursorMove(CPoint p) { }
  void OnLeftClick(CPoint p) { }
  void OnRightClick(CPoint p) { }
  void OnPaint(AnimBitmap<32>& img, const Viewport<int>& view) { }
};

class SelectionTool : public ToolBase
{
public:
  void OnSelectTool() { }
  void OnDeselectTool() { }
  void OnCursorMove(CPoint p) { }
  void OnLeftClick(CPoint p) { }
  void OnRightClick(CPoint p) { }
  void OnPaint(AnimBitmap<32>& img, const Viewport<int>& view) { }

  bool GetSelection(RECT&) const { return false; }
  void ClearSelection() { }
  bool HasSelection() const { return false; }
  /*
  SetTimer(0, 120);
  if(m_hasSelection)
  {
    m_selectionOffset ++;
    if(m_selectionOffset >= patternFrequency)
    {
      m_selectionOffset = 0;
    }
	  RECT rectInvalidate;
    GetScreenSelection(rectInvalidate);
    ::InflateRect(&rectInvalidate, 5, 5);
    RedrawWindow(&rectInvalidate, 0, RDW_INVALIDATE | RDW_UPDATENOW);
  }
  */
};

class TextTool : public ToolBase
{
public:
  void OnSelectTool() { }
  void OnDeselectTool() { }
  void OnCursorMove(CPoint p) { }
  void OnLeftClick(CPoint p) { }
  void OnRightClick(CPoint p) { }
  void OnPaint(AnimBitmap<32>& img, const Viewport<int>& view) { }
};

class IImageEditWindowEvents
{
public:
  virtual void OnCroppingSelectionChanged() = 0;
  virtual void OnCursorPositionChanged(int x, int y) = 0;
};


class CImageEditWindow :
  public CWindowImpl<CImageEditWindow>,
  public IImageEditWindowEvents
{
  // global const settings
  static const int viewMargin = 15;
  static const int patternFrequency = 8;

public:
  // ATL shit
  static ATL::CWndClassInfo& GetWndClassInfo();

  // IImageEditWindowEvents methods
  void OnCroppingSelectionChanged() { }
  void OnCursorPositionChanged(int x, int y) { }

  // Our own shit
	CImageEditWindow(util::shared_ptr<Gdiplus::Bitmap> bitmap, IImageEditWindowEvents* pNotify);

  // selection
	bool HasSelection() const;
	void ClearSelection();
	bool GetVirtualSelection(RECT& selectionRect) const;

  // zoom
  void SetZoomFactor(float n);
  float GetZoomFactor() const;

  // gets the specified section of the original image.
	util::shared_ptr<Gdiplus::Bitmap> GetBitmapRect(const RECT& rectToCopy);

protected:

	BEGIN_MSG_MAP(CImageEditWindow)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
	END_MSG_MAP()

	LRESULT OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
  LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnRButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

  // cursor stuff
	Point<float> m_lastCursorVirtual;// used for "slowing down" cursor movement... these are in VIRTUAL coords.
  CPoint m_lastCursor;// in client coords

  // members
	util::shared_ptr<Gdiplus::Bitmap> m_bitmap;// the incoming bitmap.
  AnimBitmap<32> m_dibOriginal;
  AnimBitmap<32> m_dibStretched;
  AnimBitmap<32> m_dibOffscreen;
  Viewport<int> m_view;
  IImageEditWindowEvents* m_notify;

  // tool selection stuff
  ToolBase* m_currentTool;
  PencilTool m_penTool;
  SelectionTool m_selectionTool;

  bool m_mouseEntrancy;

  // panning.
  bool m_bIsPanning;
};

#endif

