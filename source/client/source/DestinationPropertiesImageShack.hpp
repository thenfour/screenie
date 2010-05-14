// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark

#ifndef SCREENIE_DESTINATIONPROPERTIESIMAGESHACK_HPP
#define SCREENIE_DESTINATIONPROPERTIESIMAGESHACK_HPP

// for ScreenshotDestination
#include "ScreenshotDestination.hpp"

// for DestinationPropertyPage
#include "DestinationProperties.hpp"

class CDestinationPropertiesImageShack :
	public CPropertyPageImpl<CDestinationPropertiesImageShack>,
	public DestinationPropertyPage
{
public:
	enum { IDD = IDD_DESTPROP_IMAGESHACK };

	CDestinationPropertiesImageShack();
	virtual ~CDestinationPropertiesImageShack();

	void ShowSettings();

	BEGIN_MSG_MAP(CDestinationPropertiesImageShack)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_IMAGESHACK_COPYURL, BN_CLICKED, OnCopyURLClick)
		CHAIN_MSG_MAP(CPropertyPageImpl<CDestinationPropertiesImageShack>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnCopyURLClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

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