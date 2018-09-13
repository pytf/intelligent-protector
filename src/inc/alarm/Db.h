/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _DB_H_
#define _DB_H_

#include <list>
#include "common/Types.h"
#include "common/Thread.h"
#include "sqlite/sqlite3.h"
#include <stdlib.h>
#include <sstream>

//sqlite���ݿ����
#define TrapServerTable "TrapInfoTable"   //��¼ע��trap server��Ϣ��
#define AlarmTable "AlarmTable"           //��¼�ϱ��澯��Ϣ��
#define AlarmTypeTable "AlarmTypeTable"   //��¼��ˮ����Ϣ��
#define AppStatusTable "AppStatusTable"    //��¼Ӧ�ö���״̬��Ϣ��
#define FreezeObjTable "FreezeObjTable"    //��¼�������

//sqlite���ݿ�����ֶ�����
//AlarmTable���ֶ���
#define titleAlarmID "AlarmID"
#define titleAlarmLevel "AlarmLevel"
#define titleAlarmParam "AlarmParam"
#define titleAlarmType "AlarmType"
#define titleEndTime "AlarmClearTime"
#define titleStartTime "AlarmBeginTime"
#define titleAlarmSerialNo "AlarmSerialNo"

//AlarmTypeTable���ֶ�����
#define titleAlarmSN   "AlarmSN"

//FreezeObjTable���ֶ�����
#define InstanceName "InstanceName"  //���ݿ�ʵ������
#define DBName "DBName"              //���ݿ�����
#define BeginStatus "BeginStatus"    //���ݿ⿪ʼ״̬
#define LoopTime "LoopTime"          //�����ѯʱ��
#define User "User"                  //���ݿ����
#define MP "MP"                      //���ݿ����
#define JsonData "JsonData"          //�����json��Ϣ
#define AppType "AppType"            //Ӧ������
#define BeginTime "BeginTime"        //��ؿ�ʼʱ��


#define atoint32(x)  mp_int32(atoi(x)) 
#define atoint64(x)  mp_int64(atol(x))
//#define atouint32(x)  mp_uint32(atoi(x))
//#define atouint64(x)  mp_uint64(atol(x))
#define IntToString(i, s) {std::stringstream ss;ss << i;ss >> s;}

 //ʹ�ô˺����Ҫ�ֺ�
#define FREE_STMT_THEN_DISCONNECT_RETURN_MPFAILED(x) { \
  COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, x); \
  if(SQLITE_OK != sqlite3_finalize(stmt)) \
     COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "sqlite3_finalize failed"); \
  if(MP_SUCCESS!= Disconnect()) \
     COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Disconnect DB failed, iRet = %d", iRet); \
  return MP_FAILED; \
}


typedef enum
{
    DB_PARAM_TYPE_INT32,
    DB_PARAM_TYPE_INT64,
    DB_PARAM_TYPE_UINT32,
    DB_PARAM_TYPE_UINT64,
    DB_PARAM_TYPE_STRING
}DbParamType;

struct DbParam
{
    DbParamType m_type;
    mp_string m_value;
    DbParam()
    {
        m_type = DB_PARAM_TYPE_STRING;
        m_value = "";
    }
    DbParam(mp_string value)
    {
        m_type = DB_PARAM_TYPE_STRING;
        m_value = value;
    }
    DbParam(mp_int32 value);
    DbParam(mp_int64 value);
    DbParam(mp_uint32 value);
    DbParam(mp_uint64 value);
};
//ʹ��Ԥ����ģʽ��ѯʱ�����
class DbParamStream
{
public:
    DbParamStream(){}
    ~DbParamStream(){}
    mp_void Clear()
    {
        m_ParamList.clear();
    }
    mp_bool Empty()
    {
        return m_ParamList.empty();
    }
    DbParam operator>>(DbParam& param);
    DbParam operator<<(DbParam& param)
    {
        m_ParamList.push_back(param);
        return param;
    }
private:
    list<DbParam> m_ParamList;
};

class DBReader
{
public:
    DBReader();
    ~DBReader();
    mp_string operator>>(mp_string& strResult);
    mp_string operator<<(mp_string& strResult);
    mp_void Clear();
    mp_bool Empty();
private:
    list<mp_string> m_lstResult;
};


class CDB
{
public:
  
    //sql���Ԥ���뷽ʽ
    mp_int32 ExecSql(mp_string strSql, DbParamStream &dpl);
    mp_int32 QueryTable(mp_string strSql, DbParamStream &dpl, 
                                         DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount);

    static CDB& GetInstance(void)
    {
        return m_Instance;
    }

    virtual ~CDB()
    {
        CMpThread::DestroyLock(&m_InstanceLock);
    }
private:
    mp_int32 Connect();
    mp_int32 Disconnect();

    //ExecSql������֣����ͺ������Ӷ�
    sqlite3_stmt* SqlPrepare(mp_string sql);
    mp_int32 SqlBind(sqlite3_stmt* stmt, DbParamStream &dps);
    mp_int32 SqlExecute(sqlite3_stmt* stmt);
    mp_int32 SqlQuery(sqlite3_stmt* stmt, DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount);
    
    sqlite3 * m_pDB; //m_pDB���ڴ������sqlit�Լ���֤��sqlite3_openʱ���룬sqlite3_closeʱ�ͷ�
    list<mp_string> m_stringList;//���ڴ��sqlite_bind_text���ַ���
    CDB(CDB & cdb){}
    CDB & operator = (const CDB& cdb);
    CDB() : m_pDB(NULL)
    {
        CMpThread::InitLock(&m_InstanceLock);
    }

private:
    static CDB m_Instance;
    thread_lock_t m_InstanceLock;
    
};

#endif

