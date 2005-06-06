

#include "stdafx.hpp"
#include "resource.h"
#include "geom.h"

#include "clipboard.hpp"
#include "StatusDlg.hpp"

void ProgressImages::DrawHLine(long x1, long x2, long y)
{
  // draw horizontal line.
  long xleft = min(x1, x2);
  long xright = max(x1, x2) + 1;
  while(xleft != xright)
  {
    m_bmp->SetPixel(xleft, y, PositionToColor(xleft, y));
    xleft ++;
  }
}

void ProgressImages::DrawAlphaPixel(long cx, long cy, long x, long y, long f, long fmax)
{
  m_bmp->SetPixel(cx+x, cy+y, MixColorsInt(f, fmax, PositionToColor(cx+x, cy+y), m_background));
  m_bmp->SetPixel(cx+x, cy-y-1, MixColorsInt(f, fmax, PositionToColor(cx+x, cy-y-1), m_background));
  m_bmp->SetPixel(cx-x-1, cy+y, MixColorsInt(f, fmax, PositionToColor(cx-x-1, cy+y), m_background));
  m_bmp->SetPixel(cx-x-1, cy-y-1, MixColorsInt(f, fmax, PositionToColor(cx-x-1, cy-y-1), m_background));
}

// x and y are relative to the 
RgbPixel32 ProgressImages::PositionToColor(long x, long y)
{
  if(m_i == m_perimeter) return m_filled;// 100% one is all filled.
  float a = m_angles.GetAngle(x - m_radius, y - m_radius);// GetAngle() needs coords relative to the center of the circle

  // rotate it 90 degrees counter-clockwise.
  a += static_cast<float>(m_perimeter) / 4;
  if(a < 0) a += m_perimeter;
  if(a > m_perimeter) a -= m_perimeter;

  /* to fake anti-aliasing inside the pie, just calculate using a narrow gradient.
   --filled---k====a====l----unfilled------
              |---------| = blur size.
    the gradient is from k to l.  i may be anywhere in this diagram.
  */
  // get the distance from i to the center of the blurring window.
  float aa = a + (m_pieBlurringSize / 2) - static_cast<float>(m_i);
  if(aa < 0) return m_filled;
  if(aa > m_pieBlurringSize) return m_unfilled;
  // multiply by 100 because we are casting to integer
  return MixColorsInt(static_cast<long>(aa * 100.0f), static_cast<long>(m_pieBlurringSize * 100.0f), m_unfilled, m_filled);
}

void ProgressImages::InitializeProgressImages(CImageList& img, RgbPixel32 background, RgbPixel32 filled, RgbPixel32 unfilled)
{
  m_background = background;
  m_unfilled = unfilled;
  m_filled = filled;
  m_diameter = 14;
  m_radius = 7;
  m_pieBlurringSize = 2;
  m_perimeter = static_cast<int>(pi * m_diameter);
  m_background = background;
  m_images.clear();
  m_images.reserve(m_perimeter+1);

  // add 1 for complete coverage, to be on the safe side.
  m_angles.Resize(m_radius + 1, static_cast<float>(m_perimeter), 0);// the values returned from m_angles will be in the range 0-perimeter, or in other words, 0-m_images.size().

  for(m_i = 0; m_i <= m_perimeter; m_i ++)
  {
    m_bmp = new AnimBitmap<32>();
    m_bmp->SetSize(16, 16);
    m_bmp->Fill(m_background);
    FilledCircleAAG(8, 8, m_radius, this, DrawHLine, this, DrawAlphaPixel);
    // add it to the imagelist.
    HBITMAP hbm = m_bmp->DetachHandle();
    m_images.push_back(img.Add(hbm));
    DeleteObject(hbm);
    delete m_bmp;
    m_bmp = 0;
  }
}

int ProgressImages::GetImageFromProgress(int pos, int total)
{
  // pos / total = index / m_images.size();
  int index = pos * m_images.size() / total;
  // bounds checking.
  if(index < 0) index = 0;
  if(index >= (int)m_images.size()) index = m_images.size() - 1;
  return m_images[index];
}


BOOL CStatusDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CStatusDlg::OnIdle()
{
	return FALSE;
}

//
// StatusWindow implementation
//

void CStatusDlg::ClearMessages()
{
  CriticalSection::ScopeLock lock(m_cs);

  if (m_listView.IsWindow())
		m_listView.DeleteAllItems();
}

