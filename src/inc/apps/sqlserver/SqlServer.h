/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_SQL_SERVER_H__
#define __AGENT_SQL_SERVER_H__
#include <vector>
#include <list>
#include "common/Types.h"
#include "vss/requester/Requester.h"

#ifdef WIN32
#define SQL_SERVER_INFO_TOTAL_COUNT             6
#define SQL_SERVER_ONLINE                       0
#define SQL_SERVER_OFFLINE                      1
#define SQL_SERVER_RECOVERY_MODEL_UNKNOW        0
#define SQL_SERVER_RECOVERY_MODEL_FULL          1
#define SQL_SERVER_RECOVERY_MODEL_BULK_LOG      2
#define SQL_SERVER_RECOVERY_MODEL_SIMPLE        3

// ���� SQLServer ��������ݿ������;
#define SQL_SERVER_OPTCODE_STOP                 "0"
#define SQL_SERVER_OPTCODE_START                "1"
#define SQL_SERVER_OPTCODE_TESTCONN             "2"
#define SQL_SERVER_OPTCODE_TRUNCATE             "3"

#define SQL_SERVER_QUERY_REG_ERR_NOT_EXIST      2
#define SQL_SERVER_REG_KEY                      "SOFTWARE\\Microsoft\\Microsoft SQL Server\\Instance Names\\SQL"

// ����SQLServer���ݿ��е�ϵͳ���ݿ�
#define MASTER									"master"
#define TEMPDB									"tempdb"
#define MODEL									"model"
#define MSDB									"msdb"	

// sqlserver rest�ӿڲ�������;
typedef struct tag_sqlserver_info
{
    mp_string strInstName;   // ʵ������;
    mp_string strDBName;     // ���ݿ���;
    mp_string strUser;       // �û���;
    mp_string strPasswd;     // ����;
    mp_string strVersion;    // �汾;
    mp_string strIsCluster;  // ��Ⱥ��ʾ;
    mp_string strCheckType;  // ������,������ͣ���ݿ�;
    mp_string strState;      // ���ݿ�״̬;
    mp_int32 iRecoveryModel; //�ָ�ģʽ 0 -- unkonw��1 -- full, 2 -- bulk logged, 3 -- simple��
}sqlserver_info_t;


// ��ѯSQLServer���еķ�����Ϣ, ���ݿ������ϸ��Ϣ;
typedef struct tag_sqlserver_lun_info
{
    mp_string strLunID;         // lun id;
    mp_string strArraySN;       // Lun ��������SN;
    mp_string strWWN;           // Lun ���ڵ�WWN;
    mp_string strDeviceName;    // �洢������Ϣ;
    mp_string strVOLName;       // ����; \\?\Volume{997dc5aa-351b-11e3-89f5-000c29d2c650}
    mp_string strLBA;           // ����LBA�׵�ַ; 
    mp_int32  iDeviceType;      // �洢����;
                                //    0 �C �ļ�ϵͳ   ��ע:SQLServerʹ��
                                //    1 �C ���豸
                                //    2 �C ASMLib���̣�
                                //    3 �C ASM���豸��
                                //    4 �C ASM�����ӣ�
                                //    5 �C ASMOnUdev����Linuxʹ�ã���udev��ʹ�ã�
                                //    6 �C windows ASM���̱�ʶ��
}sqlserver_lun_info_t;

// SQLServer ���л�����Ϣ;
typedef struct tag_storage_basic
{
    mp_string strLunID;     // LUN ID;
    mp_string strArraySN;   // LUN�������е����к�;
    mp_string strWWN;       // LUN��WWN;
    mp_int32  iDiskNum;     // LUN�����������;
}storage_basic_t;

typedef struct tag_sqlserver_freeze_info
{
    mp_string strDBName;
    vector<mp_string> vecDriveLetters;
}sqlserver_freeze_info_t;

typedef struct tag_sqlserver_unfreeze_info
{
    mp_string strDBName;
    vector<mp_string> vecDriveLetters;
}sqlserver_unfreeze_info_t;

class CSqlServer
{
private:
    VSSRequester* m_pVssRequester;
public:
    CSqlServer();
    ~CSqlServer();

    mp_int32 IsInstalled(mp_bool& bIsInstalled);
    mp_int32 GetInfo(vector<sqlserver_info_t> &vecdbInstInfo);
    mp_int32 GetLunInfo(sqlserver_info_t& stdbinfo, vector<sqlserver_lun_info_t>& vecLunInfos);
    mp_int32 Start(sqlserver_info_t& stdbInfo);
    mp_int32 Stop(sqlserver_info_t& stdbInfo);
    mp_int32 Test(sqlserver_info_t& stdbInfo);
    mp_int32 TruncateTransLog(sqlserver_info_t& dbInfo);
    mp_int32 Freeze(vector<sqlserver_freeze_info_t> vecFreezeInfos);  
    mp_int32 UnFreeze(vector<sqlserver_unfreeze_info_t> vecUnFreezeInfos);
    mp_int32 GetFreezeStat();

private:
    mp_int32 AnalyseDBQueryResult(vector<sqlserver_info_t>& vecDbInfo, const vector<mp_string>& vecResult);
    mp_int32 QueryDBTableSpaceLUNInfo(sqlserver_info_t& stdbinfo, vector<sqlserver_lun_info_t>& vecLunInfos);
    mp_int32 GetDBFilePath(vector<mp_string> &lstPath, const mp_string &strParam);
    mp_int32 GetDBLUNInfoByPath(vector<sqlserver_lun_info_t> &vecLunInfos, const mp_string &path);
    mp_int32 GetDiskInfoList(vector<storage_basic_t> &rlstDiskInfoWin);
    mp_void  BuildScriptParams(mp_string &rstrParam, const sqlserver_info_t& stdbinfo);
    mp_void  ReplaceStr(const mp_string &oldStr, mp_string &newStr, const mp_string &old_value, const mp_string &new_value);

};

#endif
#endif //__AGENT_SQL_SERVER_H__

