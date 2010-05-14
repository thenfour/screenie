// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//
//
//
//
//

#ifndef SCREENIE_DESTINATIONPROPERTIESIMAGE_HPP
#define SCREENIE_DESTINATIONPROPERTIESIMAGE_HPP

// for ScreenshotDestination
#include "ScreenshotDestination.hpp"

// for DestinationPropertyPage
#include "DestinationProperties.hpp"

class CDestinationPropertiesImage :
	public CPropertyPageImpl<CDestinationPropertiesImage>,
	public DestinationPropertyPage
{
public:
	enum { IDD = IDD_DESTPROP_IMAGE };

	CDestinationPropertiesImage();
	virtual ~CDestinationPropertiesImage();

	void ShowSettings();

	void EnableSizingControls(BOOL enable);

	BEGIN_MSG_MAP(CDestinationPropertiesImage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnCheckboxChecked)
		CHAIN_MSG_MAP(CPropertyPageImpl<CDestinationPropertiesImage>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnCheckboxChecked(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);

	//
	// DestinationPropertyPage implementation
	//
	BOOL OnSetActive() { ShowSettings(); return TRUE; }
	BOOL OnKillActive() { GetSettings(); return TRUE; }// being hidden
	BOOL OnApply() { GetSettings(); return TRUE; }

	HPROPSHEETPAGE CreatePropertyPage();

	void SetSettings(ScreenshotDestination* destination);
	void GetSettings();

	void SetDestinationType(const ScreenshotDestination::Type type);
	void SetParentSheet(DestinationPropertySheet* parentSheet);
private:
	DestinationPropertySheet* m_parentSheet;
	ScreenshotDestination* m_settings;
};

#endif