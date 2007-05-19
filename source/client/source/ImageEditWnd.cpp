
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
  m_selectionTool(this),
  m_nextTimerID(123),
  m_haveCapture(false),
  m_lastCursor(0,0),
  m_lastCursorVirtual(0,0),
  m_panningTimer(0),
	m_currentTool(&m_selectionTool)
{
  CopyImage(m_dibOriginal, *bitmap);
	m_display.SetOriginalImage(m_dibOriginal);
  m_display.SetZoomFactor(3.0f);
  m_display.SetImageOrigin(PointF(m_bitmap->GetWidth() / 2, m_bitmap->GetHeight() / 2), "ImageEditWnd()");

  if(pNotify)
  {
    m_notify = pNotify;
  }
}

ViewPortSubPixel CImageEditWindow::GetZoomFactor() const
{
  return m_display.GetViewport().GetZoomFactor();
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
    delta = m_display.GetViewport().ViewToImageSize(delta);
    PointF newOrg = m_panningStartVirtual;
    newOrg.x += delta.x;
    newOrg.y += delta.y;

		ClampImageOrigin(newOrg);

		m_display.SetImageOrigin(newOrg, LibCC::FormatA("OnMouseMove - m_bIsPanning. z:% / v:(%,%) / i:(%,%)")
			(m_display.GetViewport().GetZoomFactor())
			(m_display.GetViewport().GetViewOrigin().x)
			(m_display.GetViewport().GetViewOrigin().y)
			(m_display.GetViewport().GetImageOrigin().x)
			(m_display.GetViewport().GetImageOrigin().y)
			.CStr());
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
      PointF newCursorPosX = m_display.GetViewport().ImageToView(m_lastCursorVirtual);
			
      CPoint newCursorPos = newCursorPosX.Round();

      cursorPos = newCursorPos;
      ClientToScreen(&newCursorPos);
      SetCursorPos(newCursorPos.x, newCursorPos.y);
    }
    else
    {
      m_lastCursorVirtual = m_display.GetViewport().ViewToImage(PointF(
        static_cast<float>(cursorPos.x),
        static_cast<float>(cursorPos.y) ));
    }
  }

  m_lastCursor = cursorPos;

  // fire tool events.
	if(m_currentTool != 0)
		m_currentTool->OnCursorMove(m_lastCursorVirtual, m_haveCapture && !m_bIsPanning);

  if(m_haveCapture)
  {
    m_panningSpec = GetPanningSpec();
    if(m_panningSpec.IsNotNull())
    {
      // start the panning timer.
      if(!m_panningTimer)
      {
        m_panningTimer = CreateTimer(30, CImageEditWindow::PanningTimerProc, this);
				OutputDebugString(LibCC::Format("CreateTimer (id %)\r\n")(m_panningTimer).CStr());
      }
    }
    else
    {
			KillPanningTimer();
		}
  }

  CPoint pt;
  pt.x = static_cast<LONG>(m_lastCursorVirtual.x);
  pt.y = static_cast<LONG>(m_lastCursorVirtual.y);
  m_notify->OnCursorPositionChanged(pt.x, pt.y);

  return MouseLeave();
}

void CImageEditWindow::KillPanningTimer()
{
  if(m_panningTimer)
  {
    DeleteTimer(m_panningTimer);
    m_panningTimer = 0;
  }
}

LRESULT CImageEditWindow::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
  if(!MouseEnter()) return 0;

  POINTS& psTemp = MAKEPOINTS(lParam);
	CPoint cursorPos(psTemp.x, psTemp.y);
  PointF pt = m_display.GetViewport().ViewToImage(PointF((float)cursorPos.x, (float)cursorPos.y));

  // fire tool events.
	if(m_currentTool != 0)
	  m_currentTool->OnLeftButtonDown(pt);

  if(!m_haveCapture)
  {
    SetCapture();
    m_haveCapture = true;
		if(m_currentTool != 0)
		  m_currentTool->OnStartDragging();
  }

  return MouseLeave();
}

LRESULT CImageEditWindow::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
  if(!MouseEnter()) return 0;

  POINTS& psTemp = MAKEPOINTS(lParam);
	CPoint cursorPos(psTemp.x, psTemp.y);
  PointF pt = m_display.GetViewport().ViewToImage(PointF((float)cursorPos.x, (float)cursorPos.y));

  if(m_haveCapture)
  {
    ReleaseCapture();
  }

  // fire tool events.
	if(m_currentTool != 0)
	  m_currentTool->OnLeftButtonUp(pt);

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
    m_panningStartVirtual = m_display.GetViewport().GetImageOrigin();
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
	OutputDebugString("OnLoseCapture\r\n");
	KillPanningTimer();
  if(m_bIsPanning)
  {
    SetCursor(m_hPreviousCursor);
    m_bIsPanning = false;
  }
  else
  {
    // fire tool events.
    m_haveCapture = false;
	 	if(m_currentTool != 0)
			m_currentTool->OnStopDragging();
  }

  return 0;
}

