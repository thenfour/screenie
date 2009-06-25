// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark


#ifndef VIEWPORT_INCLUDED
#define VIEWPORT_INCLUDED

#include "stdafx.hpp"

class Viewport
{
public:

  Viewport() :
    zoomFactor(1.0f)
  {
  }

	void SetZoomFactor(Gdiplus::REAL z)
  {
    zoomFactor = z;
		ResetMatrices();
  }
  Gdiplus::REAL GetZoomFactor() const
  {
    return zoomFactor;
  }

  void SetViewOrigin(Gdiplus::PointF o)
  {
    viewOrigin = o;
		ResetMatrices();
  }
  const Gdiplus::PointF& GetViewOrigin() const
  {
    return viewOrigin;
  }

  void SetDocumentOrigin(Gdiplus::PointF o)
  {
    documentOrigin = o;
		ResetMatrices();
  }
  const Gdiplus::PointF& GetDocumentOrigin() const
  {
    return documentOrigin;
  }

	// ********************************************** CONVERSIONS (the reason this class exists)
  inline Gdiplus::PointF ViewToDocument(const Gdiplus::PointF& p) const
  {
		Gdiplus::PointF ret = p;
		viewToDocument.TransformPoints(&ret);
		return ret;
  }
	inline Gdiplus::PointF DocumentToView(const Gdiplus::PointF& p) const
  {
		Gdiplus::PointF ret = p;
		documentToView.TransformPoints(&ret);
		return ret;
  }

	// these do not take into consider the origins. they just consider zoom.
  Gdiplus::PointF ViewToDocumentSize(const Gdiplus::PointF& p) const
  {
		Gdiplus::PointF ret = p;
		viewToDocument.TransformVectors(&ret);
		return ret;
  }
  Gdiplus::PointF DocumentToViewSize(const Gdiplus::PointF& p) const
  {
		Gdiplus::PointF ret = p;
		documentToView.TransformVectors(&ret);
		return ret;
  }

	Gdiplus::RectF ViewToDocument(const Gdiplus::RectF& rc) const
	{
		// TODO: there's got to be a more efficient way to transform a rectangle.
		Gdiplus::PointF p[2];
		p[0].X = rc.GetLeft();
		p[0].Y = rc.GetTop();
		p[1].X = rc.GetRight();
		p[1].Y = rc.GetBottom();
		viewToDocument.TransformPoints(p, 2);
		Gdiplus::RectF ret(p[0].X, p[0].Y, p[1].X - p[0].X, p[1].Y - p[0].Y);
		return ret;
	}

	Gdiplus::RectF DocumentToView(const Gdiplus::RectF& rc) const
	{
		// TODO: there's got to be a more efficient way to transform a rectangle.
		Gdiplus::PointF p[2];
		p[0].X = rc.GetLeft();
		p[0].Y = rc.GetTop();
		p[1].X = rc.GetRight();
		p[1].Y = rc.GetBottom();
		documentToView.TransformPoints(p, 2);
		Gdiplus::RectF ret(p[0].X, p[0].Y, p[1].X - p[0].X, p[1].Y - p[0].Y);
		return ret;
	}

protected:
  // basic members
  Gdiplus::PointF viewOrigin;
  Gdiplus::PointF documentOrigin;
	Gdiplus::REAL zoomFactor;

	void ResetMatrices()
	{
		viewToDocument.Reset();
		viewToDocument.Translate(documentOrigin.X, documentOrigin.Y);
		viewToDocument.Scale(1.0f / zoomFactor, 1.0f / zoomFactor);
		viewToDocument.Translate(-viewOrigin.X, -viewOrigin.Y);

		documentToView.Reset();
		documentToView.Translate(viewOrigin.X, viewOrigin.Y);
		documentToView.Scale(zoomFactor, zoomFactor);
		documentToView.Translate(-documentOrigin.X, -documentOrigin.Y);
	}

	Gdiplus::Matrix viewToDocument;
	Gdiplus::Matrix documentToView;
};

#endif

