//
//
//
//
//

#ifndef SCREENIE_DESTINATION_HPP
#define SCREENIE_DESTINATION_HPP

struct ScreenshotDestination
{
	enum Type
	{
		TYPE_NONE,
		TYPE_FILE,
		TYPE_FTP,
		TYPE_EMAIL,
		TYPE_CLIPBOARD,
	};

	enum ScaleType
	{
		SCALE_NONE,
		SCALE_SCALETOPERCENT,
		SCALE_LIMITDIMENSIONS
	};

	struct General
	{
		General() { }
		General(const General& copy) { operator=(copy); }
		~General() { }

		General& operator=(const General& rightHand)
		{
			type = rightHand.type;
			name = rightHand.name;
			imageFormat = rightHand.imageFormat;
			filenameFormat = rightHand.filenameFormat;
			path = rightHand.path;

			return (*this);
		}

		Type type;
		tstd::tstring name;
		tstd::tstring imageFormat;
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
			createThumbnail = rightHand.createThumbnail;
			useFilenameFormat = rightHand.useFilenameFormat;
			filenameFormat = rightHand.filenameFormat;
			thumbScaleType = rightHand.thumbScaleType;
			thumbScalePercent = rightHand.thumbScalePercent;
			thumbMaxDimension = rightHand.thumbMaxDimension;

			return (*this);
		}

		ScaleType scaleType;
		int scalePercent;
		int maxDimension;

		bool createThumbnail;
		bool useFilenameFormat;
		tstd::tstring filenameFormat;
		ScaleType thumbScaleType;
		int thumbScalePercent;
		int thumbMaxDimension;
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
			password = rightHand.password;
			remotePath = rightHand.remotePath;
			resultURL = rightHand.resultURL;
			copyURL = rightHand.copyURL;

			return (*this);
		}

		tstd::tstring hostname;
		unsigned short port;
		tstd::tstring username;
		tstd::tstring password;
		tstd::tstring remotePath;

		tstd::tstring resultURL;
		bool copyURL;
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

	bool enabled;
	General general;
	Image image;
	Ftp ftp;
	Email email;

	static tstd::tstring TypeToString(Type type)
	{
		switch (type)
		{
			case TYPE_FILE:
				return tstd::tstring("Save to Local File");
			case TYPE_FTP:
				return tstd::tstring("Upload via FTP");
			case TYPE_EMAIL:
				return tstd::tstring("Send Email");
			case TYPE_CLIPBOARD:
				return tstd::tstring("Copy to Clipboard");
		}

		return tstd::tstring("Unknown");
	};

	static Type StringToType(const tstd::tstring& description)
	{
		if (description == tstd::tstring("Save to Local File"))
			return TYPE_FILE;
		if (description == tstd::tstring("Upload via FTP"))
			return TYPE_FTP;
		if (description == tstd::tstring("Send Email"))
			return TYPE_EMAIL;
		if (description == tstd::tstring("Copy to Clipboard"))
			return TYPE_CLIPBOARD;

		return TYPE_NONE;
	}
};

#endif