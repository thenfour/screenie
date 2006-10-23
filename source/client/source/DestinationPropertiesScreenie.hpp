#ifndef SCREENIE_DESTINATIONPROPERTIESSCREENIE_HPP
#define SCREENIE_DESTINATIONPROPERTIESSCREENIE_HPP

// for ScreenshotDestination
#include "ScreenshotDestination.hpp"

// for DestinationPropertyPage
#include "DestinationProperties.hpp"

class CDestinationPropertiesScreenie :
	public CPropertyPageImpl<CDestinationPropertiesScreenie>,
	public DestinationPropertyPage
{
public:
	enum { IDD = IDD_DESTPROP_SCREENIE };

	CDestinationPropertiesScreenie();
	virtual ~CDestinationPropertiesScreenie();

	void ShowSettings();

	BEGIN_MSG_MAP(CDestinationPropertiesScreenie)
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
	ScreenshotDestination::Screenienet m_settings;
};

#endif