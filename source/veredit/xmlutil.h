/*
	Helper functions for MSXML.  Mostly wrappers to get around exception handling.
*/

#pragma once

LibCC::Result CreateXMLHttpRequest(MSXML::IXMLHttpRequestPtr& p);

LibCC::Result XMLToString(string& s, MSXML::IXMLDOMNodePtr p);
LibCC::Result StringToXML(MSXML::IXMLDOMDocumentPtr& p, const string& s);
LibCC::Result FileToXML(MSXML::IXMLDOMDocumentPtr& p, const string& s);

LibCC::Result XMLGetAttribute(_variant_t& out, MSXML::IXMLDOMElementPtr pEl, const string& sAttribute);
LibCC::Result XMLGetAttribute(_variant_t& out, VARTYPE destType, MSXML::IXMLDOMElementPtr pEl, const string& sAttribute);
LibCC::Result XMLGetAttribute(DWORD& out, MSXML::IXMLDOMElementPtr pEl, const string& sAttribute);
LibCC::Result XMLGetAttribute(string& out, MSXML::IXMLDOMElementPtr pEl, const string& sAttribute);

LibCC::Result XMLSetAttribute(const _variant_t& val, MSXML::IXMLDOMElementPtr pEl, const string& sAttribute);
LibCC::Result XMLSetAttribute(DWORD val, MSXML::IXMLDOMElementPtr pEl, const string& sAttribute);
LibCC::Result XMLSetAttribute(const string& val, MSXML::IXMLDOMElementPtr pEl, const string& sAttribute);

LibCC::Result GetFirstElementByTagName(MSXML::IXMLDOMElementPtr& parent, const string& tagName, MSXML::IXMLDOMElementPtr& pEl);
