/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_ERROR_CODE_H_
#define _AGENT_ERROR_CODE_H_

#include "common/Types.h"
#include <map>

//********************************************************
//BEGIN********�ڲ������룬�����ظ�Server(���ֺ��������ڲ������룬��Ҫʱֱ�ӷ��ظ�Server)
//********************************************************
#define ERROR_INNER_THAW_NOT_MATCH                          0x70000001  //�ⶳ�����Ͷ��������ƥ��
//********************************************************
//END**********�ڲ������룬�����ظ�Server
//********************************************************

//BEGIN***********************R5�汾���ظ�server�����룬��Χ0x4003291A-0x400329FF************************//
#define ERROR_COMMON_OPER_FAILED                            0xFFFFFFFF   //ִ��ʧ��

//************����������***************************��Χ0x4003291A - 0x4003294F//
#define ERROR_COMMON_INVALID_PARAM                          0x4003291A  //��������
#define ERROR_COMMON_SCRIPT_SIGN_CHECK_FAILED               0x4003291B  //���ű�ǩ��ʧ��
#define ERROR_COMMON_RECOVER_INSTANCE_NOSTART               0x4003291C  //���ݿ�ʵ��δ����
#define ERROR_COMMON_DB_USERPWD_WRONG                       0x4003291D  //���ݿ��û������������
#define ERROR_COMMON_DB_INSUFFICIENT_PERMISSION             0x4003291E  //���ݿ��û�Ȩ�޲���
#define ERROR_COMMON_FUNC_UNIMPLEMENT                       0x4003291F  //����δʵ��
#define ERROR_COMMON_READ_CONFIG_FAILED                     0x40032921  //��ȡ�����ļ�ʧ��
#define ERROR_COMMON_DLL_LOAD_FAILED                        0x40032924  //��̬�����ʧ��
#define ERROR_COMMON_SYSTEM_CALL_FAILED                     0x40032925  //ϵͳ����ʧ��(�滻�ɾ��峡��������)
#define ERROR_COMMON_CLIENT_IS_LOCKED                       0x40032926  //�ͻ��˱�����(������Ϣ������)
#define ERROR_COMMON_SCRIPT_FILE_NOT_EXIST                  0x40032927  //�ű��ļ�������
#define ERROR_COMMON_SCRIPT_EXEC_FAILED                     0x40032928  //�ű�ִ��ʧ��
#define ERROR_COMMON_PLUGIN_LOAD_FAILED                     0x40032929  //�������ʧ��
#define ERROR_COMMON_NOT_HUAWEI_LUN                         0x4003292B  //�洢��Ϣ�޷�ʶ��
#define ERROR_COMMON_USER_OR_PASSWD_IS_WRONG                0x4003292C  //�û������������
#define ERROR_COMMON_QUERY_APP_LUN_FAILED                   0x4003292F  //��ѯӦ��LUN��Ϣʧ��
#define ERROR_COMMON_DEVICE_NOT_EXIST                       0x40032931  //ָ���豸������
#define ERROR_COMMON_APP_FREEZE_FAILED                      0x40032932  //����ʧ��(Oracle/db2�����ö�������,���޸�)
#define ERROR_COMMON_APP_THAW_FAILED                        0x40032933  //�ⶳʧ��(Oracle/db2�����ö�������,���޸�)
#define ERROR_COMMON_APP_FREEZE_TIMEOUT                     0x40032934  //���ᳬʱ(Oracle/db2�����ö�������,���޸�)
#define ERROR_COMMON_NOSUPPORT_DBFILE_ON_BLOCKDEVICE        0x40032935  //��֧�����ݿ��ļ������ڲ�ͬ���͵Ĵ�����
#define ERROR_COMMON_PROC_REQUEST_BUSY                      0x40032936  //Agentҵ��æ
#define ERROR_COMMON_DB_FILE_NOT_EXIST                      0x40032937  //���ݿ��ļ�������

