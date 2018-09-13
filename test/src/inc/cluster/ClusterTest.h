/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __CLUSTERTEST_H__
#define __CLUSTERTEST_H__

#define private public

#include "cluster/Cluster.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"
#include "common/SystemExec.h"
#include "common/String.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class CClusterTest : public testing::Test
{
public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CClusterTest::m_stub;

//���庯������
/*���Ա����������������Class��+������+Type��׺
 *���Ա������Stub����������������Stubǰ׺+Class��+������+Type��׺
 *���Ա������Stub������������void* pthis + ԭ������������ôд����Ϊ�����ܻ��ԭ��������������������̬������ԭ��������һ�¡�
 *��̬������ȫ�ֺ������⺯��������������������+Type��׺��
 *��̬������ȫ�ֺ������⺯����stub����������������Stubǰ׺+������+Type��׺��
 *��̬������ȫ�ֺ������⺯����stub�����Ĳ�������ԭ��������һ�£���ôд����Ϊ�����ܻ��ԭ������������������
*/

typedef mp_int32 (*CRootCallerExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
typedef mp_int32 (*StubCRootCallerExecType)(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);

typedef mp_int32 (*CSystemExecExecSystemWithEchoType)(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect);
typedef mp_int32 (*StubCSystemExecExecSystemWithEchoType)(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect);

typedef mp_void (*DoSleepType)(mp_uint32 ms);
typedef mp_void (*StubDoSleepType)(mp_uint32 ms);

/* Stub ������ȡ������Stub+(Class��+)ԭ������+��Ҫ�ĵĽ��˵��(+�����ô�)
 * ���磺StubopenEq0��������ȡ��open�����ģ�����ֵΪ0��
 * Lt��С��    Eq������  Ok���з���ֵ�����
 * ������������������˵��
*/
mp_void StubCLoggerLogVoid(mp_void* pthis)
{
    return;
}
mp_int32 StubCRootCallerExecEq0(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    return 0;
}
mp_int32 StubCRootCallerExecActiveNode(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        pvecResult->push_back("test");
    }
    return 0;
}
mp_int32 StubCRootCallerExecClusterInfo(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        pvecResult->push_back("strClusterName:strResGrpName:strVgActiveMode");
    }
    return 0;
}
mp_int32 StubCRootCallerExecStart(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        switch (iCommandID)
        {
            case ROOT_COMMAND_HASYS:
                pvecResult->push_back("RUNNING");
            break;
            case ROOT_COMMAND_LSSRC:
            case ROOT_COMMAND_CLLSNODE:
                pvecResult->push_back("ST_STABLE");
            break;
            case ROOT_COMMAND_CMVIEWCL:
                pvecResult->push_back("running");
            break;
            case ROOT_COMMAND_RHCS_CLUSTAT:
                pvecResult->push_back("Online,Local,rgmanager");
            break;
            default:
            break;
        }
    }
    return 0;
}

mp_int32 StubCRootCallerExecStartErr(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        pvecResult->push_back("ErrStatus");
    }
    return 0;
}
mp_int32 StubCSystemExecExecSystemWithEchoEq0(mp_string& strCommand, vector<mp_string> &strEcho, mp_bool bNeedRedirect)
{
    strEcho.push_back("locking_type = 3"); 
    return 0;
}
mp_void StubDoSleepVoid(mp_uint32 ms)
{
    return;
}
#endif
