//
// ScreenshotOptions.cpp - saving and loading of screenshot options
// Copyright (c) 2003 Carl Corcoran
// Copyright (c) 2005 Roger Clark
//

#include "stdafx.hpp"
#include "resource.h"

#include "ScreenshotDestination.hpp"
#include "ScreenshotOptions.hpp"
#include "libcc/registry.h"

const wchar_t ScreenshotDestination::Ftp::xorKey[] = { 62171, 50087, 61907, 5753, 59244, 31190, 14869, 15056, 6894, 40105, 16528, 27281, 10301 };
const int ScreenshotDestination::Ftp::xorKeyLength = 13;


const TCHAR* KEY_INCLUDECURSOR = TEXT("Include Cursor");
const TCHAR* KEY_SHOWCROPPINGWINDOW = TEXT("Show Cropping Window");
const TCHAR* KEY_CONFIRMOPTIONS = TEXT("Confirm Options");
const TCHAR* KEY_SHOWSTATUS = TEXT("Show Status Dialog");
const TCHAR* KEY_SHOWSPLASH = TEXT("Show Splash");
const TCHAR* KEY_CROPPINGZOOMFACTOR = TEXT("Cropping Zoom Factor");

const TCHAR* KEY_CONFIGWINDOWPLACEMENT = TEXT("ConfigWindowPlacement");
const TCHAR* KEY_STATUSWINDOWPLACEMENT = TEXT("StatusWindowPlacement");
const TCHAR* KEY_CROPPINGWINDOWPLACEMENT = TEXT("CroppingWindowPlacement");

const TCHAR* KEY_DESTINATIONS = TEXT("Destinations");

const TCHAR* KEY_DEST_ENABLED = TEXT("Enabled");
const TCHAR* KEY_DEST_GENERAL_NAME = TEXT("Name");
const TCHAR* KEY_DEST_GENERAL_LOCALTIME = TEXT("LocalTime");
const TCHAR* KEY_DEST_GENERAL_TYPE = TEXT("Type");
const TCHAR* KEY_DEST_GENERAL_IMAGEFORMAT = TEXT("Image Format");
const TCHAR* KEY_DEST_GENERAL_FILENAMEFORMAT = TEXT("Filename Format");
const TCHAR* KEY_DEST_GENERAL_PATH = TEXT("Path");
const TCHAR* KEY_DEST_GENERAL_ID = _T("ID");

const TCHAR* KEY_DEST_FTP_HOSTNAME = TEXT("FTP Hostname");
const TCHAR* KEY_DEST_FTP_PORT = TEXT("FTP Port");
const TCHAR* KEY_DEST_FTP_USERNAME = TEXT("FTP Username");
const TCHAR* KEY_DEST_FTP_PASSWORD = TEXT("FTP Password");
const TCHAR* KEY_DEST_FTP_REMOTEPATH = TEXT("FTP Remote Path");
const TCHAR* KEY_DEST_FTP_RESULTURL = TEXT("FTP Result URL");
const TCHAR* KEY_DEST_FTP_COPYURL = TEXT("FTP Copy URL");

const TCHAR* KEY_DEST_SCREENIE_URL = TEXT("Screenie.net Base URL");
const TCHAR* KEY_DEST_SCREENIE_USERNAME = TEXT("Screenie.net Username");
const TCHAR* KEY_DEST_SCREENIE_PASSWORD = TEXT("Screenie.net Password");
const TCHAR* KEY_DEST_SCREENIE_COPYURL = TEXT("Screenie.net Copy URL");

const TCHAR* KEY_DEST_IMAGE_SCALETYPE = TEXT("Scale Type");
const TCHAR* KEY_DEST_IMAGE_SCALEPERCENT = TEXT("Scale Percent");
const TCHAR* KEY_DEST_IMAGE_MAXDIMENSION = TEXT("Maximum Width/Height");

