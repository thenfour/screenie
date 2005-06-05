//
//
//
//

#ifndef _SCREENIE_DESTPROPERTIES_H_
#define _SCREENIE_DESTPROPERTIES_H_

// for std::auto_ptr
#include <memory>

#include "DestinationProperties.hpp"
#include "ScreenshotDestination.hpp"
#include "ScreenshotOptions.hpp"
#include "utility.hpp"

#include "DestinationPropertiesGeneral.hpp"
#include "DestinationPropertiesImage.hpp"
#include "DestinationPropertiesFtp.hpp"
#include "DestinationPropertiesEmail.hpp"

class CDestinationProperties :
	public CPropertySheetImpl<CDestinationProperties>,
	public DestinationPropertySheet
{
public:
	CDestinationProperties(ScreenshotDestination& destination, ScreenshotOptions& options,
		PCTSTR title = TEXT("Destination Properties"));
	~CDestinationProperties();

	BOOL InitializePages();
	BOOL FinalizePages();

	void OnSheetInitialized();
	ScreenshotDestination GetDestination() const;

	BEGIN_MSG_MAP(CDestinationProperties)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
	END_MSG_MAP()

	LRESULT OnCommand(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);

	//
	// DestinationPropertySheet implementation
	//

	ScreenshotDestination::Type GetCurrentType();
	void Update(const ScreenshotDestination::Type type);
private:
	typedef std::vector<util::shared_ptr<DestinationPropertyPage> > PropertyPageContainer;

	PropertyPageContainer m_propertyPages;
	ScreenshotDestination::Type m_currentType;
	ScreenshotDestination m_screenshotDestination;

  ScreenshotOptions& m_options;
};

#endif