//
// ScreenshotOptions.cpp - saving and loading of screenshot options
// Copyright (c) 2003 Carl Corcoran
// Copyright (c) 2005 Roger Clark
//

#include "stdafx.hpp"
#include "resource.h"

#include "options.hpp"
#include "ScreenshotDestination.hpp"
#include "ScreenshotOptions.hpp"
#include "libcc/registry.h"// huhu mixing libraries... very shitty.

const TCHAR* KEY_INCLUDECURSOR = TEXT("Include Cursor");
const TCHAR* KEY_SHOWCROPPINGWINDOW = TEXT("Show Cropping Window");
const TCHAR* KEY_CONFIRMOPTIONS = TEXT("Confirm Options");
const TCHAR* KEY_SHOWSTATUS = TEXT("Show Status Dialog");

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

const TCHAR* KEY_DEST_FTP_HOSTNAME = TEXT("FTP Hostname");
const TCHAR* KEY_DEST_FTP_PORT = TEXT("FTP Port");
const TCHAR* KEY_DEST_FTP_USERNAME = TEXT("FTP Username");
const TCHAR* KEY_DEST_FTP_PASSWORD = TEXT("FTP Password");
const TCHAR* KEY_DEST_FTP_REMOTEPATH = TEXT("FTP Remote Path");
const TCHAR* KEY_DEST_FTP_RESULTURL = TEXT("FTP Result URL");
const TCHAR* KEY_DEST_FTP_COPYURL = TEXT("FTP Copy URL");

const TCHAR* KEY_DEST_IMAGE_SCALETYPE = TEXT("Scale Type");
const TCHAR* KEY_DEST_IMAGE_SCALEPERCENT = TEXT("Scale Percent");
const TCHAR* KEY_DEST_IMAGE_MAXDIMENSION = TEXT("Maximum Width/Height");
const TCHAR* KEY_DEST_IMAGE_CREATETHUMBNAIL = TEXT("Create Thumbnail");
const TCHAR* KEY_DEST_IMAGE_THUMBSCALETYPE = TEXT("Thumbnail Scale Type");
const TCHAR* KEY_DEST_IMAGE_THUMBSCALEPERCENT = TEXT("Thumbnail Scale Percent");
const TCHAR* KEY_DEST_IMAGE_THUMBMAXDIMENSION = TEXT("Thumbnail Max Width/Height");
const TCHAR* KEY_DEST_IMAGE_USEFILENAMEFORMAT = TEXT("Use Thumbnail Filename Format");
const TCHAR* KEY_DEST_IMAGE_THUMBFILENAMEFORMAT = TEXT("Thumbnail Filename Format");

bool ReadDestinationFromRegistry(ScreenshotDestination& destination, CRegistryKey& key)
{
	// temporary variable for integer settings
	DWORD temp;

	key.GetDWORD(KEY_DEST_ENABLED, &temp);
	destination.enabled = (temp != 0);

	////////////////////////////////////////
	// general settings
	////////////////////////////////////////

	key.GetString(KEY_DEST_GENERAL_NAME, destination.general.name);

	key.GetDWORD(KEY_DEST_GENERAL_TYPE, &temp);
	destination.general.type = static_cast<ScreenshotDestination::Type>(temp);

	key.GetString(KEY_DEST_GENERAL_IMAGEFORMAT, destination.general.imageFormat);
	key.GetString(KEY_DEST_GENERAL_FILENAMEFORMAT, destination.general.filenameFormat);
	key.GetString(KEY_DEST_GENERAL_PATH, destination.general.path);
  key.GetDWORD(KEY_DEST_GENERAL_LOCALTIME, &temp);
  destination.general.localTime = temp == 1 ? true : false;

	////////////////////////////////////////
	// image settings
	////////////////////////////////////////

	key.GetDWORD(KEY_DEST_IMAGE_SCALETYPE, &temp);
	destination.image.scaleType = static_cast<ScreenshotDestination::ScaleType>(temp);

	key.GetDWORD(KEY_DEST_IMAGE_SCALEPERCENT, &temp);
	destination.image.scalePercent = static_cast<int>(temp);

	key.GetDWORD(KEY_DEST_IMAGE_MAXDIMENSION, &temp);
	destination.image.maxDimension = static_cast<int>(temp);

	key.GetDWORD(KEY_DEST_IMAGE_CREATETHUMBNAIL, &temp);
	destination.image.createThumbnail = (temp != 0);

	key.GetDWORD(KEY_DEST_IMAGE_THUMBSCALETYPE, &temp);
	destination.image.thumbScaleType = static_cast<ScreenshotDestination::ScaleType>(temp);

	key.GetDWORD(KEY_DEST_IMAGE_THUMBSCALEPERCENT, &temp);
	destination.image.thumbScalePercent = static_cast<int>(temp);

	key.GetDWORD(KEY_DEST_IMAGE_THUMBMAXDIMENSION, &temp);
	destination.image.thumbMaxDimension = static_cast<int>(temp);

	key.GetDWORD(KEY_DEST_IMAGE_USEFILENAMEFORMAT, &temp);
	destination.image.useFilenameFormat = (temp != 0);

	key.GetString(KEY_DEST_IMAGE_THUMBFILENAMEFORMAT, destination.image.filenameFormat);

	////////////////////////////////////////
	// ftp settings
	////////////////////////////////////////

	key.GetString(KEY_DEST_FTP_HOSTNAME, destination.ftp.hostname);

	key.GetDWORD(KEY_DEST_FTP_PORT, &temp);
	destination.ftp.port = static_cast<unsigned short>(temp);

	key.GetString(KEY_DEST_FTP_USERNAME, destination.ftp.username);
	key.GetString(KEY_DEST_FTP_PASSWORD, destination.ftp.password);
	key.GetString(KEY_DEST_FTP_REMOTEPATH, destination.ftp.remotePath);
	key.GetString(KEY_DEST_FTP_RESULTURL, destination.ftp.resultURL);

	key.GetDWORD(KEY_DEST_FTP_COPYURL, &temp);
	destination.ftp.copyURL = (temp != 0);

	return true;
}

