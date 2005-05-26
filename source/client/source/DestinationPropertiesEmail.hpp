//
//
//
//
//

#ifndef SCREENIE_DESTINATIONPROPERTIESEMAIL_HPP
#define SCREENIE_DESTINATIONPROPERTIESEMAIL_HPP

// for ScreenshotDestination
#include "ScreenshotDestination.hpp"

// for DestinationPropertyPage
#include "DestinationProperties.hpp"

class CDestinationPropertiesEmail :
	public CPropertyPageImpl<CDestinationPropertiesEmail>,
	public DestinationPropertyPage
{
public:
	enum { IDD = IDD_DESTPROP_EMAIL };

	CDestinationPropertiesEmail();
	virtual ~CDestinationPropertiesEmail();

	void ShowSettings();

	BEGIN_MSG_MAP(CDestinationPropertiesEmail)
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
	ScreenshotDestination::Email m_settings;
};

#endif