//
//
//
//
//

#ifndef SCREENIE_DESTINATION_HPP
#define SCREENIE_DESTINATION_HPP

#include "utility.hpp"
#include "serialization.h"

struct ScreenshotDestination
{
	enum Type
	{
		TYPE_NONE,
		TYPE_FILE,
		TYPE_FTP,
		TYPE_EMAIL,
		TYPE_CLIPBOARD,
		TYPE_SCREENIENET
	};

	enum ScaleType
	{
		SCALE_NONE,
		SCALE_SCALETOPERCENT,
		SCALE_LIMITDIMENSIONS
	};

	struct General
	{
    General() :
      localTime(true)
    {
      id.CreateNew();
    }
		General(const General& copy) { operator=(copy); }
		~General() { }

		General& operator=(const General& rightHand)
		{
      id = rightHand.id;
			type = rightHand.type;
			name = rightHand.name;
			imageFormat = rightHand.imageFormat;
			filenameFormat = rightHand.filenameFormat;
			path = rightHand.path;
      localTime = rightHand.localTime;

			return (*this);
		}

    Guid id;// unique id for this.

		Type type;
		tstd::tstring name;
		tstd::tstring imageFormat;
    bool localTime;
		tstd::tstring filenameFormat;
		tstd::tstring path;
	};

	struct Image
	{
		Image() { }
		Image(const Image& copy) { operator=(copy); }
		~Image() { }

		Image& operator=(const Image& rightHand)
		{
			scaleType = rightHand.scaleType;
			scalePercent = rightHand.scalePercent;
			maxDimension = rightHand.maxDimension;

			return (*this);
		}

		ScaleType scaleType;
		int scalePercent;
		int maxDimension;
	};

	struct Screenienet
	{
		Screenienet() : copyURL(false) { }

		tstd::tstring url;
		tstd::tstring username;
		tstd::tstring password;
		bool copyURL;
	};

	struct Ftp
	{
    Ftp() { }
		Ftp(const Ftp& copy) { operator=(copy); }
		~Ftp() { }

		Ftp& operator=(const Ftp& rightHand)
		{
			hostname = rightHand.hostname;
			port = rightHand.port;
			username = rightHand.username;
			remotePath = rightHand.remotePath;
			resultURL = rightHand.resultURL;
			copyURL = rightHand.copyURL;

      m_passwordEncrypted.Assign(rightHand.m_passwordEncrypted);

			return (*this);
		}

		tstd::tstring hostname;
		unsigned short port;
		tstd::tstring username;
		tstd::tstring remotePath;

		tstd::tstring resultURL;
		bool copyURL;

    tstd::tstring DecryptPassword() const
    {
      tstd::tstring ret;
      DATA_BLOB in;
      DATA_BLOB out;
      in.pbData = const_cast<BYTE*>(m_passwordEncrypted.GetBuffer());
      in.cbData = m_passwordEncrypted.Size();
      if(FALSE == CryptUnprotectData(&in, NULL, NULL, NULL, 0, 0, &out))
      {
				LibCC::g_pLog->Message(LibCC::Format("DecryptPassword error. Blob length: %").ul(in.cbData));
        return _T("");
      }

      ret = reinterpret_cast<PCTSTR>(out.pbData);
      LocalFree(out.pbData);

			LibCC::g_pLog->Message(LibCC::Format("DecryptPassword returning %. Blob length: %").qs(ret).ul(in.cbData));
      return ret;
    }

    const LibCC::Blob<BYTE>& GetEncryptedPassword() const
    {
      return m_passwordEncrypted;
    }

		void SetEncryptedPassword(LibCC::Blob<BYTE>& crap)
    {
			LibCC::g_pLog->Message(LibCC::Format("SetEncryptedPassword; length %").ul(crap.Size()));
      m_passwordEncrypted.Assign(crap);
    }

    void SetPassword(const tstd::tstring& pass)
    {
      DATA_BLOB in;
      DATA_BLOB out;
      in.pbData = const_cast<BYTE*>(reinterpret_cast<const BYTE*>(pass.c_str()));
      in.cbData = sizeof(TCHAR) * (pass.size() + 1);
      if(FALSE == CryptProtectData(&in, NULL, NULL, NULL, 0, 0, &out))
      {
				LibCC::g_pLog->Message("SetPassword error");
        // omg error;
        return;
      }
      m_passwordEncrypted.Alloc(out.cbData);
			LibCC::g_pLog->Message(LibCC::Format("SetPassword %; encrypted length %").qs(pass).ul(out.cbData));
      memcpy(m_passwordEncrypted.GetBuffer(), out.pbData, out.cbData);
      LocalFree(out.pbData);
    }

  private:
    LibCC::Blob<BYTE> m_passwordEncrypted;
	};

	struct Email
	{
		Email() { }
		Email(const Email& copy) { operator=(copy); }
		~Email() { }

		Email& operator=(const Email& rightHand)
		{
			return (*this);
		}
	};

	ScreenshotDestination() { }
	ScreenshotDestination(const ScreenshotDestination& copy) { operator=(copy); }
	~ScreenshotDestination() { }

	ScreenshotDestination& operator=(const ScreenshotDestination& rightHand)
	{
		enabled = rightHand.enabled;
		general = rightHand.general;
		image = rightHand.image;
		ftp = rightHand.ftp;

		return (*this);
	}