bool ReadDestinationFromRegistry(ScreenshotDestination& destination, LibCC::RegistryKey& key)
{
	// temporary variable for integer settings
	DWORD temp;

	temp = key[KEY_DEST_ENABLED].GetDWORD();
	destination.enabled = (temp != 0);

	////////////////////////////////////////
	// general settings
	////////////////////////////////////////

	destination.general.name = key[KEY_DEST_GENERAL_NAME].GetString();

	temp = key[KEY_DEST_GENERAL_TYPE].GetDWORD();
	destination.general.type = static_cast<ScreenshotDestination::Type>(temp);

  tstd::tstring sTemp;
	sTemp = key[KEY_DEST_GENERAL_ID].GetString();
  destination.general.id.Assign(sTemp);

	destination.general.imageFormat = key[KEY_DEST_GENERAL_IMAGEFORMAT].GetString();
	destination.general.filenameFormat = key[KEY_DEST_GENERAL_FILENAMEFORMAT].GetString();
	destination.general.path = key[KEY_DEST_GENERAL_PATH].GetString();

	temp = key[KEY_DEST_GENERAL_LOCALTIME].GetDWORD();
  destination.general.localTime = temp == 1 ? true : false;

	////////////////////////////////////////
	// image settings
	////////////////////////////////////////

	temp = key[KEY_DEST_IMAGE_SCALETYPE].GetDWORD();
	destination.image.scaleType = static_cast<ScreenshotDestination::ScaleType>(temp);

	temp = key[KEY_DEST_IMAGE_SCALEPERCENT].GetDWORD();
	destination.image.scalePercent = static_cast<int>(temp);

	temp = key[KEY_DEST_IMAGE_MAXDIMENSION].GetDWORD();
	destination.image.maxDimension = static_cast<int>(temp);

	////////////////////////////////////////
	// ftp settings
	////////////////////////////////////////

	destination.ftp.hostname = key[KEY_DEST_FTP_HOSTNAME].GetString();

	temp = key[KEY_DEST_FTP_PORT].GetDWORD();
	destination.ftp.port = static_cast<unsigned short>(temp);

	destination.ftp.username = key[KEY_DEST_FTP_USERNAME].GetString();
	destination.ftp.remotePath = key[KEY_DEST_FTP_REMOTEPATH].GetString();
	destination.ftp.resultURL = key[KEY_DEST_FTP_RESULTURL].GetString();

	LibCC::Blob<BYTE> tempBuffer;
  key.GetValue(KEY_DEST_FTP_PASSWORD, tempBuffer);
	destination.ftp.DeserializePassword(tempBuffer);

	temp = key[KEY_DEST_FTP_COPYURL].GetDWORD();
	destination.ftp.copyURL = (temp != 0);

	//////////////////////////////////////////////////////////////////////////
	// screenie.net settings
	//////////////////////////////////////////////////////////////////////////

	//destination.screenie.url = key[KEY_DEST_SCREENIE_URL].GetString();
	//destination.screenie.username = key[KEY_DEST_SCREENIE_USERNAME].GetString();
	//destination.screenie.password = key[KEY_DEST_SCREENIE_PASSWORD].GetString();

//	temp = key[KEY_DEST_SCREENIE_COPYURL].GetDWORD();
//	destination.screenie.copyURL = (temp != 0);

	return true;
}

