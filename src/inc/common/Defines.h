/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_DEFINES_H_
#define _AGENT_DEFINES_H_

#include "common/Types.h"
#include "securec.h"

#define DECIMAL                        10             //ʮ����ת��

#define MAX_FULL_PATH_LEN              300
#define MAX_PATH_LEN                   260
#define MAX_FILE_NAME_LEN              80
#define MAX_LINE_SIZE                  2048
#define MAX_SINGED_INTEGER_VALUE       2147483647
#define MAX_HOSTNAME_LEN               260            //����������
#define MAX_MAIN_CMD_LENGTH            1000           //ϵͳ�����
#define MAX_ARRAY_SN_LEN               20             //SN��󳤶�
#define MAX_ERROR_MSG_LEN              256

#define HOST_TYPE_WINDOWS              1              //Windows
#define HOST_TYPE_REDHAT               2              //RedHat
#define HOST_TYPE_HP_UX_IA             3              //HPUX IA
#define HOST_TYPE_SOLARIS              4              //SOLARIS
#define HOST_TYPE_AIX                  5              //AIX
#define HOST_TYPE_SUSE                 6              //SUSE
#define HOST_TYPE_ROCKY                7              //ROCKY
#define HOST_TYPE_OEL                  8              //OEL
#define HOST_TYPE_ISOFT                9

//device type
#define DEVICE_TYPE_FILESYS             0             //�ļ�ϵͳ
#define DEVICE_TYPE_RAW                 1             //���豸
#define DEVICE_TYPE_ASM_LIB             2             //ASMLib����
#define DEVICE_TYPE_ASM_RAW             3             //ASM���豸
#define DEVICE_TYPE_ASM_LINK            4             //ASM������
#define DEVICE_TYPE_ASM_UDEV            5             //ASMOnUdev
#define DEVICE_TYPE_ASM_DISK_ID         6             //windows ASM���̱�ʶ��

//volume type
#define VOLUME_TYPE_SIMPLE              0             //�򵥾��޾����
#define VOLUME_TYPE_LINUX_LVM           1             //Linux LVM
#define VOLUME_TYPE_LINUX_VXVM          2             //Linux VxVM
#define VOLUME_TYPE_AIX_LVM             3             //AIX�����
#define VOLUME_TYPE_HP_LVM              4             //HP�����
#define VOLUME_TYPE_UDEV                5             //UDEV�豸ӳ��

#define FREEZE_STAT_FREEZED             0             //������
#define FREEZE_STAT_UNFREEZE            1             //�ⶳ
#define FREEZE_STAT_UNKNOWN             2             //δ֪

#ifdef WIN32
//windows errorcode define
#define WIN_ERROR_FILE_NOT_FOUND        2        //The system cannot find the file specified.

#endif

//�ű�����
#define SCRIPTPARAM_INSTNAME                "INSTNAME="
#define SCRIPTPARAM_DBNAME                  "DBNAME="
#define SCRIPTPARAM_DBUSERNAME              "DBUSERNAME="
#define SCRIPTPARAM_DBPASSWORD              "DBPASSWORD="
#define SCRIPTPARAM_CLUSTERTYPE             "CLUSTERTYPE="
#define SCRIPTPARAM_RESGRPNAME              "RESGRPNAME="
#define SCRIPTPARAM_DEVGRPNAME              "DEVGRPNAME="
#define SQLSERVER_SCRIPTPARAM_TABNAME       "TABLESPACENAME="
#define SQLSERVER_SCRIPTPARAM_CLUSTERFLAG   "ISCLUSTER="
#define SQLSERVER_SCRIPTPARAM_CHECKTYPE     "CHECKTYPE="
#define SCRIPTPARAM_CLUSTERNAME             "CLUSTERNAME="