bool LoadOptionsFromRegistry(ScreenshotOptions& options, HKEY root, PCTSTR keyName)
{
    CRegistryKey MainKey;
	if (!MainKey.Init(HKEY_CURRENT_USER, keyName))
		return false;

  LibCC::RegistryKey MainKey2(HKEY_CURRENT_USER, keyName);

	CRegistryKey DestsKey;
	if (MainKey.OpenSubKey(KEY_DESTINATIONS, &DestsKey))
	{
		CRegistryKeysEnum e;

		e.Reset(&DestsKey);

		CRegistryKey DestKey;
		while (e.Next(DestKey))
		{
			ScreenshotDestination destination;
			if (ReadDestinationFromRegistry(destination, DestKey))
				options.AddDestination(destination);
		}

		DWORD temp;

		MainKey.GetDWORD(KEY_INCLUDECURSOR, &temp);
		options.IncludeCursor(temp != 0);

		MainKey.GetDWORD(KEY_SHOWCROPPINGWINDOW, &temp);
		options.ShowCropWindow(temp != 0);

		MainKey.GetDWORD(KEY_CONFIRMOPTIONS, &temp);
		options.ConfirmOptions(temp != 0);

		MainKey.GetDWORD(KEY_SHOWSTATUS, &temp);
		options.ShowStatus(temp != 0);

    WINDOWPLACEMENT wpTemp;
    if(MainKey2[KEY_CONFIGWINDOWPLACEMENT].GetValue(&wpTemp, sizeof(wpTemp)))
    {
      options.SetConfigPlacement(wpTemp);
    }
    if(MainKey2[KEY_STATUSWINDOWPLACEMENT].GetValue(&wpTemp, sizeof(wpTemp)))
    {
      options.SetStatusPlacement(wpTemp);
    }
    if(MainKey2[KEY_CROPPINGWINDOWPLACEMENT].GetValue(&wpTemp, sizeof(wpTemp)))
    {
      options.SetCroppingPlacement(wpTemp);
    }
  }
  return true;
}

