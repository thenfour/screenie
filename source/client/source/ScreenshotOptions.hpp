#ifndef SCREENIE_SCREENSHOTOPTIONS_HPP
#define SCREENIE_SCREENSHOTOPTIONS_HPP

// for std::vector
#include <vector>

// for ScreenshotDestination
#include "ScreenshotDestination.hpp"

class ScreenshotOptions
{
public:
	typedef std::vector<ScreenshotDestination> DestinationCollection;

	ScreenshotOptions() { }

	ScreenshotOptions(const ScreenshotOptions& copy)
	{
		m_destinations = copy.m_destinations;

		m_includeCursor = copy.m_includeCursor;
		m_showStatus = copy.m_showStatus;
		m_confirmOptions = copy.m_confirmOptions;
		m_showCropWindow = copy.m_showCropWindow;
	}

	~ScreenshotOptions()
	{
		// move along, nothing to see here
		m_destinations.clear();
	}

	ScreenshotOptions& operator=(const ScreenshotOptions& rightHand)
	{
		m_destinations.clear();

		m_destinations = rightHand.m_destinations;

		m_includeCursor = rightHand.m_includeCursor;
		m_showStatus = rightHand.m_showStatus;
		m_confirmOptions = rightHand.m_confirmOptions;
		m_showCropWindow = rightHand.m_showCropWindow;

		return (*this);
	}

	// accessors for general options

	bool IncludeCursor() { return m_includeCursor; }
	void IncludeCursor(bool newval) { m_includeCursor = newval; }

	bool ShowStatus() { return m_showStatus; }
	void ShowStatus(bool newval) { m_showStatus = newval; }

	bool ConfirmOptions() { return m_confirmOptions; }
	void ConfirmOptions(bool newval) { m_confirmOptions = newval; }

	bool ShowCropWindow() { return m_showCropWindow; }
	void ShowCropWindow(bool newval) { m_showCropWindow = newval; }

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
	DestinationCollection m_destinations;

	bool m_includeCursor;
	bool m_showStatus;
	bool m_confirmOptions;
	bool m_showCropWindow;
};

bool LoadOptionsFromRegistry(ScreenshotOptions& options, HKEY root, PCTSTR keyName);
bool SaveOptionsToRegistry(ScreenshotOptions& options, HKEY root, PCTSTR keyName);

#endif