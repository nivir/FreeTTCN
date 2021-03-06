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
 * @file   codec.h
 * @author Mateusz Pusz
 * @date   Wed May 30 09:28:55 2007
 * 
 * @brief  
 * 
 * 
 */

#ifndef __CODEC_H__
#define __CODEC_H__

extern "C" {
#include <freettcn/ttcn3/tci.h>
}


namespace freettcn {
  
  namespace CD {
    
    class CBuffer;
    
    class CCodec {
      const char *_name;
    public:
      CCodec(const char *name);
      virtual ~CCodec() = 0;
      
      const char *Name() const;

      virtual bool CapabilityCheck(const Value &value, unsigned int &valueId) const = 0;
      virtual void Encode(unsigned int valueId, const Value &value, CBuffer &buffer) const = 0;
    };
    
  } // namespace CD

} // namespace freettcn


#endif /* __CODEC_H__ */
