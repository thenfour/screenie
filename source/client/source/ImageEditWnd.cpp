/*
	TODO: optimize rendering. We need the following surfaces:
	- original bitmap
	- "document" that can be modified by tools
	- a temporary surface that can be modified by tools
	- a zoomed surface with the zoomed image
	- another temporary surface where tools can render on the zoomed image (directly to the offscreen surface basically like for drawing selection handles)
	- whatever other surfaces are necessary for rendering

	* we should make the selection tool do all of its drawing in class SelectionTool, instead of ImageEditRenderer
	* we should make master / slave drawing optimized.
	* never redraw stuff that didn't change
	* consider partial window rendering
	* consider panning optimization
*/
#include "stdafx.hpp"
#include "ImageEditWnd.hpp"
#include "image.hpp"
#include "clipboard.hpp"


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
	m_actuallyDidPanning(false),
  m_notify(this),
  m_mouseEntrancy(false),
  m_bIsPanning(false),
  m_selectionTool(this),
  m_highlightTool(this),
  m_nextTimerID(123),
  m_lastCursor(0,0),
  m_lastCursorImage(0,0),
  m_panningTimer(0),
	m_currentTool(0),
//	m_showCursor(false),
//	m_enablePanning(true),
	m_isLeftClickDragging(false),
//	m_enableTools(true),
	m_captureRefs(0),
	m_master(0),
	m_slave(0)
{
  CopyImage(m_dibDocument, *bitmap);
	m_display.SetOriginalImage(m_dibRenderSource);
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
	Gdiplus::Bitmap* bitmapClone = m_dibDocument.GetGdiplusBitmap()->Clone(rectToCopy.left, rectToCopy.top,
		rectToCopy.right - rectToCopy.left, rectToCopy.bottom - rectToCopy.top, PixelFormatDontCare);

	return util::shared_ptr<Gdiplus::Bitmap>(bitmapClone);
}

// returns the amount to move the virtual cursor during a constrained move, based on a pixel mouse cursor delta
ViewPortSubPixel GetDeltaMin(ViewPortSubPixel val, bool bSuperDooper)
{
  if(val == 0) return 0;
  ViewPortSubPixel blah = bSuperDooper ? 0.07f : 1.0f;
  return val < 0.0f ? -blah : blah;
}

LRESULT CImageEditWindow::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  bHandled = TRUE;
  if(!MouseEnter()) return 0;

  POINTS& psTemp = MAKEPOINTS(lParam);

	//LibCC::LogScopeMessage l(LibCC::Format(L"OnMouseMove (%,%)")(psTemp.x)(psTemp.y).Str());

	// this is necessary to prevent an endless loop when calling SetCursorPos().
	CPoint lastCursorRounded = m_lastCursor.Round();
	if((psTemp.x == lastCursorRounded.x) && (psTemp.y == lastCursorRounded.y))
	{
		MouseLeave();
		return 0;
	}

	PointF cursorPos(psTemp.x, psTemp.y);
	PointF cursorPosImage = m_display.GetViewport().ViewToImage(cursorPos);

	// handle the fine-tune controls. this means after this point we need to deal in sub-pixels and image coordinates.
  if((wParam & MK_SHIFT) == MK_SHIFT)
  {
    bool bSuperDooper = ((wParam & MK_CONTROL) == MK_CONTROL);

    // figure out how many virtual units to actually move.
    ViewPortSubPixel deltax = GetDeltaMin(cursorPos.x - m_lastCursor.x, bSuperDooper);
		cursorPosImage.x = m_lastCursorImage.x + deltax;

    ViewPortSubPixel deltay = GetDeltaMin(cursorPos.y - m_lastCursor.y, bSuperDooper);
		cursorPosImage.y = m_lastCursorImage.y + deltay;

    // replace the mouse cursor to reflect the slowdown.
		cursorPos = m_display.GetViewport().ImageToView(cursorPosImage);
		//cursorPos.SelfRound();
    CPoint newCursorPos = cursorPos.Round();
    ClientToScreen(&newCursorPos);
    SetCursorPos(newCursorPos.x, newCursorPos.y);
  }

	// handle panning movement.
  if(m_bIsPanning)
  {
		m_actuallyDidPanning = true;
		m_display.SetViewOrigin(cursorPos);

		PointF imgOrg = m_lastCursorImage;
		ClampImageOrigin(imgOrg);
		m_display.SetImageOrigin(imgOrg, "");
		
		cursorPosImage = m_lastCursorImage;// during panning you can't change the position here. (but you can on the panning timer)
	}

  // fire tool events.
	if(m_currentTool != 0 && EnableTools())
		m_currentTool->OnCursorMove(cursorPosImage, m_isLeftClickDragging && HaveCapture());

	if(m_slave != 0)
	{
		m_slave->CenterOnImage(cursorPosImage);
	}

  m_notify->OnCursorPositionChanged(cursorPosImage);

	m_lastCursorImage = cursorPosImage;
  m_lastCursor = cursorPos;

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
	/*
		sometimes LButtonDown messages are not followed by a mousemove message. this fixes that.
	*/
	BOOL t;
	OnMouseMove(0, 0, lParam, t);

  if(!MouseEnter()) return 0;

  POINTS& psTemp = MAKEPOINTS(lParam);
	CPoint cursorPos(psTemp.x, psTemp.y);

	//LibCC::LogScopeMessage l(LibCC::Format(L"OnLButtonDown (%,%)")(psTemp.x)(psTemp.y).Str());

	PointF pt = m_display.GetViewport().ViewToImage(PointF((float)cursorPos.x, (float)cursorPos.y));

	m_isLeftClickDragging = true;

  // fire tool events.
	if(m_currentTool != 0 && EnableTools())
	  m_currentTool->OnLeftButtonDown(pt);

	AddCapture();

	if(m_currentTool != 0 && EnableTools())
	  m_currentTool->OnStartDragging(pt);

  return MouseLeave();
}

