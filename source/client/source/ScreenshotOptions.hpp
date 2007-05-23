#ifndef SCREENIE_SCREENSHOTOPTIONS_HPP
#define SCREENIE_SCREENSHOTOPTIONS_HPP

#include <vector>
#include "ScreenshotDestination.hpp"
#include "serialization.h"
#include "libcc/winapi.h"

class ScreenshotOptions
{
public:
	typedef std::vector<ScreenshotDestination> DestinationCollection;

  ScreenshotOptions() :
    m_haveStatusPlacement(false),
    m_haveCroppingPlacement(false),
    m_haveConfigPlacement(false),
	  m_includeCursor(true),
	  m_showStatus(true),
	  m_confirmOptions(false),
	  m_showCropWindow(true),
    m_croppingZoomFactor(2.0f),
    m_autoStartup(true),
    m_showSplash(true)
  {
  }

	ScreenshotOptions(const ScreenshotOptions& copy)
	{
		m_destinations = copy.m_destinations;

    m_autoStartup = copy.m_autoStartup;
		m_includeCursor = copy.m_includeCursor;
		m_showStatus = copy.m_showStatus;
		m_confirmOptions = copy.m_confirmOptions;
		m_showCropWindow = copy.m_showCropWindow;
    m_croppingZoomFactor = copy.m_croppingZoomFactor;
    m_showSplash = copy.m_showSplash;

    m_haveStatusPlacement = copy.m_haveStatusPlacement;
    m_haveCroppingPlacement = copy.m_haveCroppingPlacement;
    m_haveConfigPlacement = copy.m_haveConfigPlacement;
    memcpy(&m_statusPlacement, &copy.m_statusPlacement, sizeof(m_statusPlacement));
    memcpy(&m_croppingPlacement, &copy.m_croppingPlacement, sizeof(m_croppingPlacement));
    memcpy(&m_configPlacement, &copy.m_configPlacement, sizeof(m_configPlacement));
	}

	~ScreenshotOptions()
	{
	}

	ScreenshotOptions& operator=(const ScreenshotOptions& rightHand)
	{
		m_destinations = rightHand.m_destinations;

    m_autoStartup = rightHand.m_autoStartup;
		m_includeCursor = rightHand.m_includeCursor;
		m_showStatus = rightHand.m_showStatus;
		m_confirmOptions = rightHand.m_confirmOptions;
		m_showCropWindow = rightHand.m_showCropWindow;
    m_croppingZoomFactor = rightHand.m_croppingZoomFactor;
    m_showSplash = rightHand.m_showSplash;

    m_haveStatusPlacement = rightHand.m_haveStatusPlacement;
    m_haveCroppingPlacement = rightHand.m_haveCroppingPlacement;
    m_haveConfigPlacement = rightHand.m_haveConfigPlacement;
    memcpy(&m_statusPlacement, &rightHand.m_statusPlacement, sizeof(m_statusPlacement));
    memcpy(&m_croppingPlacement, &rightHand.m_croppingPlacement, sizeof(m_croppingPlacement));
    memcpy(&m_configPlacement, &rightHand.m_configPlacement, sizeof(m_configPlacement));

		return (*this);
	}

	void LoadSettings()
	{
		// try to load from XML first
		tstd::tstring fileName = GenerateXmlFileName(false);
		if(TRUE == PathFileExists(fileName.c_str()))
		{
			Xml::Deserialize(*this, fileName);
			return;
		}

		// no xml file. try from registry.
		LoadOptionsFromRegistry(*this, HKEY_CURRENT_USER, TEXT("Software\\Screenie2"));
	}

	void SaveSettings()
	{
		Xml::Serialize(*this, GenerateXmlFileName(true));
 		//SaveOptionsToRegistry(options, HKEY_CURRENT_USER, TEXT("Software\\Screenie2"));
	}

	// xml serialization
	void Serialize(Xml::Element parent) const;
	void Deserialize(Xml::Element parent);

	// accessors for general options

	bool IncludeCursor() { return m_includeCursor; }
	void IncludeCursor(bool newval) { m_includeCursor = newval; }

  float CroppingZoomFactor() const { return m_croppingZoomFactor; }
  void CroppingZoomFactor(float n) { m_croppingZoomFactor = n; }

