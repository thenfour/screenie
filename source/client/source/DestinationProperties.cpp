//
// DestinationProperties.cpp - destination property sheet
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#include "stdafx.hpp"
#include "resource.h"

#include "DestinationProperties.hpp"
#include "DestProperties.hpp"

CDestinationProperties::CDestinationProperties(ScreenshotDestination& destination, ScreenshotOptions& options, PCTSTR title) :
  CPropertySheetImpl<CDestinationProperties>(title, 0, NULL),
  m_options(options)
{
	m_screenshotDestination = destination;

	InitializePages();
}

CDestinationProperties::~CDestinationProperties()
{
	FinalizePages();
}

BOOL CDestinationProperties::InitializePages()
{
	m_propertyPages.push_back(util::shared_ptr<DestinationPropertyPage>(new CDestinationPropertiesGeneral()));
	m_propertyPages.push_back(util::shared_ptr<DestinationPropertyPage>(new CDestinationPropertiesImage()));
	m_propertyPages.push_back(util::shared_ptr<DestinationPropertyPage>(new CDestinationPropertiesFTP()));
//	m_propertyPages.push_back(util::shared_ptr<DestinationPropertyPage>(new CDestinationPropertiesEmail()));
//	m_propertyPages.push_back(util::shared_ptr<DestinationPropertyPage>(new CDestinationPropertiesScreenie()));

	for (size_t i = 0; i < m_propertyPages.size(); ++i)
	{
		AddPage(m_propertyPages[i]->CreatePropertyPage());
		m_propertyPages[i]->SetParentSheet(dynamic_cast<DestinationPropertySheet*>(this));
		m_propertyPages[i]->SetSettings(m_screenshotDestination);
	}

	return TRUE;
}

BOOL CDestinationProperties::FinalizePages()
{
	m_propertyPages.clear();

	return TRUE;
}

void CDestinationProperties::OnSheetInitialized()
{
	// 
}

ScreenshotDestination CDestinationProperties::GetDestination() const
{
	return m_screenshotDestination;
}

LRESULT CDestinationProperties::OnCommand(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	UINT controlID = LOWORD(wParam);
	UINT notifyCode = HIWORD(wParam);

	if ((notifyCode == BN_CLICKED) && (controlID == IDOK))
	{
		for (size_t i = 0; i < m_propertyPages.size(); ++i)
		{
			m_propertyPages[i]->GetSettings(m_screenshotDestination);
		}

    ///*
    //  Make sure this will work practically with the other destinations that are registered.
    //  Now we should make these pretty soft warnings because it's possible to set up
    //  conflicting destinations and just always enable/disable them to make sense.
    //  Here are the rules we should check for.  

    //  1) Only 1 clipboard destination allowed
    //  2) if there are other clipboard destinations and "copy url to clipboard" is checked, warn.
    //*/

    //size_t total = m_options.GetNumDestinations();
    //int otherClipboardDestinations = 0;
    //int otherCopyURLToClipboardDestinations = 0;

    //// this loop gets some statistics about the other destinations.
    //for(size_t i = 0; i < total; i ++)
    //{
    //  ScreenshotDestination& rhs = m_options.GetDestination(i);
    //  if(rhs.general.id != m_screenshotDestination.general.id)
    //  {
    //    if(rhs.general.type == ScreenshotDestination::TYPE_CLIPBOARD)
    //    {
    //      otherClipboardDestinations ++;
    //    }
    //  }
    //}

    //bCancel = false;

    //// now do the checks.
    //if((m_screenshotDestination.general.type == ScreenshotDestination::TYPE_CLIPBOARD) && otherClipboardDestinations)
    //{
    //  bCancel = (IDNO == MessageBox(
    //    _T("You have configured more than one 'clipboard' screenshot destination.  Enabling both "),
    //    _T("Warning"), MB_YESNO | MB_ICONASTERISK));
    //}

    //if(bCancel)
    //{
    //  // fake WPARAM.
    //  wParam = MAKEWPARAM(IDCANCEL, notifyCode);
    //}
	}

	return CPropertySheetImpl<CDestinationProperties>::OnCommand(msg, wParam, lParam, handled);
}

ScreenshotDestination::Type CDestinationProperties::GetCurrentType()
{
	return m_currentType;
}

void CDestinationProperties::Update(const ScreenshotDestination::Type type)
{
	m_currentType = type;

	// update the pages for the new type
	for (size_t i = 0; i < m_propertyPages.size(); ++i)
		m_propertyPages[i]->SetDestinationType(type);
}