// *** BEGIN *** DTS2014071801749 y00275736 20014-07-24
//�ó�ʱʱ����10�����ӵ�60�룬��IO����ʱ10�볬ʱ�������ʧ�ܸ��ʺܸ�
//Max wait time for frozen event
#define VSS_TIMEOUT_FREEZE_MSEC         60000
// *** END *** DTS2014071801749 y00275736 20014-07-24
//Call QueryStatus every 10 ms while waiting for frozen event
#define VSS_TIMEOUT_EVENT_MSEC          10
#define VSS_TIMEOUT_MSEC                (60*1000)
#define VSS_EXEC_MUTEX_TIMEOUT          60000

//VSS writer name
#define VSS_SQLSERVER_WRITER_NAME        "SqlServerWriter"
#define VSS_SQLSERVER_WRITER_NAME_W      L"SqlServerWriter"
#define VSS_EXCHANGE_WRITER_NAME         "Microsoft Exchange Writer"
#define VSS_EXCHANGE_WRITER_NAME_W       L"Microsoft Exchange Writer"
#define VSS_FILESYSTEM_WRITER_NAME       "File System Writer"
#define VSS_FILESYSTEM_WRITER_NAME_W     L"File System Writer"
//Events name
#define EVENT_NAME_FROZEN                L"Global\\RDVSSEvent-frozen"
#define EVENT_NAME_THAW                  L"Global\\RDVSSEvent-thaw"
#define EVENT_NAME_TIMEOUT               L"Global\\RDVSSEvent-timeout"
#define EVENT_NAME_ENDBACKUP             L"Global\\RDVSSEvent-endbackup"
#define RD_AGENT_SERVICE_NAME_W          L"ReplicationDirector Agent"


#define LENGTH_SEC_DESC                128
#define LENGTH_DISK_DESC               1024    //���̼����ַ�������
#define DEFAULT_RDVSS_LOG_PATH         "C:\\"

//�ָ���
#define STR_COMMA                      ","            //�ַ����еĶ���
#define STR_SEMICOLON                  ";"            //�ַ����еķֺţ�Windows���Ѵ�Ϊ�ָ�����������·���е�ð�ų�ͻ
#define STR_COLON                      ":"            //�ַ����е�ð��
#define STR_DASH                       "-"
#define STR_PLUS                       "+"
#define STR_VERTICAL_LINE              "|"
#define STR_DOUBLE_VERTICAL_LINE       "||"
#define STR_ADDRESS                    "&"
#define STR_DOUBLE_ADDRESS             "&&"
#define STR_SPACE                     " "
#define CHAR_COMMA                     ','
#define CHAR_SEMICOLON                 ';'
#define CHAR_COLON                     ':'
#define CHAR_VERTICAL_LINE             '|'
#define CHAR_SLASH                     '/'


//��־�ļ�����
#define AGENT_LOG_NAME                 "rdagent.log"
#define ROOT_EXEC_LOG_NAME             "rootexec.log"
#define CRYPTO_LOG_NAME                "crypto.log"
#define RD_VSS_LOG_NAME                "rdvss.log"
#define MONITOR_LOG_NAME               "monitor.log"
#define WIN_SERVICE_LOG_NAME           "winservice.log"
#define XML_CFG_LOG_NAME               "xmlcfg.log"
#define AGENT_CLI_LOG_NAME             "agentcli.log"
#define GET_INPUT_LOG_NAME             "getinput.log"
#define SCRIPT_SIGN_LOG_NAME           "scriptsign.log"
#define DATA_MIGRA_LOG_NAME            "datamigra.log"
#define AGENT_LOG_ZIP_NAME             "sysinfo"

//��ִ�г�������
#define AGENT_EXEC_NAME                "rdagent"
#define ROOT_EXEC_NAME                 "rootexec"
#define CRYPT_TOOL_NAME                "crypto"
#define MONITOR_EXEC_NAME              "monitor"
#define NGINX_EXEC_NAME                "rdnginx"
#define WIN_SERVICE_EXEC_NAME          "winservice"
#define XML_CFG_EXEC_NAME              "xmlcfg"
#define GET_INPUT_EXEC_NAME            "getinput"
//nginx��Ϊ������
#define NGINX_AS_PARAM_NAME            "nginx"

