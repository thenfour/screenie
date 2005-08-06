

#pragma once


#include "xmlutil.h"


class Arg
{
public:
  string name;
  string value;
  enum Type
  {
    FIXED,
    STRING,
    DELETETYPE
  } type;
  string source;// where did this arg come from?
};


class VersionXMLFile
{
public:
  vector<Arg> args;

  Result Read(const string& fileName)
  {
    Result res;
    MSXML::IXMLDOMDocumentPtr doc;
    MSXML::IXMLDOMElementPtr versionElement;
    MSXML::IXMLDOMElementPtr stringElement;
    MSXML::IXMLDOMElementPtr fixedElement;
    _bstr_t t;

    if(!(res = FileToXML(doc, fileName)))
    {
      goto Error;
    }

    if(doc->documentElement->tagName != _bstr_t("Version"))
    {
      res.Fail("No <Version> tag found.");
      goto Error;
    }

    versionElement = doc->documentElement;

    if(GetFirstElementByTagName(versionElement, "StringFileInfo", stringElement))
    {
      if(!(res = ProcessValueTag(stringElement, Arg::STRING, Format("XML file %; <StringFileInfo> element").s(fileName).Str())))
      {
        res.Prepend("Error reading <StringFileInfo> element; ");
        goto Error;
      }
    }

    if(GetFirstElementByTagName(versionElement, "FixedFileInfo", fixedElement))
    {
      if(!(res = ProcessValueTag(versionElement, Arg::FIXED, Format("XML file %; <FixedFileInfo> element").s(fileName).Str())))
      {
        res.Prepend("Error reading <FixedFileInfo> element; ");
        goto Error;
      }
    }

    return Result(S_OK);
Error:
    return res.Prepend(Format("Error loading %; ").qs(fileName));
  }

private:
  Result ProcessValueTag(MSXML::IXMLDOMElementPtr el, Arg::Type type, const string& source)
  {
    try
    {
      long length = el->attributes->length;
      for(long i = 0; i < length; i ++)
      {
        IXMLDOMAttributePtr attribute = el->attributes->item[i];
        Arg temp;
        temp.type = type;
        temp.source = source;

        BSTR bstr;
        attribute->get_name(&bstr);
        temp.name = (PCSTR)(_bstr_t(bstr));

        VARIANT v;
        attribute->get_value(&v);
        _variant_t var(v);
        var.ChangeType(VT_BSTR);
        temp.value = (PCSTR)(_bstr_t(var.bstrVal));

        args.push_back(temp);
      }
    }
    catch(_com_error& e)
    {
		  return Result(E_FAIL, Format("Com error: % %").s(e.ErrorMessage()).s(e.Description()));
    }
    return Result(S_OK);
  }
};


