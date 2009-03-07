

#pragma once


class HighlightTool :
  public ToolBase
{
public:
  HighlightTool(IToolOperations* ops) :
    m_ops(ops),
		m_pen(Gdiplus::Color(192, 255, 255, 0), 8),
		isDrawing(false)
	{
  }

  void OnSelectTool()
	{
		m_path.Reset();
		m_path.SetFillMode(Gdiplus::FillModeWinding);
	}

  void OnDeselectTool()
	{
		m_path.Reset();
		m_path.SetFillMode(Gdiplus::FillModeWinding);
	}

	virtual int GetResourceID() { return IDC_HIGHLIGHTTOOL; }

  void OnCursorMove(PointF p, bool dragging)
  {
		if(dragging)
		{
			m_path.AddLine(previousPoint.ToGdiplusPointF(), p.ToGdiplusPointF());
			previousPoint = p;
			m_ops->Redraw();
		}
  }

  void OnLeftButtonDown(PointF p)
	{
	}

  void OnLeftButtonUp(PointF p)
	{
	}

  void OnStartDragging(PointF p)
  {
		isDrawing = true;
		previousPoint = p;
  }

  void OnStopDragging()
	{
		// commit to screen.
		OnPaintClient(*m_ops->GetDocument());
		m_path.Reset();
		m_path.SetFillMode(Gdiplus::FillModeWinding);
		isDrawing = false;
		m_ops->SetDocumentDirty();
	}

  void OnPaintClient(AnimBitmap<32>& doc)
	{
		if(isDrawing)
		{
			Gdiplus::Graphics g(doc.GetDC());
			g.DrawPath(&m_pen, &m_path);
		}
	}

	IToolOperations* m_ops;

	bool isDrawing;
	PointF previousPoint;
	Gdiplus::GraphicsPath m_path;// current path the user is drawing.
	Gdiplus::Pen m_pen;

};
