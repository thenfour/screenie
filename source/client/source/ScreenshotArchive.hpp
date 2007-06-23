/*
	Interfaces with sqlite to deal with archived screenshot information.
	Basically, each screenshot is 1 row. It may have many Events.
	There is also an option in ScreenshotOptions for how much data should be saved.
	The archive file (screenie.db) is always saved in the same dir as the config xml.
*/

#pragma once

#include "ScreenshotOptions.hpp"

enum MessageIcon
{
	MSG_INFO,
  MSG_WARNING,
	MSG_ERROR,
  MSG_CHECK,
  MSG_PROGRESS
};
enum MessageType
{
  ITEM_GENERAL,
  ITEM_FTP,
  ITEM_FILE
};


class ScreenshotArchive
{
	static const int DatabaseVersion = 1;
public:
	ScreenshotArchive(ScreenshotOptions& opt);
	~ScreenshotArchive();

	class Event
	{
		MessageIcon icon;
		MessageType messageType;
		std::wstring destinationName;
		std::wstring messageText;
		std::wstring data1;// could be URL or a path name ...
		int screenshotID;
		int id;
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

	int RegisterNewScreenshot(util::shared_ptr<Gdiplus::Bitmap>);// returns a unique identifier of this screenshot.
	int RegisterNewEvent(int screenshotID,
		MessageIcon icon, MessageType messageType,
		const std::wstring& destinationName, const std::wstring& messageText, const std::wstring& data1);

private:
	ScreenshotOptions& m_options;

	std::wstring GetDBFilename() const;
	bool EnsureDatabaseHasSpace(size_t extra);
	bool OpenDatabase(sqlite3x::sqlite3_connection&);
};

