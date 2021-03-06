//
// Copyright (C) 2007 Mateusz Pusz
//
// This file is part of freettcnenv (Free TTCN Environment) library.

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
 * @file   cd.h
 * @author Mateusz Pusz
 * @date   Wed May 30 08:03:44 2007
 * 
 * @brief  
 * 
 * 
 */

#ifndef __CD_H__
#define __CD_H__

extern "C" {
// #include <freettcn/ttcn3/tri.h>
#include <freettcn/ttcn3/tci.h>
}
#include <freettcn/cd/buffer.h>
#include <freettcn/tools/logMask.h>
#include <freettcn/tools/entity.h>
#include <list>


namespace freettcn {
  
  namespace CD {
    
    class CLogMask : public freettcn::CLogMask {
    public:
      enum TCDCommands {
        CMD_CD_ENCODE,
        CMD_CD_DECODE,
      
        CMD_CD_NUM                                    /**< should be the last one on the list */
      };
    
      explicit CLogMask(bool enabled = true);
      ~CLogMask();
    };
    

    class CCodec;
    
    class CCodingDecoding : public freettcn::CEntity {
      static CCodingDecoding *_instance;
      
      typedef std::list<const CCodec *> CCodecList;
      
      CCodecList _codecList;
      
      virtual const CCodec &Codec(const Value &value, unsigned int &valueId) const;
      
    public:
      static CCodingDecoding &Instance();
      
      CCodingDecoding();
      virtual ~CCodingDecoding();
      
      void Register(const CCodec *codec);
      
      const Value Decode(const BinaryString &message, const Type &decHypothesis) const;
      BinaryString Encode(const Value &value) const;
    };
    
  } // namespace CD
  
} // namespace freettcn


#endif /* __CD_H__ */
