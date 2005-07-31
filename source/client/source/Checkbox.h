

#pragma once


class CheckboxButton : public CWindowImpl<CheckboxButton>
{
public:
  CheckboxButton() :
    m_haveCapture(false),
    m_isChecked(false),
    m_mouseOver(false)
  {
  }

	BEGIN_MSG_MAP(CheckboxButton)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkGnd)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_CAPTURECHANGED, OnCaptureChanged)
	END_MSG_MAP()

  void SetChecked(UINT i, PCTSTR szResType)
  {
    m_checked = LoadBitmapResource(MAKEINTRESOURCE(i), szResType);
  }
  void SetCheckedHover(UINT i, PCTSTR szResType)
  {
    m_checkedHover = LoadBitmapResource(MAKEINTRESOURCE(i), szResType);
  }
  void SetCheckedDown(UINT i, PCTSTR szResType)
  {
    m_checkedDown = LoadBitmapResource(MAKEINTRESOURCE(i), szResType);
  }
  void SetUnchecked(UINT i, PCTSTR szResType)
  {
    m_unchecked = LoadBitmapResource(MAKEINTRESOURCE(i), szResType);
  }
  void SetUncheckedHover(UINT i, PCTSTR szResType)
  {
    m_uncheckedHover = LoadBitmapResource(MAKEINTRESOURCE(i), szResType);
  }
  void SetUncheckedDown(UINT i, PCTSTR szResType)
  {
    m_uncheckedDown = LoadBitmapResource(MAKEINTRESOURCE(i), szResType);
  }

  LRESULT OnEraseBkGnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
  {
    bHandled = TRUE;
    return TRUE;
  }

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
  {
    bool oldMouseOver = m_mouseOver;
    CRect rc;
    GetClientRect(&rc);
    CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    if(rc.PtInRect(pt))
    {
      if(!oldMouseOver)
      {
        TRACKMOUSEEVENT tme = { 0 };
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = m_hWnd;
        tme.dwHoverTime = HOVER_DEFAULT;
        TrackMouseEvent(&tme);
      }
      m_mouseOver = true;
    }
    else
    {
      m_mouseOver = false;
    }
    if(oldMouseOver != m_mouseOver)
    {
      Invalidate();
    }
    return 0;
  }

	LRESULT OnMouseLeave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
  {
    bHandled = TRUE;
    m_mouseOver = false;
    Invalidate();
    return 0;
  }

	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
  {
    SetCapture();
    m_haveCapture = true;
    Invalidate();
    return 0;
  }

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
  {
    if(m_haveCapture)
    {
      if(m_mouseOver)
      {
        m_isChecked = !m_isChecked;
      }
      ReleaseCapture();
      Invalidate();
    }
    return 0;
  }

	LRESULT OnCaptureChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
  {
    bHandled = TRUE;
    m_haveCapture = false;
    return 0;
  }

  void DrawBitmap(HDC dc, HBITMAP bmp)
  {
    BITMAP bm;
    GetObject(bmp, sizeof(bm), &bm);
    HDC dcMem = CreateCompatibleDC(dc);
    HGDIOBJ hOld = SelectObject(dcMem, bmp);
    BitBlt(dc, 0, 0, bm.bmWidth, bm.bmHeight, dcMem, 0, 0, SRCCOPY);
    SelectObject(dcMem, hOld);
    DeleteDC(dcMem);
  }

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
  {
    bHandled = TRUE;
    PAINTSTRUCT ps;
    BeginPaint(&ps);
    if(m_isChecked && m_mouseOver && m_haveCapture)
    {
      DrawBitmap(ps.hdc, m_checkedDown.handle);
    }
    else if(m_isChecked && m_mouseOver)
    {
      DrawBitmap(ps.hdc, m_checkedHover.handle);
    }
    else if(m_isChecked)
    {
      DrawBitmap(ps.hdc, m_checked.handle);
    }
    else if(m_mouseOver && m_haveCapture)
    {
      DrawBitmap(ps.hdc, m_uncheckedDown.handle);
    }
    else if(m_mouseOver)
    {
      DrawBitmap(ps.hdc, m_uncheckedHover.handle);
    }
    else
    {
      DrawBitmap(ps.hdc, m_unchecked.handle);
    }
    EndPaint(&ps);
    return 0;
  }
  
  bool IsChecked() const
  {
    return m_isChecked;
  }

private:
  bool m_haveCapture;
  bool m_isChecked;
  bool m_mouseOver;
  AutoGdiBitmap m_checked;
  AutoGdiBitmap m_checkedHover;
  AutoGdiBitmap m_checkedDown;
  AutoGdiBitmap m_unchecked;
  AutoGdiBitmap m_uncheckedHover;
  AutoGdiBitmap m_uncheckedDown;
};
