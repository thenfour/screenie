// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark

#ifndef SCREENIE_DESTINATIONPROPERTIESFTP_HPP
#define SCREENIE_DESTINATIONPROPERTIESFTP_HPP

// for ScreenshotDestination
#include "ScreenshotDestination.hpp"

// for DestinationPropertyPage
#include "DestinationProperties.hpp"

class CDestinationPropertiesFTP :
	public CPropertyPageImpl<CDestinationPropertiesFTP>,
	public DestinationPropertyPage
{
public:
	enum { IDD = IDD_DESTPROP_FTP };

	CDestinationPropertiesFTP();
	virtual ~CDestinationPropertiesFTP();

	void ShowSettings();

	BEGIN_MSG_MAP(CDestinationPropertiesFTP)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);

	//
	// DestinationPropertyPage implementation
	//

	HPROPSHEETPAGE CreatePropertyPage();

	void SetSettings(const ScreenshotDestination& destination);
	void GetSettings(ScreenshotDestination& destination);

	void SetDestinationType(const ScreenshotDestination::Type type);
	void SetParentSheet(DestinationPropertySheet* parentSheet);
private:
	DestinationPropertySheet* m_parentSheet;
	ScreenshotDestination::Ftp m_settings;
};

#endif