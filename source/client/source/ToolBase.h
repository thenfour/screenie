/*
  Tools for use in the ImageEdit window
*/

#pragma once

typedef void (*ToolTimerProc)(void*);

// individual tools receive a pointer to this so they can tell the image window to do things
// like panning or whatever else the main image edit window is capable of.
class IToolOperations
{
public:
  // gets a panningspec based on the cursor pos & window crap.  this spec can be used to pan the
  // window when the mouse cursor is outside the view.  since many tools will probably do panning
  // like this, i'm centralizing it in the tool ops class.
  virtual void Pan(int x, int y) = 0;// x and y in virtual coords.
  virtual UINT_PTR CreateTimer(UINT elapse, ToolTimerProc, void* userData) = 0;// returns a cookie that identifies the tmier. guaranteed not to be 0, so you can use 0 to tell if an id is valid.
  virtual void DeleteTimer(UINT_PTR cookie) = 0;
  virtual PointF GetCursorPosition() = 0;
  virtual int GetImageHeight() const = 0;
  virtual int GetImageWidth() const = 0;
	virtual const Viewport& GetViewport() const = 0;
  virtual void ClampToImage(PointF& p) = 0;// clamps the point to be within the image's bounds.
  //virtual void Refresh(const RECT& imageCoords, bool now) = 0;
  //virtual void Refresh(bool now) = 0;
	virtual void ClearSelection() = 0;
	virtual void SetSelection(const RECT& rc) = 0;
	virtual bool HasSelection() const = 0;
	virtual CRect GetSelection() const = 0;
	virtual void SetCursorPosition(const PointF& img) = 0;
	virtual void PushCursor(PCWSTR newcursor) = 0;// technically, the idea of a stack of cursors might not work, because cursor changes don't necessarily happen in a stack-like behavior.... but for now it works fine.
	virtual void PopCursor(bool set) = 0;
};


class ToolBase
{
public:
  virtual void OnInitTool() = 0;
  virtual void OnSelectTool() = 0;
  virtual void OnDeselectTool() = 0;
  virtual void OnCursorMove(PointF p, bool dragging) = 0;
  virtual void OnLeftButtonDown(PointF p) = 0;
  virtual void OnLeftButtonUp(PointF p) = 0;
	// clean is guaranteed "clean" with no tools drawn on it.
	// dest is, at dirtiest, the last rendering, so it may have tool drawings on it.
  virtual void OnPaintClient(const AnimBitmap<32>& clean, AnimBitmap<32>& dest, const CRect& rcPaint) = 0;
  virtual void OnStopDragging() = 0;
  virtual void OnStartDragging() = 0;
};

