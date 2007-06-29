

#include "stdafx.hpp"
#include "ActivityListItem.hpp"


ActivityListItemSpec::ActivityListItemSpec() :
	m_eventID(0),
	m_eventImageIndex(-1),
	m_screenshotID(0),
	m_listIndex(-1),
	m_info(0),
	m_dirty(true),
	m_itemWidth(0),
	m_itemHeight(0),
	m_selectedState(false)
{
}

ActivityListItemSpec::~ActivityListItemSpec()
{
	ReleaseThumbnail();
}

bool ActivityListItemSpec::RenderTo(HDC dc, CRect rc, bool selected)
{
	_SetValue(m_selectedState, selected);
	RenderOffscreen();
	int n = ::BitBlt(dc, rc.left, rc.top, rc.Width(), rc.Height(), m_offscreen, 0, 0, SRCCOPY);
	return true;
}

int ActivityListItemSpec::GetItemHeight()
{
	CalculateDimensions();
	return m_itemHeight;
}

void ActivityListItemSpec::ReleaseThumbnail()
{
	m_dirty = true;
	m_thumb.reset();
}

void ActivityListItemSpec::SetThumbnail(Gdiplus::BitmapPtr thumbnail)
{
	ReleaseThumbnail();
	m_dirty = true;
	m_thumb = thumbnail;
}


/*

  padding      thumbmarginright
  |-|          |-|

  +---------------------------------------------------+   ---
	|                                                   |    | padding
	|  thumbthumb   Screenshot                          |   ---
	|  thumbthumb   342 x 123 pixels                    |
	|  thumbthumb                                       |   ---
	|                                                   |    | padding
  +---------------------------------------------------+   ---

	Events are basically the same, except instead of a thumbnail, it's an icon, and there is only 1 line of text, not 3
*/

void ActivityListItemSpec::CalculateDimensions()
{
	// since the window size may have changed, deal with that first
	CRect rc;
	GetClientRect(m_info->m_hwnd, &rc);
	_SetValue(m_itemWidth, rc.Width());
	if(!m_dirty)
		return;

	switch(m_type)
	{
	case Screenshot:
		{
			int contentHeight = max((UINT)m_info->m_lineHeight * 2, m_thumb->GetHeight());
			int surroundingHeight = m_info->ScreenshotSeparatorHeight + m_info->ScreenshotPadding.top +
				m_info->ScreenshotThumbBorderSize + m_info->ScreenshotThumbBorderSize + m_info->ScreenshotPadding.bottom;
			m_itemHeight = contentHeight + surroundingHeight;
			break;
		}
	case Event:
		{
			int contentHeight = max(m_info->m_lineHeight, m_info->m_iconHeight);
			m_itemHeight = contentHeight + m_info->EventPadding.top + m_info->EventPadding.bottom;
			break;
		}
	}
}

ActivityListItemSpec::RenderingInfo::RenderingInfo() :
	ScreenshotPadding(5, 5, 5, 5),
	ScreenshotCaptionMarginLeft(120),
	ScreenshotThumbBorderSize(4),
	ScreenshotSeparatorHeight(1),
	EventIconPaddingRight(10),
	EventPadding(25,5,5,5)
{
}

void ActivityListItemSpec::RenderOffscreen()
{
	if(!m_dirty)
		return;
	CalculateDimensions();

	// create basic GDI objects
	m_offscreen.Attach(0);
	HDC dcScreen = GetDC(0);
	m_offscreen.CreateCompatibleDC(dcScreen);
	m_offscreenbm.Attach(0);
	m_offscreenbm.CreateCompatibleBitmap(dcScreen, m_itemWidth, m_itemHeight);
	m_offscreen.SelectBitmap(m_offscreenbm);
	ReleaseDC(0, dcScreen);

	// render.
	m_offscreen.SetBkMode(TRANSPARENT);
	m_offscreen.SelectFont(UtilGetShellFont());

	CRect rcItem(0, 0, m_itemWidth, m_itemHeight);
	if(m_selectedState)
	{
		m_offscreen.FillRect(&rcItem, COLOR_HIGHLIGHT);
		m_offscreen.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else
	{
		m_offscreen.FillRect(&rcItem, COLOR_WINDOW);
		m_offscreen.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
	}

	// draw foreground stuff
	switch(m_type)
	{
	case Screenshot:
		{
			CRect rcSeparator(0, 0, m_itemWidth, m_info->ScreenshotSeparatorHeight);
			m_offscreen.FillRect(&rcSeparator, COLOR_GRAYTEXT);

			CRect rcThumbBorder;
			rcThumbBorder.top = m_info->ScreenshotSeparatorHeight + m_info->ScreenshotPadding.top;
			rcThumbBorder.left = m_info->ScreenshotPadding.left;
			rcThumbBorder.right = rcThumbBorder.left + m_info->ScreenshotThumbBorderSize + m_thumb->GetWidth() + m_info->ScreenshotThumbBorderSize;
			rcThumbBorder.bottom = rcThumbBorder.top + m_info->ScreenshotThumbBorderSize + m_thumb->GetHeight() + m_info->ScreenshotThumbBorderSize;
			m_offscreen.FillRect(&rcThumbBorder, COLOR_GRAYTEXT);

			Gdiplus::Graphics* gfx = new Gdiplus::Graphics(m_offscreen);
			gfx->DrawImage(m_thumb.get(),
				m_info->ScreenshotPadding.left + m_info->ScreenshotThumbBorderSize,
				m_info->ScreenshotSeparatorHeight + m_info->ScreenshotPadding.top + m_info->ScreenshotThumbBorderSize);
			delete gfx;

			// draw the captions.
			CRect rcCaption(rcItem);

			std::wstring text = L"Screenshot";
			rcCaption.top = m_info->ScreenshotPadding.top + m_info->ScreenshotThumbBorderSize;
			rcCaption.left = m_info->ScreenshotCaptionMarginLeft;
			m_offscreen.DrawTextW(text.c_str(), text.size(), &rcCaption, 0);

			text = LibCC::Format("% x % pixels").i(m_screenshotWidth).i(m_screenshotHeight).Str();
			rcCaption.top += m_info->m_lineHeight;
			rcCaption.bottom += m_info->m_lineHeight;
			m_offscreen.DrawTextW(text.c_str(), text.size(), &rcCaption, 0);
			break;
		}
	case Event:
		{
			m_info->m_imageList.Draw(m_offscreen, m_eventImageIndex, m_info->EventPadding.left, m_info->EventPadding.top, ILD_NORMAL);

			CRect rcCaption(rcItem);
			rcCaption.top = m_info->EventPadding.top;
			rcCaption.left = m_info->EventPadding.left + m_info->m_iconWidth + m_info->EventIconPaddingRight;
			m_offscreen.DrawTextW(m_eventMessage.c_str(), m_eventMessage.size(), &rcCaption, 0);
			break;
		}
	}
}
