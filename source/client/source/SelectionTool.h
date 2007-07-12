

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

	static inline PointF::T Calculate2DDistanceCheap(PointF::T x1, PointF::T y1, PointF::T x2, PointF::T y2)
	{
		PointF::T dx = x1 - x2;
		PointF::T dy = y1 - y2;
		return (dx * dx) + (dy * dy);
	}

	// helper function. takes [distance between the cursor and an on-screen element] and uses the lowest one to set
	// the cursor icon and m_gripper state. returns false if none are within the tolerance.
	bool AttemptToSetGripperStatus(const std::vector<std::pair<PointF::T, GripperType> >& items, PointF::T tolerance, bool checkTolerance)
	{
		const std::pair<PointF::T, GripperType>* lowest = 0;
		// find the lowest
		for(std::vector<std::pair<PointF::T, GripperType> >::const_iterator it = items.begin(); it != items.end(); ++ it)
		{
			if(lowest == 0 || it->first < lowest->first)
				lowest = &(*it);
		}
		if(lowest == 0)
			return false;
		if(checkTolerance && lowest->first > tolerance)
			return false;

		m_gripper = lowest->second;
		SetCursorIcon(GetCursorForGripperType(m_gripper));
		return true;
	}

  void OnCursorMove(PointF cursor, bool dragging)
  {
    if(dragging)
    {
			if(m_gripper == Moving)
			{
				m_movingLastCursor = cursor;
				m_bMoved = true;
			}
			else
			{
				if(m_movableX)
					*m_movableX = Round(cursor.x);
				if(m_movableY)
					*m_movableY = Round(cursor.y);
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
				RectF sel = RectF(m_ops->GetSelection());
				static const PointF::T ToleranceRadiusScreen = 14.0;// in screen pixels
				PointF::T toleranceRadius = m_ops->GetViewport().ViewToImageSize(PointF(ToleranceRadiusScreen, ToleranceRadiusScreen)).x;// convert to image coords
				RectF outersel = sel;
				outersel.Inflate(toleranceRadius);
				if(outersel.HitTest(cursor))
				{
					RectF innersel = sel;// the inside
					innersel.Inflate(-toleranceRadius);
					if(innersel.HitTest(cursor))
					{
						m_gripper = Moving;
						SetCursorIcon(GetCursorForGripperType(m_gripper));
					}
					else
					{
						// do the TOP / RIGHT / BOTTOM / LEFT hittest
						std::vector<std::pair<PointF::T, GripperType> > distances;
						bool withinX = (cursor.x >= innersel.left) && (cursor.x <= innersel.right);
						bool withinY = (cursor.y >= innersel.top) && (cursor.y <= innersel.bottom);
						if(withinX)
						{
							distances.push_back(std::pair<PointF::T, GripperType>(abs(cursor.y - sel.top), Top));
							distances.push_back(std::pair<PointF::T, GripperType>(abs(cursor.y - sel.bottom), Bottom));
						}
						if(withinY)
						{
							distances.push_back(std::pair<PointF::T, GripperType>(abs(cursor.x - sel.left), Left));
							distances.push_back(std::pair<PointF::T, GripperType>(abs(cursor.x - sel.right), Right));
						}

						if(!AttemptToSetGripperStatus(distances, toleranceRadius, true))
						{
							// determine which corner is closest. first rule out by the X dimension.
							std::vector<std::pair<PointF::T, GripperType> > distances;
							distances.push_back(std::pair<PointF::T, GripperType>(Calculate2DDistanceCheap(sel.left, sel.top, cursor.x, cursor.y), TopLeft));
							distances.push_back(std::pair<PointF::T, GripperType>(Calculate2DDistanceCheap(sel.left, sel.bottom, cursor.x, cursor.y), BottomLeft));
							distances.push_back(std::pair<PointF::T, GripperType>(Calculate2DDistanceCheap(sel.right, sel.top, cursor.x, cursor.y), TopRight));
							distances.push_back(std::pair<PointF::T, GripperType>(Calculate2DDistanceCheap(sel.right, sel.bottom, cursor.x, cursor.y), BottomRight));

							// if it's inside the rect, don't even check the tolerance distance. this prevents ANY "holes" inside the image where we
							// might not pass any hit tests. Because we're doing an actual distance on the corners, the hit test is an actual rounded
							// corner, which, when compared with the rectangular other hit tests, could result in funky holes.
							if(!AttemptToSetGripperStatus(distances, toleranceRadius * toleranceRadius, !sel.HitTest(cursor)))
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
				ReleaseCursor();
			}
		}
  }

  void OnLeftButtonDown(PointF p){}
  void OnLeftButtonUp(PointF p)
	{
		if(m_gripper == Moving && !m_bMoved)
		{
			m_gripper = None;
			m_ops->ClearSelection();
			ReleaseCursor();
		}
	}

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
			m_bMoved = false;
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
  CRect GetSelectionWhileDragging()
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
		// correct the cursor if necessary, because "flipping" the selection rect might cause a NWSE cursor to need to change to a 
		switch(m_gripper)
		{
		case None:
		case TopLeft:
		case TopRight:
		case BottomLeft:
		case BottomRight:
			PointF movable(*m_movableX, *m_movableY);
			m_ops->ClampToImage(movable);
			if(movable.x == ret.right && movable.y == ret.top)
			{
				m_gripper = TopRight;
				SetCursorIcon(IDC_SIZENESW);
			}
			if(movable.x == ret.left && movable.y == ret.top)
			{
				m_gripper = TopLeft;
				SetCursorIcon(IDC_SIZENWSE);
			}
			if(movable.x == ret.right && movable.y == ret.bottom)
			{
				m_gripper = BottomRight;
				SetCursorIcon(IDC_SIZENWSE);
			}
			if(movable.x == ret.left && movable.y == ret.bottom)
			{
				m_gripper = BottomLeft;
				SetCursorIcon(IDC_SIZENESW);
			}
			break;
		}
		return ret.Round();
  }

	RectF m_selection;
	RectF::T* m_movableX;
	RectF::T* m_movableY;

	RectF m_movingOrgSelection;// the original selection when a moving operation began
	PointF m_movingOrgCursor;// image position of the cursor when a moving operation began
	PointF m_movingLastCursor;// image position of the most recent cursor
	bool m_bMoved;// becomes true when the user moves the selection rect. i need this so when you just single click in the middle of the selection, it clears the selection rect.

  IToolOperations* m_ops;
	bool m_hasCursor;

	GripperType m_gripper;

	void SetCursorIcon(PCWSTR x)
	{
		if(m_hasCursor)
		{
			m_ops->PopCursor(false);
		}
		m_ops->PushCursor(x);
		m_hasCursor = true;
	}
	void ReleaseCursor()
	{
		if(m_hasCursor)
		{
			m_ops->PopCursor(true);
		}
	}
};
