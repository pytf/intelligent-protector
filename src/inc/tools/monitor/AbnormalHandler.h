/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _MONITOR_HANDLER_H_
#define _MONITOR_HANDLER_H_

#include "common/Types.h"
#include "common/Thread.h"
#include <sstream>

#define MONITOR_SECTION "Monitor"

#define THRD_CNT "thread_count"
#define HANDLER_CNT "handle_count"
#define PM_SIZE "pm_size"
#define VM_SIZE "vm_size"
#define CPU_USAGE "cpu_usage"
#define TMPFILE_SIZE "tmpfile_size"
#define RETRY_TIME "retry_time"
#define MONITOR_INTERVAL "monitor_interval"
#define NGINX_LOG_SIZE "nginx_log_size"
#define NGINX_LOG_FILE "error.log"

#ifdef WIN32
#define ROATE_NGINX_LOG_SCRIPT "rotatenginxlog.bat"
#else
#define ROATE_NGINX_LOG_SCRIPT "rotatenginxlog.sh"
#endif

#define MAX_TMP_EXIST_TIME 3600*24  //��ʱ�ļ����ڵ��ʱ�䣬��λ��
#define MONITOR_SLEEP_TIME 1000
#define ALARM_SEND_FAILED_TIME 3
#define MAX_BUF_SIZE 200
#define FAULT_NUM -1

#define DEFAULT_XML_RETRY_TIME_VALUE 3
#define DEFAULT_XML_INTERVAL_VALUE 30

#define HP_TOP_CMD "top -n 1000 -f"

#define AGENT_PROCESS        0
#define NGINX_PROCESS        1

typedef struct tag_abnormal_occur_times
{
    tag_abnormal_occur_times()
    {
        iPmSizeOverTimes = 0;
        iVmSizeOverTimes = 0;
        iHandlerOverTimes = 0;
        iThreadOverTimes = 0;
        iCpuUsageOverTimes = 0;
        iTmpFileSizeOverTimes = 0;
    }
    mp_int32 iPmSizeOverTimes;  //�����ڴ��������������ļ�����
    mp_int32 iVmSizeOverTimes;  //�����ڴ��������������ļ�����
    mp_int32 iHandlerOverTimes; //�ļ������������������������ļ�����
    mp_int32 iThreadOverTimes;  //�̸߳����������������ļ�����
    mp_int32 iCpuUsageOverTimes; //CPU�������������������ļ�����
    mp_int32 iTmpFileSizeOverTimes;
}abnormal_occur_times_t;

typedef struct tag_monitor_data
{
    tag_monitor_data()
    {
        bExist = MP_FALSE;
        iHandlerNum = 0;
        iThreadNum = 0;
        ulPmSize = 0;
        ulVmSize = 0;
        fCpuUsage = 0.0;
        ulTmpFileTotalSize = 0;
        uiNginxLogSize = 0;
    }
    mp_bool bExist;
    mp_int32 iHandlerNum;
    mp_int32 iThreadNum;
    mp_uint64 ulPmSize;  //��λKbyte
    mp_uint64 ulVmSize;  //��λKbyte
    mp_float fCpuUsage; //��λ%
    mp_uint32 uiNginxLogSize;     //��λKbyte
    mp_uint64 ulTmpFileTotalSize; //��λKbyte
}monitor_data_t;

typedef struct tag_monitor_process_config
{
    tag_monitor_process_config()
    {
        bMonitored = MP_FALSE;
        iPmSizeCfg = 0;
        iVmSizeCfg = 0;
        iHandlerNumCfg = 0;
        iThreadNumCfg = 0;
        fCpuUsageCfg = 0;
        iTmpFileTotalSizeCfg = 0;
        iNginxLogSizeCfg = 0;
    }
    mp_bool bMonitored;
    mp_int32 iPmSizeCfg;
    mp_int32 iVmSizeCfg;
    mp_int32 iHandlerNumCfg;
    mp_int32 iThreadNumCfg;
    mp_float fCpuUsageCfg;
    mp_int32 iTmpFileTotalSizeCfg;  //��λK,agent�������в���
    mp_int32 iNginxLogSizeCfg;      //��λK,nginx�������в���
}monitor_process_config_t;

typedef struct tag_monitor_common_config
{
    tag_monitor_common_config()
    {
        iRetryTime = 3;
        iMonitorInterval = 30;
    }
    mp_int32 iRetryTime;
    mp_int32 iMonitorInterval;

}monitor_common_config_t;

class CAbnormalHandler
{
public:
    static CAbnormalHandler& GetInstance()
    {
        return m_instance;
    }

    ~CAbnormalHandler()
    {
    }