//************Array&Disk***********************************��Χ0x40032950 - 0x4003295F//
#define ERROR_DISK_GET_RAW_DEVICE_NAME_FAILED               0x40032954  //��ȡ���̵����豸����ʧ��
#define ERROR_DISK_ONLINE_FAILED                            0x40032956  //���ߴ���ʧ��
#define ERROR_DISK_SCAN_FAILED                              0x40032957  //ɨ�����ʧ��
#define ERROR_DISK_GET_PARTITION_INFO_FAILED                0x40032958  //��ȡ���̷�����Ϣʧ��
#define ERROR_DISK_GET_DISK_INFO_FAILED                     0x40032959  //��ȡ������Ϣʧ��
#define ERROR_DISK_GET_VOLUME_PATH_FAILED                   0x4003295A  //��ȡ��·��ʧ��

//************Host***********************************��Χ0x40032960 - 0x4003296F//
#define ERROR_HOST_VERIFY_SNMP_FAILED                       0x40032960  //SNMPЭ�������ƥ��
#define ERROR_HOST_GETINFO_FAILED                           0x40032961  //��ѯ������Ϣʧ��
#define ERROR_HOST_UNREG_TRAPSERVER_FAILED                  0x40032962  //ɾ��Trap IP��ַʧ��
#define ERROR_HOST_THIRDPARTY_GETFILE_FAILED                0x40032963  //��ѯ�������ű�ʧ��
#define ERROR_HOST_THIRDPARTY_EXEC_FAILED                   0x40032964  //ִ�е������ű�ʧ��
#define ERROR_HOST_REG_TRAPSERVER_FAILED                    0x40032965  //ע��Trap IP��ַʧ��
#define ERROR_HOST_GET_INIATOR_FAILED                       0x40032966  //��ѯ��������Ϣʧ��
#define ERROR_HOST_GET_TIMEZONE_FAILED                      0x40032967  //��ѯ����ʱ����Ϣʧ��
#define ERROR_HOST_LOG_IS_BEENING_COLLECTED                 0x40032967  //��־�����ռ�

//************Device*********************************��Χ0x40032970 - 0x4003298F//
//filesys
#define ERROR_DEVICE_FILESYS_MOUNT_POINT_NOT_EXIST          0x40032970  //���ص�Ŀ¼������
#define ERROR_DEVICE_FILESYS_MOUTN_DEV_IS_MOUNTED           0x40032971  //�豸�Ѿ����ص��������ص�
#define ERROR_DEVICE_FILESYS_MOUNT_POINT_OCCUPIED           0x40032972  //ָ���Ĺ��ص��Ѿ���ռ��
#define ERROR_DEVICE_FILESYS_OFFLINE_VOLUME_FAILED          0x40032973  //���߾�ʧ��
#define ERROR_DEVICE_FILESYS_DELETE_DRIVER_LETTER_FAILED    0x40032974  //ɾ���̷�ʧ��
#define ERROR_DEVICE_FILESYS_UNMOUNT_FAILED                 0x40032975  //ȥ����ʧ��
#define ERROR_DEVICE_FILESYS_GET_DEV_FAILED                 0x40032976  //��ѯ�豸��Ϣʧ��
#define ERROR_DEVICE_FILESYS_MOUNT_FAILED                   0x40032977  //����ʧ��
#define ERROR_DEVICE_FILESYS_QUERY_INFO_FAILED              0x40032978  //��ѯ�ļ�ϵͳ��Ϣʧ��
//raw
#define ERROR_DEVICE_RAW_USED_BY_OTHER_DEV                  0x40032979  //���豸�ѱ�ռ��
#define ERROR_DEVICE_RAW_START_FAILED                       0x4003297A  //�������豸����ʧ��
#define ERROR_DEVICE_RAW_DELETE_FAILED                      0x4003297B  //ɾ�����豸ʧ��
#define ERROR_DEVICE_RAW_CREATE_FAILED                      0x4003297C  //�������豸ʧ��
//lvm
#define ERROR_DEVICE_LVM_QUERY_VG_STATUS_FAILED             0x4003297D  //��ѯ������Ϣʧ�� 
#define ERROR_DEVICE_LVM_EXPORT_VG_FAILED                   0x4003297E  //��������ʧ��
#define ERROR_DEVICE_LVM_IMPORT_VG_FAILED                   0x4003297F  //�������ʧ��
#define ERROR_DEVICE_LVM_GET_PV_FAILED                      0x40032980  //��ѯ�������Ϣʧ��
#define ERROR_DEVICE_LVM_ACTIVE_VG_FAILED                   0x40032981  //�������ʧ��
#define ERROR_DEVICE_LVM_DEACTIVE_VG_FAILED                 0x40032982  //ȥ�������ʧ��
#define ERROR_DEVICE_VXVM_SCAN_DISK_FAILED                  0x4003298A  //VXVMɨ�����ʧ��
//link
#define ERROR_DEVICE_LINK_USED_BY_OTHER_DEV                 0x40032983  //�������ѱ�ռ��
#define ERROR_DEVICE_LINK_CREATE_FAILED                     0x40032984  //����������ʧ��
#define ERROR_DEVICE_LINK_DELETE_FAILED                     0x40032985  //ɾ��������ʧ��
//udev
#define ERROR_DEVICE_UDEV_CREATE_FAILED                     0x40032986  //д��udev����ʧ��
#define ERROR_DEVICE_UDEV_DELETE_FAILED                     0x40032987  //ɾ��udev����ʧ��
//asm
#define ERROR_DEVICE_ASM_SCAN_ASMLIB_FAILED                 0x40032988  //ɨ��ASM����ʧ��
//permission
#define ERROR_DEVICE_PERMISSION_SET_FAILED                  0x40032989  //����Ȩ��ʧ��

//***********Oracle������****************************��Χ0x40032990 - 0x400329AF//
#define ERROR_ORACLE_ASM_DBUSERPWD_WRONG                    0x40032990  //ASM�û���������� 
#define ERROR_ORACLE_ASM_RECOVER_INSTANCE_NOSTART           0x40032991  //ASMʵ��δ���� 
#define ERROR_ORACLE_ASM_INSUFFICIENT_WRONG                 0x40032992  //ASM�û�Ȩ�޲���
#define ERROR_ORACLE_NOARCHIVE_MODE                         0x40032993  //���ݿ�δ�����鵵ģʽ
#define ERROR_ORACLE_OVER_ARCHIVE_USING                     0x40032994  //���ݿ�鵵Ŀ¼���пռ䳬����ֵ
#define ERROR_ORACLE_OVER_MAX_LINK                          0x40032995  //���ݿ������ѳ������������
#define ERROR_ORACLE_IN_BACKUP                              0x40032996  //���ݿ��Ѵ����ȱ�ģʽ
#define ERROR_ORACLE_NOT_IN_BACKUP                          0x40032997  //���ݿ�δ�����ȱ�ģʽ
#define ERROR_ORACLE_ARCHIVE_FAILED                         0x40032998  //���ݿ�ǿ�ƹ鵵ʧ��
#define ERROR_ORACLE_DB_ALREADYRUNNING                      0x40032999  //���ݿ��Ѵ�������״̬
#define ERROR_ORACLE_DB_ALREADYMOUNT                        0x4003299A  //���ݿ��Ѵ��ڹ���״̬
#define ERROR_ORACLE_DB_ALREADYOPEN                         0x4003299B  //���ݿ��Ѵ��ڴ�״̬
#define ERROR_ORACLE_ASM_DISKGROUP_ALREADYMOUNT             0x4003299C  //ASM�������ѱ�����
#define ERROR_ORACLE_ASM_DISKGROUP_NOTMOUNT                 0x4003299D  //ASM������δ������
#define ERROR_ORACLE_BEGIN_HOT_BACKUP_FAILED                0x4003299E  //���ݿ⿪���ȱ�ģʽʧ��
#define ERROR_ORACLE_END_HOT_BACKUP_FAILED                  0x4003299F  //���ݿ�����ȱ�ģʽʧ��
#define ERROR_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT               0x400329A0  //���ݿ⿪���ȱ�ģʽ��ʱ
#define ERROR_ORACLE_TRUNCATE_LOG_FAILED                    0x400329A1  //ɾ�����ݿ�ʵ���Ĺ鵵��־ʧ��
#define ERROR_ORACLE_TNS_PROTOCOL_ADAPTER                   0x400329A2  //TNS����������
#define ERROR_ORACLE_START_INSTANCES_FAILED                 0x400329A3  //�������ݿ�ʵ��ʧ��
#define ERROR_ORACLE_INSTANCE_NOT_CDB						0x400329A4	//��ѯ������ݿ�ʱ�·���ʵ�������������ݿ�ʵ��

//***********DB2������*******************************��Χ0x400329B0 - 0x400329B9//
#define ERROR_DB2_SUSPEND_IO_FAILED                         0x400329B0 //���ݿ�����IOʧ��
#define ERROR_DB2_RESUME_IO_FAILED                          0x400329B1 //���ݿ�������IOʧ��
#define ERROR_DB2_SUSPEND_IO_TIMEOUT                        0x400329B2 //���ݿ�����IO��ʱ


//**********SqlServer������**************************��Χ0x400329BA - 0x400329C9//
#define ERROR_SQLSERVER_GET_DB_STATUS_FAILED                0x400329BA  //��ѯ���ݿ�״̬ʧ��
#define ERROR_SQLSERVER_DB_STATUS_OFFLINE                   0x400329BB  //���ݿⲻ����
#define ERROR_SQLSERVER_DB_NOT_EXIST                        0x400329BC  //���ݿⲻ����
#define ERROR_SQLSERVER_INSTANCE_NOT_EXIST                  0x400329BD  //���ݿ�ʵ�������� 
#define ERROR_SQLSERVER_START_INSTANCE_FAILED               0x400329BE  //�������ݿ�ʵ��ʧ��
#define ERROR_SQLSERVER_DB_LIST_IS_NULL                     0x400329BF  //���ݿ���Ϣ�б�Ϊ��
#define ERROR_SQLSERVER_START_DB_FAILED                     0x400329C0  //�������ݿ�ʧ��
#define ERROR_SQLSERVER_STOP_DB_FAILED                      0x400329C1  //ֹͣ���ݿ�ʧ��


//**********Exchange������**************************��Χ0x400329CA - 0x400329D5//
#define ERROR_EXCHANGE_REMOVE_FAILED                        0x400329CA  //�������ݿ�ж������ʧ��
#define ERROR_EXCHANGE_MOUNT_FAILED                         0x400329CB  //�������ݿ�װ������ʧ��
#define ERROR_EXCHANGE_SOFTRECVERY_FAILED                   0x400329CC  //�������ݿ���ָ�ʧ��
#define ERROR_EXCHANE_MOUNT_INMULTIAD_FAIL                  0x400329CD  //��AD���¹���ʧ��

//**********Cluster������****************************��Χ0x400329D6 - 0x400329EF//
#define ERROR_CLUSTER_QUERY_FAILED                          0x400329D6  //��ѯ��Ⱥ��Ϣʧ��
#define ERROR_CLUSTER_QUERY_NODE_FAILED                     0x400329D7  //��ѯ��Ⱥ�ڵ���Ϣʧ��
#define ERROR_CLUSTER_QUERY_SERVICE_STATE_FAILED            0x400329D8  //��ѯ��Ⱥ����״̬ʧ��
#define ERROR_CLUSTER_START_SERVICE_FAILED                  0x400329D9  //������Ⱥ����ʧ��
#define ERROR_CLUSTER_PACKAGE_ONLINE_FAILED                 0x400329DA  //���߳����(��Դ��)ʧ��
#define ERROR_CLUSTER_PACKAGE_OFFLINE_FAILED                0x400329DD  //���߳����(��Դ��)ʧ��
#define ERROR_CLUSTER_QUERY_ACTIVE_HOST_FAILED              0x400329DE  //��ѯ��ڵ�ʧ��
#define ERROR_CLUSTER_QUERY_GROUP_INFO_FAILED               0x400329DF  //��ѯ�����(��Դ��)��Ϣʧ��
#define ERROR_CLUSTER_SQLSERVER_RESOURCE_NOT_EXIST          0x400329E0  //SQL Server��Դ�鲻����
#define ERROR_CLUSTER_GET_CLUSTER_NETWORK_NAME_FAILED       0x400329E1  //��ѯ������Դ��Ϣʧ��
#define ERROR_CLUSTER_GET_DISK_PATITION_TYPE_FAILED         0x400329E2  //��ѯ���̵ķ�������ʧ��
#define ERROR_CLUSTER_GET_DISK_RESOURCE_FAILED              0x400329E3  //��ѯ������Դ��Ϣʧ��
#define ERROR_CLUSTER_RESUME_DISK_RESOURCE_FAILED           0x400329E4  //�ָ�������Դʧ��
#define ERROR_CLUSTER_REPAIR_DISK_RESOURCE_FAILED           0x400329E5  //�޸�������Դʧ��
#define ERROR_CLUSTER_ONLINE_DISK_RESOURCE_FAILED           0x400329E6  //���ߴ�����Դʧ��
#define ERROR_CLUSTER_SUSPEND_DISK_RESOURCE_FAILED          0x400329E7  //���������Դʧ��
#define ERROR_CLUSTER_SERVICE_NOSTART                       0x400329E8  //��Ⱥ����δ����
#define ERROR_CLUSTER_DB_NOT_INCLUSTER                      0x400329E9  //���ݿ�δ���뼯Ⱥ
#define ERROR_CLUSTER_RESOURCE_STATUS_ABNORMAL              0x400329EA  //��Դ״̬�쳣
#define ERROR_CLUSTER_NOT_ACTIVE_NODE                       0x400329EB  //��Ⱥ�ڵ�Ϊ�ǻ�ڵ�

//**********VSS������****************************��Χ0x400329F0 - 0x400329FF//
#define ERROR_VSS_INIT_FILEDES_GETVOLUME_FAILED             0x400329F0  //��ʼ���ļ���Ϣʱ��ȡ����Ϣʧ��
#define ERROR_VSS_TIME_OUT                                  0x400329F1  //VSS������ʱ
#define ERROR_VSS_FREEZE_TIMEOUT                            0x400329F2  //���ᳬʱ
#define ERROR_VSS_OTHER_FREEZE_RUNNING                      0x400329F3  //�Ѿ��ж��������ִ��
#define ERROR_VSS_EXCHANGE_DB_NOT_EXIST                     0x400329F4  //ָ���Ĵ洢����������ݿⲻ����

// Sun Cluster
#define ERROR_DEVICE_VXVM_EXPORT_DG_FAILED                  0x400329F5  //����������ʧ��
#define ERROR_DEVICE_VXVM_IMPORT_DG_FAILED                  0x400329F6  //���������ʧ��
#define ERROR_DEVICE_VXVM_ACTIVE_DG_FAILED                  0x400329F7  //���������ʧ��
#define ERROR_DEVICE_VXVM_DEACTIVE_DG_FAILED                0x400329F8  //ȥ���������ʧ��
#define ERROR_DEVICE_VXVM_QUERY_DG_STATUS_FAILED            0x400329F9  //��ѯ������Ϣʧ�� 

//End***********************���ظ�server������**********************************//


//BEGIN***********************�ű������룬���ڽű����󷵻أ���Χ0-255************************//
//************����������***************************����Χ5-19//
#define ERROR_SCRIPT_COMMON_EXEC_FAILED                         5
#define ERROR_SCRIPT_COMMON_RESULT_FILE_NOT_EXIST               6
#define ERROR_SCRIPT_COMMON_TMP_FILE_IS_NOT_EXIST               7
#define ERROR_SCRIPT_COMMON_PATH_WRONG                          8
#define ERROR_SCRIPT_COMMON_PARAM_WRONG                         9
#define ERROR_SCRIPT_COMMON_DB_USERPWD_WRONG                    10
#define ERROR_SCRIPT_COMMON_INSTANCE_NOSTART                    11
#define ERROR_SCRIPT_COMMON_INSUFFICIENT_WRONG                  15
#define ERROR_SCRIPT_COMMON_NOSUPPORT_DBFILE_ON_BLOCKDEVICE     16
#define ERROR_SCRIPT_COMMON_DEVICE_FILESYS_MOUNT_FAILED			17
#define ERROR_SCRIPT_COMMON_DEVICE_FILESYS_UNMOUNT_FAILED		18

