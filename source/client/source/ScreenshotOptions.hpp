#ifndef SCREENIE_SCREENSHOTOPTIONS_HPP
#define SCREENIE_SCREENSHOTOPTIONS_HPP

#include <vector>
#include "ScreenshotDestination.hpp"
#include "serialization.h"
#include "libcc/winapi.h"

enum ScreenshotAction
{
	SA_NONE,
	SA_SHOWCROP,
	SA_SHOWDESTINATIONS
};
#define SA_NONE_STR L"Do nothing."
#define SA_SHOWCROP_STR L"Edit the image."
#define SA_SHOWDESTINATIONS_STR L"Show the options dialog."

// NOTE that these strings are used for both the GUI and the options XML.
inline std::wstring ScreenshotActionToString(ScreenshotAction sa)
{
	switch(sa)
	{
	case SA_NONE:
		return SA_NONE_STR;
	case SA_SHOWDESTINATIONS:
		return SA_SHOWDESTINATIONS_STR;
	}
	return SA_SHOWCROP_STR;
}
inline ScreenshotAction StringToScreenshotAction(const std::wstring& s)
{
	std::wstring l = LibCC::StringToLower(s);
	if(l == LibCC::StringToLower(SA_NONE_STR))
	{
		return SA_NONE;
	}
	else if(l == LibCC::StringToLower(SA_SHOWDESTINATIONS_STR))
	{
		return SA_SHOWDESTINATIONS;
	}
	return SA_SHOWCROP;
}

class ScreenshotOptions
{
public:
	typedef std::vector<ScreenshotDestination> DestinationCollection;

  ScreenshotOptions() :
		// DEFAULTS
    m_haveStatusPlacement(false),
    m_haveCroppingPlacement(false),
    m_haveConfigPlacement(false),
	  m_includeCursor(false),
	  m_showStatus(false),
		m_screenshotAction(SA_SHOWCROP),
    m_croppingZoomFactor(2.0f),
    m_autoStartup(true),
		m_savedInAppDir(false),
		m_enableArchive(false),
		m_archiveLimit(100000000),
		CroppingSplitterPosition(150)
  {
  }

	ScreenshotOptions(const ScreenshotOptions& copy)
	{
		m_destinations = copy.m_destinations;

    m_autoStartup = copy.m_autoStartup;
		m_includeCursor = copy.m_includeCursor;
		m_showStatus = copy.m_showStatus;
		m_screenshotAction = copy.m_screenshotAction;
    m_croppingZoomFactor = copy.m_croppingZoomFactor;
		CroppingSplitterPosition = copy.CroppingSplitterPosition;
		m_enableArchive = copy.m_enableArchive;
		m_archiveLimit = copy.m_archiveLimit;

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
		m_screenshotAction = rightHand.m_screenshotAction;
    m_croppingZoomFactor = rightHand.m_croppingZoomFactor;
		CroppingSplitterPosition = rightHand.CroppingSplitterPosition;
		m_enableArchive = rightHand.m_enableArchive;
		m_archiveLimit = rightHand.m_archiveLimit;

    m_haveStatusPlacement = rightHand.m_haveStatusPlacement;
    m_haveCroppingPlacement = rightHand.m_haveCroppingPlacement;
    m_haveConfigPlacement = rightHand.m_haveConfigPlacement;
    memcpy(&m_statusPlacement, &rightHand.m_statusPlacement, sizeof(m_statusPlacement));
    memcpy(&m_croppingPlacement, &rightHand.m_croppingPlacement, sizeof(m_croppingPlacement));
    memcpy(&m_configPlacement, &rightHand.m_configPlacement, sizeof(m_configPlacement));

		return (*this);
	}

	bool m_savedInAppDir;// this is true if the configuration was loaded from the application directory (as opposed to Application Data directory)

	bool LoadSettings()
	{
		// try to load from XML first
		m_savedInAppDir = false;// default
		tstd::tstring fileName;
		fileName = GenerateXmlFileNameInAppDir();
		if(TRUE == PathFileExists(fileName.c_str()))
		{
			m_savedInAppDir = true;
			Xml::Deserialize(*this, fileName);
			return true;
		}
		else
		{
			fileName = GenerateXmlFileName(false);
			if(TRUE == PathFileExists(fileName.c_str()))
			{
				Xml::Deserialize(*this, fileName);
				return true;
			}
		}

		// no xml file. try from registry.
		if (LoadOptionsFromRegistry(*this, HKEY_CURRENT_USER, TEXT("Software\\Screenie2")))
		{
			return true;
		}

		return false;
	}

	std::wstring GetConfigPath() const
	{
		if(m_savedInAppDir)
		{
			return GenerateXmlFileNameInAppDir();
		}
		return GenerateXmlFileName(true);
	}

	void SaveSettings()
	{
		Xml::Serialize(*this, GetConfigPath());
	}

	// xml serialization
	void Serialize(Xml::Element parent) const;
	void Deserialize(Xml::Element parent);

	// accessors for general options

	bool IncludeCursor() { return m_includeCursor; }
	void IncludeCursor(bool newval) { m_includeCursor = newval; }

  float CroppingZoomFactor() const { return m_croppingZoomFactor; }
  void CroppingZoomFactor(float n) { m_croppingZoomFactor = n; }

	int CroppingSplitterPosition;// because getters/setters are just unnecessary for this.

  bool AutoStartup() const { return m_autoStartup; }
  void AutoStartup(bool b) { m_autoStartup = b; }

	bool ShowStatus() { return m_showStatus; }
	void ShowStatus(bool newval) { m_showStatus = newval; }

	ScreenshotAction GetScreenshotAction() const { return m_screenshotAction; }
	void SetScreenshotAction(ScreenshotAction sa) { m_screenshotAction = sa; }

	bool EnableArchive() const { return m_enableArchive; }
	void EnableArchive(bool newval) { m_enableArchive = newval; }
	int ArchiveLimit() const { return m_archiveLimit; }
	void ArchiveLimit(int newval) { m_archiveLimit = newval; }

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

	DestinationCollection m_destinations;// making this public because, at the moment, there is no value in adding wrappers around everything here.

private:
	static bool LoadOptionsFromRegistry(ScreenshotOptions& options, HKEY root, PCTSTR keyName);
	static bool SaveOptionsToRegistry(ScreenshotOptions& options, HKEY root, PCTSTR keyName);
	tstd::tstring GenerateXmlFileName(bool createDirectories) const
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
		ret = LibCC::PathJoin(ret, tstd::tstring(_T("screenieConfig.xml")));
		return ret;
	}

	tstd::tstring GenerateXmlFileNameInAppDir() const
	{
		return GetPathRelativeToApp(_T("screenieConfig.xml"));
	}


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
	ScreenshotAction m_screenshotAction;

	bool m_enableArchive;
	int m_archiveLimit;
};



#endif