void CStatusDlg::PrintMessage(const MessageType type, const tstd::tstring& destination,
	const tstd::tstring& message)
{
  CriticalSection::ScopeLock lock(m_cs);

	if (m_listView.IsWindow())
	{
		int itemID = m_listView.GetItemCount() + 1;

		itemID = m_listView.AddItem(itemID, 0, destination.c_str(), (type == MSG_ERROR) ? 1 : 0);
		m_listView.SetItem(itemID, 1, LVIF_TEXT, message.c_str(), 0, 0, 0, 0);
	}

  m_listView.SetColumnWidth(0, LVSCW_AUTOSIZE);
  m_listView.SetColumnWidth(1, LVSCW_AUTOSIZE);
}

//

LRESULT CStatusDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow();

	// set icons
  if(m_hIcon) DestroyIcon(m_hIcon);
  if(m_hIconSmall) DestroyIcon(m_hIconSmall);
	m_hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SCREENIE), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(m_hIcon, TRUE);
	m_hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SCREENIE), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(m_hIconSmall, FALSE);

  
  DlgResize_Init(true, true, WS_CLIPCHILDREN);

	// create and populate image list for message icons
	if (m_imageList.Create(16, 16, ILC_COLOR32, 2, 0))
	{
		m_imageList.AddIcon(::LoadIcon(NULL, IDI_INFORMATION));
		m_imageList.AddIcon(::LoadIcon(NULL, IDI_ERROR));
    
    m_progress.InitializeProgressImages(m_imageList,
      COLORREFToRgbPixel32(m_listView.GetBkColor()),
      MakeRgbPixel32(0,179,0),
      MakeRgbPixel32(204,155,102) );

    // set up the list view for messages
		m_listView = GetDlgItem(IDC_MESSAGES);

		if (m_listView.IsWindow())
		{
			m_listView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
			m_listView.SetImageList(m_imageList, LVSIL_SMALL);

			m_listView.AddColumn(TEXT("Destination"), 0);
			m_listView.SetColumnWidth(0, LVSCW_AUTOSIZE);

			m_listView.AddColumn(TEXT("Message"), 1);
			m_listView.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
		}
	}

	return TRUE;
}

LRESULT CStatusDlg::OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	static bool firstShow = true;

	// wParam == TRUE if the window is being shown.
	if (firstShow && wParam)
	{
		if(m_options.HaveStatusPlacement())
		{
			SetWindowPlacement(&m_options.GetStatusPlacement());
		}

		firstShow = false;
	}

	return 0;
}

LRESULT CStatusDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CloseDialog(IDOK);

	return 0;
}

LRESULT CStatusDlg::OnChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	return 0;
}

LRESULT CStatusDlg::OnRightClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	OnCopy(0, 0, 0, bHandled);

	NMITEMACTIVATE* itemActivate = reinterpret_cast<NMITEMACTIVATE*>(pnmh);

	CMenu contextMenu(AtlLoadMenu(IDM_CONTEXTMENU));
	CMenuHandle listMenu = contextMenu.GetSubMenu(2);

	POINT cursorPos = { 0 };
	::GetCursorPos(&cursorPos);

	listMenu.TrackPopupMenu(TPM_LEFTALIGN, cursorPos.x, cursorPos.y, m_hWnd);

	return 0;
}

LRESULT CStatusDlg::OnCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	LVITEM item = { 0 };

	TCHAR textBuffer[1024] = { 0 };
	item.cchTextMax = 1024;
	item.pszText = textBuffer;
	item.mask = LVIF_TEXT;
	item.iSubItem = 1;

	if (m_listView.GetSelectedItem(&item))
	{
		try
		{
			Clipboard(m_hWnd).SetText(item.pszText);
		}
		catch (const Win32Exception& excp)
		{
#ifdef _DEBUG
			MessageBox(excp.What().c_str(), TEXT("Clipboard Error"),
				MB_OK | MB_ICONERROR);
#endif
		}
	}

	return 0;
}

LRESULT CStatusDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(IDOK);

	return 0;
}

void CStatusDlg::CloseDialog(int nVal)
{
	// the next time the user sees this dialog, he probably doesn't want to
	// see messages pertaining to previously processed screenshots
	ClearMessages();

  // save window placement
  WINDOWPLACEMENT wp;
  GetWindowPlacement(&wp);
  m_options.SetStatusPlacement(wp);

	// this doesn't actually close the dialog box.
	ShowWindow(SW_HIDE);
}