//***********Oracle�ű�������********************����Χ20-69//
#define ERROR_SCRIPT_ORACLE_ASM_DBUSERPWD_WRONG                 21
#define ERROR_SCRIPT_ORACLE_ASM_INSUFFICIENT_WRONG              22
#define ERROR_SCRIPT_ORACLE_ASM_INSTANCE_NOSTART                23
#define ERROR_SCRIPT_ORACLE_NOARCHIVE_MODE                      24
#define ERROR_SCRIPT_ORACLE_OVER_ARCHIVE_USING                  25
#define ERROR_SCRIPT_ORACLE_ASM_DISKGROUP_ALREADYMOUNT          26
#define ERROR_SCRIPT_ORACLE_ASM_DISKGROUP_NOTMOUNT              27
#define ERROR_SCRIPT_ORACLE_APPLICATION_OVER_MAX_LINK           28
#define ERROR_SCRIPT_ORACLE_DB_ALREADY_INBACKUP                 29
#define ERROR_SCRIPT_ORACLE_DB_INHOT_BACKUP                     30
#define ERROR_SCRIPT_ORACLE_DB_ALREADYRUNNING                   31
#define ERROR_SCRIPT_ORACLE_DB_ALREADYMOUNT                     32
#define ERROR_SCRIPT_ORACLE_DB_ALREADYOPEN                      33
#define ERROR_SCRIPT_ORACLE_DB_ARCHIVEERROR                     34
#define ERROR_SCRIPT_ORACLE_BEGIN_HOT_BACKUP_FAILED             35
#define ERROR_SCRIPT_ORACLE_END_HOT_BACKUP_FAILED               36
#define ERROR_SCRIPT_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT            37

#define ERROR_SCRIPT_ORACLE_TRUNCATE_ARCHIVELOG_FAILED          42
#define ERROR_SCRIPT_ORACLE_TNS_PROTOCOL_ADAPTER                43
#define ERROR_SCRIPT_ORACLE_NOT_INSTALLED                       44
#define ERROR_SCRIPT_ORACLE_INST_NOT_CDB						47
#define ERROR_SCRIPT_ORACLE_PDB_NOT_EXIT						48
#define ERROR_SCRIPT_ORACLE_START_PDB_FAILED					49
#define ERROR_SCRIPT_DB_FILE_NOT_EXIST					        50

//**********DB2�ű�������*********************����Χ 70-99 //
#define ERROR_SCRIPT_DB2_SUSPEND_IO_FAILED                      70
#define ERROR_SCRIPT_DB2_RESUME_IO_FAILED                       71
#define ERROR_SCRIPT_DB2_SUSPEND_IO_TIMEOUT                     72
//**********Exchange�ű�������*********************����Χ 100-129 //
#define ERROR_SCRIPT_EXCHANGE_REMOVE_FAILED                     100
#define ERROR_SCRIPT_EXCHANGE_SOFTRECVERY_FAILED                101
#define ERROR_SCRIPT_EXCHANGE_MOUNT_FAILED                      102
#define ERROR_SCRIPT_EXCHANGE_MOUNT_INMULTIAD_FAIL              103
//**********SqlServer�ű�������*********************����Χ 130-159 //
#define ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR                    130
#define ERROR_SCRIPT_SQLSERVER_GET_CLUSTER_INFO_FAILED          131
#define ERROR_SCRIPT_SQLSERVER_132                              132
#define ERROR_SCRIPT_SQLSERVER_133                              133
#define ERROR_SCRIPT_SQLSERVER_QUERY_DB_STATUS_FAILED           134
#define ERROR_SCRIPT_SQLSERVER_DB_STATUS_OFFLINE                135
#define ERROR_SCRIPT_SQLSERVER_INSTANCE_NOT_EXIST               136
#define ERROR_SCRIPT_SQLSERVER_DB_NOT_EXIST                     137
#define ERROR_SCRIPT_SQLSERVER_GET_OSQL_TOOL_FAILED             138
#define ERROR_SCRIPT_SQLSERVER_START_INSTANCE_FAILED            139
//**********Cluster�ű�������*********************����Χ 160-189 //
#define ERROR_SCRIPT_CLUSTER_SERVICE_NOSTART                    160
#define ERROR_SCRIPT_CLUSTER_DB_NOT_INCLUSTER                   161
#define ERROR_SCRIPT_CLUSTER_RESOURCE_STATUS_ABNORMAL           162
#define ERROR_SCRIPT_CLUSTER_RESOURCE_ONLINE_FAILED             163
#define ERROR_SCRIPT_CLUSTER_RESOURCE_OFFLINE_FAILED            164
#define ERROR_SCRIPT_CLUSTER_NOT_ACTIVE_NODE                    165

