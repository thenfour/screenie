#include "StdAfx.hpp"
#include "ScreenshotArchive.hpp"
#include "codec.hpp"
#include "..\sqlite\sqlite3.h"
#include "image.hpp"
#include "resource.h"
#include "utility.hpp"

std::wstring QuoteSql(const std::wstring& in)
{
	std::wstring ret;
	ret.reserve(in.length());
	for(std::wstring::const_iterator it = in.begin(); it != in.end(); ++ it)
	{
		if(*it == L'\'')
		{
			ret.push_back(L'\'');
			ret.push_back(L'\'');
		}
		else
		{
			ret.push_back(*it);
		}
	}
	return ret;
}

util::shared_ptr<Gdiplus::Bitmap> ScreenshotArchive::Screenshot::RetrieveImage()
{
	Gdiplus::Bitmap* p = 0;
	if(!m_owner->m_options.EnableArchive())
		return util::shared_ptr<Gdiplus::Bitmap>(p);

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!m_owner->OpenDatabase(conn))
			return util::shared_ptr<Gdiplus::Bitmap>(p);

		sqlite3x::sqlite3_command cmd(conn, LibCC::Format("select [bitmapData] where [id] = %").i((int)id).Str());
		sqlite3x::sqlite3_reader rdr = cmd.executereader();
		if(rdr.read())
		{
			std::string data = rdr.getblob(0);
			// now read that sucker into a gdiplus bitmap
			BlobStream stream;
			stream.m_blob.Alloc(data.size());
			memcpy(stream.m_blob.GetBuffer(), data.c_str(), data.size());
			p = new Gdiplus::Bitmap(&stream);
		}
		
		conn.close();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception in EventSetIcon. %").qs(er.what()));
	}
	return util::shared_ptr<Gdiplus::Bitmap>(p);
}

util::shared_ptr<Gdiplus::Bitmap> ScreenshotArchive::Screenshot::RetrieveThumbnail()
{
	util::shared_ptr<Gdiplus::Bitmap> ret;
	if(!ResizeBitmap(ret, *RetrieveImage(), 100))
	{
		return util::shared_ptr<Gdiplus::Bitmap>((Gdiplus::Bitmap*)0);
	}
	return ret;
}
bool ResizeBitmap(util::shared_ptr<Gdiplus::Bitmap>& destination, Gdiplus::Bitmap& source, int dimensionLimit);


ScreenshotArchive::Screenshot::Screenshot(ScreenshotArchive* owner) :
	m_owner(owner)
{
}

std::vector<ScreenshotArchive::Event> ScreenshotArchive::Screenshot::RetreiveEvents()
{
	std::vector<ScreenshotArchive::Event> ret;
	if(!m_owner->m_options.EnableArchive())
		return ret;

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!m_owner->OpenDatabase(conn))
			return ret;

		sqlite3x::sqlite3_command cmd(conn, LibCC::Format("select [id], screenshotID, icon, eventType, destinationName, eventText, url, [date] from Events where screenshotID = %").i((int)id).Str());
		sqlite3x::sqlite3_reader rdr = cmd.executereader();
		while(rdr.read())
		{
			ret.push_back(Event());
			//fill ret.back();
			ret.back().id = (EventID)rdr.getint(0);
			ret.back().screenshotID = (ScreenshotID)rdr.getint(1);
			ret.back().icon = (EventIcon)rdr.getint(2);
			ret.back().destinationName = rdr.getstring16(3);
			ret.back().messageText = rdr.getstring16(4);
			ret.back().url = rdr.getstring16(5);
			ret.back().date = rdr.getstring16(6);
		}
		
		conn.close();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception in EventSetIcon. %").qs(er.what()));
	}
	return ret;
}

ScreenshotArchive::ScreenshotArchive(ScreenshotOptions& opt) :
	m_options(opt),
	m_nextScreenshotID(0),
	m_nextEventID(0),
	m_pNotify(0)
{
}

ScreenshotArchive::~ScreenshotArchive()
{
}

std::vector<ScreenshotArchive::Screenshot> ScreenshotArchive::RetreiveScreenshots()
{
	std::vector<ScreenshotArchive::Screenshot> ret;
	if(!m_options.EnableArchive())
		return ret;

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn))
			return ret;
		sqlite3x::sqlite3_command cmd(conn, "select [id] from Screenshots");
		sqlite3x::sqlite3_reader rdr = cmd.executereader();
		while(rdr.read())
		{
			ret.push_back(Screenshot(this));
			//fill ret.back();
			ret.back().id = (ScreenshotID)rdr.getint(0);
		}
		
		conn.close();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception in EventSetIcon. %").qs(er.what()));
	}
	return ret;
}

std::wstring ScreenshotArchive::GetDBFilename() const
{
	// generate the filename.
	std::wstring path = m_options.GetConfigPath();
	path = LibCC::PathRemoveFilename(path);
	LibCC::PathAppendX(path, L"screenie.db");
	return path;
}

