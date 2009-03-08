/*
  Tools for use in the ImageEdit window

	Tools may need to:
	1) modify the original image
	2) display their own custom controls like sizing handles and stuff.
	
	right now the selection tool (cropping tool) is rendered specially in imageEditRenderer because
	it's slow and needs all the optimization it can get, and it doesn't modify the original in a typical way.
	But the highlight tool needs to do both of these things (potentially).

	For now, the highlighter tool will just draw on the original image.
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

	// eventually i should probably make this cleaner. right now tools can just do whatever they want with the original image
	// and they should call SetDirty() when it's updated.
	virtual AnimBitmap<32>* GetDocument() = 0;
	virtual void SetDocumentDirty() = 0;
	virtual void SetTemporarySurfaceDirty() = 0;
	virtual void Redraw() = 0;
};


class ToolBase
{
public:
  virtual void OnSelectTool() = 0;
  virtual void OnDeselectTool() = 0;
  virtual void OnCursorMove(PointF p, bool dragging) = 0;
  virtual void OnLeftButtonDown(PointF p) = 0;
  virtual void OnLeftButtonUp(PointF p) = 0;
  virtual void OnPaintClient(AnimBitmap<32>& document) = 0;
  virtual void OnStopDragging() = 0;
  virtual void OnStartDragging(PointF p) = 0;
	virtual int GetResourceID() = 0;// IDC_CROPPINGTOOL, IDC_HIGHLIGHTTOOL
};
