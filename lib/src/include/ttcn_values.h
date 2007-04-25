//
// Copyright (C) 2007 Mateusz Pusz
//
// This file is part of freettcn (Free TTCN) library.

// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


/**
 * @file   ttcn_values.h
 * @author Mateusz Pusz
 * @date   Tue Apr 24 21:05:20 2007
 * 
 * @brief  
 * 
 * 
 */

#ifndef __TTCN_VALUES_H__
#define __TTCN_VALUES_H__

#include "ttcn_types.h"
#include "exception.h"


namespace freettcn {

  namespace TE {

    class CValue {
      const CType &_type;
      const bool _omit;
    
    public:
      CValue(const CType &type, bool omit);
      virtual ~CValue();
    
      const CType &Type() const;
      bool Omit() const;
      const String Encoding() const;
      const String EncodingVariant() const;
      const String Extension() const;
    };
  
    class CBooleanValue : public CValue {
      bool _value;
    public:
      CBooleanValue(const CType &type, bool omit);
      bool Value() const;
      void Value(bool value);
    };
  
    class CIntegerValue : public CValue {
      unsigned long _absValue;
      bool _sign;
    public:
      //     Integer Value() const;
      //    void Value(Integer value);
      void AbsValue(String value) throw(freettcn::EOperationFailed);
      void DigitsNum(unsigned long int dig_num);
      void Sign(bool sign);
    };
  
    class CFloatValue : public CValue {
      Float _value;
    public:
      Float Value() const;
      void Value(Float value);
    };
  
    class CObjIdValue : public CValue {
      TciObjidValue _value;
    public:
      TciObjidValue Value() const;
      void Value(TciObjidValue value);
    };
  
    class CEnumeratedValue : public CValue {
      String _value;
    public:
      String Value() const;
      void Value(String value);
    };
  
    class CVerdictValue : public CValue {
      Integer _value;
    public:
      Integer Value() const;
      void Value(Integer value);
    };
  
    class CAddressValue : public CValue {
      TciValue _value;
    public:
      TciValue Value() const;
      void Value(TciValue value);
    };
  
    class CCharstringValue : public CValue {
      String _value;
    public:
      const String Value() const;
      void Value(const String value);
    
      Char Element(Integer position) const;
      void Element(Integer position, Char value);
    
      Integer Length() const;
      void Length(Integer len);
    };
  
    class CUniversalCharstringValue : public CValue {
      String _value;
    public:
      String Value() const;
      void Value(String value);
    
      TciUCValue *Element(Integer position) const;
      void Element(Integer position, TciUCValue value);
    
      Integer Length() const;
      void Length(Integer len);
    };
  
    class CHexstringValue : public CValue {
      String _value;
    public:
      String Value() const;
      void Value(String value);
    
      Integer Element(Integer position) const;
      void Element(Integer position, Integer value);
    
      Integer Length() const;
      void Length(Integer len);
    };
  
    class COctetstringValue : public CValue {
      String _value;
    public:
      String Value() const;
      void Value(String value);
    
      Integer Element(Integer position) const;
      void Element(Integer position, Integer value);
    
      Integer Length() const;
      void Length(Integer len);
    };
  
    class CBitstringValue : public CValue {
      String _value;
    public:
      String Value() const;
      void Value(String value);
    
      Integer Element(Integer position) const;
      void Element(Integer position, Integer value);
    
      Integer Length() const;
      void Length(Integer len);
    };
  
    class CUnionValue : public CValue {
      TciValue _value;
    public:
      TciValue Variant(String variantName) const;
      void Variant(String variantName, TciValue value);
      String PresentVariantName() const;
      StringSeq VariantNames() const;
    };
  
    class CRecordValue : public CValue {
      TciValue _value;
    public:
      CRecordValue(const CType &type, bool omit);
      TciValue Field(String fieldName) const;
      void Field(String fieldName, TciValue value);
      StringSeq FieldNames() const;
    };
    
    class CRecordOfValue : public CValue {
      TciValue _value;
    public:
      TciValue Field(Integer position) const;
      void Field(Integer position, TciValue value);
      void Append(TciValue value);
      TciType ElementType() const;
      Integer Length() const;
      void Length(Integer len);
    };
    
  } // namespace TE
  
} // namespace freettcn

#endif /* __TTCN_VALUES_H__ */
