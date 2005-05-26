//
// autogdi.hpp - simple template for passing around GDI objects
// Copyright (c) 2004 Roger Clark
//

#ifndef LIBLOL_AUTOGDI_HPP
#define LIBLOL_AUTOGDI_HPP

template<typename ObjectType>
struct AutoGdiHandle;

typedef AutoGdiHandle<HGDIOBJ> AutoGdiObject;
typedef AutoGdiHandle<HFONT> AutoGdiFont;
typedef AutoGdiHandle<HBITMAP> AutoGdiBitmap;
typedef AutoGdiHandle<HPEN> AutoGdiPen;
typedef AutoGdiHandle<HICON> AutoGdiIcon;
typedef AutoGdiHandle<HCURSOR> AutoGdiCursor;

template<typename ObjectType>
struct AutoGdiHandle
{
	typedef ObjectType Handle;

	//	template<typename OtherObjectType>
	//	AutoGdiObject(AutoGdiObject<OtherObjectType>& object)
	//	{
	//		handle = reinterpret_cast<Handle>object.handle;
	//		object.handle = NULL;
	//	}

	AutoGdiHandle() : handle(NULL) { }

//	AutoGdiHandle(AutoGdiObject& object)
//	{
//		operator=(object);
//	}

	AutoGdiHandle(AutoGdiHandle<Handle>& object)
	{
		operator=(object);
	}

	AutoGdiHandle(const Handle& handleIn)
		: handle(handleIn) { }

	~AutoGdiHandle()
	{
		::DeleteObject(reinterpret_cast<HGDIOBJ>(handle));
	}

//	AutoGdiHandle<Handle> operator=(AutoGdiObject& object)
//	{
//		handle = reinterpret_cast<Handle>(object.handle);
//		object.handle = NULL;
//
//		return (*this);
//	}

	AutoGdiHandle<Handle>& operator=(AutoGdiHandle<Handle>& object)
	{
		handle = object.handle;
		object.handle = NULL;

		return (*this);
	}

	Handle handle;
};

template<typename HandleIn, typename HandleOut = HGDIOBJ>
struct AutoSelectObject
{
//	template<typename HandleIn, typename HandleOut>
	AutoSelectObject(HDC hdc, AutoGdiHandle<HandleIn>& object)
	{
		deviceContext = hdc;
		oldObject = AutoGdiHandle<HandleOut>(::SelectObject(deviceContext,
			reinterpret_cast<HGDIOBJ>(object.handle)));
	}

	~AutoSelectObject()
	{
		::SelectObject(deviceContext, reinterpret_cast<HGDIOBJ>(oldObject.handle));
	}

	HDC deviceContext;
	AutoGdiHandle<HandleOut> oldObject;
};

struct AutoIcon
{
	AutoIcon(HICON icon) { operator=(icon); }
	AutoIcon(AutoIcon& copy) { operator=(copy); }
	~AutoIcon()
	{
		reset();
	}

	void reset(HICON icon = NULL)
	{
		if (iconHandle != NULL)
			::DestroyIcon(iconHandle);

		iconHandle = icon;
	}

	AutoIcon& operator=(HICON icon)
	{
		reset(icon);
		return (*this);
	}

	AutoIcon& operator=(AutoIcon& rightHand)
	{
		reset(rightHand.iconHandle);
		rightHand.reset(NULL);

		return (*this);
	}

	HICON iconHandle;
};

#endif