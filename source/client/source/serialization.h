// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark

#pragma once

#include <map>
#include <vector>
#include "libcc/blob.hpp"
#import "msxml3.dll"

extern "C" size_t Curl_base64_encode(const char *input, size_t size, char **str);
extern "C" size_t Curl_base64_decode(const char *source, unsigned char **outptr);



inline _bstr_t _Format(MSXML2::IXMLDOMNodePtr ptrNode, int iTabLevel)
{
 _bstr_t bstrNode;

 switch (ptrNode->GetnodeType())
 {
 case MSXML2::NODE_DOCUMENT:
 case MSXML2::NODE_DOCUMENT_FRAGMENT:
  {
   // All children of document/document fragment are at same level.
   for (long i=0; i<ptrNode->GetchildNodes()->Getlength(); i++)
    bstrNode += _Format(ptrNode->GetchildNodes()->Getitem(i), iTabLevel);
  }
  break;

 case MSXML2::NODE_TEXT:
  // Format like IE5 renders.
  {
   bool bPartOfMixedContent =
    ptrNode->GetparentNode()->GetchildNodes()->Getlength() > 1;
   if (bPartOfMixedContent)
    // Indent if part of mixed content.
		bstrNode = std::wstring(iTabLevel, '\t').c_str();

   // Call Getxml() here, otherwise entities (&gt;) get expanded.
   // Trim leading and trailing whitespace to mimic Gettext().
   CString strXML((PWSTR)ptrNode->Getxml());
   strXML.TrimLeft(); strXML.TrimRight();
   bstrNode += (LPCTSTR)strXML;

   if (bPartOfMixedContent)
    // Add carriage-return/line feed if part of mixed content.
    bstrNode += L"\r\n";
  }
  break;

 case MSXML2::NODE_ELEMENT:
  {
   // Indent.
	bstrNode = std::wstring(iTabLevel, '\t').c_str();

   // Open the start tag.
   bstrNode += L"<" + ptrNode->GetnodeName();

   // Add the attributes.
   for (long i=0; i<ptrNode->Getattributes()->Getlength(); i++)
    bstrNode +=
     L" " + ptrNode->Getattributes()->Getitem(i)->Getxml();

   // Determine how to close start tag with help of following bools.
   bool bHasChildNodes = ptrNode->hasChildNodes() == VARIANT_TRUE;
   bool bHasOnlyATextChildNode =
    bHasChildNodes &&
    ptrNode->GetchildNodes()->Getlength() == 1 &&
    ptrNode->GetchildNodes()->Getitem(0)->GetnodeType() ==
    MSXML2::NODE_TEXT;

   if (!bHasChildNodes)
    // It is an empty element; close it.
    bstrNode += L"/>\r\n";
   else if (bHasOnlyATextChildNode)
    // It has a single text node; don't add carriage return.
    bstrNode += L">";
   else
    // It has more than one text child node; add carriage return.
    bstrNode += L">\r\n";

   // Recurse if it has children.
   if (bHasChildNodes)
    for (int i=0; i<ptrNode->GetchildNodes()->Getlength(); i++)
     bstrNode +=
      _Format
      (
       ptrNode->GetchildNodes()->Getitem(i),
       iTabLevel+1
      );

   // Indent properly and add end tag.
   if (!bHasOnlyATextChildNode && bHasChildNodes)
		 bstrNode += std::wstring(iTabLevel, '\t').c_str();
   if (bHasChildNodes)
    bstrNode += L"</" + ptrNode->GetnodeName() + L">\r\n";
  }
  break;

 case MSXML2::NODE_ENTITY:
 case MSXML2::NODE_ATTRIBUTE:
  // These cases should never occur, but don't do anything if they do.
  break;

 default:
  // All other node types should return their XML (properly indented).
  bstrNode = std::wstring(iTabLevel, '\t').c_str() + ptrNode->Getxml() + L"\r\n";
  break;
 } 

 return bstrNode;
}

namespace Xml
{
	typedef MSXML2::IXMLDOMDocumentPtr Document;
	typedef MSXML2::IXMLDOMElementPtr Element;
	typedef MSXML2::IXMLDOMNodePtr Node;

