/*
  Tools for use in the ImageEdit window
*/

#pragma once

template<typename T>
void PointsToRect(Point<T> a, Point<T> b, RECT& rc)
{
  rc.top = min(a.y, b.y);
  rc.bottom = max(a.y, b.y);
  rc.left = min(a.x, b.x);
  rc.right = max(a.x, b.x);
}

// individual tools receive a pointer to this so they can tell the image window to do things
// like panning or whatever else the main image edit window is capable of.
class IToolOperations
{
public:
  virtual void Pan(int x, int y, bool updateNow) = 0;// x and y in virtual coords.
  virtual void CreateTimer(UINT elapse, TIMERPROC, void* userData) = 0;
  virtual HWND GetHWND() = 0;
  virtual Point<int> GetCursorPosition() = 0;
  virtual int GetImageHeight() = 0;
  virtual int GetImageWidth() = 0;
  virtual void ClampToImage(Point<int>& p) = 0;// clamps the point to be within the image's bounds.
  virtual void ClampToImageF(Point<float>& p) = 0;// clamps the point to be within the image's bounds.
  virtual void Refresh(const RECT& imageCoords, bool now) = 0;
  virtual void Refresh(bool now) = 0;
};

class ToolBase
{
public:
  virtual void OnInitTool() = 0;
  virtual void OnSelectTool() = 0;
  virtual void OnDeselectTool() = 0;
  virtual void OnCursorMove(Point<int> p) = 0;
  virtual void OnLeftButtonDown(Point<int> p) = 0;
  virtual void OnLeftButtonUp(Point<int> p) = 0;
  virtual void OnLoseCapture() = 0;
  virtual void OnPaint(AnimBitmap<32>& img, const Viewport<int>& view, int marginX, int marginY) = 0;
};
class PencilTool : public ToolBase
{
public:
  void OnInitTool() { }
  void OnSelectTool() { }
  void OnDeselectTool() { }
  void OnCursorMove(Point<int> p) { }
  void OnLeftButtonDown(Point<int> p) { }
  void OnLeftButtonUp(Point<int> p) { }
  void OnLoseCapture() { }
  void OnPaint(AnimBitmap<32>& img, const Viewport<int>& view, int marginX, int marginY) { }
};

class TextTool : public ToolBase
{
public:
  void OnInitTool() { }
  void OnSelectTool() { }
  void OnDeselectTool() { }
  void OnCursorMove(Point<int> p) { }
  void OnLeftButtonDown(Point<int> p) { }
  void OnLeftButtonUp(Point<int> p) { }
  void OnLoseCapture() { }
  void OnPaint(AnimBitmap<32>& img, const Viewport<int>& view, int marginX, int marginY) { }
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

  void OnCursorMove(Point<int> p)
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

      RECT rcNew;
      GetSelection(rcNew);
      RECT rc;
      UnionRect(&rc, &rcNew, &rcExisting);
      m_ops->Refresh(rc, true);
    }
  }

  void OnLeftButtonDown(Point<int> p)
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

  void OnLeftButtonUp(Point<int> p)
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

  void OnPaint(AnimBitmap<32>& img, const Viewport<int>& view, int marginX, int marginY)
  {
    if(m_haveSelection)
    {
      RECT rc;
      Point<int> a = view.VirtualToView(m_selectionOrg);
      Point<int> b = view.VirtualToView(m_selectionPt);
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
  Point<int> m_selectionOrg;// origin of the selection, in image coords.
  Point<int> m_selectionPt;// the "other" point of the selection, in image coords.
  IToolOperations* m_ops;

  bool m_haveCapture;

  int m_patternOffset;
};
