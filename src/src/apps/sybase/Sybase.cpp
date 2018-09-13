/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/


#include "apps/sybase/Sybase.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/String.h"
#include "common/Defines.h"
#include "common/RootCaller.h"

CSybase::CSybase()
{
}

CSybase::~CSybase()
{
}


/*------------------------------------------------------------ 
Description  : ����ִ�б������Ժ�����ֹͣ�ű��������
Input        : stdbinfo -- ���ݿ���Ϣ
                  strOperType --��������
Output       : strParam -- ��������ַ���
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void CSybase::BuildScriptParam(sybase_db_info_t &stdbInfo, mp_string strOperType, mp_string &strParam)
{
    strParam = mp_string(SCRIPTPARAM_INSTNAME) + stdbInfo.strinstName + mp_string(NODE_COLON)
        + mp_string(SCRIPTPARAM_DBNAME) + stdbInfo.strdbName + mp_string(NODE_COLON)
        + mp_string(SCRIPTPARAM_DBUSERNAME) + stdbInfo.strDBUsername + mp_string(NODE_COLON)
        + mp_string(SCRIPTPARAM_DBPASSWORD) + stdbInfo.strDBPassword + mp_string(NODE_COLON)
        + mp_string(SCRIPTPARAM_OPERTYPE) + strOperType;
}


/*------------------------------------------------------------ 
Description  : ����ִ�б������Ժ�����ֹͣ�ű��������
Input        : stdbinfo -- ���ݿ���Ϣ
Output       : strParam -- ��������ַ���
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void CSybase::BuildScriptParam(sybase_db_info_t &stdbInfo, mp_string &strParam)
{
    strParam = mp_string(SCRIPTPARAM_INSTNAME) + stdbInfo.strinstName + mp_string(NODE_COLON)
        + mp_string(SCRIPTPARAM_DBNAME) + stdbInfo.strdbName + mp_string(NODE_COLON)
        + mp_string(SCRIPTPARAM_DBUSERNAME) + stdbInfo.strDBUsername + mp_string(NODE_COLON)
        + mp_string(SCRIPTPARAM_DBPASSWORD) + stdbInfo.strDBPassword;
}

/*------------------------------------------------------------ 
Description  : �������ݿ�
Input        :stdbInfo -- ���ݿ���Ϣ
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CSybase::StartDB(sybase_db_info_t &stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to start sybase database, instance %s, database %s.", 
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());
    BuildScriptParam(stdbInfo, SCRIPTPARAM_STARTDB, strParam);
    
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_STARTSYBASE, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute start sybase database, instance %s, database %s script failed, iRet %d.", 
            stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "start sybase database succ, instance %s, database %s.", 
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :ֹͣ���ݿ�
Input        :stdbInfo -- ���ݿ���Ϣ
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CSybase::StopDB(sybase_db_info_t &stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to stop sybase database, instance %s, database %s.", 
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());
    BuildScriptParam(stdbInfo, SCRIPTPARAM_STOPDB, strParam);
    
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_STOPSYBASE, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute stop sybase database, instance %s, database %s script failed, iRet %d.", 
            stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Stop sybase database succ, instance %s, database %s.", 
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :�������ݿ⣬�������ݿ�������ͣ�����ݿ�
Input        :vecdbInfo --����������ݿ��б�
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CSybase::FreezeDB(sybase_db_info_t &stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to freeze sybase database, instance %s, database %s.", 
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());
    BuildScriptParam(stdbInfo, SCRIPTPARAM_FREEZEDB, strParam);
    
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_FREEZESYBASE, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute freeze sybase database failed, instance %s, database %s script failed, iRet %d.", 
            stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze sybase database succ, instance %s, database %s.", 
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :�ⶳ���ݿ⣬�������ݿ��������ͷ����ݿ�
Input        :vecdbInfo --���ͷŵ����ݿ��б�
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CSybase::ThawDB(sybase_db_info_t &stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to thaw sybase database, instance %s, database %s.", 
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_THAWDB, strParam);
    
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_THAWSYBASE, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute thaw sybase database failed, instance %s, database %s script failed, iRet %d.", 
            stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Thaw sybase database succ, instance %s, database %s.", 
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :��ѯ���ݿⶳ��״̬���������ݿ��������ѯ���ݿ�״̬
Input        :stdbInfo --���ݿ���Ϣ
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CSybase::GetFreezeStatus(sybase_db_info_t &stdbInfo, mp_int32 &iFreezeState)
{
    mp_string strParam;
    vector<mp_string> vecResult;
    mp_int32 iRet = MP_SUCCESS;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get sybase database freeze status, instance %s, database %s.", 
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_FREEZESTATUS, strParam);
    
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYFREEZESTATUS, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute get sybase database freeze status, instance %s, database %s script failed, iRet %d.", 
            stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    
    if (vecResult.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The result of get sybase freeze state is empty.");
        return ERROR_COMMON_OPER_FAILED;
    }
    iFreezeState = atoi(vecResult.front().c_str());
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get sybase database status succ, instance %s, database %s, state %d.", 
        stdbInfo.strinstName.c_str(), stdbInfo.strdbName.c_str(), iFreezeState);
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : �����������ݿ�
Input        :stdbInfo -- ���ݿ���Ϣ
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CSybase::Test(sybase_db_info_t &stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to test instance(%s).", stdbInfo.strinstName.c_str());    
    BuildScriptParam(stdbInfo, strParam);
    
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_TESTSYBASE, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute test instance(%s) script failed, iRet %d.", stdbInfo.strinstName.c_str(), iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Test instance(%s) succ.", stdbInfo.strinstName.c_str());
    return MP_SUCCESS;
}

