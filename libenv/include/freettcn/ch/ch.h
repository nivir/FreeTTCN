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
 * @file   ch.h
 * @author Mateusz Pusz
 * @date   Tue Apr 10 19:43:39 2007
 * 
 * @brief  
 * 
 * 
 */

#ifndef __CH_H__
#define __CH_H__

extern "C" {
#include <freettcn/ttcn3/tri.h>
#include <freettcn/ttcn3/tci.h>
}
#include <freettcn/tools/logMask.h>
#include <freettcn/tools/entity.h>

namespace freettcn {
  
  namespace CH {

    class CLogMask : public freettcn::CLogMask {
    public:
      enum TCHCommands {
        CMD_CH_M_SEND_C,
        CMD_CH_M_SEND_C_BC,
        CMD_CH_M_SEND_C_MC,
        CMD_CH_M_DETECTED_C,
    
        CMD_CH_PR_CALL_C,
        CMD_CH_PR_CALL_C_BC,
        CMD_CH_PR_CALL_C_MC,
        CMD_CH_PR_GET_CALL_DETECTED_C,
        CMD_CH_PR_REPLY_C,
        CMD_CH_PR_REPLY_C_BC,
        CMD_CH_PR_REPLY_C_MC,
        CMD_CH_PR_GET_REPLY_DETECTED_C,
        CMD_CH_PR_RAISE_C,
        CMD_CH_PR_RAISE_C_BC,
        CMD_CH_PR_RAISE_C_MC,
        CMD_CH_PR_CATCH_DETECTED_C,
        CMD_CH_PR_CATCH_C,
    
        CMD_CH_P_CONNECT,
        CMD_CH_P_DISCONNECT,

        CMD_CH_NUM                                    /**< should be the last one on the list */
      };
    
      explicit CLogMask(bool enabled = true);
      ~CLogMask();
    };
    
    
    class CComponentHandler : public freettcn::CEntity {
      static CComponentHandler *_instance;

    protected:
      virtual void ConnectedMsgEnqueue(const TriPortId &sender, const TriComponentId &receiver, const Value &rcvdMessage);
      
      virtual TriComponentId TestComponentCreate(TciTestComponentKindType kind, const Type &componentType, String name);
      virtual void TestComponentStart(const TriComponentId &component, const TciBehaviourIdType &behavior, const TciParameterListType &parameterList);
      virtual void TestComponentStop(const TriComponentId &component);
      virtual void TestComponentTerminated(const TriComponentId &component, const VerdictValue &verdict);
      virtual void TestComponentKill(const TriComponentId &component);

      virtual void TestCaseExecute(const TciTestCaseIdType &testCaseId, const TriPortIdList &tsiPortList);

      virtual void Connect(const TriPortId &fromPort, const TriPortId &toPort);
      virtual void Disconnect(const TriPortId &fromPort, const TriPortId &toPort);
      virtual void Map(const TriPortId &fromPort, const TriPortId &toPort);
      virtual void Unmap(const TriPortId &fromPort, const TriPortId &toPort);

    public:
      static CComponentHandler &Instance();
      
      CComponentHandler();
      virtual ~CComponentHandler();
      
      void ResetReq();
      
      void ConnectedSend(const TriPortId &sender, const TriComponentId &receiver, const Value &sendMessage);
      
      TriComponentId TestComponentCreateReq(TciTestComponentKindType kind, const Type &componentType, String name);
      void TestComponentStartReq(const TriComponentId &component, const TciBehaviourIdType &behavior, const TciParameterListType &parameterList); 
      void TestComponentStopReq(const TriComponentId &component); 
      void TestComponentTerminatedReq(const TriComponentId &component, const VerdictValue &verdict);
      void TestComponentKillReq(const TriComponentId &component); 
      
      void TestCaseExecuteReq(const TciTestCaseIdType &testCaseId, const TriPortIdList &tsiPortList);

      void ConnectReq(const TriPortId &fromPort, const TriPortId &toPort);
      void DisconnectReq(const TriPortId &fromPort, const TriPortId &toPort);
      void MapReq(const TriPortId &fromPort, const TriPortId &toPort);
      void UnmapReq(const TriPortId &fromPort, const TriPortId &toPort);
    };
    
  } // namespace CH
  
} // namespace freettcn


#endif /* __CH_H__ */
