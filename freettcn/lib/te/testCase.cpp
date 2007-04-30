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
 * @file   testCase.cpp
 * @author Mateusz Pusz
 * @date   Wed Apr 25 11:04:10 2007
 * 
 * @brief  
 * 
 * 
 */

#include "freettcn/te/testCase.h"
#include "freettcn/te/te.h"
#include "freettcn/te/module.h"
#include "freettcn/te/behavior.h"
#include "freettcn/te/testComponent.h"
#include "freettcn/te/sourceData.h"
#include "freettcn/tools/logMask.h"
#include "freettcn/tools/timeStamp.h"
extern "C" {
#include "freettcn/ttcn3/tci_te_tm.h"
#include "freettcn/ttcn3/tci_te_ch.h"
#include "freettcn/ttcn3/tri_te_sa.h"
#include "freettcn/ttcn3/tci_tl.h"
}
#include <iostream>



freettcn::TE::CTestCase::CTestCase(CModule &module, const char *name, const freettcn::TE::CSourceData *srcData,
                                   const freettcn::TE::CTestComponentType &mtcType, freettcn::TE::CBehavior *behavior,
                                   const freettcn::TE::CTestComponentType *systemType /* 0 */):
  CInitObject(name), _module(module), _srcData(srcData),
  _mtcType(mtcType), _behavior(behavior), _systemType(systemType ? *systemType : mtcType)
{
  _id.moduleName = const_cast<char *>(_module.Name());
  _id.objectName = const_cast<char *>(name);
  _id.aux = 0;
}

freettcn::TE::CTestCase::~CTestCase()
{
  if (_srcData)
    delete _srcData;
}

TciParameterTypeListType freettcn::TE::CTestCase::Parameters() const
{
  TciParameterTypeListType tcParams;
  tcParams.length = 0;
  tcParams.parList = 0;
  
  return tcParams;
}

TriPortIdList freettcn::TE::CTestCase::Ports() const
{
  TriPortIdList portList;
  portList.portIdList = 0;
  portList.length = 0;
  
  return portList;
}


void freettcn::TE::CTestCase::Start(const char *src, int line,
                                    const freettcn::TE::CTestComponent *creator,
                                    TciParameterListType parameterlist,
                                    TriTimerDuration dur)
{
  TriComponentId creatorId;
  
  if (creator) {
    creatorId = creator->Id();
  }
  else {
    src = _srcData->Source();
    line = _srcData->Line();
    creatorId = _module.ModuleComponentId();
  }
  
  freettcn::TE::CTTCNExecutable &te = freettcn::TE::CTTCNExecutable::Instance();
  if (te.Logging() && te.LogMask().Get(LOG_TE_TC_START)) {
    // log
    TriParameterList parList;                     /**< @todo Paramters list should be translated */
    parList.length = 0;
    parList.parList = 0;
    
    tliTcStart(0, te.TimeStamp().Get(), const_cast<char *>(src), line, creatorId, _id, parList, dur);
  }
  
  // create MTC
  _mtcId = _module.TestComponentCreateReq(src, line, creatorId, TCI_MTC_COMP, &_mtcType, "MTC");
  
  // create SYSTEM
  _systemId = _module.TestComponentCreateReq(src, line, _mtcId, TCI_SYS_COMP, &_systemType, "SYSTEM");
  
  // give a chance to create static connections and the initialization of TSI ports
  if (te.Logging() && te.LogMask().Get(LOG_TE_TC_EXECUTE)) {
    // log
    TriTimerDuration dur = 0.0;                   /**< @todo Timer duration should be set */
    TriParameterList parList;                     /**< @todo Paramters list should be translated */
    parList.length = 0;
    parList.parList = 0;
    
    tliTcExecute(0, te.TimeStamp().Get(), const_cast<char *>(src), line, _mtcId, _id, parList, dur);
  }
  tciExecuteTestCaseReq(_id, Ports());
  
  // start MTC
  /// @todo define parameter list
  TciParameterListType parameterList;
  parameterList.length = 0;
  parameterList.parList = 0;
  
  _module.TestComponentStartReq(src, line, creatorId, _mtcId, _behavior->Id(), parameterList);
}


void freettcn::TE::CTestCase::Execute(TciTestCaseIdType testCaseId, TriPortIdList tsiPortList)
{
  // set current test case
  _module.TestCase(this);
  
  if (triExecuteTestCase(&testCaseId, &tsiPortList) != TRI_OK) {
    /// @todo do something
  }
}


void freettcn::TE::CTestCase::Stop()
{
  _module.TestCase(0);
  
  freettcn::TE::CTTCNExecutable &te = freettcn::TE::CTTCNExecutable::Instance();
  if (te.Logging() && te.LogMask().Get(LOG_TE_TC_STOP))
    // log
    tliTcStop(0, te.TimeStamp().Get(), 0, 0, _mtcId);
  
  tciResetReq();
  
  /// @todo set verdict to ERROR
  
  // notify TM about test case termination
  TciValue error = 0;
  TciParameterListType parList;
  parList.length = 0;
  parList.parList = 0;
  tciTestCaseTerminated(error, parList);
}