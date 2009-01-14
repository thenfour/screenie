/*
  LibCC
  Float Module (previously part of StringUtil.hpp)
  (c) 2004-2008 Carl Corcoran, carlco@gmail.com
  Documentation: http://wiki.winprog.org/wiki/LibCC
	Official source code: http://svn.winprog.org/personal/carl/LibCC

	== License:

  All software on this site is provided 'as-is', without any express or
  implied warranty, by its respective authors and owners. In no event will
  the authors be held liable for any damages arising from the use of this
  software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
  claim that you wrote the original software. If you use this software in
  a product, an acknowledgment in the product documentation would be
  appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not
  be misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.
*/

#pragma once


namespace LibCC
{
  // this simple class just "attaches" to a float and provides a window into it's inner workings.
  // based on information from http://www.duke.edu/~twf/cps104/floating.html
  // ... and http://stevehollasch.com/cgindex/coding/ieeefloat.html
  // e notation:    d.dd...En
  // 1 bit = sign
  // 8 bits = exp
  // 23 bits = mantissa
  /*
    What are denormalized numbers?  they are normal numbers but without the leading 1 and assumed exp of -126
  */
  template<typename _BasicType,
    typename _InternalType,
    typename _Exponent,
    typename _Mantissa,
    long _ExponentBits,
    long _MantissaBits>
  class IEEEFloat
  {
  public:
    typedef _BasicType BasicType;
    typedef _InternalType InternalType;
    typedef _Exponent Exponent;
    typedef _Mantissa Mantissa;
    typedef IEEEFloat<BasicType, InternalType, Exponent, Mantissa, _ExponentBits, _MantissaBits> This;

    static const long ExponentBits = _ExponentBits;// 0x08 / 0x0b
    static const long MantissaBits = _MantissaBits;// 0x17 / 0x34
    static const _InternalType SignMask = (InternalType)1 << (ExponentBits + MantissaBits);// 0x80000000 / 0x8000000000000000
    static const _InternalType MantissaMask = ((InternalType)1 << MantissaBits) - 1;// 0x007fffff / 0x000fffffffffffff
    static const _InternalType MantissaHighBit = MantissaMask ^ (MantissaMask >> 1);// 0x00400000 / 0x0008000000000000
    static const _InternalType ExponentMask = ((((InternalType)1 << ExponentBits) - 1) << MantissaBits);// 0x7f800000 / 0x7ff0000000000000
    static const _InternalType PositiveInfinity = ExponentMask;// 0x7f800000 / 0x7ff0000000000000
    static const _InternalType NegativeInfinity = SignMask | ExponentMask;// 0xff800000 / 0xfff0000000000000
    static const Exponent ExponentBias = (((InternalType)1 << (ExponentBits-1)) - 1);// 0x7f / 0x3ff

    IEEEFloat(BasicType f) :
      m_val(reinterpret_cast<InternalType&>(m_BasicVal)),
      m_BasicVal(f)
    {
    }

    IEEEFloat(const This& r) :
      m_val(reinterpret_cast<InternalType&>(m_BasicVal)),
      m_BasicVal(r.m_BasicVal)
    {
    }

    IEEEFloat(_InternalType r) :
      m_val(reinterpret_cast<InternalType&>(m_BasicVal))
    {
      m_val = r;
    }

    bool IsPositive() const
    {
      return m_val & SignMask ? false : true;
    }
    bool IsNegative() const
    {
      return !IsPositive();
    }
    bool IsZero() const// exponent == 0  &&  mantissa == 0
    {
      return m_val & (MantissaMask | ExponentMask) ? false : true;
    }
    bool IsDenormalized() const// exponent = 0  &&  mantissa != 0
    {
      return (!(m_val & ExponentMask)) && (m_val & MantissaMask);
    }
    bool IsPositiveInfinity() const// exponent == MAX
    {
      return m_val == PositiveInfinity;
    }
    bool IsNegativeInfinity() const// exponent == MAX
    {
      return m_val == NegativeInfinity;
    }
    bool IsInfinity() const// exponent == MAX  &&  mantissa == MAX
    {
      return (m_val & (MantissaMask | ExponentMask)) == (MantissaMask | ExponentMask);
    }
    bool IsNaN() const// exponent == MAX  && mantissa != 0
    {
      return ((m_val & ExponentMask) == ExponentMask) && (m_val & MantissaMask);
    }
    bool IsQNaN() const// exponent == MAX  && mantissa high bit set
    {
      return ((m_val & ExponentMask) == ExponentMask) && (m_val & MantissaHighBit);
    }
    bool IsSNaN() const// exponent == MAX  && mantissa != 0  && mantissa high bit 0
    {
      return ((m_val & ExponentMask) == ExponentMask) && (m_val & MantissaMask) && !(m_val & MantissaHighBit);
    }
    Exponent GetExponent() const
    {
      return static_cast<Exponent>(((m_val & ExponentMask) >> MantissaBits) - ExponentBias);
    }

