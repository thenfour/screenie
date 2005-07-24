
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
  m_bIsPanning(false),
  m_selectionTool(this, this),
  m_nextTimerID(0),
  m_haveCapture(false)
{
  CopyImage(m_dibOriginal, *bitmap);
  m_view.SetZoomFactor(3);
  m_view.SetVirtualOrigin(PointI(250,250));

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
  Refresh(false);
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
    PointI delta;
    delta.x = m_panningStart.x - cursorPos.x;
    delta.y = m_panningStart.y - cursorPos.y;
    delta = m_view.ViewToVirtualSize(delta);
    PointI newOrg = m_panningStartVirtual;
    newOrg.x += delta.x;
    newOrg.y += delta.y;
    ClampToImage(newOrg);
    m_view.SetVirtualOrigin(newOrg);
    Refresh(true);
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

      ClampToImage(m_lastCursorVirtual);

      // replace the mouse cursor to reflect the slowdown.
      PointI newCursorPosX = m_view.VirtualToView(PointFtoI(m_lastCursorVirtual));
      CPoint newCursorPos(newCursorPosX.x, newCursorPosX.y);
      newCursorPos.Offset(viewMargin, viewMargin);

      cursorPos = newCursorPos;
      ClientToScreen(&newCursorPos);
      SetCursorPos(newCursorPos.x, newCursorPos.y);
    }
    else
    {
      cursorPos.Offset(-viewMargin, -viewMargin);
      m_lastCursorVirtual = m_view.ViewToVirtual(PointF(
        static_cast<float>(cursorPos.x),
        static_cast<float>(cursorPos.y) ));
      cursorPos.Offset(viewMargin, viewMargin);
    }
  }

  m_lastCursor = cursorPos;

  // fire tool events.
  m_selectionTool.OnCursorMove(PointFtoI(m_lastCursorVirtual));

  CPoint pt;
  pt.x = static_cast<LONG>(m_lastCursorVirtual.x);
  pt.y = static_cast<LONG>(m_lastCursorVirtual.y);
  m_notify->OnCursorPositionChanged(pt.x, pt.y);

  return MouseLeave();
}

LRESULT CImageEditWindow::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
  if(!MouseEnter()) return 0;

  POINTS& psTemp = MAKEPOINTS(lParam);
	CPoint cursorPos(psTemp.x, psTemp.y);
  cursorPos.x -= viewMargin;
  cursorPos.y -= viewMargin;
  PointI pt = m_view.ViewToVirtual(PointI(cursorPos.x, cursorPos.y));

  // fire tool events.
  m_selectionTool.OnLeftButtonDown(pt);

  return MouseLeave();
}

LRESULT CImageEditWindow::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
  if(!MouseEnter()) return 0;

  POINTS& psTemp = MAKEPOINTS(lParam);
	CPoint cursorPos(psTemp.x, psTemp.y);
  cursorPos.x -= viewMargin;
  cursorPos.y -= viewMargin;
  PointI pt = m_view.ViewToVirtual(PointI(cursorPos.x, cursorPos.y));

  // fire tool events.
  m_selectionTool.OnLeftButtonUp(pt);

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
  }
  return MouseLeave();
}

LRESULT CImageEditWindow::OnLoseCapture(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  if(m_bIsPanning)
  {
    SetCursor(m_hPreviousCursor);
    m_bIsPanning = false;
  }
  else
  {
    // fire tool events.
    m_selectionTool.OnLoseCapture();
  }

  return 0;
}

LRESULT CImageEditWindow::OnSize(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*handled*/)
{
	CRect clientRect;
	GetClientRect(&clientRect);
  m_view.SetViewOrigin(PointI(clientRect.right/2, clientRect.bottom/2));
  if(m_dibOffscreen.GetWidth() < clientRect.right || m_dibOffscreen.GetHeight() < clientRect.bottom)
  {
    m_dibOffscreen.SetSize(clientRect.right, clientRect.bottom);
    m_dibStretched.SetSize(clientRect.right, clientRect.bottom);
  }
  m_dibStretched.Fill(0);

  PointI viewOrigin = m_view.GetViewOrigin();

  // figure out the dimensions of the original image to draw to the LEFT and TOP of the view origin.
  // this will tell us the upper-left coord where to draw the image
  PointI ulViewSize(
    viewOrigin.x + static_cast<int>(m_view.GetZoomFactor()),
    viewOrigin.y + static_cast<int>(m_view.GetZoomFactor()));
  PointI ulVirtualSize = m_view.ViewToVirtualSize(ulViewSize);
  ulViewSize = m_view.VirtualToViewSize(ulVirtualSize);// correct for rounding

  PointI ulDrawOrg(viewOrigin.x - ulViewSize.x, viewOrigin.y - ulViewSize.y);

  // figure out the upper-left coord of the original image
  PointI ulVirtualImage = m_view.ViewToVirtual(ulDrawOrg);

  // figure out the dimensions of the entire original image to draw
  PointI wholeViewSize(
    clientRect.Width() + static_cast<int>(m_view.GetZoomFactor()*2),
    clientRect.Height() + static_cast<int>(m_view.GetZoomFactor()*2));
  PointI wholeVirtualSize = m_view.ViewToVirtualSize(wholeViewSize);
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

  // fire tool events.
  m_selectionTool.OnPaint(m_dibOffscreen, m_view, viewMargin, viewMargin);

  m_dibOffscreen.Blit(dc, 0, 0, width, height);

	EndPaint(&paintStruct);
	return 0;
}