  bool AutoStartup() const { return m_autoStartup; }
  void AutoStartup(bool b) { m_autoStartup = b; }

  bool ShowSplash() const { return m_showSplash; }
  void ShowSplash(bool b) { m_showSplash = b; }

	bool ShowStatus() { return m_showStatus; }
	void ShowStatus(bool newval) { m_showStatus = newval; }

	bool ConfirmOptions() { return m_confirmOptions; }
	void ConfirmOptions(bool newval) { m_confirmOptions = newval; }

	bool ShowCropWindow() { return m_showCropWindow; }
	void ShowCropWindow(bool newval) { m_showCropWindow = newval; }

  bool HaveStatusPlacement() const { return m_haveStatusPlacement; }
  bool HaveCroppingPlacement() const { return m_haveCroppingPlacement; }
  bool HaveConfigPlacement() const { return m_haveConfigPlacement; }
  const WINDOWPLACEMENT& GetStatusPlacement() const { return m_statusPlacement; }
  const WINDOWPLACEMENT& GetCroppingPlacement() const { return m_croppingPlacement; }
  const WINDOWPLACEMENT& GetConfigPlacement() const { return m_configPlacement; }
  void SetStatusPlacement(const WINDOWPLACEMENT& x)
  {
    m_haveStatusPlacement = true;
    memcpy(&m_statusPlacement, &x, sizeof(x));
  }
  void SetCroppingPlacement(const WINDOWPLACEMENT& x)
  {
    m_haveCroppingPlacement = true;
    memcpy(&m_croppingPlacement, &x, sizeof(x));
  }
  void SetConfigPlacement(const WINDOWPLACEMENT& x)
  {
    m_haveConfigPlacement = true;
    memcpy(&m_configPlacement, &x, sizeof(x));
  }

	void AddDestination(ScreenshotDestination& destination)
	{
		m_destinations.push_back(destination);
	}

	// unchecked overload
	ScreenshotDestination& GetDestination(size_t pos)
	{
		// ATLASSERT(pos < GetNumDestinations())
		return m_destinations[pos];
	}

	// checked overload
	bool GetDestination(ScreenshotDestination& destination, size_t pos)
	{
		if (pos >= GetNumDestinations())
			return false;

		destination = m_destinations[pos];

		return true;
	}

	bool SetDestination(const ScreenshotDestination& destination, size_t pos)
	{
		if (pos >= GetNumDestinations())
			return false;

		m_destinations[pos] = destination;

		return true;
	}

	size_t GetNumDestinations() const { return m_destinations.size(); }

	bool RemoveDestination(size_t pos)
	{
		if (pos >= GetNumDestinations())
			return false;

		m_destinations.erase(m_destinations.begin() + pos);

		return true;
	}

	void RemoveAllDestinations()
	{
		m_destinations.clear();
	}
private:
	static bool LoadOptionsFromRegistry(ScreenshotOptions& options, HKEY root, PCTSTR keyName);
	static bool SaveOptionsToRegistry(ScreenshotOptions& options, HKEY root, PCTSTR keyName);
	tstd::tstring GenerateXmlFileName(bool createDirectories)
	{
		// use the users' application data path
		tstd::tstring ret;
		GetSpecialFolderPath(ret, CSIDL_APPDATA);
		ret = LibCC::PathJoin(ret, tstd::tstring(_T("Screenie")));
		if(createDirectories)
		{
			DWORD attrib = GetFileAttributes(ret.c_str());
			if((attrib == 0xffffffff) || (!(attrib & FILE_ATTRIBUTE_DIRECTORY)))
			{
				CreateDirectory(ret.c_str(), 0);
			}
		}
		ret = LibCC::PathJoin(ret, tstd::tstring(_T("config.xml")));
		return ret;
	}

	DestinationCollection m_destinations;

  bool m_haveStatusPlacement;
  WINDOWPLACEMENT m_statusPlacement;
  bool m_haveCroppingPlacement;
  WINDOWPLACEMENT m_croppingPlacement;
  bool m_haveConfigPlacement;
  WINDOWPLACEMENT m_configPlacement;

  float m_croppingZoomFactor;

	bool m_autoStartup;
	bool m_includeCursor;
	bool m_showStatus;
	bool m_confirmOptions;
	bool m_showCropWindow;
  bool m_showSplash;
};



#endif