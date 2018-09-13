/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __ORACLETEST_H__
#define __ORACLETEST_H__

#define private public

#include "apps/oracle/Oracle.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/String.h"
#include "common/RootCaller.h"
#include "common/SystemExec.h"
#include "array/Array.h"
#include <sstream>
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class COracleTest : public testing::Test
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
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* COracleTest::m_stub;

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

typedef mp_int32 (*CArrayGetArrayVendorAndProductType)(mp_string& strDev, mp_string& strvendor, mp_string& strproduct);
typedef mp_int32 (*StubCArrayGetArrayVendorAndProductType)(mp_string& strDev, mp_string& strvendor, mp_string& strproduct);

typedef mp_int32 (*CArrayGetArraySNType)(mp_string& strDev, mp_string& strSN);
typedef mp_int32 (*StubCArrayGetArraySNType)(mp_string& strDev, mp_string& strSN);

typedef mp_int32 (*CArrayGetLunInfoType)(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID);
typedef mp_int32 (*StubCArrayGetLunInfoType)(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID);

typedef mp_void (*StrSplitType)(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep);
typedef mp_void (*StubStrSplitType)(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep);

/* Stub ������ȡ������Stub+(Class��+)ԭ������+��Ҫ�ĵĽ��˵��(+�����ô�)
 * ���磺StubopenEq0��������ȡ��open�����ģ�����ֵΪ0��
 * Lt��С��    Eq������  Ok���з���ֵ�����
 * ������������������˵��
*/
mp_void StubStrSplit(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep)
{
	int num = 0;
	for(num = 0; num < 6;num++)
	{
		vecTokens.push_back("test");
	}
	
    return;
}

mp_void StubStrSplit0(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep)
{	
    return;
}

mp_void StubCLoggerLogVoid(mp_void* pthis)
{
    return;
}
mp_int32 StubCRootCallerExecEq0(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        pvecResult->push_back("0");
    }
    return 0;
}
mp_int32 StubCRootCallerExecGetDBInfo(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        pvecResult->push_back("v11;test;test;1;test;/oralcehome");
    }
    return 0;
}
mp_int32 StubCRootCallerExecGetDBLUNInfo(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (!pvecResult)
    {
        return 0;
    }
    if (ROOT_COMMAND_SCRIPT_QUERYORACLELUNINFO == iCommandID)
    {
        pvecResult->push_back("1;1;strSystemDevice;strDeviceName;strDevicePath;strVgName;strASMDiskGroup;strUDEVRes;strUDEVDevice");
    }
    return 0;
}
mp_int32 StubCRootCallerExecGetInstances(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        pvecResult->push_back("1");
        pvecResult->push_back("test");
    }
    return 0;
}
mp_int32 StubCRootCallerExecGetUDEVInfo(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        pvecResult->push_back("test");
        pvecResult->push_back("test");
    }
    return 0;
}
mp_int32 StubCRootCallerExecGetUDEVInfo1(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        pvecResult->push_back("test");
        pvecResult->push_back("test");
    }
    return ERROR_SCRIPT_ORACLE_NOT_INSTALLED;
}
mp_int32 StubCRootCallerExecGetUDEVInfo2(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        pvecResult->push_back("test");
        pvecResult->push_back("test");
    }
    return -1;
}
mp_int32 StubCArrayGetArrayVendorAndProductOk(mp_string& strDev, mp_string& strvendor, mp_string& strproduct)
{
    strvendor = ARRAY_VENDER_HUAWEI;
    strproduct = ARRAY_VENDER_HUAWEI;
    return 0;
}
mp_int32 StubCArrayGetArraySNEq0(mp_string& strDev, mp_string& strSN)
{
    strSN = "test";
    return 0;
}
mp_int32 StubCArrayGetLunInfoEq0(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID)
{
    return 0;
}

mp_int32 StubCRootCallerExecCheckCDB(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        pvecResult->push_back("1");
    }    
    return 0;
}

mp_int32 StubCRootCallerExecCheckCDB2(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (pvecResult)
    {
        pvecResult->push_back("0");
    }    
    return 0;
}

mp_int32 StubCRootCallerExecCheckCDB3(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{ 
    return ERROR_SCRIPT_ORACLE_ASM_INSTANCE_NOSTART;
}

mp_int32 StubCRootCallerExecGetPDBInfo(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
	static mp_int32 iCounter = 0;
	if (iCounter++ == 0)
	{
		return MP_FAILED;
	}
	if (iCounter++ == 1)
	{
		return MP_SUCCESS;
	}
    if (!pvecResult)
    {
        return MP_SUCCESS;
    }
    if (ROOT_COMMAND_SCRIPT_QUERYORACLEPDBINFO == iCommandID)
    {
        pvecResult->push_back("2;PDB$SEED;READ ONLY");
    }
    return MP_SUCCESS;
}
mp_int32 StubCRootCallerExecStartPDB(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
	static mp_int32 iCounter = 0;
	if (iCounter++ == 0)
	{
		return MP_FAILED;
	}
	return MP_SUCCESS;
}

mp_int32 StubGetLunInfoByStorageType(oracle_db_info_t stDBInfo, vector<oracle_lun_info_t> &vecLUNInfos, mp_string strStorageType)
{
	return MP_SUCCESS;
}

#endif
