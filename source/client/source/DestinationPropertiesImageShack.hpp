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
	ScreenshotDestination::ImageShack m_settings;
};

#endif