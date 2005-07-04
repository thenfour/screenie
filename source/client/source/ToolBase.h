/*
  Tools for use in the ImageEdit window
*/

#pragma once

inline void PointsToRect(PointI a, PointI b, RECT& rc)
{
  rc.top = min(a.y, b.y);
  rc.bottom = max(a.y, b.y);
  rc.left = min(a.x, b.x);
  rc.right = max(a.x, b.x);
}

inline void PointsToRect(PointF a, PointF b, RECT& rc)
{
  PointsToRect(PointFtoI(a), PointFtoI(b), rc);
}

// individual tools receive a pointer to this so they can tell the image window to do things
// like panning or whatever else the main image edit window is capable of.
class IToolOperations
{
public:
  virtual void Pan(int x, int y, bool updateNow) = 0;// x and y in virtual coords.
  virtual void CreateTimer(UINT elapse, TIMERPROC, void* userData) = 0;
  virtual HWND GetHWND() = 0;
  virtual PointI GetCursorPosition() = 0;
  virtual int GetImageHeight() = 0;
  virtual int GetImageWidth() = 0;
  virtual void ClampToImage(PointI& p) = 0;// clamps the point to be within the image's bounds.
  virtual void ClampToImage(PointF& p) = 0;// clamps the point to be within the image's bounds.
  virtual void Refresh(const RECT& imageCoords, bool now) = 0;
  virtual void Refresh(bool now) = 0;
};

class ToolBase
{
public:
  virtual void OnInitTool() = 0;
  virtual void OnSelectTool() = 0;
  virtual void OnDeselectTool() = 0;
  virtual void OnCursorMove(PointI p) = 0;
  virtual void OnLeftButtonDown(PointI p) = 0;
  virtual void OnLeftButtonUp(PointI p) = 0;
  virtual void OnLoseCapture() = 0;
  virtual void OnPaint(AnimBitmap<32>& img, const Viewport& view, int marginX, int marginY) = 0;
};
class PencilTool : public ToolBase
{
public:
  void OnInitTool() { }
  void OnSelectTool() { }
  void OnDeselectTool() { }
  void OnCursorMove(PointI p) { }
  void OnLeftButtonDown(PointI p) { }
  void OnLeftButtonUp(PointI p) { }
  void OnLoseCapture() { }
  void OnPaint(AnimBitmap<32>& img, const Viewport& view, int marginX, int marginY) { }
};

class TextTool : public ToolBase
{
public:
  void OnInitTool() { }
  void OnSelectTool() { }
  void OnDeselectTool() { }
  void OnCursorMove(PointI p) { }
  void OnLeftButtonDown(PointI p) { }
  void OnLeftButtonUp(PointI p) { }
  void OnLoseCapture() { }
  void OnPaint(AnimBitmap<32>& img, const Viewport& view, int marginX, int marginY) { }
};

class SelectionTool : public ToolBase
{
  static const int patternFrequency = 8;
public:
  SelectionTool(IToolOperations* ops) :
    m_haveSelection(false),
    m_haveCapture(false),
    m_ops(ops),
    m_patternOffset(0)
  {
  }

  void OnInitTool()
  {
    m_ops->CreateTimer(120, TimerProc, this);
  }

  static VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
  {
    SelectionTool* pThis = reinterpret_cast<SelectionTool*>(idEvent);
    pThis->m_patternOffset ++;
    if(pThis->m_patternOffset >= patternFrequency)
    {
      pThis->m_patternOffset = 0;
    }
    if(pThis->m_haveSelection)
    {
      RECT rc;
      pThis->GetSelection(rc);
      pThis->m_ops->Refresh(rc, false);
    }
  }

  void OnSelectTool()
  {
    // don't need to do anything.
  }

  void OnDeselectTool()
  {
    // don't need to do anything.
  }

  void OnCursorMove(PointI p)
  {
    if(m_haveCapture)
    {
      RECT rcExisting;
      if(m_haveSelection)
      {
        GetSelection(rcExisting);
      }

      m_haveSelection = true;
      m_selectionPt = p;
      m_ops->ClampToImage(m_selectionPt);

      // fix up the selection to make it realistic.
      CRect rcNew;
      GetSelection(rcNew);

      if(rcNew.Width() == 0) m_haveSelection = false;
      if(rcNew.Height() == 0) m_haveSelection = false;

      // refresh.
      RECT rc;
      if(m_haveSelection)
      {
        GetSelection(rcNew);
        UnionRect(&rc, &rcNew, &rcExisting);
      }
      else
      {
        CopyRect(&rc, &rcExisting);
      }
      m_ops->Refresh(rc, true);
    }
  }

  void OnLeftButtonDown(PointI p)
  {
    if(!m_haveCapture)
    {
      if(m_haveSelection)
      {
        RECT rc;
        GetSelection(rc);
        m_haveSelection = false;
        m_ops->Refresh(rc, true);
      }

      m_haveCapture = true;
      SetCapture(m_ops->GetHWND());
      m_selectionOrg = m_ops->GetCursorPosition();
      m_ops->ClampToImage(m_selectionOrg);
    }
  }

  void OnLeftButtonUp(PointI p)
  {
    if(m_haveCapture)
    {
      ReleaseCapture();
      m_haveCapture = false;
    }
  }

  void OnLoseCapture()
  {
    // don't need to do anything.
  }

  void OnPaint(AnimBitmap<32>& img, const Viewport& view, int marginX, int marginY)
  {
    if(m_haveSelection)
    {
      RECT rc;
      PointI a = view.VirtualToView(m_selectionOrg);
      PointI b = view.VirtualToView(m_selectionPt);
      PointsToRect(a, b, rc);
      OffsetRect(&rc, marginX, marginY);

      // now we have a view rect; figure out if we can draw all edges.
      img.DrawSelectionRectSafe<patternFrequency, 64>(m_patternOffset, rc);
    }
  }

  bool GetSelection(RECT& rc) const
  {
    if(m_haveSelection)
    {
      PointsToRect(m_selectionOrg, m_selectionPt, rc);
      return true;
    }

    return false;
  }

  void ClearSelection()
  {
    m_haveSelection = false;
  }

  bool HasSelection() const
  {
    return m_haveSelection;
  }

  bool m_haveSelection;
  PointI m_selectionOrg;// origin of the selection, in image coords.
  PointI m_selectionPt;// the "other" point of the selection, in image coords.
  IToolOperations* m_ops;

  bool m_haveCapture;

  int m_patternOffset;
};
