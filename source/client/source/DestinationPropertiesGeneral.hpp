// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark

#ifndef SCREENIE_DESTINATIONPROPERTIESGENERAL_HPP
#define SCREENIE_DESTINATIONPROPERTIESGENERAL_HPP

// for ScreenshotDestination
#include "ScreenshotDestination.hpp"

// for DestinationPropertyPage
#include "DestinationProperties.hpp"

class CDestinationPropertiesGeneral :
	public CPropertyPageImpl<CDestinationPropertiesGeneral>,
	public DestinationPropertyPage
{
public:
	enum { IDD = IDD_DESTPROP_GENERAL };

	CDestinationPropertiesGeneral();
	virtual ~CDestinationPropertiesGeneral();

	void SetSettings(const ScreenshotDestination::General& settings);
	//ScreenshotDestination::General GetSettings();

	ScreenshotDestination::Type GetType();

	tstd::tstring GetImageFormat();
	void SetImageFormat(const tstd::tstring& imageFormat);

	void PopulateFormatList();

	void ShowSettings();

	BEGIN_MSG_MAP(CDestinationPropertiesGeneral)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		COMMAND_HANDLER(IDC_FILE_FOLDER_BROWSE, BN_CLICKED, OnFolderBrowse)
		COMMAND_HANDLER(IDC_GENERAL_TYPE, CBN_SELCHANGE, OnTypeChanged)
		COMMAND_HANDLER(IDC_GENERAL_FORMAT, CBN_SELCHANGE, OnImageFormatChanged)
		COMMAND_HANDLER(IDC_FILE_PATH, EN_CHANGE, OnFilenameFormatChanged)
    COMMAND_HANDLER(IDC_FILENAME_LOCAL, BN_CLICKED, OnFilenameFormatChanged)
    COMMAND_HANDLER(IDC_FILENAME_UTC, BN_CLICKED, OnFilenameFormatChanged)
		CHAIN_MSG_MAP(CPropertyPageImpl<CDestinationPropertiesGeneral>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnHScroll(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);

	LRESULT OnFolderBrowse(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnTypeChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnImageFormatChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnFilenameFormatChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);

	//
	// DestinationPropertyPage implementation
	//
	BOOL OnSetActive() { ShowSettings(); return TRUE; }
	BOOL OnKillActive() { GetSettings(); return TRUE; }// being hidden
	BOOL OnApply() { GetSettings(); return TRUE; }

	HPROPSHEETPAGE CreatePropertyPage();

	void SetSettings(ScreenshotDestination* destination);
	void GetSettings();

	void SetDestinationType(const ScreenshotDestination::Type type);
	void SetParentSheet(DestinationPropertySheet* parentSheet);
private:
	DestinationPropertySheet* m_parentSheet;
	ScreenshotDestination* m_settings;

	void UpdateQualityLabel();
};

#endif
