

#pragma once


class SelectionTool :
  public ToolBase
{
  static const int patternFrequency = 8;
public:
  SelectionTool(IToolOperations* ops) :
    m_ops(ops)
  {
  }

	void OnInitTool(){ }
  void OnSelectTool(){}
  void OnDeselectTool(){}

  void OnCursorMove(PointF p, bool dragging)
  {
    if(dragging)
    {
			m_selectionPt = p;
			m_ops->SetSelection(GetSelection());
			m_ops->Refresh(true);
    }
  }

  void OnLeftButtonDown(PointF p){}
  void OnLeftButtonUp(PointF p){}

  void OnStartDragging()
  {
    if(m_ops->HasSelection())
    {
      m_ops->ClearSelection();
		m_ops->Refresh(true);
    }
    m_selectionOrg = m_ops->GetCursorPosition();
		m_selectionOrg.SelfRound();// always start the selection on an integral point.
    m_ops->ClampToImage(m_selectionOrg);
  }

  void OnStopDragging(){}
  void OnPaint(AnimBitmap<32>& img){}

private:
	// returns selection in image coordinates.
  CRect GetSelection() const
  {
		PointF ul, br;
		ul.x = min(m_selectionOrg.x, m_selectionPt.x);
		ul.y = min(m_selectionOrg.y, m_selectionPt.y);
		br.x = max(m_selectionOrg.x, m_selectionPt.x);
		br.y = max(m_selectionOrg.y, m_selectionPt.y);
		m_ops->ClampToImage(ul);
		m_ops->ClampToImage(br);
		return CRect(ul.Round(), br.Round());
  }

  PointF m_selectionOrg;// origin of the selection, in image coords.
  PointF m_selectionPt;// the "other" point of the selection, in image coords.

  IToolOperations* m_ops;
};