LRESULT CImageEditWindow::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
  if(!MouseEnter()) return 0;

  POINTS& psTemp = MAKEPOINTS(lParam);
	//LibCC::LogScopeMessage l(LibCC::Format(L"OnLButtonUp (%,%)")(psTemp.x)(psTemp.y).Str());
	CPoint cursorPos(psTemp.x, psTemp.y);
  PointF pt = m_display.GetViewport().ViewToImage(PointF((float)cursorPos.x, (float)cursorPos.y));

  ReleaseCapture();
	m_isLeftClickDragging = false;

  // fire tool events.
	if(m_currentTool != 0 && EnableTools())
	  m_currentTool->OnLeftButtonUp(pt);

  return MouseLeave();
}

LRESULT CImageEditWindow::OnRButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	/*
		sometimes RButtonDown messages are not followed by a mousemove message. this fixes that.
	*/
	BOOL t;
	OnMouseMove(0, 0, lParam, t);

	if(m_master != 0) return 0;
  if(!MouseEnter()) return 0;
  POINTS& psTemp = MAKEPOINTS(lParam);
	//LibCC::LogScopeMessage l(LibCC::Format(L"OnRButtonDown (%,%)")(psTemp.x)(psTemp.y).Str());

  m_bIsPanning = true;
	m_actuallyDidPanning = false;

	AddCapture();

  PushCursor(IDC_HAND);
  return MouseLeave();
}

LRESULT CImageEditWindow::OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;// necessary to send WM_CONTEXTMENU
	if(m_master != 0) return 0;
  if(!MouseEnter()) return 0;
  POINTS& psTemp = MAKEPOINTS(lParam);
	//LibCC::LogScopeMessage l(LibCC::Format(L"OnRButtonUp (%,%)")(psTemp.x)(psTemp.y).Str());

	if(m_bIsPanning)
  {
		if(m_actuallyDidPanning)
			bHandled = TRUE;

    ReleaseCapture();
		PopCursor(true);
		m_bIsPanning = false;
		m_actuallyDidPanning = false;
  }

  return MouseLeave();
}

LRESULT CImageEditWindow::OnContextMenu(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	if(m_master != 0) return 0;// do not show context menus if this is a "slave" (e.g. the zoom window)
  if(!MouseEnter()) return 0;
  POINTS& psTemp = MAKEPOINTS(lParam);
	CPoint cursorPos(psTemp.x, psTemp.y);

	//LibCC::LogScopeMessage l(LibCC::Format(L"OnContextMenu (%,%)")(psTemp.x)(psTemp.y).Str());

	CMenu contextMenu(AtlLoadMenu(IDR_CROPMENU));
	CMenuHandle trayMenu = contextMenu.GetSubMenu(0);

	if(!Clipboard::ContainsBitmap())
		trayMenu.EnableMenuItem(ID_EDIT_PASTE, MF_BYCOMMAND | MF_GRAYED);

	BOOL r = trayMenu.TrackPopupMenu(TPM_RIGHTALIGN, cursorPos.x, cursorPos.y, m_hWnd);

  return MouseLeave();
}


