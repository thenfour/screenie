

#include "StdAfx.h"
#include "xmlutil.h"

using namespace LibCC;

Result CreateXMLHttpRequest(MSXML::IXMLHttpRequestPtr& p)
{
	Result r;

	try
	{
		HRESULT hr;
		if(FAILED(hr = p.CreateInstance("msxml2.xmlhttp")))
		{
			r.Fail(Format("Failed to create instance of msxml2.xmlhttp.  Error code: %").ul(hr));
		}
		else
		{
			r.Succeed();
		}
	}
	catch(_com_error& e)
	{
		r.Fail(Format("Error creating msxml2.xmlhttp. % %").s(e.ErrorMessage()).s(e.Description()));
	}

	return r;
}



Result XMLToString(string& s, MSXML::IXMLDOMNodePtr p)
{
	Result r;

	try
	{
		s = (PCTSTR)p->xml;
	}
	catch(_com_error& e)
	{
		r.Fail(Format("Error extracting XML. % %").s(e.ErrorMessage()).s(e.Description()));
	}

	return r;
}

Result StringToXML(MSXML::IXMLDOMDocumentPtr& p, const string& s)
{
	Result r;

	try
	{
		HRESULT hr;
		if(FAILED(hr = p.CreateInstance("msxml.domdocument")))
		{
			r.Fail(Format("Failed to create instance of msxml.domdocument.  Error code: %").ul(hr));
		}
		else
		{
      VARIANT_BOOL b = p->loadXML(s.c_str());
			if(b == VARIANT_FALSE)
			{
				MSXML::IXMLDOMParseErrorPtr pErr = p->GetparseError();
				if(pErr == NULL)
				{
					r.Fail(_T("Failed to load XML into msxml.domdocument.  Could not get IXMLDOMParseErrorPtr for details."));
				}
				else
				{
					r.Fail(Format("Failed to parse xml.  %").s(pErr->reason));
				}
			}
			else
			{
				r.Succeed();
			}
		}
	}
	catch(_com_error& e)
	{
		r.Fail(Format("Error loading XML into msxml.domdocument. % %").s(e.ErrorMessage()).s(e.Description()));
	}

	return r;
}

Result FileToXML(MSXML::IXMLDOMDocumentPtr& p, const string& fileName)
{
	Result r;

	try
	{
		HRESULT hr;
		if(FAILED(hr = p.CreateInstance("msxml.domdocument")))
		{
			r.Fail(Format("Failed to create instance of msxml.domdocument.  Error code: %").ul(hr));
		}
		else
		{
      VARIANT_BOOL b = p->load(fileName.c_str());
			if(b == VARIANT_FALSE)
			{
				MSXML::IXMLDOMParseErrorPtr pErr = p->GetparseError();
				if(pErr == NULL)
				{
					r.Fail(_T("Failed to load XML into msxml.domdocument.  Could not get IXMLDOMParseErrorPtr for details."));
				}
				else
				{
					r.Fail(Format("Failed to parse xml.  %").s(pErr->reason));
				}
			}
			else
			{
				r.Succeed();
			}
		}
	}
	catch(_com_error& e)
	{
		r.Fail(Format("Error loading XML into msxml.domdocument. % %").s(e.ErrorMessage()).s(e.Description()));
	}

	return r;
}

Result XMLGetAttribute(_variant_t& out, MSXML::IXMLDOMElementPtr pEl, const string& sAttribute)
{
	Result r;
	try
	{
    out = pEl->getAttribute(sAttribute.c_str());
		if(out.vt == VT_NULL)
		{
			r.Fail(Format("Attribute '%' not found, or VT_NULL").s(sAttribute));
		}
		else
		{
			r.Succeed();
		}
	}
	catch(_com_error& e)
	{
		r.Fail(Format("Error reading the %s attribute from xml. % %").s(sAttribute).s(e.ErrorMessage()).s(e.Description()));
	}
	return r;
}

Result XMLGetAttribute(_variant_t& out, VARTYPE destType, MSXML::IXMLDOMElementPtr pEl, const string& sAttribute)
{
	Result r;
	if(r = XMLGetAttribute(out, pEl, sAttribute))
	{
		try
		{
			out.ChangeType(destType);
		}
		catch(_com_error& e)
		{
			r.Fail(Format("Error converting %s attribute to its destination type. % %").s(sAttribute).s(e.ErrorMessage()).s(e.Description()));
		}
	}
	return r;
}

Result XMLGetAttribute(DWORD& out, MSXML::IXMLDOMElementPtr pEl, const string& sAttribute)
{
	Result r;
	_variant_t v;
	if(r = XMLGetAttribute(v, VT_UI4, pEl, sAttribute))
	{
		out = v.intVal;
	}
	return r;
}

Result XMLGetAttribute(string& out, MSXML::IXMLDOMElementPtr pEl, const string& sAttribute)
{
	Result r;
	_variant_t v;
	if(r = XMLGetAttribute(v, VT_BSTR, pEl, sAttribute))
	{
		out = static_cast<PCTSTR>(_bstr_t(v.bstrVal));
	}
	return r;
}

Result GetFirstElementByTagName(MSXML::IXMLDOMElementPtr& parent, const string& tagName, MSXML::IXMLDOMElementPtr& pEl)
{
	Result r;

	try
	{
		pEl = 0;
		MSXML::IXMLDOMNodeListPtr pList = parent->getElementsByTagName(tagName.c_str());
		if(pList == NULL)
		{
			r.Fail(Format("No '%' nodes found (pList = 0)").s(tagName));
		}
		else
		{
			if(!pList->length)
			{
				r.Fail(Format("No '%' nodes found (length = 0)").s(tagName));
			}
			else
			{
				pEl = pList->nextNode();
				r.Succeed();
			}
		}
	}
	catch(_com_error& e)
	{
		r.Fail(Format("Com error: % %").s(e.ErrorMessage()).s(e.Description()));
	}

	return r;
}

Result XMLSetAttribute(const _variant_t& val, MSXML::IXMLDOMElementPtr pEl, const string& sAttribute)
{
	Result r;
	try
	{
		pEl->setAttribute(sAttribute.c_str(), val);
		r.Succeed();
	}
	catch(_com_error& e)
	{
		r.Fail(Format("Com error: % %").s(e.ErrorMessage()).s(e.Description()));
	}
	return r;
}

Result XMLSetAttribute(DWORD val, MSXML::IXMLDOMElementPtr pEl, const string& sAttribute)
{
	Result r;
	_variant_t v(val);
	v.ChangeType(VT_BSTR);
	r = XMLSetAttribute(v, pEl, sAttribute);
	return r;
}

Result XMLSetAttribute(const string& val, MSXML::IXMLDOMElementPtr pEl, const string& sAttribute)
{
	Result r;
	_variant_t v(val.c_str());
	v.ChangeType(VT_BSTR);
	r = XMLSetAttribute(v, pEl, sAttribute);
	return r;
}