	// Fundamental conversion functions ----------------------------------
	inline void ToString(const std::wstring& in, std::wstring& out)
	{
		out = in;
	}
	inline void FromString(const std::wstring& in, std::wstring& out)
	{
		out = in;
	}

	inline void ToString(const bool& in, std::wstring& out)
	{
		out = in ? L"yes" : L"no";
	}
	inline void FromString(const std::wstring& in, bool& out)
	{
		out = (in == L"yes");
	}

	template<typename T>
	inline void ToStringViaVariant(const T& in, std::wstring& out)
	{
		_variant_t temp(in);
		temp.ChangeType(VT_BSTR);
		out = temp.bstrVal;
	}
	inline _variant_t ToVariant(const std::wstring& in, VARTYPE vt)
	{
		_variant_t temp(in.c_str());
		temp.ChangeType(vt);
		return temp;
	}

	inline void ToString(const float& in, std::wstring& out)
	{
		ToStringViaVariant(in, out);
	}
	inline void FromString(const std::wstring& in, float& out)
	{
		out = ToVariant(in, VT_R4).fltVal;
	}
	inline void ToString(const int& in, std::wstring& out)
	{
		ToStringViaVariant(in, out);
	}
	inline void FromString(const std::wstring& in, int& out)
	{
		out = ToVariant(in, VT_I4).intVal;
	}
	inline void ToString(const unsigned short& in, std::wstring& out)
	{
		ToStringViaVariant((int)in, out);
	}
	inline void FromString(const std::wstring& in, unsigned short& out)
	{
		out = (unsigned short)(ToVariant(in, VT_I4).intVal);
	}

	class BinaryData
	{
	public:
		BinaryData() :
			p(0),
			size(0)
		{
		}
		template<typename T>
		BinaryData(const T* x, size_t elements = 1) :
			p(x),
			size(elements * sizeof(T))
		{
		}
		const void* p;
		size_t size;
	};
	inline void ToString(const BinaryData& in, std::wstring& out)
	{
		char* outA;
		size_t size = Curl_base64_encode((const char*)in.p, in.size, &outA);
		out = LibCC::ToUTF16(std::string(outA, size));
		free(outA);
	}
	inline void FromString(const std::wstring& in, LibCC::Blob<BYTE>& out)
	{
		std::string a;
		a = LibCC::ToANSI(in);
		unsigned char* buf;
		size_t size = Curl_base64_decode(a.c_str(), &buf);
		out.Alloc(size);
		memcpy(out.GetBuffer(), buf, size);
		free(buf);
	}

	// for serializing wstring
	template<typename T>
	inline void SerializeToAttribute(Element parent, const std::wstring& name, const T& val)
	{
		try
		{
			std::wstring str;
			ToString(val, str);
			parent->setAttribute(name.c_str(), str.c_str());
		}
		catch(_com_error&)
		{
		}
	}

	template<typename T>
	inline bool DeserializeFromAttribute(Element parent, const std::wstring& name, T& val)
	{
		try
		{
			_variant_t var = parent->getAttribute(name.c_str());
			if (var.vt == VT_EMPTY || var.vt == VT_NULL)
			{
				return false;
			}
			FromString(var.bstrVal, val);
			return true;
		}
		catch(_com_error&)
		{
		}
		return false;
	}

	// ----------------------------------

	// catch-all for custom types.
	template<typename T>
	inline void Serialize(Element parent, const std::wstring& name, const T& val)
	{
		try
		{
			Element el = parent->ownerDocument->createElement(name.c_str());
			parent->appendChild(el);
			val.Serialize(el);
		}
		catch(_com_error&)
		{
		}
	}

	inline void Serialize(Element parent, const std::wstring& name, const std::wstring& val)
	{
		SerializeToAttribute(parent, name, val);
	}
	inline void Serialize(Element parent, const std::wstring& name, const bool& val)
	{
		SerializeToAttribute(parent, name, val);
	}
	inline void Serialize(Element parent, const std::wstring& name, const float& val)
	{
		SerializeToAttribute(parent, name, val);
	}
	inline void Serialize(Element parent, const std::wstring& name, const BinaryData& val)
	{
		SerializeToAttribute(parent, name, val);
	}
	inline void Serialize(Element parent, const std::wstring& name, const int& val)
	{
		SerializeToAttribute(parent, name, val);
	}
	inline void Serialize(Element parent, const std::wstring& name, const unsigned short& val)
	{
		SerializeToAttribute(parent, name, val);
	}