LRESULT CImageEditWindow::OnLoseCapture(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_captureRefs = 0;
	KillPanningTimer();
  if(m_bIsPanning)
  {
    m_bIsPanning = false;
		m_actuallyDidPanning = false;
  }

	m_isLeftClickDragging = false;

	// fire tool events.
 	if(m_currentTool != 0 && EnableTools())
		m_currentTool->OnStopDragging();

  return 0;
}

void CImageEditWindow::CenterImage()
{
  CRect rc;
  GetClientRect(&rc);
	m_lastCursor.Assign(rc.Width() / 2, rc.Height() / 2);
  PointF viewOrg((ViewPortSubPixel)m_lastCursor.x, (ViewPortSubPixel)m_lastCursor.y);
  m_lastCursorImage = m_display.GetViewport().ViewToImage(viewOrg);
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

void CImageEditWindow::RenderOffscreen()
{
	CRect rcUpdate;
	GetClientRect(&rcUpdate);
	if(m_master != 0)// we are a slave (zoom window)
	{
		m_master->RenderOffscreen();
		m_display.Render(m_offscreen, rcUpdate);
	}
	else// we are a master or independent
	{
		m_dibRenderSource.SetSize(m_dibDocument.GetWidth(), m_dibDocument.GetHeight());
		m_dibDocument.Blit(m_dibRenderSource, 0, 0);

		if(m_currentTool != 0)
			m_currentTool->OnPaintClient(m_dibRenderSource);

		m_display.Render(m_offscreen, rcUpdate);
	}
}

void CImageEditWindow::Render(HDC target, CRect rc)
{
	GetClientRect(&rc);// just always draw the whole window.

	RenderOffscreen();

	m_offscreen.Blit(target, 0, 0, rc.right, rc.bottom);

	if(m_master != 0)// if this is a slave
	{
		ICONINFO ii = {0};
		GetIconInfo(LoadCursor(0, IDC_CROSS), &ii);
		DeleteObject(ii.hbmColor);
		DeleteObject(ii.hbmMask);
		::DrawIcon(target,
			(rc.right / 2) - ii.xHotspot,
			(rc.bottom / 2) - ii.yHotspot,
			LoadCursor(0, IDC_CROSS));
	}
}

LRESULT CImageEditWindow::OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	// when slave is being painted, it should tell the master to paint as well.
	if(m_master != 0)
	{
		if(0 != m_master->GetUpdateRect(NULL))
		{
			CRect rcMaster;
			m_master->GetClientRect(&rcMaster);
			HDC dcMaster = m_master->GetDC();
			m_master->Render(dcMaster, rcMaster);
			m_master->ReleaseDC(dcMaster);
			m_master->ValidateRect(rcMaster);
		}
	}

	if(m_slave != 0)
	{
		if(0 != m_slave->GetUpdateRect(NULL))
		{
			CRect rcSlave;
			m_slave->GetClientRect(&rcSlave);
			HDC dcSlave = m_slave->GetDC();
			m_slave->Render(dcSlave, rcSlave);
			m_slave->ReleaseDC(dcSlave);
			m_slave->ValidateRect(rcSlave);
		}
	}

	PAINTSTRUCT paintStruct = { 0 };
	BeginPaint(&paintStruct);
	EndPaint(&paintStruct);

	CRect rc;
	this->GetClientRect(&rc);
	HDC dc = this->GetDC();
	this->Render(dc, rc);
	this->ReleaseDC(dc);

	return 0;
}

void CImageEditWindow::SetCursorPosition(const PointF& img)
{
	CPoint ptClient = m_display.GetViewport().ImageToView(img).Round();
	ClientToScreen(&ptClient);
	SetCursorPos(ptClient.x, ptClient.y);
}

