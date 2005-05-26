#include "stdafx.hpp"
#include "resource.h"

#include "utility.hpp"

#include "DestinationPropertiesEmail.hpp"

CDestinationPropertiesEmail::CDestinationPropertiesEmail()
{
}

CDestinationPropertiesEmail::~CDestinationPropertiesEmail()
{
}

void CDestinationPropertiesEmail::ShowSettings()
{
	EnableChildWindows(m_hWnd,
		(m_parentSheet->GetCurrentType() == ScreenshotDestination::TYPE_EMAIL));
}

LRESULT CDestinationPropertiesEmail::OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	ShowSettings();

	return 0;
}

//
// DestinationPropertyPage implementation
//

HPROPSHEETPAGE CDestinationPropertiesEmail::CreatePropertyPage()
{
	return CPropertyPageImpl<CDestinationPropertiesEmail>::Create();
}

void CDestinationPropertiesEmail::SetSettings(const ScreenshotDestination& destination)
{
	m_settings = destination.email;

	if (IsWindow())
		ShowSettings();
}

void CDestinationPropertiesEmail::GetSettings(ScreenshotDestination& destination)
{
}

void CDestinationPropertiesEmail::SetDestinationType(const ScreenshotDestination::Type type)
{
	if (IsWindow())
		EnableChildWindows(m_hWnd, (type == ScreenshotDestination::TYPE_EMAIL));
}

void CDestinationPropertiesEmail::SetParentSheet(DestinationPropertySheet* parentSheet)
{
	m_parentSheet = parentSheet;
}

