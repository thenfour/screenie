//
//
//
//
//

#include "stdafx.hpp"
#include "ImageEditWnd.hpp"
#include "image.hpp"

ATL::CWndClassInfo& CImageEditWindow::GetWndClassInfo()
{
  static ATL::CWndClassInfo wc =
	{
		{ sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, StartWindowProc,
		  0, 0, NULL, NULL, NULL, NULL, NULL, _T("ScreenieImageEditWnd"), NULL },
		NULL, NULL, IDC_ARROW, TRUE, 0, _T("")
	};
  wc.m_lpszCursorID = IDC_CROSS;
  wc.m_bSystemCursor = TRUE;
	return wc;
}


CImageEditWindow::CImageEditWindow(util::shared_ptr<Gdiplus::Bitmap> bitmap, IImageEditWindowEvents* pNotify) :
  m_bitmap(bitmap),
  m_currentTool(&m_penTool),
  m_notify(this),
  m_mouseEntrancy(false),
  m_bIsPanning(false)
{
  CopyImage(m_dibOriginal, *bitmap);
  m_view.SetZoomFactor(3);
  m_view.SetVirtualOrigin(Point<int>(250,250));

  if(pNotify)
  {
    m_notify = pNotify;
  }
}

bool CImageEditWindow::HasSelection() const
{
  return m_selectionTool.HasSelection();
}

void CImageEditWindow::ClearSelection()
{
  m_selectionTool.ClearSelection();
}

bool CImageEditWindow::GetVirtualSelection(RECT& selectionRect) const
{
  return m_selectionTool.GetSelection(selectionRect);
}

void CImageEditWindow::SetZoomFactor(float n)
{
  m_view.SetZoomFactor(n);
  BOOL temp;
  OnSize(0,0,0,temp);
  RedrawWindow(0, 0, RDW_INVALIDATE);
}

float CImageEditWindow::GetZoomFactor() const
{
  return m_view.GetZoomFactor();
}

util::shared_ptr<Gdiplus::Bitmap> CImageEditWindow::GetBitmapRect(const RECT& rectToCopy)
{
	Gdiplus::Bitmap* bitmapClone = m_bitmap->Clone(rectToCopy.left, rectToCopy.top,
		rectToCopy.right - rectToCopy.left, rectToCopy.bottom - rectToCopy.top, PixelFormatDontCare);

	return util::shared_ptr<Gdiplus::Bitmap>(bitmapClone);
}

// returns the amount to move the virtual cursor during a constrained move, based on a pixel mouse cursor delta
float GetDeltaMin(int val, bool bSuperDooper)
{
  if(!val) return 0;
  float blah = bSuperDooper ? 0.07f : 1.0f;
  return val < 0.0f ? -blah : blah;
}

LRESULT CImageEditWindow::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  bHandled = TRUE;
  if(!MouseEnter()) return 0;

  POINTS& psTemp = MAKEPOINTS(lParam);
	CPoint cursorPos(psTemp.x, psTemp.y);

  if(m_bIsPanning)
  {
    Point<int> delta;
    delta.x = m_panningStart.x - cursorPos.x;
    delta.y = m_panningStart.y - cursorPos.y;
    delta = m_view.ViewToVirtualSize(delta);
    Point<int> newOrg = m_panningStartVirtual;
    newOrg.x += delta.x;
    newOrg.y += delta.y;
    if(newOrg.x < 0) newOrg.x = 0;
    if(newOrg.y < 0) newOrg.y = 0;
    if(newOrg.x > (int)m_bitmap->GetWidth()) newOrg.x = (int)m_bitmap->GetWidth();
    if(newOrg.y > (int)m_bitmap->GetHeight()) newOrg.y = (int)m_bitmap->GetHeight();
    m_view.SetVirtualOrigin(newOrg);
    BOOL temp;
    OnSize(0,0,0,temp);
    RedrawWindow(0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
  }
  else
  {
    if((wParam & MK_SHIFT) == MK_SHIFT)
    {
      bool bSuperDooper = ((wParam & MK_CONTROL) == MK_CONTROL);

      // figure out how many virtual units to actually move.
      float deltax = GetDeltaMin(cursorPos.x - m_lastCursor.x, bSuperDooper);
      m_lastCursorVirtual.x += deltax;

      float deltay = GetDeltaMin(cursorPos.y - m_lastCursor.y, bSuperDooper);
      m_lastCursorVirtual.y += deltay;

      // bounds checking
      if(m_lastCursorVirtual.x < 0) m_lastCursorVirtual.x = 0;
      if(static_cast<DWORD>(m_lastCursorVirtual.x) > m_bitmap->GetWidth()) m_lastCursorVirtual.x = static_cast<float>(m_bitmap->GetWidth());

      if(m_lastCursorVirtual.y < 0) m_lastCursorVirtual.y = 0;
      if(static_cast<DWORD>(m_lastCursorVirtual.y) > m_bitmap->GetHeight()) m_lastCursorVirtual.y = static_cast<float>(m_bitmap->GetHeight());

      // replace the mouse cursor to reflect the slowdown.
      Point<int> newCursorPosX = m_view.VirtualToView(Point<int>(static_cast<int>(m_lastCursorVirtual.x), static_cast<int>(m_lastCursorVirtual.y)));
      CPoint newCursorPos(newCursorPosX.x, newCursorPosX.y);
      newCursorPos.Offset(viewMargin, viewMargin);

      cursorPos = newCursorPos;
      ClientToScreen(&newCursorPos);
      SetCursorPos(newCursorPos.x, newCursorPos.y);
    }
    else
    {
      cursorPos.Offset(-viewMargin, -viewMargin);
      m_lastCursorVirtual = m_view.ViewToVirtual<float>(Point<float>(
        static_cast<float>(cursorPos.x),
        static_cast<float>(cursorPos.y) ));
      cursorPos.Offset(viewMargin, viewMargin);
    }
  }

  m_lastCursor = cursorPos;

  // fire tool events.

  CPoint pt;
  pt.x = static_cast<LONG>(m_lastCursorVirtual.x);
  pt.y = static_cast<LONG>(m_lastCursorVirtual.y);
  m_notify->OnCursorPositionChanged(pt.x, pt.y);

  return MouseLeave();
}