#define INTER_ERROR_SRCIPT_FILE_NOT_EXIST                       255
//END***********************�ű������룬���ڽű����󷵻أ���Χ0-255************************//


//�ű�����a����ʵ������ת������
class CErrorCodeMap
{
private:
    std::map<mp_int32, mp_int32> m_mapErrorCode;

public:
    CErrorCodeMap()
    {    //��ʼ��������
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_EXEC_FAILED, ERROR_COMMON_SCRIPT_EXEC_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_COMMON_SCRIPT_SIGN_CHECK_FAILED, ERROR_COMMON_SCRIPT_SIGN_CHECK_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_RESULT_FILE_NOT_EXIST, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_TMP_FILE_IS_NOT_EXIST, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_PATH_WRONG, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_PARAM_WRONG, ERROR_COMMON_INVALID_PARAM));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_DB_USERPWD_WRONG, ERROR_COMMON_DB_USERPWD_WRONG));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_INSTANCE_NOSTART, ERROR_COMMON_RECOVER_INSTANCE_NOSTART));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_INSUFFICIENT_WRONG, ERROR_COMMON_DB_INSUFFICIENT_PERMISSION));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_NOSUPPORT_DBFILE_ON_BLOCKDEVICE, ERROR_COMMON_NOSUPPORT_DBFILE_ON_BLOCKDEVICE));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_DB_FILE_NOT_EXIST, ERROR_COMMON_DB_FILE_NOT_EXIST));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_DEVICE_FILESYS_MOUNT_FAILED, ERROR_DEVICE_FILESYS_MOUNT_FAILED));
		m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_COMMON_DEVICE_FILESYS_UNMOUNT_FAILED, ERROR_DEVICE_FILESYS_UNMOUNT_FAILED));
        //oracle������
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_ASM_DBUSERPWD_WRONG, ERROR_ORACLE_ASM_DBUSERPWD_WRONG));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_ASM_INSUFFICIENT_WRONG, ERROR_ORACLE_ASM_INSUFFICIENT_WRONG));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_ASM_INSTANCE_NOSTART, ERROR_ORACLE_ASM_RECOVER_INSTANCE_NOSTART));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_NOARCHIVE_MODE, ERROR_ORACLE_NOARCHIVE_MODE));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_OVER_ARCHIVE_USING, ERROR_ORACLE_OVER_ARCHIVE_USING));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_ASM_DISKGROUP_ALREADYMOUNT, ERROR_ORACLE_ASM_DISKGROUP_ALREADYMOUNT));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_ASM_DISKGROUP_NOTMOUNT, ERROR_ORACLE_ASM_DISKGROUP_NOTMOUNT));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_APPLICATION_OVER_MAX_LINK, ERROR_ORACLE_OVER_MAX_LINK));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_DB_ALREADY_INBACKUP, ERROR_ORACLE_IN_BACKUP));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_DB_INHOT_BACKUP, ERROR_ORACLE_NOT_IN_BACKUP));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_DB_ALREADYRUNNING, ERROR_ORACLE_DB_ALREADYRUNNING));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_DB_ALREADYMOUNT, ERROR_ORACLE_DB_ALREADYMOUNT));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_DB_ALREADYOPEN, ERROR_ORACLE_DB_ALREADYOPEN));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_DB_ARCHIVEERROR, ERROR_ORACLE_ARCHIVE_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_BEGIN_HOT_BACKUP_FAILED, ERROR_ORACLE_BEGIN_HOT_BACKUP_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_END_HOT_BACKUP_FAILED, ERROR_ORACLE_END_HOT_BACKUP_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT, ERROR_ORACLE_BEGIN_HOT_BACKUP_TIMEOUT));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_TRUNCATE_ARCHIVELOG_FAILED, ERROR_ORACLE_TRUNCATE_LOG_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_TNS_PROTOCOL_ADAPTER, ERROR_ORACLE_TNS_PROTOCOL_ADAPTER));
		m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_NOT_INSTALLED, ERROR_SCRIPT_ORACLE_NOT_INSTALLED));
		m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_INST_NOT_CDB, ERROR_ORACLE_INSTANCE_NOT_CDB));
		m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_PDB_NOT_EXIT, ERROR_SQLSERVER_DB_NOT_EXIST));
		m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_ORACLE_START_PDB_FAILED, ERROR_SQLSERVER_START_DB_FAILED));
		
        //DB2������
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_DB2_SUSPEND_IO_FAILED, ERROR_DB2_SUSPEND_IO_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_DB2_RESUME_IO_FAILED, ERROR_DB2_RESUME_IO_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_DB2_SUSPEND_IO_TIMEOUT, ERROR_DB2_SUSPEND_IO_TIMEOUT));
       
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_EXCHANGE_REMOVE_FAILED, ERROR_EXCHANGE_REMOVE_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_EXCHANGE_SOFTRECVERY_FAILED, ERROR_EXCHANGE_SOFTRECVERY_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_EXCHANGE_MOUNT_FAILED, ERROR_EXCHANGE_MOUNT_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_EXCHANGE_MOUNT_INMULTIAD_FAIL, ERROR_EXCHANE_MOUNT_INMULTIAD_FAIL));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(INTER_ERROR_SRCIPT_FILE_NOT_EXIST, ERROR_COMMON_SCRIPT_FILE_NOT_EXIST));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_DEFAULT_ERROR, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_GET_CLUSTER_INFO_FAILED, ERROR_CLUSTER_QUERY_FAILED));
        
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_CLUSTER_RESOURCE_ONLINE_FAILED, ERROR_CLUSTER_PACKAGE_ONLINE_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_CLUSTER_RESOURCE_OFFLINE_FAILED, ERROR_CLUSTER_PACKAGE_OFFLINE_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_CLUSTER_SERVICE_NOSTART, ERROR_CLUSTER_SERVICE_NOSTART));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_CLUSTER_DB_NOT_INCLUSTER, ERROR_CLUSTER_DB_NOT_INCLUSTER));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_CLUSTER_RESOURCE_STATUS_ABNORMAL, ERROR_CLUSTER_RESOURCE_STATUS_ABNORMAL));
          
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_QUERY_DB_STATUS_FAILED, ERROR_SQLSERVER_GET_DB_STATUS_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_DB_STATUS_OFFLINE, ERROR_SQLSERVER_DB_STATUS_OFFLINE));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_INSTANCE_NOT_EXIST, ERROR_SQLSERVER_INSTANCE_NOT_EXIST));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_DB_NOT_EXIST, ERROR_SQLSERVER_DB_NOT_EXIST));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_GET_OSQL_TOOL_FAILED, ERROR_COMMON_OPER_FAILED));
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_SQLSERVER_START_INSTANCE_FAILED, ERROR_SQLSERVER_START_INSTANCE_FAILED));
        
        m_mapErrorCode.insert(map<mp_int32, mp_int32>::value_type(ERROR_SCRIPT_CLUSTER_NOT_ACTIVE_NODE, ERROR_CLUSTER_NOT_ACTIVE_NODE));
    }
    mp_int32 GetErrorCode(mp_int32 iRet)
    {
        map<mp_int32, mp_int32>::iterator it = m_mapErrorCode.find(iRet);
        return it != m_mapErrorCode.end() ? it->second : MP_FAILED;
    }
};

/*------------------------------------------------------------
Function Name: TRANSFORM_RETURN_CODE
Description  : ת�������룬���ڷ��ص�-1���д������滻
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
#define TRANSFORM_RETURN_CODE(iRet, RETURN_CODE) \
    iRet = (iRet == MP_FAILED ? RETURN_CODE : iRet)


/*------------------------------------------------------------
Function Name: MP_RETURN
Description  : �������أ�����Call���÷���-1��������������滻
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
#define MP_RETURN(Call, RETURN_CODE) \
{\
    mp_int32 iFuncRet = Call;\
    return iFuncRet == MP_FAILED ? RETURN_CODE : iFuncRet;\
}

/*------------------------------------------------------------
Function Name: CHECK_MP_RETURN
Description  : ������ʧ������·��أ�����Call���÷���-1��������������滻
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
#define CHECK_MP_RETURN(Call, RETURN_CODE) \
{\
    mp_int32 iFuncRet = Call;\
    if (iFuncRet != MP_SUCCESS)\
        return iFuncRet == MP_FAILED ? RETURN_CODE : iFuncRet;\
}

#endif //_AGENT_ERROR_CODE_H_

