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
	return m_owner->RetrieveImage(id);
}


util::shared_ptr<Gdiplus::Bitmap> ScreenshotArchive::Screenshot::RetrieveThumbnail()
{
	Gdiplus::Bitmap* p = 0;
	if(!m_owner->ArchiveEnabled())
		return util::shared_ptr<Gdiplus::Bitmap>(p);

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!m_owner->OpenDatabase(conn))
			return util::shared_ptr<Gdiplus::Bitmap>(p);

		{// necessary for cmd to close
			sqlite3x::sqlite3_command cmd(conn, LibCC::Format("select [thumbnailData] from Screenshots where [id] = %").i((int)id).Str());
			sqlite3x::sqlite3_reader rdr = cmd.executereader();
			if(rdr.read())
			{
				std::string data = rdr.getblob(0);
				// now read that sucker into a gdiplus bitmap.
				// for some reason, using BlobStream here does not work. it will cause some crazy crash when the Gdiplus object is freed.
				// it's very strange, so for now i will just choose the path of least resistance and do something i know works.
				HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, data.size());
				void* pMem = GlobalLock(hmem);
				memcpy(pMem, data.c_str(), data.size());
				GlobalUnlock(hmem);

				IStream* pStream = 0;
				CreateStreamOnHGlobal(hmem, FALSE, &pStream);

				p = Gdiplus::Bitmap::FromStream(pStream);

				pStream->Release();
				pStream = 0;
				GlobalFree(hmem);
			}
			rdr.close();
		}
		
		conn.close();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception in ScreenshotArchive::Screenshot::RetrieveThumbnail. %").qs(er.what()));
	}
	return util::shared_ptr<Gdiplus::Bitmap>(p);
}


ScreenshotArchive::Screenshot::Screenshot(ScreenshotArchive* owner) :
	m_owner(owner)
{
}

std::vector<ScreenshotArchive::Event> ScreenshotArchive::Screenshot::RetreiveEvents()
{
	std::vector<ScreenshotArchive::Event> ret;
	if(!m_owner->ArchiveEnabled())
		return ret;

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!m_owner->OpenDatabase(conn))
			return ret;

		{// necessary for cmd to close properly
			sqlite3x::sqlite3_command cmd(conn, LibCC::Format("select [id], screenshotID, icon, eventType, destinationName, eventText, url, [date] from Events where screenshotID = %").i((int)id).Str());
			sqlite3x::sqlite3_reader rdr = cmd.executereader();
			while(rdr.read())
			{
				ret.push_back(Event());
				//fill ret.back();
				Event& pnew = ret.back();
				pnew.id = (EventID)rdr.getint(0);
				pnew.screenshotID = (ScreenshotID)rdr.getint(1);
				pnew.icon = (EventIcon)rdr.getint(2);
				pnew.type = (EventType)rdr.getint(3);
				pnew.destinationName = rdr.getstring16(4);
				pnew.messageText = rdr.getstring16(5);
				pnew.url = rdr.getstring16(6);
				pnew.date = rdr.getstring16(7);
			}
			rdr.close();
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
	m_pNotify(0),
	m_enableArchive(true)
{
}

ScreenshotArchive::~ScreenshotArchive()
{
}

std::vector<ScreenshotArchive::Screenshot> ScreenshotArchive::RetreiveScreenshots()
{
	std::vector<ScreenshotArchive::Screenshot> ret;
	if(!ArchiveEnabled())
		return ret;

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn))
			return ret;
		{// necessary for cmd to close before conn.close()
			sqlite3x::sqlite3_command cmd(conn, "select [id], [width], [height] from Screenshots");
			sqlite3x::sqlite3_reader rdr = cmd.executereader();
			while(rdr.read())
			{
				ret.push_back(Screenshot(this));
				//fill ret.back();
				ret.back().id = (ScreenshotID)rdr.getint(0);
				ret.back().width = rdr.getint(1);
				ret.back().height = rdr.getint(2);
			}
			rdr.close();
		}
		conn.close();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception in EventSetIcon. %").qs(er.what()));
	}
	return ret;
}

