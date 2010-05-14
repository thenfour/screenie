//
// DestinationProperties.hpp - interfaces for the destination properties dialog
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//

#ifndef SCREENIE_DESTINATIONPROPERTIES_HPP
#define SCREENIE_DESTINATIONPROPERTIES_HPP

#include "ScreenshotDestination.hpp"

// i was always confused by the terminology regarding property sheets
// and property pages. property sheets 'contain' property pages, but
// a "sheet" and a "page," in english, are pretty much the same thing.
//
// they really should've called property sheets "property books," or
// even perhaps the less novel, but more explicit, "property page
// containers."

// this interface is implemented by the property sheet dialog class, and is
// passed along to implementations of DestinationPropertyPage.

struct DestinationPropertySheet
{
	virtual ~DestinationPropertySheet() { }

	virtual ScreenshotDestination::Type GetCurrentType() = 0;

	virtual void Update(const ScreenshotDestination::Type type) = 0;
};

// this interface is implemented by the individual property page classes, and
// is used to notify the parent property sheet whenever something important
// changes (like the type of a destination) in case anything needs to be done.

struct DestinationPropertyPage
{
	virtual ~DestinationPropertyPage() { }

	virtual HPROPSHEETPAGE CreatePropertyPage() = 0;

	// each property page should make a local copy of the relevant settings.
	virtual void SetSettings(ScreenshotDestination* destination) = 0;

	// only the settings relevant to the property page should be modified.
	virtual void GetSettings() = 0;

	virtual void SetDestinationType(const ScreenshotDestination::Type type) = 0;
	virtual void SetParentSheet(DestinationPropertySheet* parentSheet) = 0;
};

#endif