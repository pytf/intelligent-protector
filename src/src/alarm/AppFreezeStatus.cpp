/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "alarm/AppFreezeStatus.h" 
#include "common/Defines.h"
#include "common/Log.h"
#include <sstream>
/*------------------------------------------------------------ 
Description  : ����Ӧ��״̬��sqlite���ݿ�
Input        : stStatus---����״̬
Output       : 
Return       :  MP_SUCCESS---״̬�����Ѿ����ڻ�ִ��sql�ɹ�
               iRet---ִ��sqlʧ�ܣ����ض�Ӧ������
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CAppFreezeStatus::Insert(const freeze_status& stStatus)
{
    LOGGUARD("");
    if (IsExist(stStatus))
    {
        //�Ѿ����ڣ���ӡ��־
        COMMLOG(OS_LOG_INFO,LOG_COMMON_INFO,"%s is exist",stStatus.strKey.c_str());
        return MP_SUCCESS;
    }

    ostringstream buff;
    buff << "insert into " << AppStatusTable <<"(Key) values(?);";
    mp_string sql = buff.str();

    DbParamStream dps;
    DbParam dp = stStatus.strKey;
    dps << dp;
    
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.ExecSql(sql, dps);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "db.ExecSql failed,iRet = %d.",iRet);
    }
    return iRet;
}

/*------------------------------------------------------------ 
Description  : ɾ��Ӧ��״̬���е�ĳ����Ϣ
Input        : stStatus---����״̬
Output       : 
Return       :  MP_SUCCESS---״̬���в����ڻ�ִ��sql�ɹ�
               iRet---ִ��sqlʧ�ܣ����ض�Ӧ������
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CAppFreezeStatus::Delete(const freeze_status& stStatus)
{
    LOGGUARD("");
    if (!IsExist(stStatus))
    {
        //�����ڣ���ӡ��־
        COMMLOG(OS_LOG_INFO,LOG_COMMON_INFO,"%s is not exist",stStatus.strKey.c_str());
        return MP_SUCCESS;
    }

    std::ostringstream buff;
    buff<<"delete from " << AppStatusTable <<" where Key == ?";
    mp_string sql = buff.str();
    
    DbParamStream dps;
    DbParam dp = stStatus.strKey;
    dps << dp;
    
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.ExecSql(sql, dps);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "db.ExecSql failed,iRet = %d.",iRet);
    }
    return iRet;
}
/*------------------------------------------------------------ 
Description  : ��ȡӦ��״̬
Input        : 
Output       : stStatus---����״̬
Return       :  
               
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_void CAppFreezeStatus::Get(freeze_status& stStatus)
{
    LOGGUARD("");
    if (IsExist(stStatus))
    {
        stStatus.iStatus = DB_FREEZE;
    }
    else
    {
        stStatus.iStatus = DB_UNFREEZE;
    }
}
/*------------------------------------------------------------ 
Description  : ��ȡ����Ӧ��״̬
Input        : 
Output       : vecStatus---״̬�б�
Return       :  MP_SUCCESS---��ȡ״̬�ɹ�
               iRet---��ȡʧ�ܣ����ش�����
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CAppFreezeStatus::GetAll(vector<freeze_status>& vecStatus)
{
    LOGGUARD("");
    ostringstream buff;
    buff<<"select Key from " << AppStatusTable;
    
    DbParamStream dps;
    
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.QueryTable(buff.str(), dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "db.QueryTable failed, iRet = %d.",iRet);
        return iRet;
    }

    for (mp_int32 iRow = 1; iRow <= iRowCount; ++iRow)
    {
        freeze_status stStatus;
        readBuff >> stStatus.strKey;
        stStatus.iStatus = DB_FREEZE;
        vecStatus.push_back(stStatus);
    }

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : ��ѯӦ��״̬�Ƿ���ڱ���
Input        : stStatus---����״̬
Output       : 
Return       :  MP_TRUE---��ѯ��Ӧ��״̬
               MP_FALSE---iRowCount>=0��false---��ѯ��ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_bool CAppFreezeStatus::IsExist(const freeze_status& stStatus)
{
    LOGGUARD("");
    ostringstream buff;
    buff<<"select Key from " << AppStatusTable <<" where Key == ?";

    mp_string strSql = buff.str();
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "buff is = %s.", strSql.c_str());
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    DbParamStream dps;
    DbParam dp = stStatus.strKey;
    dps << dp;
    
    CDB &db = CDB::GetInstance();
    mp_int32 iRet = db.QueryTable(strSql, dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "db.QueryTable failed, iRet = %d.",iRet);
        return MP_FALSE;
    }

    if (0 < iRowCount)
    {
        return MP_TRUE;
    }
    else
    {
        return MP_FALSE;
    }
}