PanningSpec CImageEditWindow::GetPanningSpec()
{
  PanningSpec ret(0, 0);

	RECT clientRect;
	GetClientRect(&clientRect);

  PointF ul = m_view.ViewToVirtual(PointF(0, 0));
  PointF br = m_view.ViewToVirtual(PointF((float)clientRect.right, (float)clientRect.bottom));

  PointF pos = m_lastCursorVirtual;

  if(pos.x < ul.x)
  {
    ret.m_x = (int)(pos.x - ul.x);
    ret.m_x /= 3;
    if(ret.m_x == 0) ret.m_x = -1;
  }
  if(pos.x > br.x)
  {
    ret.m_x = (int)(pos.x - br.x);
    ret.m_x /= 3;
    if(ret.m_x == 0) ret.m_x = 1;
  }

  if(pos.y < ul.y)
  {
    ret.m_y = (int)(pos.y - ul.y);
    ret.m_y /= 3;
    if(ret.m_y == 0) ret.m_y = -1;
  }
  if(pos.y > br.y)
  {
    ret.m_y = (int)(pos.y - br.y);
    ret.m_y /= 3;
    if(ret.m_y == 0) ret.m_y = 1;
  }

  return ret;
}

void CImageEditWindow::Pan(const PanningSpec& ps, bool updateNow)
{
  Pan(ps.m_x, ps.m_y, updateNow);
}

void CImageEditWindow::Pan(int x, int y, bool updateNow)
{
  PointI org = m_view.GetVirtualOrigin();
  org.x += x;
  org.y += y;
  ClampToImage(org);
  m_view.SetVirtualOrigin(org);

  // the thing about panning is that now the mouse cursor position also changes (relative to the view).
  // so, send a mousemove if necessary.
  POINT pt;
  GetCursorPos(&pt);
  ScreenToClient(&pt);
  bool doit = true;// always send if we are capturing.
  if(!m_haveCapture)
  {
    // is the cursor inside the window?
    RECT rcClient;
    GetClientRect(&rcClient);
    doit = (PtInRect(&rcClient, pt) == TRUE);
  }
  if(doit)
  {
    BOOL temp;
    LPARAM lParam = MAKELPARAM(pt.x, pt.y);
    OnMouseMove(WM_MOUSEMOVE, 0, lParam, temp);
  }

  Refresh(updateNow);
}

HWND CImageEditWindow::GetHWND()
{
  return m_hWnd;
}

PointI CImageEditWindow::GetCursorPosition()
{
  return PointFtoI(m_lastCursorVirtual);
}

int CImageEditWindow::GetImageHeight()
{
  return m_bitmap->GetHeight();
}

int CImageEditWindow::GetImageWidth()
{
  return m_bitmap->GetWidth();
}

void CImageEditWindow::ClampToImage(PointI& p)
{
  if(p.x < 0) p.x = 0;
  if(p.y < 0) p.y = 0;
  if(p.x > GetImageWidth()) p.x = static_cast<int>(GetImageWidth());
  if(p.y > GetImageHeight()) p.y = static_cast<int>(GetImageHeight());
}

void CImageEditWindow::ClampToImage(PointF& p)
{
  if(p.x < 0) p.x = 0;
  if(p.y < 0) p.y = 0;
  if(p.x > GetImageWidth()) p.x = static_cast<float>(GetImageWidth());
  if(p.y > GetImageHeight()) p.y = static_cast<float>(GetImageHeight());
}

void CImageEditWindow::Refresh(bool now)
{
  BOOL temp;
  OnSize(0,0,0,temp);
  RedrawWindow(0, 0, RDW_INVALIDATE | (now ? RDW_UPDATENOW : 0));
}

void CImageEditWindow::Refresh(const RECT& imageCoords, bool now)
{
  BOOL temp;
  OnSize(0,0,0,temp);

  PointI ulImage(imageCoords.left, imageCoords.top);
  PointI brImage(imageCoords.right, imageCoords.bottom);
  PointI ulScreen = m_view.VirtualToView(ulImage);
  PointI brScreen = m_view.VirtualToView(brImage);

  CRect rcScreen(ulScreen.x, ulScreen.y, brScreen.x, brScreen.y);

  rcScreen.OffsetRect(viewMargin, viewMargin);
  rcScreen.InflateRect(5,5);

  RedrawWindow(&rcScreen, 0, RDW_INVALIDATE | (now ? RDW_UPDATENOW : 0));
}

LRESULT CImageEditWindow::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  bHandled = TRUE;
  m_timerMap[wParam].first(m_timerMap[wParam].second);
  return 0;
}

UINT_PTR CImageEditWindow::CreateTimer(UINT elapse, ToolTimerProc proc, void* userData)
{
  UINT_PTR cookie = m_nextTimerID ++;
  m_timerMap[cookie] = std::pair<ToolTimerProc, void*>(proc, userData);
  SetTimer(cookie, elapse);
  return cookie;
}

void CImageEditWindow::DeleteTimer(UINT_PTR cookie)
{
  KillTimer(cookie);
  m_timerMap.erase(cookie);
}

LRESULT CImageEditWindow::OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
  // fire tool events
  m_selectionTool.OnInitTool();
  return 0;
}

void CImageEditWindow::OnSelectionToolSelectionChanged()
{
  m_notify->OnSelectionChanged();
}

void CImageEditWindow::SetCapture_()
{
  SetCapture();
  m_haveCapture = true;
}

void CImageEditWindow::ReleaseCapture_()
{
  if(m_haveCapture)
  {
    ReleaseCapture();
    m_haveCapture = false;
  }
}

bool CImageEditWindow::HaveCapture()
{
  return m_haveCapture;
}


