
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
  m_notify(this),
  m_mouseEntrancy(false),
  m_bIsPanning(false),
  m_selectionTool(this, this),
  m_nextTimerID(0),
  m_haveCapture(false),
  m_lastCursor(0,0),
  m_lastCursorVirtual(0,0)
{
  CopyImage(m_dibOriginal, *bitmap);
  m_view.SetZoomFactor(3.0f);
  m_view.SetVirtualOrigin(PointF(250.0f , 250.0f));
  m_view.SetViewOrigin(PointF(0.0f, 0.0f));

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
    PointF delta;
    delta.x = (float)(m_panningStart.x - cursorPos.x);
    delta.y = (float)(m_panningStart.y - cursorPos.y);
    delta = m_view.ViewToVirtualSize(delta);
    PointF newOrg = m_panningStartVirtual;
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

      cursorPos = newCursorPos;
      ClientToScreen(&newCursorPos);
      SetCursorPos(newCursorPos.x, newCursorPos.y);
    }
    else
    {
      m_lastCursorVirtual = m_view.ViewToVirtual(PointF(
        static_cast<float>(cursorPos.x),
        static_cast<float>(cursorPos.y) ));
    }
  }

  m_lastCursor = cursorPos;

  // fire tool events.
  m_selectionTool.OnCursorMove(m_lastCursorVirtual);

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
  PointF pt = m_view.ViewToVirtual(PointF((float)cursorPos.x, (float)cursorPos.y));

  // fire tool events.
  m_selectionTool.OnLeftButtonDown(pt);

  return MouseLeave();
}

LRESULT CImageEditWindow::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
  if(!MouseEnter()) return 0;

  POINTS& psTemp = MAKEPOINTS(lParam);
	CPoint cursorPos(psTemp.x, psTemp.y);
  PointF pt = m_view.ViewToVirtual(PointF((float)cursorPos.x, (float)cursorPos.y));

  // fire tool events.
  m_selectionTool.OnLeftButtonUp(pt);

  return MouseLeave();
}

LRESULT CImageEditWindow::OnRButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
  if(!MouseEnter()) return 0;
  if(m_haveCapture == false)
  {
    m_bIsPanning = true;

    POINTS& psTemp = MAKEPOINTS(lParam);
    m_panningStart.SetPoint(psTemp.x, psTemp.y);
    m_panningStartVirtual = m_view.GetVirtualOrigin();
    SetCapture();
    m_hPreviousCursor = SetCursor(LoadCursor(0, IDC_HAND));
  }
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


void CImageEditWindow::ResetOffscreenBitmaps()
{
	CRect clientRect;
	GetClientRect(&clientRect);

  if(m_dibOffscreen.GetWidth() < clientRect.right || m_dibOffscreen.GetHeight() < clientRect.bottom)
  {
    m_dibOffscreen.SetSize(clientRect.right, clientRect.bottom);
    m_dibStretched.SetSize(clientRect.right, clientRect.bottom);
  }

  // figure out an integral place to start drawing (client 0,0 may be at a fraction of a virtual coord)
  PointF ulVirtual = m_view.ViewToVirtual(PointF((float)clientRect.left, (float)clientRect.top));
  ulVirtual.x = floor(ulVirtual.x);// make integral
  ulVirtual.y = floor(ulVirtual.y);// make integral
  PointF ulClient = m_view.VirtualToView(ulVirtual);// now this is guaranteed to be at the start of an integral virtual coord

  // do the similar stuff for bottom-right
  PointF brVirtual = m_view.ViewToVirtual(PointF((float)clientRect.right, (float)clientRect.bottom));
  brVirtual.x = ceil(brVirtual.x);// make integral
  brVirtual.y = ceil(brVirtual.y);// make integral
  PointF brClient = m_view.VirtualToView(brVirtual);// now this is guaranteed to be at the start of an integral virtual coord

  // figure out the dimensions of the entire original image to draw
  PointF viewSize(brClient.x - ulClient.x, brClient.y - ulClient.y);
  PointF virtualSize(brVirtual.x - ulVirtual.x, brVirtual.y - ulVirtual.y);

  m_dibStretched.Fill(0);
  m_dibOriginal.StretchBlit(m_dibStretched,
    (long)(ulClient.x),
    (long)(ulClient.y),
    (long)(viewSize.x),
    (long)(viewSize.y),
    (long)(ulVirtual.x),
    (long)(ulVirtual.y),
    (long)(virtualSize.x),
    (long)(virtualSize.y),
    COLORONCOLOR
    );
}

LRESULT CImageEditWindow::OnSize(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*handled*/)
{
  ResetOffscreenBitmaps();
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
  m_selectionTool.OnPaint(m_dibOffscreen, m_view, 0, 0);

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
    ret.m_x /= 4;
    if(ret.m_x == 0) ret.m_x = -1;
  }
  if(pos.x > br.x)
  {
    ret.m_x = (int)(pos.x - br.x);
    ret.m_x /= 4;
    if(ret.m_x == 0) ret.m_x = 1;
  }

  if(pos.y < ul.y)
  {
    ret.m_y = (int)(pos.y - ul.y);
    ret.m_y /= 4;
    if(ret.m_y == 0) ret.m_y = -1;
  }
  if(pos.y > br.y)
  {
    ret.m_y = (int)(pos.y - br.y);
    ret.m_y /= 4;
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
  PointF org = m_view.GetVirtualOrigin();
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

PointF CImageEditWindow::GetCursorPosition()
{
  return m_lastCursorVirtual;
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
  ResetOffscreenBitmaps();
  RedrawWindow(0, 0, RDW_INVALIDATE | (now ? RDW_UPDATENOW : 0));
}

void CImageEditWindow::Refresh(const RECT& imageCoords, bool now)
{
  ResetOffscreenBitmaps();

  PointI ulImage(imageCoords.left, imageCoords.top);
  PointI brImage(imageCoords.right, imageCoords.bottom);
  PointI ulScreen = m_view.VirtualToView(ulImage);
  PointI brScreen = m_view.VirtualToView(brImage);

  CRect rcScreen(ulScreen.x, ulScreen.y, brScreen.x, brScreen.y);

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

void CImageEditWindow::SetZoomFactor(float n)
{
  // set the origins to where the cursor is, so when the user zooms,
  // it will zoom into the mouse cursor.
  CRect rcClient;
  GetClientRect(&rcClient);
  PointF client((float)m_lastCursor.x, (float)m_lastCursor.y);
  if(client.x < 1) client.x = 1;
  if(client.y < 1) client.y = 1;
  if(client.x > (rcClient.right - 1)) client.x = rcClient.right - 1;
  if(client.y > (rcClient.bottom - 1)) client.x = rcClient.bottom - 1;
  m_view.SetViewOrigin(client);
  PointF virtual_(m_lastCursorVirtual);
  ClampToImage(virtual_);
  m_view.SetVirtualOrigin(virtual_);

  m_view.SetZoomFactor(n);
  m_notify->OnZoomFactorChanged();
  Refresh(false);
}