// TODO: Optimize this.
// Idea #1: Use transactions to optimize 1 pass
// Idea #2: Group passes into 1 (remove multiple screenshots at the same time)
// Idea #3: To reduce the frequency that this needs to do cleanup, have a buffer zone. So, when it gets to 50mb, then vacuum up until 30mb.
// Idea #4: ???
bool ScreenshotArchive::EnsureDatabaseHasSpace(size_t extra)
{
	size_t extraMB = extra;
	std::wstring filename = GetDBFilename();
	
	if(extraMB > (size_t)m_options.ArchiveLimit())
		return false;// there's no possible way you could store this amount of data.

	do
	{
		HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		if(LibCC::IsBadHandle(hFile))
			return true;// probably just not found.
		DWORD sizeMB = GetFileSize(hFile, 0);
		CloseHandle(hFile);
		if((sizeMB + extraMB) < (size_t)m_options.ArchiveLimit())
		{
			return true;
		}

		try
		{
			// delete some stuff.
			sqlite3x::sqlite3_connection conn;
			if(!OpenDatabase(conn))
				return true;

			int count = conn.executeint("select count(*) from screenshots");
			if(count == 0)// there are no records, but we know that there's no space. nothing we can do.
			{
				return false;
			}

			std::wstring screenshotDate = conn.executestring16("select min([date]) from screenshots");
			int screenshotID = conn.executeint(LibCC::Format("select [id] from screenshots where [date] = '%'").s(screenshotDate).Str());
			conn.executenonquery(LibCC::Format("delete from Events where screenshotID = %")(screenshotID).Str());
			conn.executenonquery(LibCC::Format("delete from Screenshots where [id] = %")(screenshotID).Str());

			// now vacuum
			conn.executenonquery("vacuum");
			conn.close();

			// notify others that we just changed stuff.
			if(m_pNotify)
				m_pNotify->OnPruneScreenshot((ScreenshotID)screenshotID);
		}
		catch(sqlite3x::database_error& er)
		{
			LibCC::g_pLog->Message(LibCC::Format("Database exception while shrinking the database. %").qs(er.what()));
			return false;
		}
	}
	while(true);

	// unreachable code.
	return true;
}


ScreenshotID ScreenshotArchive::RegisterScreenshot(util::shared_ptr<Gdiplus::Bitmap> bmp)
{
	if(!m_options.EnableArchive())
	{
		m_nextScreenshotID ++;
		return m_nextScreenshotID;
	}

	ScreenshotID ret = 0;

	// save a lossless PNG file to a memory stream
	BlobStream stream;
	ImageCodecsEnum codecs;
	Gdiplus::ImageCodecInfo* codecInfo = codecs.GetCodecByMimeType(_T("image/png"));
	Gdiplus::Status status = bmp->Save(&stream, &codecInfo->Clsid);
	if(status != Gdiplus::Ok)
		return 0;

	try
	{
		// make sure there is space in the db file.
		if(!EnsureDatabaseHasSpace(stream.GetLength()))
			return 0;

		// and save to the database.
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn))
			return 0;
		sqlite3x::sqlite3_command cmd(conn, "insert into Screenshots ([bitmapData], [date]) values (?, strftime('%Y-%m-%dT%H:%M:%f', 'now'))");
		cmd.bind(1, stream.GetBuffer(), stream.GetLength());

		//LARGE_INTEGER li;
		//li.QuadPart = 0;
		//stream.Seek(li, STREAM_SEEK_SET, 0);
		//Gdiplus::Bitmap* x = new Gdiplus::Bitmap(&stream);
		//DumpBitmap(*x);
		//delete x;

		cmd.executenonquery();
		ret = (ScreenshotID)conn.insertid();
		conn.close();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception while inserting screenshot data. %").qs(er.what()));
	}

	return ret;
}

EventID ScreenshotArchive::RegisterEvent(ScreenshotID screenshotID, EventIcon icon, EventType type, const std::wstring& destination, const std::wstring& message, const std::wstring& url)
{
	if(!m_options.EnableArchive())
	{
		m_nextEventID ++;
		return m_nextEventID;
	}

	EventID ret = 0;
	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn)) return 0;
		std::wstring sql = LibCC::Format("insert into Events (screenshotID, icon, eventType, destinationName, eventText, url, [date]) values "
			"(%, %, %, '%', '%', '%', strftime('^%Y-^%m-^%dT^%H:^%M:^%f', 'now'))")
			((int)screenshotID)
			((int)icon)
			((int)type)
			(QuoteSql(destination))
			(QuoteSql(message))
			(QuoteSql(url))
			.Str();

		conn.executenonquery(sql);

		ret = (EventID)conn.insertid();
		conn.close();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception while inserting an event. %").qs(er.what()));
	}
	return ret;
}

void ScreenshotArchive::EventSetIcon(EventID id, EventIcon icon)
{
	if(!m_options.EnableArchive())
		return;

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn)) return;

		std::wstring sql = LibCC::Format("update Events set icon = %").i((int)icon).Str();

		conn.executenonquery(sql);
		conn.close();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception in EventSetIcon. %").qs(er.what()));
	}
}

