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
	m_propertyPages.push_back(util::shared_ptr<DestinationPropertyPage>(new CDestinationPropertiesImageShack()));

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
