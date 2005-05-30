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


CCroppingWindow::CCroppingWindow(util::shared_ptr<Gdiplus::Bitmap> bitmap) :
  m_bitmap(bitmap),
  m_selecting(false),
  m_hasSelection(false)
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
	m_selecting = false;
	m_hasSelection = false;

	RECT rectInvalidate;
  GetScreenSelection(rectInvalidate);
  ::InflateRect(&rectInvalidate, 5, 5);
	InvalidateRect(&rectInvalidate);
}

CPoint CCroppingWindow::ScreenToImageCoords(CPoint x)
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
  ret.x = x.x * (m_bitmap->GetWidth()) / rcClient.right;
  ret.y = x.y * (m_bitmap->GetHeight()) / rcClient.bottom;

  // bounds checking
  if(ret.x > m_bitmap->GetWidth()) { ret.x = m_bitmap->GetWidth(); }
  if(ret.y > m_bitmap->GetHeight()) { ret.y = m_bitmap->GetHeight(); }

  return ret;
}

// returns the selection rect in screen coords (relative to client)
void CCroppingWindow::GetScreenSelection(RECT& rc)
{
  RECT rcClient;
  GetClientRect(&rcClient);
  rcClient.bottom -= g_CropBorder * 2;
  rcClient.right -= g_CropBorder * 2;

  // scale m_selectionOrg : rcClient :: rc : orgSIze
  rc.top = m_selectionOrg.top * rcClient.bottom / m_bitmap->GetHeight();
  rc.bottom = m_selectionOrg.bottom * rcClient.bottom / m_bitmap->GetHeight();
  rc.left = m_selectionOrg.left * rcClient.right / m_bitmap->GetWidth();
  rc.right = m_selectionOrg.right * rcClient.right / m_bitmap->GetWidth();

  rc.top += g_CropBorder;
  rc.left += g_CropBorder;
  rc.bottom += g_CropBorder;
  rc.right += g_CropBorder;
}


void CCroppingWindow::BeginSelection(int x, int y)
{
	ClearSelection();

	m_hasSelection = false;
	m_selecting = true;

	::GetCapture();

  m_selBegin = ScreenToImageCoords(CPoint(x,y));
}

void CCroppingWindow::UpdateSelection(int x, int y)
{
	RECT rectInvalidate;
  GetScreenSelection(rectInvalidate);
  ::InflateRect(&rectInvalidate, 5, 5);

  CPoint pt = ScreenToImageCoords(CPoint(x,y));
 	m_selectionOrg.left = std::min<int>(pt.x, m_selBegin.x);
	m_selectionOrg.top = std::min<int>(pt.y, m_selBegin.y);
	m_selectionOrg.bottom = std::max<int>(pt.y, m_selBegin.y);
	m_selectionOrg.right = std::max<int>(pt.x, m_selBegin.x);

  RedrawWindow(&rectInvalidate);
}

void CCroppingWindow::EndSelection(int x, int y)
{
	::ReleaseCapture();

	m_selecting = false;
	m_hasSelection = true;

	RedrawWindow(NULL);
}

bool CCroppingWindow::GetSelection(RECT& selectionRect)
{
	if (!m_hasSelection) return false;
  //GetScreenSelection(selectionRect);
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
  DrawSelectionBox(m_dibOffscreen.GetDC());
  m_dibOffscreen.Blit(dc, 0, 0, width, height);

	EndPaint(&paintStruct);

	return 0;
}

void CCroppingWindow::DrawSelectionBox(HDC dc)
{
	if (m_selecting || m_hasSelection)
	{
	  RECT rcSelection;
    GetScreenSelection(rcSelection);
    rcSelection.bottom += 0;
    rcSelection.right += 0;
    rcSelection.left += 0;
    DrawFocusRect(dc, &rcSelection);
	}
}


