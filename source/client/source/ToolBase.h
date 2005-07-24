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

/*
  i realize i could have just used a Point for this, but i am almost certain that this will change
  dramatically so i'm starting it off like this.
*/
class PanningSpec
{
public:
  PanningSpec() :
    m_x(0),
    m_y(0)
  {
  }
  PanningSpec(int x, int y) :
    m_x(x),
    m_y(y)
  {
  }

  bool IsNotNull()
  {
    return (m_x != 0) || (m_y != 0);
  }

  int m_x;
  int m_y;
};

typedef void (*ToolTimerProc)(void*);

// individual tools receive a pointer to this so they can tell the image window to do things
// like panning or whatever else the main image edit window is capable of.
class IToolOperations
{
public:
  // gets a panningspec based on the cursor pos & window crap.  this spec can be used to pan the
  // window when the mouse cursor is outside the view.  since many tools will probably do panning
  // like this, i'm centralizing it in the tool ops class.
  virtual PanningSpec GetPanningSpec() = 0;
  virtual void Pan(const PanningSpec&, bool updateNow) = 0;
  virtual void Pan(int x, int y, bool updateNow) = 0;// x and y in virtual coords.
  virtual UINT_PTR CreateTimer(UINT elapse, ToolTimerProc, void* userData) = 0;// returns a cookie that identifies the tmier. guaranteed not to be 0, so you can use 0 to tell if an id is valid.
  virtual void DeleteTimer(UINT_PTR cookie) = 0;
  //virtual HWND GetHWND() = 0;
  virtual PointI GetCursorPosition() = 0;
  virtual int GetImageHeight() = 0;
  virtual int GetImageWidth() = 0;
  virtual void ClampToImage(PointI& p) = 0;// clamps the point to be within the image's bounds.
  virtual void ClampToImage(PointF& p) = 0;// clamps the point to be within the image's bounds.
  virtual void Refresh(const RECT& imageCoords, bool now) = 0;
  virtual void Refresh(bool now) = 0;
  virtual void SetCapture_() = 0;
  virtual void ReleaseCapture_() = 0;
  virtual bool HaveCapture() = 0;
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

class ISelectionToolCallback
{
public:
  virtual void OnSelectionToolSelectionChanged() = 0;
};

class SelectionTool :
  public ToolBase,
  public ISelectionToolCallback
{
  static const int patternFrequency = 8;
public:
  SelectionTool(IToolOperations* ops, ISelectionToolCallback* notify) :
    m_haveSelection(false),
    m_notify(this),
    m_ops(ops),
    m_patternOffset(0),
    m_panningTimer(0)
  {
    if(notify)
    {
      m_notify = notify;
    }
  }

  // ISelectionToolCallback methods
  void OnSelectionToolSelectionChanged()
  {
  }

  void OnInitTool()
  {
    m_ops->CreateTimer(120, TimerProc, this);
  }

  static void PanningTimerProc(void* pUser)
  {
    // pan based on current velocity / direction
    SelectionTool* pThis = reinterpret_cast<SelectionTool*>(pUser);
    pThis->m_ops->Pan(pThis->m_panningSpec, true);
  }

  static void TimerProc(void* pUser)
  {
    SelectionTool* pThis = reinterpret_cast<SelectionTool*>(pUser);
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
    if(m_ops->HaveCapture())
    {
      RECT rcExisting;
      if(m_haveSelection)
      {
        GetSelection(rcExisting);
      }

      m_haveSelection = true;
      m_selectionPt = p;
      m_notify->OnSelectionToolSelectionChanged();
      m_ops->ClampToImage(m_selectionPt);

      // fix up the selection to make it realistic.
      CRect rcNew;
      GetSelection(rcNew);

      if(rcNew.Width() == 0) m_haveSelection = false;
      if(rcNew.Height() == 0) m_haveSelection = false;

      m_panningSpec = m_ops->GetPanningSpec();

      if(m_panningSpec.IsNotNull())
      {
        // start the panning timer.
        m_panningTimer = m_ops->CreateTimer(500, SelectionTool::PanningTimerProc, this);
      }
      else
      {
        if(m_panningTimer)
        {
          m_ops->DeleteTimer(m_panningTimer);
          m_panningTimer = 0;
        }

        // no panning going on; refresh.
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
  }

  void OnLeftButtonDown(PointI p)
  {
    if(!m_ops->HaveCapture())
    {
      if(m_haveSelection)
      {
        RECT rc;
        GetSelection(rc);
        m_haveSelection = false;
        m_ops->Refresh(rc, true);
      }

      m_ops->SetCapture_();
      m_selectionOrg = m_ops->GetCursorPosition();
      m_notify->OnSelectionToolSelectionChanged();
      m_ops->ClampToImage(m_selectionOrg);
    }
  }

  void OnLeftButtonUp(PointI p)
  {
    if(m_ops->HaveCapture())
    {
      m_ops->ReleaseCapture_();
    }
  }

  void OnLoseCapture()
  {
    if(m_panningTimer)
    {
      m_ops->DeleteTimer(m_panningTimer);
      m_panningTimer = 0;
    }
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
      img.DrawSelectionRectSafe<patternFrequency, 55>(m_patternOffset, rc);
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
  ISelectionToolCallback* m_notify;

  int m_patternOffset;

  // for panning when the cursor is off the display
  PanningSpec m_panningSpec;
  UINT_PTR m_panningTimer;
};
