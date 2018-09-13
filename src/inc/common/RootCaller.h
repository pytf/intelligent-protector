/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __ROOT_CALLER_H__
#define __ROOT_CALLER_H__

#include "common/Types.h"
#include <vector>
#include <map>

typedef enum ROOT_COMMAND_E
{
    ROOT_COMMAND_SCRIPT_BEGIN = 0,
    ROOT_COMMAND_SCRIPT_ORACLE,
    ROOT_COMMAND_SCRIPT_DB2,
    ROOT_COMMAND_SCRIPT_CACHE,
    ROOT_COMMAND_SCRIPT_QUERYDB2INFO, //db2info.sh
    ROOT_COMMAND_SCRIPT_QUERYDB2LUNINFO, //db2luninfo.sh
    ROOT_COMMAND_SCRIPT_RECOVERDB2, //db2recover.sh
    ROOT_COMMAND_SCRIPT_SAMPLEDB2, //db2sample.sh
    ROOT_COMMAND_SCRIPT_QUERYDB2CLUSTERINFO, //db2clusterinfo.sh
    ROOT_COMMAND_SCRIPT_DB2RESOURCEGROUP, //db2resourcegroup.sh
    ROOT_COMMAND_SCRIPT_QUERYORACLEINFO, //
    ROOT_COMMAND_SCRIPT_QUERYORACLEPDBINFO, //query PDB list
    ROOT_COMMAND_SCRIPT_STARTORACLEPDB, //start PDB
    ROOT_COMMAND_SCRIPT_QUERYORACLELUNINFO, //
    ROOT_COMMAND_SCRIPT_QUERYORACLECLUSTERINFO,//oracleclusterinfo.sh
    ROOT_COMMAND_SCRIPT_ORACLERESOURCEGROUP, //oracleresourcegroup.sh
    ROOT_COMMAND_SCRIPT_ORACLECHECKCDB, //oraclecheckcdb.sh
    ROOT_COMMAND_SCRIPT_TESTORACLE, //
    ROOT_COMMAND_SCRIPT_CHECKARCHIVETHRESHOLD, //
    ROOT_COMMAND_SCRIPT_FREEZEORACLE, //
    ROOT_COMMAND_SCRIPT_THAWORACLE, //
    ROOT_COMMAND_SCRIPT_ARCHIVEORACLE, //
    ROOT_COMMAND_SCRIPT_TRUNCATEARCHIVELOGORACLE,
    ROOT_COMMAND_SCRIPT_STARTASMINSTANCE, //
    ROOT_COMMAND_SCRIPT_STOPASMINSTANCE, //
    ROOT_COMMAND_SCRIPT_STARTORACLEDB, //
    ROOT_COMMAND_SCRIPT_STOPORACLEDB, //
    ROOT_COMMAND_SCRIPT_QUERYCACHEINFO,//cacheinfo.sh
    ROOT_COMMAND_SCRIPT_QUERYCACHELUNINFO,//cacheluninfo.sh
    ROOT_COMMAND_SCRIPT_SAMPLECACHE,//cachesample.sh
    ROOT_COMMAND_SCRIPT_QUERYCACHECLUSTERINFO, //cacheclusterinfo.sh
    ROOT_COMMAND_SCRIPT_INIT, //initiator.sh
    ROOT_COMMAND_THIRDPARTY,  //�������ű�
    ROOT_COMMAND_SCRIPT_PACKAGELOG,  //�����־�ű�
    ROOT_COMMAND_SCRIPT_SCANDISK,      //ɨ�̽ű�
    ROOT_COMMAND_SCRIPT_FREEZESYBASE,       //ͣ��Sybase���ݿ�
    ROOT_COMMAND_SCRIPT_THAWSYBASE,         //�ͷ�Sybase���ݿ�
    ROOT_COMMAND_SCRIPT_QUERYFREEZESTATUS,  //��ѯ����״̬
    ROOT_COMMAND_SCRIPT_STARTSYBASE,      //����Sybase���ݿ�
    ROOT_COMMAND_SCRIPT_STOPSYBASE,      //ֹͣSybase���ݿ�
    ROOT_COMMAND_SCRIPT_TESTSYBASE,      //����Sybase���ݿ�
    ROOT_COMMAND_SCRIPT_FREEZEHANA,     //����hana���ݿ�
    ROOT_COMMAND_SCRIPT_THAWHANA,       //�ⶳhana���ݿ�
    ROOT_COMMAND_SCRIPT_QUERYHANAFREEZESTATUS,  //��ѯ����״̬
    ROOT_COMMAND_SCRIPT_STARTHANA,      //����hana���ݿ�
    ROOT_COMMAND_SCRIPT_STOPHANA,       //ֹͣhana���ݿ�
    ROOT_COMMAND_SCRIPT_TESTHANA,       //����hana���ݿ�
    ROOT_COMMAND_SCRIPT_END,
    ROOT_COMMAND_SYSTEM_BEGIN = 100,
    ROOT_COMMAND_NETSTAT, //netstat
    ROOT_COMMAND_FSCK,      //fsck
    ROOT_COMMAND_FSTYP,      //fstyp
    ROOT_COMMAND_PS,      //ps -aef
    ROOT_COMMAND_ECHO,    //echo
    ROOT_COMMAND_DF,      //df -i
    ROOT_COMMAND_CAT,     //cat
    ROOT_COMMAND_LS,      //ls -l
    ROOT_COMMAND_PVSCAN,  //pvscan
    ROOT_COMMAND_VXDISK_SCAN, //vxdisk scandisks
    ROOT_COMMAND_VXDISK_E,   //vxdisk -e list
    ROOT_COMMAND_BLKID,   //blkid
    ROOT_COMMAND_LSB,     //lsb_release -a
    ROOT_COMMAND_UDEVADM,  //udevadm info
    ROOT_COMMAND_MOUNT,    //mount
    ROOT_COMMAND_LSLV,     //lslv
    ROOT_COMMAND_VGDISPLAY, //vgdisplay
    ROOT_COMMAND_LVDISPLAY,  //lvdisplay
    ROOT_COMMAND_PVS,       //pvs
    ROOT_COMMAND_LVS,       //lvs
    ROOT_COMMAND_LSVG,      //lsvg
    ROOT_COMMAND_LSPV,      //lspv
    ROOT_COMMAND_HAGRP,     //hagrp
    ROOT_COMMAND_HARES,     //hares
    ROOT_COMMAND_HASYS,    //hasys
    ROOT_COMMAND_HASTART,    //hastart
    ROOT_COMMAND_HOSTNAME,   //hostname
    ROOT_COMMAND_CLLSNODE,   //cllsnode
    ROOT_COMMAND_LSSRC,      //lssrc
    ROOT_COMMAND_CLFINDRES, //clfindres
    ROOT_COMMAND_CMVIEWCL, //cmviewcl,��ʾserviceguard��Ⱥ״̬
    ROOT_COMMAND_CMRUNCL, //cmviewcl,����serviceguard��Ⱥ
    ROOT_COMMAND_STARTPOWERHA, //startpowerha, ����powerha��Ⱥ
    ROOT_COMMAND_RHCS_CLUSTAT, //clustat, ��ʾRHCS��Ⱥ״̬
    ROOT_COMMAND_RHCS_SERVICE, //service, RHCS��Ⱥ�������
    ROOT_COMMNAD_RHCS_CLUSCVADM, //cluscvadm,ĳ��RHCS��Ⱥ�������
    ROOT_COMMNAD_CLRG, //suncluster��Դ�����
    ROOT_COMMAND_UMOUNT,    //umount
    ROOT_COMMAND_SED,       //sed
    ROOT_COMMAND_LSMOD,     //lsmod
    ROOT_COMMAND_LVCHANGE,  //lvchange
    ROOT_COMMAND_VGCHANGE,  //vgchange
    ROOT_COMMAND_VGS,       //vgs
    ROOT_COMMAND_PVDISPLAY,  //pvdisplay
    ROOT_COMMAND_VARYONVG,   //varyonvg
    ROOT_COMMAND_VARYOFFVG,  //varyoffvg
    ROOT_COMMAND_HOTADD,     //hot_add
    ROOT_COMMAND_IOSCAN,     //ioscan
    ROOT_COMMAND_IOSCANFNC,     //ioscan -fnC disk
    ROOT_COMMAND_RMSF,   //rmsf
    ROOT_COMMAND_MKNOD,   //mknod
    ROOT_COMMAND_SCSIMGR,   //scsimgr
    ROOT_COMMAND_CFGMGR,     //cfgmgr -v
    ROOT_COMMAND_CFGADM,     //cfgadm
    ROOT_COMMAND_RENDEV,
    ROOT_COMMAND_DEVFSADM,     //devfsadm
    ROOT_COMMAND_CHMOD,      //chmod
    ROOT_COMMAND_CHOWN,      //chown
    ROOT_COMMAND_ORACLEASM,   //oracleasm scandisks
    ROOT_COMMAND_SERVICE,    //service
    ROOT_COMMAND_RAW,        //raw
    ROOT_COMMAND_LN,         //ln -f -s
    ROOT_COMMAND_RM,         //rm
    ROOT_COMMAND_STRINGS,   //strings
    ROOT_COMMAND_VGIMPORT,   //vgimport
    ROOT_COMMAND_VGEXPORT,   //vgexport
    ROOT_COMMAND_RMDEV,      //rmdev
    ROOT_COMMAND_EXPORTVG,   //exportvg
    ROOT_COMMAND_IMPORTVG,   //importvg
    ROOT_COMMAND_VXDG_LIST,   //vxdg list
    ROOT_COMMAND_VXDG_IMPORT,   //vxdg import
    ROOT_COMMAND_VXDG_DEPORT,   //vxdg deport
    ROOT_COMMAND_VXVOL,   //vxvol
    ROOT_COMMAND_VXPRINT,   //vxprint
    ROOT_COMMAND_VXDISK,   //vxdisk
    ROOT_COMMAND_UPDATE_DRV, //update_drv
    ROOT_COMMAND_SCANASMLIB,  //oracleasm scandisks
    ROOT_COMMAND_SYSTEM_END,
    ROOT_COMMAND_MAC,
    ROOT_COMMAND_80PAGE,     //��ѯ80ҳ��Ϣ
    ROOT_COMMAND_83PAGE,     //��ѯ83ҳ��Ϣ
    ROOT_COMMAND_00PAGE,    //��ѯ00ҳ��Ϣ
    ROOT_COMMAND_C8PAGE,    //��ѯC8ҳ��Ϣ
    ROOT_COMMAND_CAPACITY,   //��ѯ��������
    ROOT_COMMAND_VENDORANDPRODUCT,     //��ѯ������Ϣ
    ROOT_COMMAND_RAW_MINOR_MAJOR,      //��ѯraw�������ʹη�����Ϣ
    ROOT_COMMAND_FILESYS_FREEZE,        //�����ļ�ϵͳ
    ROOT_COMMAND_FILESYS_THAW,          //�ⶳ�ļ�ϵͳ
    // ��:����/dev/sdb;/dev/sdc,��ȡ/dev/sdb,HUAWEI,S5500T,210235G6GR10D7000004,218,6200bc71001f37540769e56b000000da;/dev/sdc,HUAWEI,S5500T,210235G6GR10D7000004,218,6200bc71001f37540769e56b000000da
    ROOT_COMMAND_BATCH_GETLUN_INFO,		// ���������豸��Ϣ����ȡLUN�Ļ�����Ϣ
    ROOT_COMMAND_UDEV_RELOAD_RULES,		// ���¼���UDEV�����ļ�
    ROOT_COMMAND_SYNC_DATA_FILE,        //ˢ�»�����Ӳ��
    ROOT_COMMAND_BUTT
}ROOT_COMMAND;

class CRootCaller
{
public:
    static mp_int32 Exec(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult);
private:
    static mp_int32 ReadResultFile(mp_int32 iCommandID, mp_string& strUniqueID, vector<mp_string>* pvecResult);
};

//���ڼ�鷵�ؽ�������岿�ֺ궨��
#define ROOT_EXEC(iCommandID, strParam, pvecResult) \
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" with param \"%s\" will be executed by root.", #iCommandID, strParam.c_str()); \
    CHECK_FAIL_EX(CRootCaller::Exec(iCommandID, strParam, pvecResult))

//��鷵�ؽ�������Է���ֵΪ-1�����滻
#define ROOT_EXEC_TRANS_RETCODE(iCommandID, strParam, pvecResult, RETURN_CODE) \
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" with param \"%s\" will be executed by root.", #iCommandID, strParam.c_str()); \
    CHECK_MP_RETURN(CRootCaller::Exec(iCommandID, strParam, pvecResult), RETURN_CODE)

#endif


