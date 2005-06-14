//
//
//
//
//

#include "stdafx.hpp"
#include "resource.h"

#include <algorithm>
#include "image.hpp"

#include "CroppingWnd.hpp"


CCroppingWindow::CCroppingWindow(util::shared_ptr<Gdiplus::Bitmap> bitmap, ICroppingWindowEvents* pNotify) :
  m_bitmap(bitmap),
  m_hasSelection(false),
  m_notify(pNotify == 0 ? this : pNotify),
  m_selectionEntrancy(false)
{
  CopyImage(m_dibOriginal, *bitmap);
}

CCroppingWindow::~CCroppingWindow()
{
}

BOOL CCroppingWindow::PreTranslateMessage(MSG* pMsg)
{
	return TRUE;
}

void CCroppingWindow::ClearSelection()
{
	m_hasSelection = false;

	RECT rectInvalidate;
  GetScreenSelection(rectInvalidate);
  m_notify->OnCroppingSelectionChanged();
  ::InflateRect(&rectInvalidate, 5, 5);
	InvalidateRect(&rectInvalidate);
}

CPoint CCroppingWindow::ClientToImageCoords(CPoint x)
{
  RECT rcClient;
  GetClientRect(&rcClient);
  rcClient.right -= g_CropBorder * 2;
  rcClient.bottom -= g_CropBorder * 2;

  x.x -= g_CropBorder;
  x.y -= g_CropBorder;

  // bounds checking
  if(x.x < 0) x.x = 0;
  if(x.y < 0) x.y = 0;

  // x : screen :: ret : image;
  // aka x * image / screen = ret;
  CPoint ret;
  ret.x = (int)(0.5f + ((float)x.x * (m_bitmap->GetWidth()) / rcClient.right));
  ret.y = (int)(0.5f + ((float)x.y * (m_bitmap->GetHeight()) / rcClient.bottom));

  // bounds checking
  if(static_cast<DWORD>(ret.x) > m_bitmap->GetWidth()) { ret.x = m_bitmap->GetWidth(); }
  if(static_cast<DWORD>(ret.y) > m_bitmap->GetHeight()) { ret.y = m_bitmap->GetHeight(); }

  return ret;
}

CPoint CCroppingWindow::ImageToClient(CPoint p)
{
  CPoint ret;
  RECT rcClient;
  GetClientRect(&rcClient);
  rcClient.bottom -= g_CropBorder * 2;
  rcClient.right -= g_CropBorder * 2;

  // scale p : imagesize :: ret : client
  // we do need float ops here because we need to do rounding up.  this is to get exact pixel precision.
  ret.y = (int)(0.5f + ((float)p.y * rcClient.bottom / m_bitmap->GetHeight()));
  ret.x = (int)(0.5f + ((float)p.x * rcClient.right / m_bitmap->GetWidth()));

  ret.x += g_CropBorder;
  ret.y += g_CropBorder;
  return ret;
}

// returns the selection rect in screen coords (relative to client)
void CCroppingWindow::GetScreenSelection(RECT& rc)
{
  CPoint ul = ImageToClient(CPoint(m_selectionOrg.left, m_selectionOrg.top));
  CPoint br = ImageToClient(CPoint(m_selectionOrg.right, m_selectionOrg.bottom));
  SetRect(&rc, ul.x, ul.y, br.x, br.y);
}

bool CCroppingWindow::GetSelection(RECT& selectionRect)
{
	if (!m_hasSelection) return false;
  CopyRect(&selectionRect, &m_selectionOrg);
	return true;
}

util::shared_ptr<Gdiplus::Bitmap> CCroppingWindow::GetBitmapRect(const RECT& rectToCopy)
{
	Gdiplus::Bitmap* bitmapClone = m_bitmap->Clone(rectToCopy.left, rectToCopy.top,
		rectToCopy.right - rectToCopy.left, rectToCopy.bottom - rectToCopy.top, PixelFormatDontCare);

	return util::shared_ptr<Gdiplus::Bitmap>(bitmapClone);
}

LRESULT CCroppingWindow::OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	RECT clientRect;
	GetClientRect(&clientRect);
  if(m_dibOffscreen.GetWidth() < clientRect.right || m_dibOffscreen.GetHeight() < clientRect.bottom)
  {
    m_dibOffscreen.SetSize(clientRect.right, clientRect.bottom);
    m_dibStretched.SetSize(clientRect.right, clientRect.bottom);
  }
  m_dibStretched.Fill(0);
  m_dibOriginal.StretchBlit(m_dibStretched, g_CropBorder, g_CropBorder, clientRect.right - (g_CropBorder*2), clientRect.bottom - (g_CropBorder*2));

  return 0;
}

LRESULT CCroppingWindow::OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	PAINTSTRUCT paintStruct = { 0 };
	HDC dc = BeginPaint(&paintStruct);

	RECT clientRect;
	GetClientRect(&clientRect);

	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

  m_dibStretched.Blit(m_dibOffscreen, 0, 0);
	if(m_hasSelection)
	{
    //DrawSelectionBox(m_dibOffscreen.GetDC());
	  RECT rcSelection;
    GetScreenSelection(rcSelection);
    m_dibOffscreen.DrawSelectionRectSafe<8,64>(0, rcSelection);
  }
  m_dibOffscreen.Blit(dc, 0, 0, width, height);

	EndPaint(&paintStruct);

	return 0;
}

