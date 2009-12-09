// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//
//
//
//
//

#ifndef SCREENIE_DESTINATION_HPP
#define SCREENIE_DESTINATION_HPP

#include "utility.hpp"
#include "serialization.h"
#include "libcc\winapi.hpp"
#include "codec.hpp"


struct ScreenshotDestination
{
	enum Type
	{
		TYPE_NONE = 0,
		TYPE_FILE = 1,
		TYPE_FTP = 2,
		TYPE_CLIPBOARD = 4,
		TYPE_IMAGESHACK = 5
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
			// defaults
      localTime(true),
			imageQuality(80)
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
			imageQuality = rightHand.imageQuality;

			return (*this);
		}

    Guid id;// unique id for this.

		Type type;
		tstd::tstring name;
		tstd::tstring imageFormat;
    bool localTime;
		tstd::tstring filenameFormat;
		tstd::tstring path;
		int imageQuality;
	};

	struct Image
	{
		ScaleType scaleType;
		int scalePercent;
		int maxDimension;
	};

	//struct Screenienet
	//{
	//	Screenienet() : copyURL(false) { }

	//	tstd::tstring url;
	//	tstd::tstring username;
	//	tstd::tstring password;
	//	bool copyURL;
	//};

	struct Ftp
	{
		Ftp() :
			passwordOptions(PO_Protected)
		{
		}
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
			shortenURL = rightHand.shortenURL;
			passwordOptions = rightHand.passwordOptions;

      m_passwordEncrypted.Assign(rightHand.m_passwordEncrypted);

			return (*this);
		}

		enum PasswordOptions
		{
			PO_Plaintext = 0,
			PO_XOR = 1,
			PO_Protected = 2
		} passwordOptions;

		tstd::tstring hostname;
		unsigned short port;
		tstd::tstring username;
		tstd::tstring remotePath;

		tstd::tstring resultURL;
		bool copyURL;
		bool shortenURL;

    tstd::tstring DecryptPassword() const
    {
      tstd::tstring ret;
      DATA_BLOB in;
      DATA_BLOB out;
      in.pbData = const_cast<BYTE*>(m_passwordEncrypted.GetBuffer());
      in.cbData = m_passwordEncrypted.Size();
      if(FALSE == CryptUnprotectData(&in, NULL, NULL, NULL, 0, 0, &out))
      {
        return _T("");
      }

      ret = reinterpret_cast<PCTSTR>(out.pbData);
      LocalFree(out.pbData);

      return ret;
    }

    void SetPassword(const tstd::tstring& pass)
    {
      DATA_BLOB in;
      DATA_BLOB out;
      in.pbData = const_cast<BYTE*>(reinterpret_cast<const BYTE*>(pass.c_str()));
      in.cbData = sizeof(TCHAR) * (pass.size() + 1);
      if(FALSE == CryptProtectData(&in, NULL, NULL, NULL, 0, 0, &out))
      {
        // omg error;
        return;
      }
      m_passwordEncrypted.Alloc(out.cbData);
			//LibCC::g_pLog->Message(LibCC::Format("SetPassword %; encrypted length %").qs(pass).ul(out.cbData));
      memcpy(m_passwordEncrypted.GetBuffer(), out.pbData, out.cbData);
      LocalFree(out.pbData);
    }


    const LibCC::Blob<BYTE>& SerializePasswordEncrypted() const
    {
      return m_passwordEncrypted;
    }

		static const wchar_t xorKey[];// defined in screenshotoptions.cpp
		static const int xorKeyLength;

		const void SerializePasswordXOR(LibCC::Blob<BYTE>& out) const
    {
			//LibCC::LogScopeMessage x("Serialize Password");

			// assume 16-bit chars & 8-bit BYTES
			std::wstring pass = DecryptPassword();
			out.Alloc(pass.length() * 2);
			for(size_t i = 0; i < pass.length(); ++ i)
			{
				wchar_t n = pass[i] ^ xorKey[i % xorKeyLength];
				//LibCC::g_pLog->Message(LibCC::Format("i=%")(i));
				//LibCC::g_pLog->Message(LibCC::Format("  pass[i]=%").c(pass[i]));
				//LibCC::g_pLog->Message(LibCC::Format("  xorkey index = %").i(i % xorKeyLength));
				//LibCC::g_pLog->Message(LibCC::Format("  xorkey val = %").i<16>(xorKey[i % xorKeyLength]));
				//LibCC::g_pLog->Message(LibCC::Format("  result = % { % % }").i<16>(n).i<16>((BYTE)(n & 0xFF)).i<16>((BYTE)((n >> 8) & 0xFF)));
				out[i*2] = (BYTE)(n & 0xFF);
				out[i*2+1] = (BYTE)((n >> 8) & 0xFF);
			}
    }

		void DeserializeXORPassword(LibCC::Blob<BYTE>& crap)
    {
			//LibCC::LogScopeMessage x("Deserialize Password");

			std::wstring pass;
			for(size_t i = 0; i < crap.Size() / 2; ++ i)
			{
				wchar_t n = (wchar_t)crap[i*2] | (crap[i*2+1] << 8);
				n ^= xorKey[i % xorKeyLength];
				pass.push_back(n);
			}
      SetPassword(pass);
    }

		void DeserializePassword(LibCC::Blob<BYTE>& crap)
    {
      m_passwordEncrypted.Assign(crap);
    }

  private:
    LibCC::Blob<BYTE> m_passwordEncrypted;
	};

	struct ImageShack
	{
		ImageShack() : copyURL(true), shortenURL(true) { }

		bool copyURL;
		bool shortenURL;
	};

	std::wstring GetFormatInfo() const
	{
		ImageCodecsEnum codecs;
		Gdiplus::ImageCodecInfo* p = codecs.GetCodecByMimeType(general.imageFormat.c_str());
		bool qualityCounts = ImageCodecsEnum::SupportsQualitySetting(p);

		if(qualityCounts)
		{
			return LibCC::Format(L"% (%^%)")(p->FormatDescription)(general.imageQuality).Str();
		}

		return p->FormatDescription;
	}

	std::wstring GetGeneralInfo() const
	{
		switch(general.type)
		{
		case TYPE_CLIPBOARD:
			return L"Clipboard";
		case TYPE_FILE:
			return LibCC::Format(L"% - %")
				(GetFormatInfo())
				(LibCC::PathAppendX(general.path, general.filenameFormat)).Str();
			break;
		case TYPE_FTP:
			if(!ftp.resultURL.empty())
			{
				return LibCC::Format(L"% - %")
					(GetFormatInfo())
					(LibCC::PathAppendX(ftp.resultURL, general.filenameFormat))
					.Str();
			}
			if(ftp.port != 80)
			{
				return LibCC::Format(L"% - %:%/%")
					(GetFormatInfo())
					(ftp.hostname)
					(ftp.port)
					(LibCC::PathAppendX(ftp.remotePath, general.filenameFormat))
					.Str();
			}
			return LibCC::Format(L"% - %/%")
				(GetFormatInfo())
				(ftp.hostname)
				(LibCC::PathAppendX(ftp.remotePath, general.filenameFormat))
				.Str();
			break;
		case TYPE_IMAGESHACK:
			return GetFormatInfo();
			break;
		}
		return L"Unknown";
	}

	// xml serialization
	void Serialize(Xml::Element parent) const
	{
		Xml::Serialize(parent, L"Enabled", enabled);
		// general settings
		Xml::Serialize(parent, L"GeneralName", general.name);
		Xml::Serialize(parent, L"GeneralType", (int)general.type);
		Xml::Serialize(parent, L"GeneralImageFormat", general.imageFormat);
		Xml::Serialize(parent, L"GeneralImageQuality", general.imageQuality);
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
		Xml::Serialize(parent, L"FtpShortenURL", ftp.shortenURL);
		Xml::Serialize(parent, L"FtpPasswordOptions", (int)ftp.passwordOptions);

		switch(ftp.passwordOptions)
		{
		case Ftp::PO_Plaintext:
			Xml::Serialize(parent, L"FtpPassword", ftp.DecryptPassword());
			break;
		case Ftp::PO_XOR:
			{
				LibCC::Blob<BYTE> temp;
				ftp.SerializePasswordXOR(temp);
				Xml::Serialize(parent, L"FtpPassword", Xml::BinaryData(temp.GetBuffer(), temp.Size()));
				break;
			}
		case Ftp::PO_Protected:
			{
				const LibCC::Blob<BYTE>& temp = ftp.SerializePasswordEncrypted();
				Xml::Serialize(parent, L"FtpPassword", Xml::BinaryData(temp.GetBuffer(), temp.Size()));
				break;
			}
		default:
			break;
		}

		// imageshack settings
		Xml::Serialize(parent, L"ImageShackCopyURL", imageshack.copyURL);
		Xml::Serialize(parent, L"ImageShackShortenURL", imageshack.shortenURL);

		// screenie.net settings
		//Xml::Serialize(parent, L"ScreenieNetURL", screenie.url);
		//Xml::Serialize(parent, L"ScreenieNetUserName", screenie.username);
		//Xml::Serialize(parent, L"ScreenieNetPassword", screenie.password);
		//Xml::Serialize(parent, L"ScreenieNetCopyURL", screenie.copyURL);
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
		Xml::Deserialize(parent, L"GeneralImageQuality", general.imageQuality);
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
		Xml::Deserialize(parent, L"FtpShortenURL", ftp.shortenURL);
		Xml::Deserialize(parent, L"FtpPasswordOptions", (int&)ftp.passwordOptions);

		switch(ftp.passwordOptions)
		{
		case Ftp::PO_Plaintext:
			{
				std::wstring str;
				Xml::Deserialize(parent, L"FtpPassword", str);
				ftp.SetPassword(str);
				break;
			}
		case Ftp::PO_XOR:
			{
				LibCC::Blob<BYTE> binaryTemp;
				Xml::Deserialize(parent, L"FtpPassword", binaryTemp);
				ftp.DeserializeXORPassword(binaryTemp);
				break;
			}
		case Ftp::PO_Protected:
			{
				LibCC::Blob<BYTE> binaryTemp;
				Xml::Deserialize(parent, L"FtpPassword", binaryTemp);
				ftp.DeserializePassword(binaryTemp);
				break;
			}
			break;
		default:
			ftp.SetPassword(L"");
			break;
		}

		//LibCC::Blob<BYTE> binaryTemp;
		//Xml::Deserialize(parent, L"FtpPassword", binaryTemp);
		//ftp.DeserializePassword(binaryTemp);

		// imageshack settings
		Xml::Deserialize(parent, L"ImageShackCopyURL", imageshack.copyURL);
		Xml::Deserialize(parent, L"ImageShackShortenURL", imageshack.shortenURL);

		// screenie.net settings
		//Xml::Deserialize(parent, L"ScreenieNetURL", screenie.url);
		//Xml::Deserialize(parent, L"ScreenieNetUserName", screenie.username);
		//Xml::Deserialize(parent, L"ScreenieNetPassword", screenie.password);
		//Xml::Deserialize(parent, L"ScreenieNetCopyURL", screenie.copyURL);
	}

	bool enabled;
	General general;
	Image image;
	Ftp ftp;
	ImageShack imageshack;
//	Screenienet screenie;

  void GetNowBasedOnTimeSettings(SYSTEMTIME& st)
  {
	  // now the time is already local, so just do nothing
	  // unless local time is turned off

	  if (!general.localTime)
	  {
		  SYSTEMTIME out;
		  ::TzSpecificLocalTimeToSystemTime(NULL, &st, &out);

		  st = out;
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
			case TYPE_CLIPBOARD:
				return tstd::tstring(_T("Copy to Clipboard"));
			case TYPE_IMAGESHACK:
				return tstd::tstring(_T("ImageShack"));
		}

		return tstd::tstring(_T("Unknown"));
	};

	static Type StringToType(const tstd::tstring& description)
	{
		if (description == tstd::tstring(_T("Save to Local File")))
			return TYPE_FILE;
		if (description == tstd::tstring(_T("Upload via FTP")))
			return TYPE_FTP;
		if (description == tstd::tstring(_T("Copy to Clipboard")))
			return TYPE_CLIPBOARD;
		if (description == tstd::tstring(_T("ImageShack")))
			return TYPE_IMAGESHACK;

		return TYPE_NONE;
	}
};

#endif