//�����ļ�
#define AGENT_XML_CONF                 "agent_cfg.xml"
#define AGENT_PLG_CONF                 "pluginmgr.xml"
#define AGENT_NGINX_CONF_FILE          "nginx.conf"
#define AGENT_SCRIPT_SIGN              "script.sig"

//nginx�Ż�����
#define CFG_SSL_KEY_PASSWORD            "ssl_key_password"
#define SSL_PASSWORD_TEMP_FILE          "nginxPassword.tmp"
#define NGINX_START                     "startnginx"
#define AGENTCLI_UNIX                   "agentcli"
//�ű��ļ�
#define SCRIPT_QUERY_INITIATOR         "initialtor.sh"
#define SCRIPT_ACTION_DB2              "Db2_sample.sh"
#ifndef WIN32
#define START_SCRIPT                   "agent_start.sh"
#define STOP_SCRIPT                    "agent_stop.sh"
#define ZIP_SUFFIX                     ".tar.gz"
#else
#define START_SCRIPT                   "process_start.bat"
#define STOP_SCRIPT                    "process_stop.bat"
#define ZIP_SUFFIX                     ".zip"

//nginx�Ż�����
#define AGENTCLI_EXE                   "agentcli.exe"

#endif

//Ŀ¼����
#define AGENT_TMP_DIR                  "tmp"
#define AGENT_CONF_DIR                 "conf"
#define AGENT_BIN_DIR                  "bin"
#define AGENT_PLUGIN_DIR               "plugins"
#define AGENT_LOG_DIR                  "log"
#define AGENT_THIRDPARTY_DIR           "thirdparty"
#define AGENT_DB                       "db"
#define AGENT_NGINX                    "nginx"
#define AGENT_NGINX_LOGS               "logs"
#define AGENT_NGINX_CONF               "conf"

//һЩ���÷��Ŷ���
#define NODE_COLON                        ":"            //�ַ����е�ð��
#define NODE_SEMICOLON                    ";"            //�ַ����еķֺ�

//windows�������
#define MONITOR_SERVICE_NAME "RdMonitor"
#define AGENT_SERVICE_NAME "RdAgent"
#define NGINX_SERVICE_NAME "RdNginx"
#define INSTALL_OPERATOR "install"
#define UNINSTALL_OPERATOR "uninstall"
#define RUN_OPERATOR "run"

//����pid�ļ�
#define AGENT_PID "rdagent.pid"
#define NGINX_PID "nginx.pid"
#define MONITOR_PID "monitor.pid"

//host SN�ļ���
#define HOSTSN_FILE                   "HostSN"

//��������ڲ�json����key
#define REST_PARAM_ATTACHMENT_NAME            "attachmentName"    //��������
#define REST_PARAM_ATTACHMENT_PATH            "attachmentPath"    //��������·���������ļ�����

//�����߳�����
//#define MAX_WORKER_NUM                 1

#ifdef WIN32
#define DLLAPI             //�����̬����fcgi�Ҳ������ŵ����⣬��ʾ�Ҳ���__imp_FXGXxxxxx���ţ���__imp_�����Ƕ�̬�⵼���
                           //�ķ���ǰ׺������ʹ��DLLIMPORT��ʱ�����ɵ�.