bool ScreenshotOptions::LoadOptionsFromRegistry(ScreenshotOptions& options, HKEY root, PCTSTR keyName)
{
	LibCC::RegistryKey MainKey(HKEY_CURRENT_USER, keyName);
	if(!MainKey.Exists()) return false;

	LibCC::RegistryKey DestsKey = MainKey.SubKey(KEY_DESTINATIONS);
	if(DestsKey.Exists())
	{
		std::vector<LibCC::RegistryKey>::iterator it;// oddly vc8 does not recognize SubKeyIterator as a type! i think this is a bug.
		for(it = DestsKey.EnumSubKeysBegin(); it != DestsKey.EnumSubKeysEnd(); ++ it)
		{
			ScreenshotDestination destination;
			if (ReadDestinationFromRegistry(destination, *it))
				options.AddDestination(destination);
		}

		options.IncludeCursor(MainKey[KEY_INCLUDECURSOR].GetDWORD() != 0);
//		options.ShowCropWindow(MainKey[KEY_SHOWCROPPINGWINDOW].GetDWORD() != 0);
//		options.ConfirmOptions(MainKey[KEY_CONFIRMOPTIONS].GetDWORD() != 0);
		options.ShowStatus(MainKey[KEY_SHOWSTATUS].GetDWORD() != 0);
//		options.ShowSplash(MainKey[KEY_SHOWSPLASH].GetDWORD() != 0);

    float fTemp;
    if(MainKey[KEY_CROPPINGZOOMFACTOR].GetValue(&fTemp, sizeof(fTemp)))
    {
      options.CroppingZoomFactor(fTemp);
    }

    {
      LibCC::RegistryKey autoStartKey(root, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"));
      options.AutoStartup(autoStartKey.ValueExists(_T("Screenie")));
    }

    WINDOWPLACEMENT wpTemp;
    if(MainKey[KEY_CONFIGWINDOWPLACEMENT].GetValue(&wpTemp, sizeof(wpTemp)))
    {
      options.SetConfigPlacement(wpTemp);
    }
    if(MainKey[KEY_STATUSWINDOWPLACEMENT].GetValue(&wpTemp, sizeof(wpTemp)))
    {
      options.SetStatusPlacement(wpTemp);
    }
    if(MainKey[KEY_CROPPINGWINDOWPLACEMENT].GetValue(&wpTemp, sizeof(wpTemp)))
    {
      options.SetCroppingPlacement(wpTemp);
    }
  }
  return true;
}


void ScreenshotOptions::Serialize(Xml::Element parent) const
{
	Xml::Serialize(parent, KEY_DESTINATIONS, m_destinations);

	Xml::Serialize(parent, L"IncludeCursor", m_includeCursor);
//	Xml::Serialize(parent, L"ConfirmOptions", m_confirmOptions);
	Xml::Serialize(parent, L"ShowStatus", m_showStatus);
//	Xml::Serialize(parent, L"ShowCropWindow", m_showCropWindow);
	Xml::Serialize(parent, L"ScreenshotAction", ScreenshotActionToString(m_screenshotAction));
	//Xml::Serialize(parent, L"ShowSplash", m_showSplash);
	Xml::Serialize(parent, L"CroppingZoomFactor", m_croppingZoomFactor);
	Xml::Serialize(parent, L"EnableArchive", m_enableArchive);
	Xml::Serialize(parent, L"ArchiveLimit", m_archiveLimit);

	if(m_haveConfigPlacement)
	{
		Xml::Serialize(parent, L"ConfigWindowPlacement", Xml::BinaryData(&m_configPlacement));
	}
	if(m_haveCroppingPlacement)
	{
		Xml::Serialize(parent, L"CroppingWindowPlacement", Xml::BinaryData(&m_croppingPlacement));
	}
	Xml::Serialize(parent, L"CroppingSplitterPosition", CroppingSplitterPosition);
	if(m_haveStatusPlacement)
	{
		Xml::Serialize(parent, L"StatusWindowPlacement", Xml::BinaryData(&m_statusPlacement));
	}

  {
    LibCC::RegistryKey autoStartKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"));
    if(AutoStartup())
    {
      TCHAR fileName[1024];
      GetModuleFileName(NULL, fileName, 1024);
      autoStartKey.SetValue(_T("Screenie"), fileName);
    }
    else
    {
      // make sure it's not in the registry.
      autoStartKey.DeleteValue(_T("Screenie"));
    }
  }
}

void ScreenshotOptions::Deserialize(Xml::Element parent)
{
	Xml::Deserialize(parent, KEY_DESTINATIONS, m_destinations);

	Xml::Deserialize(parent, L"IncludeCursor", m_includeCursor);
	Xml::Deserialize(parent, L"ShowStatus", m_showStatus);
	std::wstring stemp;
	Xml::Deserialize(parent, L"ScreenshotAction", stemp);
	m_screenshotAction = StringToScreenshotAction(stemp);

	Xml::Deserialize(parent, L"CroppingZoomFactor", m_croppingZoomFactor);
	Xml::Deserialize(parent, L"CroppingSplitterPosition", CroppingSplitterPosition);
	Xml::Deserialize(parent, L"EnableArchive", m_enableArchive);
	Xml::Deserialize(parent, L"ArchiveLimit", m_archiveLimit);

	LibCC::Blob<BYTE> temp;
	if(Xml::Deserialize(parent, L"ConfigWindowPlacement", temp))
	{
		if(temp.Size() == sizeof(m_configPlacement))
		{
			memcpy(&m_configPlacement, temp.GetBuffer(), temp.Size());
			m_haveConfigPlacement = true;
		}
	}

	if(Xml::Deserialize(parent, L"CroppingWindowPlacement", temp))
	{
		if(temp.Size() == sizeof(m_croppingPlacement))
		{
			memcpy(&m_croppingPlacement, temp.GetBuffer(), temp.Size());
			m_haveCroppingPlacement = true;
		}
	}

	if(Xml::Deserialize(parent, L"StatusWindowPlacement", temp))
	{
		if(temp.Size() == sizeof(m_statusPlacement))
		{
			memcpy(&m_statusPlacement, temp.GetBuffer(), temp.Size());
			m_haveStatusPlacement = true;
		}
	}

  {
    LibCC::RegistryKey autoStartKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"));
    m_autoStartup = autoStartKey.ValueExists(_T("Screenie"));
  }
}
