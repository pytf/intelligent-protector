/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

//------------------------------------------------------------------------------
//  filename:   App.h
//  desc:   ���ݲ�����Ӧ�����Ͳ����ؽӿڶ��壬������ͬһ������װ��ͬӦ�õĳ���������Ҫ֧��ͬһ������װͬһӦ��
//      ����ͬһ��������ͬӦ�õĲ�ͬʵ����Ҫ�ṩ��ͬ��Ӧ�ü�Ȩ���û������롣����ǰʵ��SQLServer����֧�ֲ���ϵͳ��Ȩ��ʽ����
//      �����������ӿڣ�������һ������ִ��ʧ��ʱ���жϵ�ǰ�ӿڲ�����ִ�У����ض�Ӧ��ʧ����Ϣ��
//  Copyright (C), 2015-2025, Huawei Tech. Co., Ltd.
//------------------------------------------------------------------------------

#ifndef __AGENT_APP_H__
#define __AGENT_APP_H__

#include <vector>
#include <list>
#include "common/Types.h"
#include "vss/requester/Requester.h"
#include "apps/sqlserver/SqlServer.h"
#include "apps/oracle/Oracle.h"


//�����windows�ϵ�vss������ʧ����Ӧ�е�"dbName"�ֶε�ֵ�̶���д"VSS"
#define DBNAME_FOR_VSS_IN_ERR_RESPONSE     "VSS"

typedef enum
{
    APP_TYPE_ORACLE = 1,
    APP_TYPE_SQLSERVER,
    APP_TYPE_EXCHANGE,
    APP_TYPE_DB2,
    APP_TYPE_BUTT
}APP_TYPE_E;

typedef struct tag_app_info
{
    APP_TYPE_E enAppType;      //Ӧ������
    mp_string strInstName;     //ʵ������
    mp_string strDBName;       //���ݿ���(Ӧ������ΪExchange��ʱ���ʾ�������ݿ�)
    mp_string strVersion;      //�汾
    mp_string strStorageGroup; //�洢��(��Exchange 2007��Ч)
}app_info_t;

typedef struct tag_app_auth_info
{
    mp_string strUserName;    //Ӧ�ü�Ȩ�û���
    mp_string strPasswd;      //Ӧ�ü�Ȩ����
}app_auth_info_t;

typedef struct tag_app_failed_info
{
    mp_int32 iErrorCode;     //������
    mp_string strDbName;     //�������ƣ�Windows�ϵ�VSS����ʱ�̶���д"VSS"��
}app_failed_info_t;

class CApp
{
private:
#ifdef WIN32
    VSSRequester* m_pVssRequester;
    CSqlServer m_sqlserver;
#endif
    COracle m_oracle;
    
public:
    CApp();
    ~CApp();
    
    mp_int32 QueryInfo(vector<app_info_t>& vecAppInfos);
    mp_int32 Freeze(app_auth_info_t& appAuthInfo, mp_time& tFreezeTime, vector<app_failed_info_t>& vecAppFailedList);
    mp_int32 UnFreeze(app_auth_info_t& appAuthInfo, vector<app_failed_info_t>& vecAppFailedList);    
    mp_int32 EndBackup(app_auth_info_t& appAuthInfo, mp_int32 iBackupSucc, vector<app_failed_info_t>& vecAppFailedList);    
    mp_int32 TruncateLog(app_auth_info_t& appAuthInfo, mp_time tTruncateTime, vector<app_failed_info_t>& vecAppFailedList);    
    mp_int32 QueryFreezeState(app_auth_info_t& appAuthInfo, mp_int32& iState);
    mp_int32 UnFreezeEx(app_auth_info_t& appAuthInfo);
    
private:
    mp_int32 QuerySqlInfo(vector<app_info_t>& vecAppInfos);
    mp_int32 QueryOracleInfo(vector<app_info_t>& vecAppInfos);
    mp_int32 QueryOracleFreezeState(app_auth_info_t& appAuthInfo, mp_int32& iState);
    mp_int32 FreezeOracle(app_auth_info_t& appAuthInfo, vector<app_failed_info_t>& vecAppFailedList);
    mp_int32 UnFreezeOracle(app_auth_info_t& appAuthInfo, vector<app_failed_info_t>& vecAppFailedList);
    mp_int32 UnFreezeExOracle(app_auth_info_t& appAuthInfo);
    mp_int32 TruncateOracleLog(app_auth_info_t& appAuthInfo, mp_time tTruncateTime, vector<app_failed_info_t>& vecAppFailedList);
#ifdef WIN32
    mp_int32 IsAppInstalled(mp_bool& bIsAppInstalled);
    mp_int32 FreezeVss(vector<app_failed_info_t>& vecAppFailedList);
    mp_int32 UnFreezeVss(vector<app_failed_info_t>& vecAppFailedLists);
    mp_int32 EndBackupVss(mp_int32 iBackupSucc, vector<app_failed_info_t>& vecAppFailedList);
    mp_int32 TruncateSqlLog(app_auth_info_t& appAuthInfo, vector<app_failed_info_t>& vecAppFailedList);
#endif
};

#endif //__AGENT_APP_H__