    Mantissa GetMantissa() const
    {
      return static_cast<Mantissa>(m_val & MantissaMask | (static_cast<Mantissa>(1) << MantissaBits));// add the implied 1
    }

    void CopyValue(BasicType& out) const
    {
      memcpy(&out, &m_val, sizeof(m_val));
    }

    static This Build(bool Sign, Exponent ex, Mantissa m)
    {
      InternalType r;
      r = Sign ? SignMask : 0;// sign
      r |= (ex + ExponentShift) << MantissaBits;// exponent
      r |= m & MantissaMask;// mantissa
      return *(reinterpret_cast<BasicType*>(&r));
    }

    /*
      Infinity
      The values +infinity and -infinity are denoted with an exponent of all 1s and a
      fraction of all 0s. The sign bit distinguishes between negative infinity and positive
      infinity.
    */
    static This BuildPositiveInfinity()
    {
      InternalType r;
      r = ExponentMask;
      return *(reinterpret_cast<BasicType*>(&r));
    }

    static This BuildNegativeInfinity()
    {
      InternalType r;
      r = SignMask | ExponentMask;
      return *(reinterpret_cast<BasicType*>(&r));
    }

    // The value NaN (Not a Number) is used to represent a value that does not represent a real number.
    // NaN's are represented by a bit pattern with an exponent of all 1s and a non-zero fraction.
    // There are two categories of NaN: QNaN (Quiet NaN) and SNaN (Signalling NaN).
    // A QNaN is a NaN with the most significant fraction bit set.
    // QNaN's propagate freely through most arithmetic operations. These values pop out of an operation when the result is not mathematically defined.
    static This BuildQNaN()
    {
      InternalType r;
      r = ExponentMask | MantissaMask;
      return *(reinterpret_cast<BasicType*>(&r));
    }

    // An SNaN is a NaN with the most significant fraction bit clear. It is used to signal an exception when used in operations. SNaN's can be handy to assign to uninitialized variables to trap premature usage. 
    static This BuildSNaN()
    {
      InternalType r;
      r = ExponentMask | (MantissaMask >> 1);
      return *(reinterpret_cast<BasicType*>(&r));
    }

    void AbsoluteValue()
    {
      m_val &= ~SignMask;
    }

    /*
      say you have a mantissa:
      1.0101101 with exponent 3.
      that's: 1010.1101
      to remove the decimal, it would be:
      that's: 1010.0
      or, 1.01, exponent 3
      In the case of a negative exponent, set the float to zero.

      So the idea is simply to mask out the bits that are right of
      the decimal point.
    */
    void RemoveDecimal()
    {
      InternalType e = (m_val & ExponentMask) >> MantissaBits;
      Mantissa m = static_cast<Mantissa>(m_val & MantissaMask);

      //if(m)
      {
        if(e >= ExponentBias)
        {
          e -= ExponentBias;
          if(e < MantissaBits)// if we did this when e is greater than the mantissa bits, it would get all screwy.
          {
            // positive exponent.  mask out the decimal part.
            InternalType mask = 1;
            mask <<= (MantissaBits - e);// bits
            mask -= 1;// mask
            m_val &= ~mask;
          }
        }
        else
        {
          // negative exponent; set this float to zero, retaining the current sign.
          m_val &= ~(ExponentMask | MantissaMask);
        }
      }
    }

    InternalType& m_val;
    BasicType m_BasicVal;

    private:
      This& operator =(const This& rhs) { return *this; }// do not allow assignment.  this is also to prevent warning C4512 "'class' : assignment operator could not be generated"

  };
  typedef IEEEFloat<float, unsigned __int32, signed __int8, unsigned __int32, 8, 23> SinglePrecisionFloat;
  typedef IEEEFloat<double, unsigned __int64, signed __int16, unsigned __int64, 11, 52> DoublePrecisionFloat;
}