bool WriteDestinationToRegistry(const ScreenshotDestination& destination, CRegistryKey& key)
{
	key.SetDWORD(KEY_DEST_ENABLED, destination.enabled);

	////////////////////////////////////////
	// general settings
	////////////////////////////////////////

	key.SetString(KEY_DEST_GENERAL_NAME, destination.general.name);
	key.SetDWORD(KEY_DEST_GENERAL_TYPE, destination.general.type);
	key.SetString(KEY_DEST_GENERAL_IMAGEFORMAT, destination.general.imageFormat);
	key.SetString(KEY_DEST_GENERAL_FILENAMEFORMAT, destination.general.filenameFormat);
	key.SetString(KEY_DEST_GENERAL_PATH, destination.general.path);
  key.SetDWORD(KEY_DEST_GENERAL_LOCALTIME, destination.general.localTime ? 1 : 0);

	////////////////////////////////////////
	// image settings
	////////////////////////////////////////

	key.SetDWORD(KEY_DEST_IMAGE_SCALETYPE, destination.image.scaleType);
	key.SetDWORD(KEY_DEST_IMAGE_SCALEPERCENT, destination.image.scalePercent);
	key.SetDWORD(KEY_DEST_IMAGE_MAXDIMENSION, destination.image.maxDimension);
	key.SetDWORD(KEY_DEST_IMAGE_CREATETHUMBNAIL, destination.image.createThumbnail);
	key.SetDWORD(KEY_DEST_IMAGE_THUMBSCALETYPE, destination.image.thumbScaleType);
	key.SetDWORD(KEY_DEST_IMAGE_THUMBSCALEPERCENT, destination.image.thumbScalePercent);
	key.SetDWORD(KEY_DEST_IMAGE_THUMBMAXDIMENSION, destination.image.thumbMaxDimension);
	key.SetDWORD(KEY_DEST_IMAGE_USEFILENAMEFORMAT, destination.image.useFilenameFormat);
	key.SetString(KEY_DEST_IMAGE_THUMBFILENAMEFORMAT, destination.image.filenameFormat);

	////////////////////////////////////////
	// ftp settings
	////////////////////////////////////////

	key.SetString(KEY_DEST_FTP_HOSTNAME, destination.ftp.hostname);
	key.SetDWORD(KEY_DEST_FTP_PORT, destination.ftp.port);
	key.SetString(KEY_DEST_FTP_USERNAME, destination.ftp.username);
	key.SetString(KEY_DEST_FTP_PASSWORD, destination.ftp.password);
	key.SetString(KEY_DEST_FTP_REMOTEPATH, destination.ftp.remotePath);
	key.SetString(KEY_DEST_FTP_RESULTURL, destination.ftp.resultURL);
	key.SetDWORD(KEY_DEST_FTP_COPYURL, destination.ftp.copyURL);

	return true;
}

bool SaveOptionsToRegistry(ScreenshotOptions& options, HKEY root, PCTSTR keyName)
{
	if (::SHDeleteKey(root, keyName) != ERROR_SUCCESS)
		return false;

	bool success = false;

	CRegistryKey MainKey;
  if(MainKey.Init(root, keyName))
  {
    success = true;

    LibCC::RegistryKey MainKey2(root, keyName);

    CRegistryKey DestsKey;
    if(MainKey.OpenSubKey(KEY_DESTINATIONS, &DestsKey))
    {
			ScreenshotDestination destination;
			for (size_t i = 0; i < options.GetNumDestinations(); ++i)
			{
				if (options.GetDestination(destination, i))
				{
					// generate the key name based on the destination name and index
					tstd::tstring keyName = LibCC::Format(TEXT("% (%)")).s(destination.general.name).ul(i).Str();

					CRegistryKey DestKey;
					if (!DestsKey.OpenSubKey(keyName, &DestKey))
					{
						success = false;
						break;
					}

					WriteDestinationToRegistry(destination, DestKey);
				}
			}
		}

		MainKey.SetDWORD(KEY_INCLUDECURSOR, options.IncludeCursor());
		MainKey.SetDWORD(KEY_CONFIRMOPTIONS, options.ConfirmOptions());
		MainKey.SetDWORD(KEY_SHOWSTATUS, options.ShowStatus());
		MainKey.SetDWORD(KEY_SHOWCROPPINGWINDOW, options.ShowCropWindow());

    if(options.HaveConfigPlacement())
    {
      MainKey2[KEY_CONFIGWINDOWPLACEMENT].SetValue(&options.GetConfigPlacement(), sizeof(WINDOWPLACEMENT));
    }
    if(options.HaveCroppingPlacement())
    {
      MainKey2[KEY_CROPPINGWINDOWPLACEMENT].SetValue(&options.GetCroppingPlacement(), sizeof(WINDOWPLACEMENT));
    }
    if(options.HaveStatusPlacement())
    {
      MainKey2[KEY_STATUSWINDOWPLACEMENT].SetValue(&options.GetStatusPlacement(), sizeof(WINDOWPLACEMENT));
    }

		MainKey.Uninit();
	}

	return true;
}