void ScreenshotArchive::EventSetText(EventID id, const std::wstring& msg)
{
	if(!m_options.EnableArchive())
		return;

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn)) return;
		std::wstring sql = LibCC::Format("update Events set eventText = '%'").s(QuoteSql(msg)).Str();
		conn.executenonquery(sql);
		conn.close();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception in EventSetText. %").qs(er.what()));
	}
}

void ScreenshotArchive::EventSetURL(EventID id, const std::wstring& url)
{
	if(!m_options.EnableArchive())
		return;

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn)) return;
		std::wstring sql = LibCC::Format("update Events set url = '%'").s(QuoteSql(url)).Str();
		conn.executenonquery(sql);
		conn.close();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception in EventSetExtraData. %").qs(er.what()));
	}
}

void ScreenshotArchive::DeleteEvent(EventID eventID)
{
	if(!m_options.EnableArchive())
		return;

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn))
			return;
		std::wstring sql = LibCC::Format("delete from Events where [id] = %").i((int)eventID).Str();
		conn.executenonquery(sql);
		conn.close();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception in DeleteEvent. %").qs(er.what()));
	}
}

void ScreenshotArchive::DeleteScreenshot(ScreenshotID screenshotID)
{
	if(!m_options.EnableArchive())
		return;

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn))
			return;
		conn.executenonquery(LibCC::Format("delete from Events where [screenshotID] = %").i((int)screenshotID).Str());
		conn.executenonquery(LibCC::Format("delete from Screenshots where [id] = %").i((int)screenshotID).Str());
		conn.close();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception in DeleteScreenshot. %").qs(er.what()));
	}
}

void ScreenshotArchive::DeleteAll()
{
	if(!m_options.EnableArchive())
		return;

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn))
			return;
		conn.executenonquery("delete from Events");
		conn.executenonquery("delete from Screenshots");
		conn.close();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception in DeleteScreenshot. %").qs(er.what()));
	}
}

bool ScreenshotArchive::OpenDatabase(sqlite3x::sqlite3_connection& out)
{
	std::wstring path = GetDBFilename(); 
	bool openSuccessful = false;

	std::wstring DatabaseVersion = L"1";

	if(PathFileExists(path.c_str()))
	{
		try
		{
			out.open(path.c_str());
			sqlite3x::sqlite3_command cmd(out, "select [stringValue] from Settings where [Name] like 'SchemaVersion'");
			std::wstring cur = cmd.executestring16();

			if(cur == GetDatabaseSchemaVersion())
			{
				openSuccessful = true;
			}
			else
			{
				LibCC::g_pLog->Message(LibCC::Format("Database % is the wrong version (it's %, but I expected %)").qs(path)(cur)(DatabaseVersion));
				out.close();
			}
		}
		catch(sqlite3x::database_error& er)
		{
			// no big deal; the database probable doesn't exist.
			LibCC::g_pLog->Message(LibCC::Format("Database exception while opening %: %; trying to open a database.").qs(path).qs(er.what()));
		}
	}

	if(!openSuccessful)
	{
		try
		{
			out.close();
			DeleteFile(path.c_str());
			// create schema
			std::wstring createScript = LoadTextFileResource(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_SCHEMA), L"TEXT");

			std::vector<std::wstring> commands;
			LibCC::StringSplit(createScript, L";", std::back_inserter(commands));

			out.open(path.c_str());
			for(std::vector<std::wstring>::iterator it = commands.begin(); it != commands.end(); ++ it)
			{
				std::wstring q = LibCC::StringTrim(*it, L"\r\n \t");
				if(q.length())
					out.executenonquery(q);
			}

			out.executenonquery(LibCC::Format("insert into Settings ([Name], [stringValue]) values ('SchemaVersion', '%')")(GetDatabaseSchemaVersion()).Str());
			openSuccessful = true;
		}
		catch(sqlite3x::database_error& er)
		{
			// no big deal; the database probable doesn't exist.
			LibCC::g_pLog->Message(LibCC::Format("Error while setting up new database (%). Sorry, man, archiving not available.").qs(er.what()));
		}
	}

	if(!openSuccessful)
	{
		return false;
	}

	sqlite3_enable_shared_cache(1);
	return true;
}

extern "C" void Curl_md5it(unsigned char *outbuffer, const unsigned char *input);
extern "C" size_t Curl_base64_encode(const char *input, size_t size, char **str);

std::wstring ScreenshotArchive::GetDatabaseSchemaVersion()
{
	if(!m_schemaVersion.empty())
		return m_schemaVersion;

	// the schema version is just a MD5 of the original schema create script. so when the schema changes,
	// the database knows to re-create.
	unsigned char md5a[17] = {0};
	std::wstring schemaTextW = LoadTextFileResource(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_SCHEMA), L"TEXT");
	std::string schemaTextA = LibCC::ToMBCS(schemaTextW);
	Curl_md5it(md5a, (const unsigned char*)schemaTextA.c_str());
	char* encodedA;
	Curl_base64_encode((const char*)md5a, 16, &encodedA);
	std::string ret = encodedA;
	free(encodedA);
	m_schemaVersion = LibCC::ToUnicode(ret);
	return m_schemaVersion;
}
