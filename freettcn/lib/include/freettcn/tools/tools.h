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
 * @file   tools.h
 * @author Mateusz Pusz
 * @date   Fri Apr 20 11:26:04 2007
 * 
 * @brief  
 * 
 * 
 */

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <algorithm>
#include <queue>

namespace freettcn {

  struct CPtrCmp {
    template <class T>
    bool operator()(T v1,T v2) const
    {
      return *v1 < *v2;
    }
  };
  

  // Delete pointers in an STL sequence container.
  template<class Seq> void Purge(Seq &container)
  {
    typename Seq::iterator it;
    for(it = container.begin(); it != container.end(); ++it)
      delete *it;
    container.clear();
  }
  

  // Delete pointers in an STL map container.
  template<class Map> void PurgeMap(Map &container)
  {
    typename Map::iterator it;
    for(it=container.begin(); it!=container.end(); ++it)
      delete it->second;
    container.clear();
  }
  
  
  class CList {
  public:
//     void Add(item)
//     {
// //       push_front();
//     }
    
//     void Add(sublist)
//     {
// //       push_front();
//     }
    
//     void Append(item)
//     {
// //       push_back();
//     }

//     void Append(sublist)
//     {
// //       push_back();
//     }

//     void Delete(item)
//     {
//     }
    
//     bool Member(item) const
//     {
//     }
    
//     Item &First() const
//     {
//     }
    
//     Item &Last() const
//     {
//     }
    
//     unsigned Length() const
//     {
//     }
    
//     Item &Next(item) const
//     {
//     }

    // for 'all' and 'any' support
//     Item *Random(BoolCondition) const {
//     }
    
//     void Change(Operation) {
//     }
  };
  
  class CStack {
  public:
//     void Push(item);
//     void Pop();
//     Item *Top() const;
//     void Clear();
//     void ClearUntil(item);
  };
  

  template <class Item>
  class CQueue {
    std::queue<Item> _queue;
  public:
    void Enqueue(Item item)
    {
      _queue.push(item);
    }
    
    void Dequeue()
    {
      delete First();
      _queue.pop();
    }
    
    Item First() const
    {
      return _queue.empty() ? 0 : _queue.front();
    }
    
    void Clear()
    {
      while(First())
        Dequeue();
    }
  };
    
  
} // namespace freettcn


#endif /* __TOOLS_H__ */