PanningSpec CImageEditWindow::GetPanningSpec()
{
  PanningSpec ret(0, 0);

	RECT clientRect;
	GetClientRect(&clientRect);

  PointF ul = m_display.GetViewport().ViewToImage(PointF(0, 0));
  PointF br = m_display.GetViewport().ViewToImage(PointF((ViewPortSubPixel)clientRect.right, (ViewPortSubPixel)clientRect.bottom));

  PointF pos = m_lastCursorImage;

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

// x/y in image coords
void CImageEditWindow::Pan(int x, int y)
{
  PointF org = m_display.GetViewport().GetImageOrigin();
  org.x -= x;
  org.y -= y;
  
	RECT rcClient;
  GetClientRect(&rcClient);

	ClampImageOrigin(org);

  m_display.SetImageOrigin(org, "Pan()");
}

PointF CImageEditWindow::GetCursorPosition()
{
  return m_lastCursorImage;
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
	m_display.SetHWND(this->m_hWnd);
  return 0;
}

LRESULT CImageEditWindow::OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	util::shared_ptr<Gdiplus::Bitmap> n;
	Clipboard x(*this);
	LibCC::Result r = x.GetBitmap(n);
	if(r.Failed())
	{
		MessageBox(LibCC::Format(L"There was a problem pasting.|%")(r.str()).CStr(), L"Screenie", MB_ICONERROR | MB_OK);
	}
	else
	{
		SetBitmap(n);
		m_notify->OnPaste(n);
	}
	return 0;
}

LRESULT CImageEditWindow::OnCopy(WORD /*wNotifyCode*/, WORD /* wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	util::shared_ptr<Gdiplus::Bitmap> n(m_bitmap);
	if(HasSelection())
	{
		CRect selectionRect = GetSelection();
		n = GetBitmapRect(selectionRect);
	}
	Clipboard c(*this);
	LibCC::Result r = c.SetBitmap(n.get());
	if(r.Failed())
	{
		MessageBox(LibCC::Format(L"There was a problem copying.|%")(r.str()).CStr(), L"Screenie", MB_ICONERROR | MB_OK);
	}
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
  PointF virtual_(m_lastCursorImage);
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

void CImageEditWindow::CenterOnImage(const PointF& p)
{
	m_display.SetImageOrigin(p, "CenterOnImage");

	RECT rc;
	this->GetClientRect(&rc);
	m_display.SetViewOrigin(PointF(rc.right / 2, rc.bottom / 2));

	return;
}

void CImageEditWindow::AddCapture()
{
	if(m_captureRefs == 0)
	{
		SetCapture();
	}
	m_captureRefs ++;
}

void CImageEditWindow::ReleaseCapture()
{
	m_captureRefs --;
	if(m_captureRefs < 0)
		m_captureRefs = 0;

	if(m_captureRefs == 0)
	{
		::ReleaseCapture();
		m_isLeftClickDragging = false;
	}
}
  
LRESULT CImageEditWindow::OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = TRUE;
	if(m_cursorStack.size())
	{
		SetCursor(m_cursorStack.top());
	}
	else
	{
		SetCursor(LoadCursor(0, IDC_CROSS));
	}
	return TRUE;
}

void CImageEditWindow::PushCursor(PCWSTR newcursor)
{
	m_cursorStack.push(LoadCursor(0, newcursor));
	SetCursor(m_cursorStack.top());
	//LibCC::g_pLog->Message(LibCC::Format("PushCursor(%); size now = %").p(newcursor).i(m_cursorStack.size()));
}

void CImageEditWindow::PopCursor(bool set)
{
	if(m_cursorStack.size())
	{
		m_cursorStack.pop();
	}

	if(set)
	{
		if(m_cursorStack.size())
		{
			SetCursor(m_cursorStack.top());
		}
		else
		{
			SetCursor(LoadCursor(0, IDC_CROSS));
		}
	}
	//LibCC::g_pLog->Message(LibCC::Format("PopCursor; size now = %").i(m_cursorStack.size()));
}


void CImageEditWindow::SetBitmap(util::shared_ptr<Gdiplus::Bitmap> n)
{
	m_bitmap = n;
  CopyImage(m_dibDocument, *n);
	m_display.SetOriginalImage(m_dibRenderSource);
	m_display.ClearSelection();
  m_display.SetImageOrigin(PointF(m_bitmap->GetWidth() / 2, m_bitmap->GetHeight() / 2), "ImageEditWnd()");
}


