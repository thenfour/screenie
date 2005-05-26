//
// path.hpp - some convenient crossplatform path-splitting utilities
// Copyright (c) 2005 Roger Clark
//

#ifndef LIBLOL_PATH_HPP
#define LIBLOL_PATH_HPP

// this class splits a path into three components: a "directory,"
// a "filename," and an "extension." here's what the output looks like
// for a given input. (it works with both windows/dos and unix paths.)
//
// input: c:\
// output:
//    directory = c:\
//    filename = <empty>
//    extension = <empty>
//
// input: c:\lol
// output:
//    directory = c:\
//    filename = lol
//    extension = <empty>
//
// input: c:\lol\wtc.
// output:
//    directory = c:\lol
//    filename = wtc.
//    extension = <empty>
//
// input: c:\lol\wtc.txt
// output:
//    directory = c:\lol
//    filename = wtc
//    extension = txt
//
// input: lol\wtc.txt
// output:
//    directory = lol
//    filename = wtc
//    extension = txt
//
// input: c:\
// output:
//    directory = c:\
//    filename = <empty>
//    extension = <empty>
//
// input: /home/username/.bash_profile
// output:
//    directory = /home/username
//    filename = .bash_profile
//    extension = <empty>

struct Path
{
	Path() { }
	Path(const tstd::tchar_t* pathToParse) { operator=(Split(pathToParse)); }
	Path(const tstd::tstring& pathToParse) { operator=(Split(pathToParse)); }
	Path(const Path& copy) { operator=(copy); }
	~Path() { }

	Path& operator=(const Path& rightHand)
	{
		path = rightHand.path;

		directory = rightHand.directory;
		filename = rightHand.filename;
		extension = rightHand.extension;

		return (*this);
	}

	// the next three routines exemplify the kind of disgusting,
	// character-by-character string parsing that a nice regular expression
	// library could do away with. nobody can possibly like doing this stuff.

	static tstd::tstring GetDirectory(const tstd::tstring& pathToParse, bool trailingSlash = false)
	{
		tstd::tstring::size_type slashPos = pathToParse.find_last_of(_TT("\\/"));

		if (slashPos == tstd::tstring::npos)
			return tstd::tstring();

		return pathToParse.substr(0, trailingSlash ? (slashPos + 1) : slashPos);
	}

	// TODO: add an argument to this function that allows you to keep the extension
	static tstd::tstring GetFilename(const tstd::tstring& pathToParse)
	{
		tstd::tstring::size_type slashPos = pathToParse.find_last_of(_TT("\\/"));

		if (slashPos == tstd::tstring::npos)
		{
			tstd::tstring::size_type dotPos = pathToParse.rfind(_TT('.'));

			if (dotPos != tstd::tstring::npos)
			{
				// if the dot is the first character in the filename,
				// it's probably a configuration file. return the whole thing.

				if (dotPos == 0)
					return pathToParse;

				return pathToParse.substr(0, dotPos);
			}
			
			return pathToParse;
		}

		tstd::tstring::size_type dotPos = pathToParse.rfind(_TT('.'));

		if (dotPos != tstd::tstring::npos)
		{
			if (dotPos == (slashPos + 1))
				return pathToParse.substr(slashPos + 1, pathToParse.length() - slashPos - 1);

			return pathToParse.substr(slashPos + 1, dotPos - slashPos - 1);
		}

		return pathToParse.substr(slashPos + 1, pathToParse.length() - slashPos - 1);
	}

	static tstd::tstring GetExtension(const tstd::tstring& pathToParse, bool dotPrefix = false)
	{
		tstd::tstring::size_type dotPos = pathToParse.rfind(_TT('.'));
		tstd::tstring::size_type slashPos = pathToParse.find_last_of(_TT("\\/"));

		if (dotPos == tstd::tstring::npos)
			return tstd::tstring();

		// check and see if there is a directory part to the filename
		if (slashPos != tstd::tstring::npos)
		{
			// if there is, check and see if the dot we found is the first
			// character if the filename. if it is, it's probably a unix
			// configuration file, and we shouldn't treat it as an extension.

			if (dotPos == (slashPos + 1))
				return tstd::tstring();
		}

		// see above
		if (dotPos == 0)
			return tstd::tstring();

		if (dotPrefix)
			pathToParse.substr(dotPos, pathToParse.length() - dotPos - 1);

		return pathToParse.substr(dotPos + 1, pathToParse.length() - dotPos);
	}

	static Path Split(const tstd::tstring& pathToParse)
	{
		Path splitPath;

		splitPath.path = pathToParse;

		splitPath.directory = GetDirectory(pathToParse, true);
		splitPath.filename = GetFilename(pathToParse);
		splitPath.extension = GetExtension(pathToParse);

		return splitPath;
	}

	tstd::tstring path;

	tstd::tstring directory;
	tstd::tstring filename;
	tstd::tstring extension;
};

#endif