#ifdef AGENT_DLL_EXPORTS   //specified in vs
#define AGENT_API                      __declspec(dllexport)
#else
#define AGENT_API                      __declspec(dllimport)
#endif
#define AGENT_EXPORT                   __declspec(dllexport)
#define AGENT_IMPORT                   __declspec(dllimport)
#define PATH_SEPARATOR                 "\\"
#define IS_DIR_SEP_CHAR(x)             ('\\'==(x) || '/'==(x))
#define LIB_SUFFIX                     ".dll"
#define PATH_SEPCH                     ';'
#define GETCH                          _getch()
#define SNPRINTF_S(dest, length, count, pszformat,...)    _snprintf_s(dest, length, count, pszformat, __VA_ARGS__)
#define SNWPRINTF_S(dest, length, count, pszformat, ...)  _snwprintf_s(dest, length, count, pszformat, __VA_ARGS__)
#define ITOA(val, buf, size, radix)                       _itoa_s(val, buf, size, radix)
#define I64ITOA(val, buf, size, radix)                     _i64toa_s(val, buf, size, radix)
#define VSNPRINTF_S(dest, length, count, format, valist)  vsnprintf_s(dest, length, count, format, valist);
#define VSNWPRINTF_S(dest, length, count, format, valist) _vsnwprintf_s(dest, length, count, format, valist);
#else //WIN32
#define AGENT_API
#define AGENT_EXPORT
#define AGENT_IMPORT
#define PATH_SEPARATOR                 "/"
#define IS_DIR_SEP_CHAR(x)             ('/'==(x))
#define LIB_SUFFIX                     ".so"
#define PATH_SEPCH                     ':'
#define GETCH                          getch()
#define SNPRINTF_S(dest, length, count, pszformat,...)   snprintf_s(dest, length, count, pszformat, __VA_ARGS__)
#define ITOA(val, buf, size, radix)                      itoa(val, buf, radix)
#define VSNPRINTF_S(dest, length, count, format, valist) vsnprintf_s(dest, length, count, format, valist);
#endif


#define IS_SPACE(x) ( x == ' ' || x == '\t')
#define strempty(str) (0 == str || 0 == (*str))

//Ϊ���ʹ�ù�˾��ȫ����securec��Ҫʹ�ú�CHECK_FAIL��CHECK_NOT_OK
//��ȫ����snprintf_s��Ҫ��CHECK_FAIL�����жϣ�ֻ��-1ʱ��ʾ����
#define CHECK_FAIL( Call )                                                                        \
{                                                                                                 \
    mp_int32 iCheckFailRet = Call;                                                                \
    if (MP_FAILED == iCheckFailRet)                                                               \
    {                                                                                             \
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call %s failed, ret %d.", #Call, iCheckFailRet); \
        return MP_FAILED;                                                                         \
    }                                                                                             \
}

#define CHECK_NOT_OK( Call )                                                                      \
{                                                                                                 \
    mp_int32 iCheckNotOkRet = Call;                                                               \
    if (EOK != iCheckNotOkRet)                                                                    \
    {                                                                                             \
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call %s failed, ret %d.", #Call, iCheckNotOkRet);\
        return MP_FAILED;                                                                         \
    }                                                                                             \
}

#define CHECK_FAIL_NOLOG( Call )                                                                  \
{                                                                                                 \
    mp_int32 iCheckFailRet = Call;                                                                \
    if (MP_FAILED == iCheckFailRet)                                                               \
    {                                                                                             \
        return MP_FAILED;                                                                         \
    }                                                                                             \
}

//�����÷����뷵��
#define CHECK_FAIL_EX( Call )                                                                     \
{                                                                                                 \
    mp_int32 iCheckNotOkRet = Call;                                                               \
    if (MP_SUCCESS != iCheckNotOkRet)                                                             \
    {                                                                                             \
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call %s failed, ret %d.", #Call, iCheckNotOkRet);\
        return iCheckNotOkRet;                                                                    \
    }                                                                                             \
}



//��ӡ������
#define MACR(x) #x

//���ݿⶳ��ⶳ״̬�����ģ�鹫��
enum DB_STATUS
{
    DB_FREEZE = 0,
    DB_UNFREEZE,
    DB_UNKNOWN,
    DB_BUTT
};


#endif //_AGENT_DEFINES_H_