    mp_int32 Init();
    mp_int32 Handle();
private:
    CAbnormalHandler()
    {
        m_bAgentCpuAlarmSend = MP_FALSE;
        m_bAgentCpuAlarmResumed = MP_FALSE;
        m_bNginxCpuAlarmSend = MP_FALSE;
        m_bNginxCpuAlarmResumed = MP_FALSE;
        m_bNeedResumed = MP_TRUE;
        m_iAgentAlarmSendFailedTimes = 0;
        m_iNginxAlarmSendFailedTimes = 0;
        (mp_void)memset_s(&m_hHandleThread, sizeof(m_hHandleThread), 0, sizeof(m_hHandleThread));
        //Coverity&Fortify��:UNINIT_CTOR
        //Coveirty&Fortify����ʶ��˾��ȫ����memset_s����ʾm_dispatchTid.os_idδ��ʼ��
        m_iThreadStatus = THREAD_STATUS_IDLE;
        m_bNeedExit = MP_FALSE;
    }

    mp_int32 GetAgentMonitorData(monitor_data_t& stMonitorData);
    mp_int32 GetNginxMonitorData(monitor_data_t& stMonitorData);
    mp_int32 GetMonitorData(mp_string strProcessID, monitor_data_t& stMonitorData);
    mp_int32 GetHPMonitorData(mp_string strProcessID, monitor_data_t& stMonitorData);
    mp_int32 StartAgent();
    mp_int32 StartNginx();
    mp_int32 RestartAgent();
    mp_int32 RestartNginx();
    mp_int32 SendCPUAlarm(mp_int32 iProcessType);
    mp_int32 ResumeCPUAlarm();
    monitor_process_config_t& GetAgentMonitorCfg();
    monitor_process_config_t& GetNginxMonitorCfg();
    monitor_common_config_t& GetCommonMonitorCfg();
    mp_int32 MonitorAgent();
    mp_int32 MonitorNginx();
    static monitor_data_t AddMonitorData(monitor_data_t& stMonitorData1, monitor_data_t& stMonitorData2);
    static mp_void ClearMonitorData(monitor_data_t& stMonitorData);
    static mp_void ClearAbnormalOccurTimes(abnormal_occur_times_t& stAbnormalOccurTimes);
    static mp_int32 DeleteTmpFile();
    static mp_int32 GetTmpFileTotalSize(mp_uint64& ulSize);
    static mp_uint64 GetKSize(mp_string strSize);
    mp_bool NeedExit();
    mp_void SetThreadStatus(mp_int32 iThreadStatus);
    mp_void RotateNginxLog();
    mp_void CheckNginxMonitorValue(monitor_data_t& monitorNginxData);
    mp_void CheckAgentMonitorValue(monitor_data_t& monitorAgentData);
    mp_void CheckAgentCpuUsage(mp_float fCpuUsage);
    mp_void CheckNginxCpuUsage(mp_float fCpuUsage);

#ifdef WIN32
    static DWORD WINAPI HandleFunc(mp_void* pThis);
    mp_int32 GetWinMonitorData(mp_string strPorcessName, monitor_data_t& stMonitorData);
    mp_float GetWinCPUUsage(const HANDLE hAgent);
    static mp_bool SetWinPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
    static mp_int64 FileTimeTOUTC(const FILETIME& inputTime);
#else
    static mp_void* HandleFunc(mp_void* pThis);
#endif

private:
    static CAbnormalHandler m_instance;         //��������
    thread_id_t m_hHandleThread;                //�߳̾��
    abnormal_occur_times_t m_stAgentAbnormal;   //Agent��ؽ����쳣��������
    abnormal_occur_times_t m_stNginxAbnormal;   //Nginx��ؽ����쳣��������
    monitor_process_config_t m_stAgentMointorCfg;       //Agent���̼����������
    monitor_process_config_t m_stNginxMointorCfg;       //Nginx���̼����������
    monitor_common_config_t m_stCommonMonitorCfg; //���̼�ع�������
    mp_bool m_bAgentCpuAlarmSend;  //�Ƿ��Ѿ�����agent cpu�����ʸ߸澯��ʶ
    mp_bool m_bAgentCpuAlarmResumed;  //�Ƿ��Ѿ�����agent cpu�����ʸ߻ָ��澯��ʶ
    mp_bool m_bNginxCpuAlarmSend;  //�Ƿ��Ѿ�����nginx cpu�����ʸ߸澯��ʶ
    mp_bool m_bNginxCpuAlarmResumed;  //�Ƿ��Ѿ�����nginx cpu�����ʸ߻ָ��澯��ʶ
    mp_bool m_bNeedResumed; //�Ƿ���Ҫ���ͻָ��澯
    mp_int64 m_iAgentAlarmSendFailedTimes;  //Agent cpu�澯����ʧ�ܴ���
    mp_int64 m_iNginxAlarmSendFailedTimes;  //Nginx cpu�澯����ʧ�ܴ���
    volatile mp_int32 m_iThreadStatus;
    volatile mp_bool m_bNeedExit;
};

#define CHECK_VALUE(iValue, iConfigValue, iOverTimes)  (iValue > iConfigValue) ? iOverTimes++ : (iOverTimes = 0); \
    if (iValue > iConfigValue)  \
    {\
        ostringstream oss;\
        oss << #iValue << " overceed config value(" << iConfigValue << ") " << iOverTimes << " times.";\
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "%s", oss.str().c_str());\
    }\

#endif

