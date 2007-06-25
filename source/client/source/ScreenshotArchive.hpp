/*
	Interfaces with sqlite to deal with archived screenshot information.
	Basically, each screenshot is 1 row. It may have many Events.
	There is also an option in ScreenshotOptions for how much data should be saved.
	The archive file (screenie.db) is always saved in the same dir as the config xml,
	so if you travel with your config xml, you will travel with your archive as well
*/

#pragma once

#include "ScreenshotOptions.hpp"

enum EventIcon
{
	EI_INFO,
  EI_WARNING,
	EI_ERROR,
  EI_CHECK,
  EI_PROGRESS
};
enum EventType
{
  ET_GENERAL,
  ET_FTP,
  ET_FILE
};

// because activity is sent to different places, this interface exists (fka class AsyncStatusWindow)
// BE CAREFUL though - different classes will have different uses for the IDs. So don't confuse one
// "event id" for another. Don't confuse the ActivityList's returned ScreenshotID (a pointer to some stuff)
// for the ScreenshotArchive's returned ScreenshotID (the database primary key).
typedef struct { } *EventID;
typedef struct { } *ScreenshotID;

struct IActivity
{
	virtual ScreenshotID RegisterScreenshot(Gdiplus::BitmapPtr image) = 0;
  virtual EventID RegisterEvent(ScreenshotID screenshotID, EventIcon icon, EventType type, const std::wstring& destination, const std::wstring& message, const std::wstring& url = L"") = 0;
  virtual void EventSetIcon(EventID eventID, EventIcon icon) = 0;
  virtual void EventSetProgress(EventID eventID, int pos, int total) = 0;
  virtual void EventSetText(EventID eventID, const tstd::tstring& msg) = 0;
  virtual void EventSetURL(EventID eventID, const tstd::tstring& url) = 0;
};


class ScreenshotArchive :
	public IActivity
{
public:
	ScreenshotArchive(ScreenshotOptions& opt);
	~ScreenshotArchive();

	class Event
	{
		EventID id;
		ScreenshotID screenshotID;
		EventIcon icon;
		EventType type;
		std::wstring destinationName;
		std::wstring messageText;
		std::wstring url;
	};

	// basically the same as a message in the status dlg
	class Screenshot
	{
	public:
		int id;
		util::shared_ptr<Gdiplus::Bitmap> RetrieveImage();
		std::vector<Event> RetreiveEvents();// returns a list of events
	};

	std::vector<Screenshot> RetreiveScreenshots();

	// IActivity events
	ScreenshotID RegisterScreenshot(Gdiplus::BitmapPtr image);
	EventID RegisterEvent(ScreenshotID screenshotID, EventIcon icon, EventType type, const std::wstring& destination, const std::wstring& message, const std::wstring& url = L"");
	void EventSetIcon(EventID eventID, EventIcon icon);
	void EventSetProgress(EventID eventID, int pos, int total) { }// not implemented; this is not stored in the archive.
	void EventSetText(EventID eventID, const std::wstring& msg);
	void EventSetURL(EventID eventID, const std::wstring& url);

private:
	ScreenshotOptions& m_options;

	std::wstring m_schemaVersion;
	std::wstring GetDatabaseSchemaVersion();

	std::wstring GetDBFilename() const;
	bool EnsureDatabaseHasSpace(size_t extra);
	bool OpenDatabase(sqlite3x::sqlite3_connection&);
};

