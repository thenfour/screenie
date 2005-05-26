//
//
//
//
//

#include "stdafx.hpp"
#include "resource.h"

#include <algorithm>

#include "CroppingWnd.hpp"

CCroppingWindow::CCroppingWindow(util::shared_ptr<Gdiplus::Bitmap> bitmap)
	: m_bitmap(bitmap), m_selecting(false), m_hasSelection(false)
{
}

CCroppingWindow::~CCroppingWindow()
{
}

BOOL CCroppingWindow::PreTranslateMessage(MSG* pMsg)
{
	pMsg;

	return TRUE;
}

void CCroppingWindow::ClearSelection()
{
	m_selecting = false;
	m_hasSelection = false;

	::SetRect(&m_selection, 0, 0, 0, 0);

	InvalidateRect(NULL);
}

void CCroppingWindow::BeginSelection(int x, int y)
{
	ClearSelection();

	m_hasSelection = false;
	m_selecting = true;

	::GetCapture();

	m_selBegin.x = x;
	m_selBegin.y = y;
}

void CCroppingWindow::UpdateSelection(int x, int y)
{
	m_selection.left = std::min<int>(x, m_selBegin.x);
	m_selection.top = std::min<int>(y, m_selBegin.y);
	m_selection.bottom = std::max<int>(y, m_selBegin.y);
	m_selection.right = std::max<int>(x, m_selBegin.x);

	RECT rectInvalidate = m_selection;
	::InflateRect(&rectInvalidate, 5, 5);

	InvalidateRect(&rectInvalidate);
}

void CCroppingWindow::EndSelection(int x, int y)
{
	::ReleaseCapture();

	m_selecting = false;
	m_hasSelection = true;

	InvalidateRect(NULL);
}

bool CCroppingWindow::GetSelection(RECT& selectionRect)
{
	if (!m_hasSelection)
		return false;

	RECT clientRect = { 0 };
	GetClientRect(&clientRect);

	selectionRect.left = int(m_selection.left * (float(m_bitmap->GetWidth()) / clientRect.right));
	selectionRect.right = int(m_selection.right * (float(m_bitmap->GetWidth()) / clientRect.right));
	selectionRect.top = int(m_selection.top * (float(m_bitmap->GetHeight()) / clientRect.bottom));
	selectionRect.bottom = int(m_selection.bottom * (float(m_bitmap->GetHeight()) / clientRect.bottom));

	return true;
}

util::shared_ptr<Gdiplus::Bitmap> CCroppingWindow::GetBitmapRect(const RECT& rectToCopy)
{
	Gdiplus::Bitmap* bitmapClone = m_bitmap->Clone(rectToCopy.left, rectToCopy.top,
		rectToCopy.right - rectToCopy.left, rectToCopy.bottom - rectToCopy.top, PixelFormatDontCare);

	return util::shared_ptr<Gdiplus::Bitmap>(bitmapClone);
}

LRESULT CCroppingWindow::OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	return 0;
}

LRESULT CCroppingWindow::OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	ClearSelection();

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

	Gdiplus::Graphics windowGraphics(dc);

	Gdiplus::Bitmap memBitmap(width, height, &windowGraphics);
	Gdiplus::Graphics memGraphics(&memBitmap);

	memGraphics.DrawImage(m_bitmap.get(), clientRect.left, clientRect.top,
		clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

	if (m_selecting || m_hasSelection)
	{
		Gdiplus::Pen pen(Gdiplus::Color(255, 255, 255), 2.0f);
		pen.SetDashStyle(Gdiplus::DashStyleDot);
		memGraphics.DrawRectangle(&pen, m_selection.left, m_selection.top,
			m_selection.right - m_selection.left, m_selection.bottom - m_selection.top);
	}

	windowGraphics.DrawImage(&memBitmap, 0, 0, width, height);

	EndPaint(&paintStruct);

	return 0;
}