util::shared_ptr<Gdiplus::Bitmap> ScreenshotArchive::RetrieveImage(ScreenshotID id)
{
	Gdiplus::Bitmap* p = 0;
	if(!ArchiveEnabled())
		return util::shared_ptr<Gdiplus::Bitmap>(p);

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn))
			return util::shared_ptr<Gdiplus::Bitmap>(p);

		{// necessary for cmd to close
			sqlite3x::sqlite3_command cmd(conn, LibCC::Format("select [bitmapData] from Screenshots where [id] = %").i((int)id).Str());

			sqlite3x::sqlite3_reader rdr = cmd.executereader();
			if(rdr.read())
			{
				std::string data = rdr.getblob(0);
				// now read that sucker into a gdiplus bitmap.
				// for some reason, using BlobStream here does not work. it will cause some crazy crash when the Gdiplus object is freed.
				// it's very strange, so for now i will just choose the path of least resistance and do something i know works.
				HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, data.size());
				void* pMem = GlobalLock(hmem);
				memcpy(pMem, data.c_str(), data.size());
				GlobalUnlock(hmem);

				IStream* pStream = 0;
				CreateStreamOnHGlobal(hmem, FALSE, &pStream);

				p = Gdiplus::Bitmap::FromStream(pStream);

				pStream->Release();
				pStream = 0;
				GlobalFree(hmem);

			}
			rdr.close();
		}
		
		conn.close();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception in ScreenshotArchive::Screenshot::RetrieveImage. %").qs(er.what()));
	}
	return util::shared_ptr<Gdiplus::Bitmap>(p);
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


ScreenshotID ScreenshotArchive::RegisterScreenshot(Gdiplus::BitmapPtr bmp, Gdiplus::BitmapPtr thumbnail)
{
	if(!ArchiveEnabled())
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

	// do the same for thumbnail
	BlobStream streamThumb;
	status = thumbnail->Save(&streamThumb, &codecInfo->Clsid);
	if(status != Gdiplus::Ok)
		return 0;

	try
	{
		// make sure there is space in the db file.
		if(!EnsureDatabaseHasSpace(stream.GetLength() + streamThumb.GetLength()))
			return 0;

		// and save to the database.
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn))
			return 0;

		{// necessary for cmd to close up properly
			sqlite3x::sqlite3_command cmd(conn, LibCC::Format("insert into Screenshots ([bitmapData], [thumbnailData], [width], [height], [date]) "
				"values (?, ?, %, %, strftime('^%Y-^%m-^%dT^%H:^%M:^%f', 'now'))").i(bmp->GetWidth()).i(bmp->GetHeight()).CStr());
			cmd.bind(1, stream.GetBuffer(), stream.GetLength());
			cmd.bind(2, streamThumb.GetBuffer(), streamThumb.GetLength());

			cmd.executenonquery();
			ret = (ScreenshotID)conn.insertid();
		}
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
	if(!ArchiveEnabled())
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
	if(!ArchiveEnabled())
		return;

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn)) return;

		std::wstring sql = LibCC::Format("update Events set icon = % where id=%").i((int)icon).i((int)id).Str();

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
	if(!ArchiveEnabled())
		return;

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn)) return;
		std::wstring sql = LibCC::Format("update Events set eventText = '%' where id=%").s(QuoteSql(msg)).i((int)id).Str();
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
	if(!ArchiveEnabled())
		return;

	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn)) return;
		std::wstring sql = LibCC::Format("update Events set url = '%' where id=%").s(QuoteSql(url)).i((int)id).Str();
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
	if(!ArchiveEnabled())
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
	if(!ArchiveEnabled())
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
	if(!ArchiveEnabled())
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
	m_enableArchive = false;
	bool openSuccessful = false;
	bool wrongVersion = false;

	if(PathFileExists(path.c_str()))
	{
		try
		{
			out.open(path.c_str());
			std::wstring cur;

			{// necessary for cmd to cleanup properly
				sqlite3x::sqlite3_command cmd(out, "select [stringValue] from Settings where [Name] like 'SchemaVersion'");
				cur = cmd.executestring16();
			}

			if(cur == GetDatabaseSchemaVersion())
			{
				openSuccessful = true;
			}
			else
			{
				out.close();
				if(IDCANCEL == MessageBox(0, L"The history database format has changed since the last time you ran Screenie. Click OK to delete the existing history database and start from scratch, or click Cancel to disable saving history for now.", L"Screenie", MB_ICONINFORMATION | MB_OKCANCEL))
				{
					return false;
				}
				LibCC::g_pLog->Message(LibCC::Format("Database % is the wrong version (it's %, but I expected %)").qs(path)(cur)(GetDatabaseSchemaVersion()));
				wrongVersion = true;
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
			if(0 == DeleteFile(path.c_str()))
			{
				if(wrongVersion && (GetLastError() == ERROR_SHARING_VIOLATION))
				{
					while(true)
					{
						if(IDCANCEL == MessageBox(0, LibCC::Format("Screenie cannot delete the archive at % because it is in use by another program. Please close any program that is using the file and click Retry. Otherwise, click Cancel to continue without history support.")(path).CStr(),L"Screenie", MB_ICONERROR | MB_RETRYCANCEL))
						{
							return false;
						}
						if(0 != DeleteFile(path.c_str()))
							break;// success.					
					}
				}
			}
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
	m_enableArchive = true;
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
