

#pragma once


class SelectionTool :
  public ToolBase
{
  static const int patternFrequency = 8;

	enum GripperType
	{
		None,
		Moving,
		Left,
		Top,
		Right,
		Bottom,
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight
	};
	LPCWSTR GetCursorForGripperType(GripperType g)
	{
		switch(g)
		{
		case Moving:
			return IDC_SIZEALL;
		case TopLeft:
		case BottomRight:
			return IDC_SIZENWSE;
		case TopRight:
		case BottomLeft:
			return IDC_SIZENESW;
		case Right:
		case Left:
			return IDC_SIZEWE;
		case Top:
		case Bottom:
			return IDC_SIZENS;
		}
		return 0;
	}

public:
  SelectionTool(IToolOperations* ops) :
    m_ops(ops),
		m_gripper(None),
		m_movableX(0),
		m_movableY(0),
		m_hasCursor(false)
	{
  }

	void OnInitTool(){ }
  void OnSelectTool(){}
  void OnDeselectTool(){}

	static inline int Calculate2DDistanceCheap(int x1, int y1, int x2, int y2)
	{
		int dx = x1 - x2;
		int dy = y1 - y2;
		return (dx * dx) + (dy * dy);
	}

	// helper function. takes [distance between the cursor and an on-screen element] and uses the lowest one to set
	// the cursor icon and m_gripper state. returns false if none are within the tolerance.
	bool AttemptToSetGripperStatus(const std::vector<std::pair<int, GripperType> >& items, int tolerance, bool checkTolerance)
	{
		const std::pair<int, GripperType>* lowest = 0;
		// find the lowest
		for(std::vector<std::pair<int, GripperType> >::const_iterator it = items.begin(); it != items.end(); ++ it)
		{
			if(lowest == 0 || it->first < lowest->first)
				lowest = &(*it);
		}
		if(lowest == 0)
			return false;
		if(checkTolerance && lowest->first > tolerance)
			return false;

		m_gripper = lowest->second;
		if(m_hasCursor)
		{
			m_ops->PopCursor(false);
		}
		m_ops->PushCursor(GetCursorForGripperType(m_gripper));
		m_hasCursor = true;
		return true;
	}

  void OnCursorMove(PointF p, bool dragging)
  {
		CPoint cursor = p.Round();
    if(dragging)
    {
			if(m_gripper == Moving)
			{
				m_movingLastCursor = p;
			}
			else
			{
				if(m_movableX)
					*m_movableX = p.x;
				if(m_movableY)
					*m_movableY = p.y;
			}
			m_ops->SetSelection(GetSelectionWhileDragging());
    }
		else
		{
			m_gripper = None;
			// are we close to the selection handles?
			// NOTE: the reason i am using distance calculations and not just a boundary check is so that i don't give preference to
			// any corner. if i just checked "am i in the tolerance region of top-left? no, then how about right... etc" then if the selection
			// rect is really small, it would ONLY be possible to drag the top-left handle, even if the cursor is over another handle.
			if(m_ops->HasSelection())
			{
				CRect sel = m_ops->GetSelection();
				static const int ToleranceRadiusScreen = 17;// in screen pixels
				int toleranceRadius = Round(m_ops->GetViewport().ViewToImageSize(PointF(ToleranceRadiusScreen, ToleranceRadiusScreen)).x);// convert to image coords
				CRect outersel = sel;
				outersel.InflateRect(CSize(toleranceRadius, toleranceRadius));
				if(TRUE == outersel.PtInRect(cursor))
				{
					CRect innersel = sel;// the inside
					innersel.InflateRect(CSize(-toleranceRadius, -toleranceRadius));
					if(TRUE == innersel.PtInRect(cursor))
					{
						m_gripper = Moving;
						if(m_hasCursor)
						{
							m_ops->PopCursor(false);
						}
						m_ops->PushCursor(GetCursorForGripperType(m_gripper));
						m_hasCursor = true;
					}
					else
					{
						// do the TOP / RIGHT / BOTTOM / LEFT hittest
						std::vector<std::pair<int, GripperType> > distances;
						bool withinX = (cursor.x >= innersel.left) && (cursor.x <= innersel.right);
						bool withinY = (cursor.y >= innersel.top) && (cursor.y <= innersel.bottom);
						if(withinX)
						{
							distances.push_back(std::pair<int, GripperType>(abs(cursor.y - sel.top), Top));
							distances.push_back(std::pair<int, GripperType>(abs(cursor.y - sel.bottom), Bottom));
						}
						if(withinY)
						{
							distances.push_back(std::pair<int, GripperType>(abs(cursor.x - sel.left), Left));
							distances.push_back(std::pair<int, GripperType>(abs(cursor.x - sel.right), Right));
						}

						if(!AttemptToSetGripperStatus(distances, toleranceRadius, true))
						{
							// determine which corner is closest. first rule out by the X dimension.
							std::vector<std::pair<int, GripperType> > distances;
							distances.push_back(std::pair<int, GripperType>(Calculate2DDistanceCheap(sel.left, sel.top, cursor.x, cursor.y), TopLeft));
							distances.push_back(std::pair<int, GripperType>(Calculate2DDistanceCheap(sel.left, sel.bottom, cursor.x, cursor.y), BottomLeft));
							distances.push_back(std::pair<int, GripperType>(Calculate2DDistanceCheap(sel.right, sel.top, cursor.x, cursor.y), TopRight));
							distances.push_back(std::pair<int, GripperType>(Calculate2DDistanceCheap(sel.right, sel.bottom, cursor.x, cursor.y), BottomRight));

							// if it's inside the rect, don't even check the tolerance distance. this prevents ANY "holes" inside the image where we
							// might not pass any hit tests. Because we're doing an actual distance on the corners, the hit test is an actual rounded
							// corner, which, when compared with the rectangular other hit tests, could result in funky holes.
							if(!AttemptToSetGripperStatus(distances, toleranceRadius * toleranceRadius, !sel.PtInRect(cursor)))
							{
								// this will only happen outside of the rounded corners.
								// no problem; just do nothing.
							}
						}
					}
				}
			}
			if(m_gripper == None)
			{
				if(m_hasCursor)
				{
					m_ops->PopCursor(true);
					m_hasCursor = false;
				}
			}
		}
  }

  void OnLeftButtonDown(PointF p){}
  void OnLeftButtonUp(PointF p){}

	// only left/right/none or top/bottom/none are options in the params.
	void SetupDragHelper(GripperType movableX, GripperType movableY)
	{
		ATLASSERT(m_ops->HasSelection());
		m_selection = RectF(m_ops->GetSelection());
		PointF newCursorPos = m_ops->GetCursorPosition();

		m_movableX = 0;
		m_movableY = 0;

		switch(movableX)
		{
		case Left:
			m_movableX = &m_selection.left;
			newCursorPos.x = m_selection.left;
			break;
		case Right:
			m_movableX = &m_selection.right;
			newCursorPos.x = m_selection.right;
			break;
		}

		switch(movableY)
		{
		case Top:
			m_movableY = &m_selection.top;
			newCursorPos.y = m_selection.top;
			break;
		case Bottom:
			m_movableY = &m_selection.bottom;
			newCursorPos.y = m_selection.bottom;
			break;
		}

		m_ops->SetCursorPosition(newCursorPos);
	}

  void OnStartDragging()
  {
		switch(m_gripper)
		{
		case TopLeft:
			SetupDragHelper(Left, Top);
			break;
		case Top:
			SetupDragHelper(None, Top);
			break;
		case TopRight:
			SetupDragHelper(Right, Top);
			break;
		case Right:
			SetupDragHelper(Right, None);
			break;
		case BottomRight:
			SetupDragHelper(Right, Bottom);
			break;
		case Bottom:
			SetupDragHelper(None, Bottom);
			break;
		case BottomLeft:
			SetupDragHelper(Left, Bottom);
			break;
		case Left:
			SetupDragHelper(Left, None);
			break;
		case None:
			// start a new selection.
			if(m_ops->HasSelection())
			{
				m_ops->ClearSelection();
			}
			m_selection.ul = m_ops->GetCursorPosition();
			m_movableX = &m_selection.left;
			m_movableY = &m_selection.top;
			m_ops->ClampToImage(m_selection.ul);
			m_selection.br = m_selection.ul;
			break;
		case Moving:
			// Moves are done completely separately from other operations.
			// start by saving the selection rect and the place in it where the cursor currently is.
			m_movingOrgCursor = m_ops->GetCursorPosition();
			m_movingLastCursor = m_movingOrgCursor;
			m_selection = RectF(m_ops->GetSelection());
			m_movingOrgSelection = m_selection;
			break;
		}
  }

  void OnStopDragging()
	{
	}

  void OnPaintClient(const AnimBitmap<32>& clean, AnimBitmap<32>& dest, const CRect& rcPaint)
	{
	}

private:
	// returns selection in image coordinates.
  CRect GetSelectionWhileDragging() const
  {
		if(m_gripper == Moving)
		{
			PointF::T cx = m_movingLastCursor.x - m_movingOrgCursor.x;
			PointF::T cy = m_movingLastCursor.y - m_movingOrgCursor.y;
			// shift the original selection rect around, but keep it completely within the image bounds.
			if(cx < 0 && -cx > m_movingOrgSelection.left)// if you are attempting to move it too far left, restrict it.
				cx = -m_movingOrgSelection.left;

			PointF::T roomToTheRight = m_ops->GetImageWidth() - m_movingOrgSelection.right;
			if(cx > 0 && cx > roomToTheRight)
				cx = roomToTheRight;

			if(cy < 0 && -cy > m_movingOrgSelection.top)
				cy = -m_movingOrgSelection.top;

			PointF::T roomToTheBottom = m_ops->GetImageHeight() - m_movingOrgSelection.bottom;
			if(cy > 0 && cy > roomToTheBottom)
				cy = roomToTheBottom;

			RectF ret = m_movingOrgSelection;
			return ret.Offset(PointF(cx, cy)).Round();
		}
		// "normalize" the rect.
		RectF ret;
		ret.left = min(m_selection.left, m_selection.right);
		ret.top = min(m_selection.top, m_selection.bottom);
		ret.right = max(m_selection.left, m_selection.right);
		ret.bottom = max(m_selection.top, m_selection.bottom);
		m_ops->ClampToImage(ret.ul);
		m_ops->ClampToImage(ret.br);
		return ret.Round();
  }

	RectF m_selection;
	RectF::T* m_movableX;
	RectF::T* m_movableY;

	RectF m_movingOrgSelection;// the original selection when a moving operation began
	PointF m_movingOrgCursor;// image position of the cursor when a moving operation began
	PointF m_movingLastCursor;// image position of the most recent cursor

  IToolOperations* m_ops;
	bool m_hasCursor;

	GripperType m_gripper;
};