void CImageEditWindow::CenterImage()
{
  CRect rc;
  GetClientRect(&rc);
  m_lastCursor.SetPoint(rc.Width() / 2, rc.Height() / 2);
  PointF viewOrg((ViewPortSubPixel)m_lastCursor.x, (ViewPortSubPixel)m_lastCursor.y);
  m_lastCursorVirtual = m_display.GetViewport().ViewToImage(viewOrg);
  m_display.SetViewOrigin(viewOrg);
  m_display.SetImageOrigin(PointF(m_bitmap->GetWidth() / 2, m_bitmap->GetHeight() / 2), "center image");
}

LRESULT CImageEditWindow::OnSize(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*handled*/)
{
  CRect rc;
  GetClientRect(&rc);
	m_display.SetClientSize(rc.Width(), rc.Height());
	PointF io = m_display.GetViewport().GetImageOrigin();
	ClampImageOrigin(io);
	m_display.SetImageOrigin(io, "on size");

	m_offscreen.SetSize(rc.Width(), rc.Height());
  return 0;
}

LRESULT CImageEditWindow::OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	PAINTSTRUCT paintStruct = { 0 };
	HDC dc = BeginPaint(&paintStruct);

	m_display.Render(m_offscreen, paintStruct.rcPaint);
	m_offscreen.Blit(paintStruct.hdc, paintStruct.rcPaint.left, paintStruct.rcPaint.top, paintStruct.rcPaint.right - paintStruct.rcPaint.left,
		paintStruct.rcPaint.bottom - paintStruct.rcPaint.top,
		paintStruct.rcPaint.left, paintStruct.rcPaint.top);

	EndPaint(&paintStruct);
	return 0;
}

PanningSpec CImageEditWindow::GetPanningSpec()
{
  PanningSpec ret(0, 0);

	RECT clientRect;
	GetClientRect(&clientRect);

  PointF ul = m_display.GetViewport().ViewToImage(PointF(0, 0));
  PointF br = m_display.GetViewport().ViewToImage(PointF((ViewPortSubPixel)clientRect.right, (ViewPortSubPixel)clientRect.bottom));

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

void CImageEditWindow::Pan(const PanningSpec& ps)
{
  Pan(ps.m_x, ps.m_y);
}

void CImageEditWindow::Pan(int x, int y)
{
  PointF org = m_display.GetViewport().GetImageOrigin();
  org.x += x;
  org.y += y;
  
	RECT rcClient;
  GetClientRect(&rcClient);

	ClampImageOrigin(org);

  m_display.SetImageOrigin(org, "Pan()");

  // the thing about panning is that now the mouse cursor position also changes (relative to the view).
  // so, send a mousemove if necessary.
  POINT pt;
  GetCursorPos(&pt);
  ScreenToClient(&pt);
  bool doit = true;// always send if we are capturing.
  if(!m_haveCapture)
  {
    // is the cursor inside the window?
    doit = (PtInRect(&rcClient, pt) == TRUE);
  }
  if(doit)
  {
    BOOL temp;
    LPARAM lParam = MAKELPARAM(pt.x, pt.y);
    OnMouseMove(WM_MOUSEMOVE, 0, lParam, temp);
  }
}

PointF CImageEditWindow::GetCursorPosition()
{
  return m_lastCursorVirtual;
}

int CImageEditWindow::GetImageHeight() const
{
  return m_bitmap->GetHeight();
}

int CImageEditWindow::GetImageWidth() const
{
  return m_bitmap->GetWidth();
}


void CImageEditWindow::ClampToImage(PointF& p)
{
  if(p.x < 0) p.x = 0;
  if(p.y < 0) p.y = 0;
  if(p.x > GetImageWidth()) p.x = static_cast<float>(GetImageWidth());
  if(p.y > GetImageHeight()) p.y = static_cast<float>(GetImageHeight());
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
	if(m_currentTool != 0)
	  m_currentTool->OnInitTool();
	m_display.SetHWND(this->m_hWnd);
  return 0;
}

void CImageEditWindow::SetZoomFactor(float n)
{
  // set the origins to where the cursor is, so when the user zooms,
  // it will zoom into the mouse cursor.
  CRect rcClient;
  GetClientRect(&rcClient);
  PointF client((float)m_lastCursor.x, (float)m_lastCursor.y);
  // clamp
  if(client.x < 1) client.x = 1;
  if(client.y < 1) client.y = 1;
  if(client.x > (rcClient.right - 1)) client.x = rcClient.right - 1;
  if(client.y > (rcClient.bottom - 1)) client.x = rcClient.bottom - 1;
  m_display.SetViewOrigin(client);
  PointF virtual_(m_lastCursorVirtual);
  ClampToImage(virtual_);
	m_display.SetImageOrigin(virtual_, LibCC::FormatA("SetZoomFactor(z:% / v:(%,%) / i:(%,%) )")(n)
		(m_display.GetViewport().GetViewOrigin().x)
		(m_display.GetViewport().GetViewOrigin().y)
		(m_display.GetViewport().GetImageOrigin().x)
		(m_display.GetViewport().GetImageOrigin().y)
		.CStr());

  m_display.SetZoomFactor(n);

	// if we are currently panning, this is going to screw up some coords

  m_notify->OnZoomFactorChanged();
}