	template<typename T>
	inline void Serialize(Element parent, const std::wstring& name, const std::vector<T>& src)
	{
		try
		{
			// create a single element to contain the elements.
			Element el = parent->ownerDocument->createElement(name.c_str());
			parent->appendChild(el);

			for(std::vector<T>::const_iterator it = src.begin(); it != src.end(); ++ it)
			{
				// create an element for the item itself
				Element itemEl = parent->ownerDocument->createElement(L"item");
				el->appendChild(itemEl);
				Serialize(itemEl, L"value", *it);
			}
		}
		catch(_com_error&)
		{
		}
	}

	template<typename T>
	inline void Serialize(const T& o, const std::wstring& fileName)
	{
		CoInitialize(NULL);
		{
			try
			{
				Document doc;
				doc.CreateInstance(L"Msxml2.DOMDocument");
				if(doc != 0)
				{
					// create the root element.
					Element root;
					root = doc->createElement(L"xml");
					doc->appendChild(root);

					// serialize everything
					Serialize(root, L"Settings", o);

					// save
					_bstr_t newXml = _Format(doc, 0);
					Document n;
					n.CreateInstance(L"Msxml2.DOMDocument");
					n->loadXML(newXml);
					n->save(fileName.c_str());
				}
			}
			catch(_com_error&)
			{
			}
		}
		CoUninitialize();
	}

	/////////////////////
	// catch-all for custom types.
	template<typename T>
	inline bool Deserialize(Element parent, const std::wstring& name, T& val)
	{
		try
		{
			Element el = parent->selectSingleNode(name.c_str());
			if(el == 0)
			{
				return false;
			}
			val.Deserialize(el);
			return true;
		}
		catch(_com_error&)
		{
		}
		return false;
	}

	inline bool Deserialize(Element parent, const std::wstring& name, std::wstring& val)
	{
		return DeserializeFromAttribute(parent, name, val);
	}
	inline bool Deserialize(Element parent, const std::wstring& name, bool& val)
	{
		return DeserializeFromAttribute(parent, name, val);
	}
	inline bool Deserialize(Element parent, const std::wstring& name, float& val)
	{
		return DeserializeFromAttribute(parent, name, val);
	}
	inline bool Deserialize(Element parent, const std::wstring& name, LibCC::Blob<BYTE>& val)
	{
		return DeserializeFromAttribute(parent, name, val);
	}
	inline bool Deserialize(Element parent, const std::wstring& name, int& val)
	{
		return DeserializeFromAttribute(parent, name, val);
	}
	inline bool Deserialize(Element parent, const std::wstring& name, unsigned short& val)
	{
		return DeserializeFromAttribute(parent, name, val);
	}

	template<typename T>
	inline bool Deserialize(Element parent, const std::wstring& name, std::vector<T>& src)
	{
		try
		{
			Element el = parent->selectSingleNode(name.c_str());
			if(el == 0)
			{
				return false;
			}

			MSXML2::IXMLDOMNodeListPtr x = el->childNodes;
			if(x == 0)
			{
				return true;// no items; that's fine.
			}

			long l = x->length;
			for(long i = 0; i < l; ++ i)
			{
				Element item = x->item[i];
				src.push_back(T());
				if(!Deserialize(item, L"value", src.back()))
				{
					return false;// this may cut deserialization short.
				}
			}
		}
		catch(_com_error&)
		{
		}
		return false;
	}

	template<typename T>
	inline void Deserialize(T& o, const std::wstring& fileName)
	{
		CoInitialize(NULL);
		{
			Document doc;
			if(SUCCEEDED(doc.CreateInstance(L"Msxml2.DOMDocument")))
			{
				if(VARIANT_TRUE == doc->load(fileName.c_str()))
				{
					Element root = doc->selectSingleNode(L"xml");
					if(root != 0)
					{
						Deserialize(root, L"Settings", o);
					}
				}
			}
		}
		CoUninitialize();
	}
}
