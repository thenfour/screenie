// Compile with /D_UNICODE /DUNICODE and /Zc:wchar_t to enable unicode, both off for no unicode. Should work either way.

#include "tstring.hpp"
#include "tiostream.hpp"

using namespace std;
using namespace tstd;

int main()
{
	wstring ws(L"I'm fine.");
	tstring str = _ST("Why, hello world. How are you today?");
	tcout << str << _ST("\n");

	tcout << convert(ws) << _ST("\n");
}