	// xml serialization
	void Serialize(Xml::Element parent) const
	{
		Xml::Serialize(parent, L"Enabled", enabled);
		// general settings
		Xml::Serialize(parent, L"GeneralName", general.name);
		Xml::Serialize(parent, L"GeneralType", (int)general.type);
		Xml::Serialize(parent, L"GeneralImageFormat", general.imageFormat);
		Xml::Serialize(parent, L"GeneralFilenameFormat", general.filenameFormat);
		Xml::Serialize(parent, L"GeneralPath", general.path);
		Xml::Serialize(parent, L"GeneralLocalTime", general.localTime);
		Xml::Serialize(parent, L"GeneralID", general.id.ToString());

		// image settings
		Xml::Serialize(parent, L"ImageScaleType", (int)image.scaleType);
		Xml::Serialize(parent, L"ImageScalePercent", image.scalePercent);
		Xml::Serialize(parent, L"ImageMaxDimension", image.maxDimension);

		// ftp settings
		Xml::Serialize(parent, L"FtpHostName", ftp.hostname);
		Xml::Serialize(parent, L"FtpPort", ftp.port);
		Xml::Serialize(parent, L"FtpUsername", ftp.username);
		Xml::Serialize(parent, L"FtpRemotePath",  ftp.remotePath);
		Xml::Serialize(parent, L"FtpResultURL", ftp.resultURL);
		Xml::Serialize(parent, L"FtpCopyURL", ftp.copyURL);

		//((ScreenshotDestination*)this)->ftp.SetPassword(L"c7b9b13");
		const LibCC::Blob<BYTE>& temp = ftp.GetEncryptedPassword();
		Xml::Serialize(parent, L"FtpPassword", Xml::BinaryData(temp.GetBuffer(), temp.Size()));

		// screenie.net settings
		Xml::Serialize(parent, L"ScreenieNetURL", screenie.url);
		Xml::Serialize(parent, L"ScreenieNetUserName", screenie.username);
		Xml::Serialize(parent, L"ScreenieNetPassword", screenie.password);
		Xml::Serialize(parent, L"ScreenieNetCopyURL", screenie.copyURL);
	}

	void Deserialize(Xml::Element parent)
	{
		Xml::Deserialize(parent, L"Enabled", enabled);
		//DWORD temp;
		std::wstring strTemp;

		// general settings
		Xml::Deserialize(parent, L"GeneralName", general.name);
		Xml::Deserialize(parent, L"GeneralType", (int&)general.type);

		Xml::Deserialize(parent, L"GeneralImageFormat", general.imageFormat);
		Xml::Deserialize(parent, L"GeneralFilenameFormat", general.filenameFormat);
		Xml::Deserialize(parent, L"GeneralPath", general.path);
		Xml::Deserialize(parent, L"GeneralLocalTime", general.localTime);
		Xml::Deserialize(parent, L"GeneralID", strTemp);
		general.id.Assign(strTemp);

		// image settings
		Xml::Deserialize(parent, L"ImageScaleType", (int&)image.scaleType);
		Xml::Deserialize(parent, L"ImageScalePercent", image.scalePercent);
		Xml::Deserialize(parent, L"ImageMaxDimension", image.maxDimension);

		// ftp settings
		Xml::Deserialize(parent, L"FtpHostName", ftp.hostname);
		Xml::Deserialize(parent, L"FtpPort", ftp.port);
		Xml::Deserialize(parent, L"FtpUsername", ftp.username);
		Xml::Deserialize(parent, L"FtpRemotePath",  ftp.remotePath);
		Xml::Deserialize(parent, L"FtpResultURL", ftp.resultURL);
		Xml::Deserialize(parent, L"FtpCopyURL", ftp.copyURL);

		LibCC::Blob<BYTE> binaryTemp;
		Xml::Deserialize(parent, L"FtpPassword", binaryTemp);
		ftp.SetEncryptedPassword(binaryTemp);
		//std::wstring pass = ftp.DecryptPassword();

		// screenie.net settings
		Xml::Deserialize(parent, L"ScreenieNetURL", screenie.url);
		Xml::Deserialize(parent, L"ScreenieNetUserName", screenie.username);
		Xml::Deserialize(parent, L"ScreenieNetPassword", screenie.password);
		Xml::Deserialize(parent, L"ScreenieNetCopyURL", screenie.copyURL);
	}

	bool enabled;
	General general;
	Image image;
	Ftp ftp;
	Email email;
	Screenienet screenie;

  void GetNowBasedOnTimeSettings(SYSTEMTIME& st)
  {
    if(general.localTime)
    {
	    ::GetLocalTime(&st);
    }
    else
    {
	    ::GetSystemTime(&st);
    }
  }

	static tstd::tstring TypeToString(Type type)
	{
		switch (type)
		{
			case TYPE_FILE:
				return tstd::tstring(_T("Save to Local File"));
			case TYPE_FTP:
				return tstd::tstring(_T("Upload via FTP"));
			case TYPE_EMAIL:
				return tstd::tstring(_T("Send Email"));
			case TYPE_CLIPBOARD:
				return tstd::tstring(_T("Copy to Clipboard"));
			case TYPE_SCREENIENET:
				return tstd::tstring(_T("Screenie.net"));
		}

		return tstd::tstring(_T("Unknown"));
	};

	static Type StringToType(const tstd::tstring& description)
	{
		if (description == tstd::tstring(_T("Save to Local File")))
			return TYPE_FILE;
		if (description == tstd::tstring(_T("Upload via FTP")))
			return TYPE_FTP;
		if (description == tstd::tstring(_T("Send Email")))
			return TYPE_EMAIL;
		if (description == tstd::tstring(_T("Copy to Clipboard")))
			return TYPE_CLIPBOARD;
		if (description == tstd::tstring(_T("Screenie.net")))
			return TYPE_SCREENIENET;

		return TYPE_NONE;
	}
};

#endif