void CCroppingWindow::DrawSelectionBox(HDC dc)
{
	//if(m_hasSelection)
	//{
	//  RECT rcSelection;
 //   GetScreenSelection(rcSelection);
 //   rcSelection.bottom += 0;
 //   rcSelection.right += 0;
 //   rcSelection.left += 0;

 //   DrawFocusRect(dc, &rcSelection);
	//}
}

// returns the amount to move the virtual cursor during a constrained move, based on a pixel mouse cursor delta
float GetDeltaMin(int val, bool bSuperDooper)
{
  if(!val) return 0;
  float blah = bSuperDooper ? 0.07f : 1.0f;
  return val < 0.0f ? -blah : blah;
}

LRESULT CCroppingWindow::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
  bHandled = TRUE;
  if(m_selectionEntrancy) return 0;
  m_selectionEntrancy = true;

	CPoint cursorPos(0);
	GetCursorPos(&cursorPos);
  ScreenToClient(&cursorPos);

  CPoint pt;

  if((wParam & MK_SHIFT) == MK_SHIFT)
  {
    bool bSuperDooper = ((wParam & MK_CONTROL) == MK_CONTROL);

    // constrain movement by only allowing 1 image pixel movement.
    // determine if we went left (-1), right (1) or nowhere (0) in the x coordinate
    float deltax = GetDeltaMin(cursorPos.x - m_lastSelectionCursorPos.x, bSuperDooper);
    m_lastSelectionPointX += deltax;

    float deltay = GetDeltaMin(cursorPos.y - m_lastSelectionCursorPos.y, bSuperDooper);
    m_lastSelectionPointY += deltay;

    if(m_lastSelectionPointX < 0) m_lastSelectionPointX = 0;
    if(static_cast<DWORD>(m_lastSelectionPointX) > m_bitmap->GetWidth()) m_lastSelectionPointX = static_cast<float>(m_bitmap->GetWidth());

    if(m_lastSelectionPointY < 0) m_lastSelectionPointY = 0;
    if(static_cast<DWORD>(m_lastSelectionPointY) > m_bitmap->GetHeight()) m_lastSelectionPointY = static_cast<float>(m_bitmap->GetHeight());

    pt.y = static_cast<LONG>(m_lastSelectionPointY);
    pt.x = static_cast<LONG>(m_lastSelectionPointX);

    // replace the mouse cursor to reflect the slowdown.
    CPoint newCursorPos = ImageToClient(pt);

    cursorPos = newCursorPos;
    ClientToScreen(&newCursorPos);
    SetCursorPos(newCursorPos.x, newCursorPos.y);
  }
  else
  {
    pt = ClientToImageCoords(cursorPos);
    m_lastSelectionPointX = static_cast<float>(pt.x);
    m_lastSelectionPointY = static_cast<float>(pt.y);
  }

  if(::GetCapture() == m_hWnd)
  {
    UpdateSelection(pt.x, pt.y);
  }

  m_lastSelectionCursorPos = cursorPos;
  m_notify->OnCroppingPositionChanged(pt.x, pt.y);
  m_selectionEntrancy = false;
	return 0;
}

LRESULT CCroppingWindow::OnLButtonDown(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
  if(m_selectionEntrancy) return 0;
  m_selectionEntrancy = true;

  if(::GetCapture() != m_hWnd)
  {
    SetCapture();
    BeginSelection(static_cast<int>(m_lastSelectionPointX), static_cast<int>(m_lastSelectionPointY));
  }

  m_selectionEntrancy = false;
	return 0;
}

LRESULT CCroppingWindow::OnLButtonUp(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
  if(m_selectionEntrancy) return 0;
  m_selectionEntrancy = true;

  if(::GetCapture() == m_hWnd)
  {
    ReleaseCapture();
  }

  m_selectionEntrancy = false;
	return 0;
}

void CCroppingWindow::BeginSelection(int x, int y)
{
	ClearSelection();
	m_hasSelection = true;
  m_selBegin = CPoint(x,y);
  UpdateSelection(x,y);
}

void CCroppingWindow::UpdateSelection(int imagex, int imagey)
{
	RECT rectInvalidate;
	RECT rectOldSelection;
	RECT rectNewSelection;
  GetScreenSelection(rectOldSelection);
  ::InflateRect(&rectOldSelection, 5, 5);

 	m_selectionOrg.left = std::min<int>(imagex, m_selBegin.x);
	m_selectionOrg.top = std::min<int>(imagey, m_selBegin.y);
	m_selectionOrg.bottom = std::max<int>(imagey, m_selBegin.y);
	m_selectionOrg.right = std::max<int>(imagex, m_selBegin.x);
  CopyRect(&rectNewSelection, &m_selectionOrg);
  ::InflateRect(&rectNewSelection, 5, 5);

  // include the new rect in the invalidation
  UnionRect(&rectInvalidate, &rectOldSelection, &rectNewSelection);

  m_notify->OnCroppingSelectionChanged();

  RedrawWindow(&rectInvalidate, 0, RDW_INVALIDATE | RDW_UPDATENOW);
}