LRESULT CImageEditWindow::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  if(!MouseEnter()) return 0;
  // fire tool events.
  return MouseLeave();
}

LRESULT CImageEditWindow::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  if(!MouseEnter()) return 0;
  // fire tool events.
  return MouseLeave();
}

LRESULT CImageEditWindow::OnRButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
  if(!MouseEnter()) return 0;
  m_bIsPanning = true;

  POINTS& psTemp = MAKEPOINTS(lParam);
  m_panningStart.SetPoint(psTemp.x, psTemp.y);
  m_panningStartVirtual = m_view.GetVirtualOrigin();
  SetCapture();
  m_hPreviousCursor = SetCursor(LoadCursor(0, IDC_HAND));
  return MouseLeave();
}

LRESULT CImageEditWindow::OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  if(!MouseEnter()) return 0;
  if(m_bIsPanning)
  {
    ReleaseCapture();
    SetCursor(m_hPreviousCursor);
    m_bIsPanning = false;
  }
  return MouseLeave();
}

LRESULT CImageEditWindow::OnSize(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*handled*/)
{
	CRect clientRect;
	GetClientRect(&clientRect);
  m_view.SetViewOrigin(Point<int>(clientRect.right/2, clientRect.bottom/2));
  if(m_dibOffscreen.GetWidth() < clientRect.right || m_dibOffscreen.GetHeight() < clientRect.bottom)
  {
    m_dibOffscreen.SetSize(clientRect.right, clientRect.bottom);
    m_dibStretched.SetSize(clientRect.right, clientRect.bottom);
  }
  m_dibStretched.Fill(0);

  Point<int> viewOrigin = m_view.GetViewOrigin();

  // figure out the dimensions of the original image to draw to the LEFT and TOP of the view origin.
  // this will tell us the upper-left coord where to draw the image
  Point<int> ulViewSize(
    viewOrigin.x + static_cast<int>(m_view.GetZoomFactor()),
    viewOrigin.y + static_cast<int>(m_view.GetZoomFactor()));
  Point<int> ulVirtualSize = m_view.ViewToVirtualSize(ulViewSize);
  ulViewSize = m_view.VirtualToViewSize(ulVirtualSize);// correct for rounding

  Point<int> ulDrawOrg(viewOrigin.x - ulViewSize.x, viewOrigin.y - ulViewSize.y);

  // figure out the upper-left coord of the original image
  Point<int> ulVirtualImage = m_view.ViewToVirtual(ulDrawOrg);

  // figure out the dimensions of the entire original image to draw
  Point<int> wholeViewSize(
    clientRect.Width() + static_cast<int>(m_view.GetZoomFactor()*2),
    clientRect.Height() + static_cast<int>(m_view.GetZoomFactor()*2));
  Point<int> wholeVirtualSize = m_view.ViewToVirtualSize(wholeViewSize);
  wholeViewSize = m_view.VirtualToViewSize(wholeVirtualSize);// correct for rounding

  m_dibOriginal.StretchBlit(m_dibStretched,
    viewMargin + ulDrawOrg.x,
    viewMargin + ulDrawOrg.y,
    wholeViewSize.x,
    wholeViewSize.y,
    ulVirtualImage.x,
    ulVirtualImage.y,
    wholeVirtualSize.x,
    wholeVirtualSize.y,
    COLORONCOLOR
    );

  // draw black borders.
  m_dibStretched.Rect(0, 0, clientRect.right, viewMargin, 0);// top
  m_dibStretched.Rect(0, clientRect.bottom - viewMargin, clientRect.right, clientRect.bottom, 0);// bottom
  m_dibStretched.Rect(0, viewMargin, viewMargin, clientRect.bottom - viewMargin, 0);// left
  m_dibStretched.Rect(clientRect.right - viewMargin, viewMargin, clientRect.right, clientRect.bottom - viewMargin, 0);

  return 0;
}

LRESULT CImageEditWindow::OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	PAINTSTRUCT paintStruct = { 0 };
	HDC dc = BeginPaint(&paintStruct);

	RECT clientRect;
	GetClientRect(&clientRect);

	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

  m_dibStretched.Blit(m_dibOffscreen, 0, 0, width, height);

  // send tool notifications.

  m_dibOffscreen.Blit(dc, 0, 0, width, height);

	EndPaint(&paintStruct);
	return 0;
}

