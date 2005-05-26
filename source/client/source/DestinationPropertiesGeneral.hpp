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
	ScreenshotDestination::General GetSettings();

	ScreenshotDestination::Type GetType();

	tstd::tstring GetImageFormat();
	void SetImageFormat(const tstd::tstring& imageFormat);

	void PopulateFormatList();

	void ShowSettings();

	BEGIN_MSG_MAP(CDestinationPropertiesGeneral)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_FILE_FOLDER_BROWSE, BN_CLICKED, OnFolderBrowse)
		COMMAND_HANDLER(IDC_GENERAL_TYPE, CBN_SELCHANGE, OnTypeChanged)
		COMMAND_HANDLER(IDC_GENERAL_FORMAT, CBN_SELCHANGE, OnImageFormatChanged)
		COMMAND_HANDLER(IDC_GENERAL_FILENAME, EN_CHANGE, OnFilenameFormatChanged)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);

	LRESULT OnFolderBrowse(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnTypeChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnImageFormatChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnFilenameFormatChanged(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);

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
	ScreenshotDestination::General m_settings;
};

#endif