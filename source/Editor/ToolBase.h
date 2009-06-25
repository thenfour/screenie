// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark

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
	virtual Gdiplus::PointF GetCursorPosition() = 0;
  virtual int GetImageHeight() const = 0;
  virtual int GetImageWidth() const = 0;
  virtual void ClampToImage(Gdiplus::PointF& p) = 0;// clamps the point to be within the image's bounds.
	virtual void SetCursorPosition(const Gdiplus::PointF& img) = 0;
	virtual void PushCursor(PCWSTR newcursor) = 0;// technically, the idea of a stack of cursors might not work, because cursor changes don't necessarily happen in a stack-like behavior.... but for now it works fine.
	virtual void PopCursor(bool set) = 0;

	// eventually i should probably make this cleaner. right now tools can just do whatever they want with the original image
	// and they should call SetDirty() when it's updated.
	virtual Gdiplus::Bitmap* GetDocument() = 0;
	virtual void SetDocumentModified(const Gdiplus::Rect& rc) = 0;// see ImageEditRenderer.h for info on the vocabulary.
	virtual void SetVisibleDocumentModified(const Gdiplus::Rect& rc) = 0;// see ImageEditRenderer.h for info on the vocabulary.
	virtual void SetOffscreenModified(const Gdiplus::Rect& rc) = 0;// see ImageEditRenderer.h for info on the vocabulary.
};


class ToolBase
{
public:
  virtual void OnSelectTool() = 0;
  virtual void OnDeselectTool() = 0;
  virtual void OnCursorMove(const Gdiplus::PointF& p, bool dragging) = 0;
  virtual void OnLeftButtonDown(const Gdiplus::PointF& p) = 0;
  virtual void OnLeftButtonUp(const Gdiplus::PointF& p) = 0;
  virtual void OnVisibleDocumentPaint(Gdiplus::Bitmap* document, const Gdiplus::Rect& rcUpdate) = 0;// // see ImageEditRenderer.h for info on the vocabulary.  coords are in DOCUMENT coords even though the bitmap is cropped to the visible area
  virtual void OnOffscreenPaint(Gdiplus::Bitmap* document, const Gdiplus::Rect& rcUpdate) = 0;// see ImageEditRenderer.h for info on the vocabulary.
  virtual void OnStopDragging() = 0;
	virtual void OnStartDragging(const Gdiplus::PointF& p) = 0;
	virtual int GetResourceID() = 0;// IDC_CROPPINGTOOL, IDC_HIGHLIGHTTOOL
};
