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
		COMMAND_HANDLER(IDC_IMAGESHACK_COPYURL, BN_CLICKED, OnCopyURLClick)
		COMMAND_HANDLER(IDC_FTP_HTTPURL, EN_CHANGE, OnURLChanged)
		COMMAND_HANDLER(IDC_FTP_REMOTEPATH, EN_CHANGE, OnPathChanged)
		CHAIN_MSG_MAP(CPropertyPageImpl<CDestinationPropertiesFTP>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnCopyURLClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnURLChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnPathChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);

	//
	// DestinationPropertyPage implementation
	//
	BOOL OnSetActive() { ShowSettings(); return TRUE; }
	BOOL OnKillActive() { GetSettings(); return TRUE; }// being hidden
	BOOL OnApply() { GetSettings(); return TRUE; }

	void UpdatePreview(UINT src, UINT dest, UINT other);

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

