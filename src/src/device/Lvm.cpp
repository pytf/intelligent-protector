/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "device/Lvm.h"
#include "common/File.h"
#include "common/Path.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"
#include "common/SystemExec.h"
#include "array/Array.h"

#ifndef WIN32
/*------------------------------------------------------------ 
Description  :��ѯvg��Ϣ
Input        : struVgInfo --����ѯ��vg
Output       : struVgInfo--��ѯ����vg��Ϣ
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CLvm::QueryVgInfo(vg_info_t& struVgInfo)
{
    mp_int32 iVolType = atoi(struVgInfo.strVolType.c_str());
    switch(iVolType)
    {
    case VOLUME_TYPE_LINUX_LVM:
        return QueryVgInfo_LLVM(struVgInfo);
    case VOLUME_TYPE_HP_LVM:
        return QueryVgInfo_HLVM(struVgInfo);
    default:
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invalid Volume Type: %d.", iVolType);
        
        return ERROR_COMMON_INVALID_PARAM;
    }
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: QueryVgInfo_LLVM
Description  : ��ȡ����VG��Ϣ,linux
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::QueryVgInfo_LLVM(vg_info_t& struVgInfo)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
#ifndef WIN32
    vector<mp_string> vecPvname;
    vector<mp_string>::iterator iter;

    mp_string strParam = " | awk '$2 == \"" + struVgInfo.strVgName + "\" {print $1}'";
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to read vg(%s)  pv name.", struVgInfo.strVgName.c_str());

    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_PVS, strParam, &vecPvname);
    if (MP_SUCCESS != iRet || vecPvname.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query Vg(%s) Pv name failed, iRet %d.", struVgInfo.strVgName.c_str(), iRet);
 
        return ERROR_DEVICE_LVM_QUERY_VG_STATUS_FAILED;
    }

    for (iter = vecPvname.begin(); iter != vecPvname.end(); ++iter)
    {
        struVgInfo.vecPvs.push_back(*iter);
        
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The Vg(%s) contains pv %s.", struVgInfo.strVgName.c_str(), iter->c_str());
    }
    
#endif    
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: QueryVgInfo_HLVM
Description  : ��ȡ����VG��Ϣ, hp-ux
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::QueryVgInfo_HLVM(vg_info_t& struVgInfo)
{
#ifdef HP_UX_IA
    mp_int32 iRet = MP_SUCCESS;
    mp_string strMapInfo;
    mp_string strParam;
    mp_string strVgMapInfoFile;
    mp_string strVgMapInfoFileName;
    vector <mp_string> vecMapInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to read vg(%s) mapinfo.", struVgInfo.strVgName.c_str());

    strVgMapInfoFileName = struVgInfo.strVgName + ".map";
    strVgMapInfoFile = CPath::GetInstance().GetTmpFilePath(strVgMapInfoFileName);
    strVgMapInfoFile = "\"" + strVgMapInfoFile + "\"";
    strParam = " -p -s -m " + strVgMapInfoFile + " /dev/" + struVgInfo.strVgName;

    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_VGEXPORT, strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Export vg(%s) mapinfo failed, iRet = %d.",
            struVgInfo.strVgName.c_str(), iRet);
        return ERROR_DEVICE_LVM_QUERY_VG_STATUS_FAILED;
    }
    
    strVgMapInfoFile = CPath::GetInstance().GetTmpFilePath(strVgMapInfoFileName);
    iRet = CIPCFile::ReadFile(strVgMapInfoFile, vecMapInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read vg(%s) mapinfo file failed, iRet = %d.",
            struVgInfo.strVgName.c_str(), iRet);
        return ERROR_DEVICE_LVM_QUERY_VG_STATUS_FAILED;
    }

    if (vecMapInfo.size() == 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The vg(%s) mapinfo is empty.", struVgInfo.strVgName.c_str());
        return ERROR_DEVICE_LVM_QUERY_VG_STATUS_FAILED;
    }

    for (vector <mp_string>::iterator it = vecMapInfo.begin(); it != vecMapInfo.end(); it++)
    {
        if (it == vecMapInfo.begin())
        {
            strMapInfo = *it;
            continue;
        }
        strMapInfo = strMapInfo + "\n" + *it;
    }
    struVgInfo.strMapInfo = strMapInfo;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Read vg(%s) mapinfo(%s) succ.", 
        struVgInfo.strVgName.c_str(), struVgInfo.strMapInfo.c_str());

#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: QueryVgInfo
Description  : ��ȡVG������LV��Ϣ���ݲ��ṩ
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::QueryLvInfo(mp_string& strVgName, vector<lv_info_t>& vecLvs)
{
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ExportVg
Description  : ����vg��ں���
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::ExportVg(mp_string& strVgName, mp_int32 iVolType)
{
    switch(iVolType)
    {
    case VOLUME_TYPE_LINUX_LVM:
        return ExportVg_LLVM(strVgName);
    case VOLUME_TYPE_AIX_LVM:
        return ExportVg_ALVM(strVgName);
    case VOLUME_TYPE_HP_LVM:
        return ExportVg_HLVM(strVgName);
    case VOLUME_TYPE_LINUX_VXVM:
        return ExportVg_VXVM(strVgName);
    default:
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invalid Volume Type: %d.", iVolType);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ImportVg
Description  : ����vg��ں���
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::ImportVg(vector<mp_string>& vecPriPvName, mp_string& strVgName, mp_int32 iVolType, mp_string& strMapInfo, vector<mp_string>& vecWWN)
{
    LOGGUARD("");
    //��ʱ���������������ĺ�ɾ��
    switch(iVolType)
    {
        case VOLUME_TYPE_LINUX_LVM:
            return ImportVg_LLVM(strVgName, vecWWN);
        case VOLUME_TYPE_AIX_LVM:
            return ImportVg_ALVM(strVgName, vecWWN);
        case VOLUME_TYPE_HP_LVM:
            return ImportVg_HLVM(vecPriPvName, strVgName, strMapInfo, vecWWN);
        case VOLUME_TYPE_LINUX_VXVM:
            return ImportVg_VXVM(strVgName, strMapInfo);
        default:
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invalid Volume Type: %d.", iVolType);
            return MP_FAILED;
    }

}

/*------------------------------------------------------------
Function Name: ActivateVg
Description  : ����vg��ں���
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::ActivateVg(mp_string& strVgName, mp_int32 iVolType, mp_string strVgActiveMode, mp_int32 iRecoverType)
{
    LOGGUARD("");
    switch(iVolType)
    {
    case VOLUME_TYPE_LINUX_LVM:
        return ActivateVg_LLVM(strVgName);
    case VOLUME_TYPE_AIX_LVM:
        return ActivateVg_ALVM(strVgName);
    case VOLUME_TYPE_LINUX_VXVM:
        return ActivateVg_VXVM(strVgName);
    case VOLUME_TYPE_HP_LVM:
        return ActivateVg_HLVM(strVgName, strVgActiveMode, iRecoverType);
    default:
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invalid Volume Type: %d.", iVolType);
        return MP_FAILED;
    }
}

/*------------------------------------------------------------
Function Name: DeActivateVg
Description  : ȥ����vg��ں���
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::DeActivateVg(mp_string& strVgName, mp_int32 iVolType)
{
    LOGGUARD("");
    switch(iVolType)
    {
    case VOLUME_TYPE_LINUX_LVM:
        return DeActivateVg_LLVM(strVgName);
    case VOLUME_TYPE_AIX_LVM:
        return DeActivateVg_ALVM(strVgName);
    case VOLUME_TYPE_LINUX_VXVM:
        return DeActivateVg_VXVM(strVgName);
    case VOLUME_TYPE_HP_LVM:
        return DeActivateVg_HLVM(strVgName);
    default:
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invalid Volume Type: %d.", iVolType);
        return MP_FAILED;
    }
}

/*------------------------------------------------------------
Function Name: DeActivateVg
Description  : ɨ��VXVM�Ĵ��̺���
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::ScanDisks_VXVM()
{
    LOGGUARD("");
    mp_string strParam;
    strParam = "scandisks";
    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_VXDISK, strParam, NULL, ERROR_DEVICE_VXVM_SCAN_DISK_FAILED);
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ImportVg_LLVM
Description  : �����ļ�ϵͳ����Ϊllvm��vg
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::ImportVg_LLVM(mp_string& strVgName, vector<mp_string>& vecWWN)
{
    LOGGUARD("");
#ifndef WIN32
    //���л������
    //����WWN��ȡdisk name
    //����disk name��ȡ������vg����
    //�жϸ�vg�Ƿ��ѵ���
    //���Ϊ���룬�����ƺ�strVgName��һ�£�����ʧ��
    //��ȡ����disk name
    //����WWN��ȡdisk name
    mp_int32 cycle = 3;
    mp_int32 iRet = MP_SUCCESS;
    /* ����ѭ��,���������г��ֲ�һ�µ�����; */
    while (--cycle)
    {
        //CodeDex��,KLOCWORK.ITER.END.DEREF.MIGHT
        mp_int32 iSuccessTimes = 0;
        iRet = CheckVgStatus_LLVM(strVgName, vecWWN, iSuccessTimes);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "check this vg status failed, iRet = %d.", iRet);
            return iRet;
        }

        if (vecWWN.size() == iSuccessTimes)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Vg \"%s\" is already imported.",
                  strVgName.c_str());
            return MP_SUCCESS;
        }
        
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" with param \"%s\" will be executed by root.", "ROOT_COMMAND_VGIMPORT", strVgName.c_str());
        iRet = CRootCaller::Exec(ROOT_COMMAND_VGIMPORT, strVgName, NULL);
        if (MP_SUCCESS == iRet)
        {
            break;
        }
        else
        {
            DoSleep(1000);
        }
    }

    if (0 == cycle)
    {
        return ERROR_DEVICE_LVM_IMPORT_VG_FAILED;
    }
#endif
    return MP_SUCCESS;
}
/*------------------------------------------------------------
Function Name: CheckVgIsImport_LLVM
Description  : ��ѯ�ļ�ϵͳ����Ϊllvm��vg�Ƿ��ѵ���
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::CheckVgStatus_LLVM(mp_string& strVgName, vector<mp_string>& vecWWN, mp_int32 &iSuccessTimes)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strDeviceName;
    vector<mp_string> vecRealVgName;
    mp_string strRealVgName;
    LOGGUARD("");
    
    for (vector<mp_string>::iterator it = vecWWN.begin(); it != vecWWN.end(); it++)
    {
        iRet = CDisk::GetDevNameByWWN(strDeviceName, *it);
        TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_NOT_HUAWEI_LUN);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get device name by wwn failed, iRet = %d.", iRet);
            return iRet;
        }
        //����disk name��ȡ����vg name
        iRet = GetVgName_LLVM(strDeviceName, vecRealVgName);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_LVM_QUERY_VG_STATUS_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Vg name failed, iRet=%d.", iRet);
            return iRet;
        }
        for(vector<mp_string>::iterator itVgName = vecRealVgName.begin(); itVgName != vecRealVgName.end(); itVgName++)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Vg name %s is comparing.", (*itVgName).c_str());
            if(strVgName == *itVgName)
            {
                strRealVgName = *itVgName;
                COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get the Vg name %s.", (*itVgName).c_str());
                break;
            }
        }
        if(strRealVgName.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Vg \"%s\" of hard disk %s cannot be found.",
                strVgName.c_str(), strDeviceName.c_str());
            return ERROR_COMMON_INVALID_PARAM;
        }
        mp_bool bIsExported = MP_FALSE;
        iRet = IsVgExported(strRealVgName, bIsExported);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_LVM_QUERY_VG_STATUS_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Vg exported status failed, iRet = %d.", iRet);
            return iRet;
        }
    
        //�ѵ���
        if (!bIsExported)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "LV(Vg[%s], WWN[%s]) is already imported",
                  strRealVgName.c_str(), it->c_str());
            iSuccessTimes++;
        }
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Check VG status success.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : �����ļ�ϵͳ����ΪHLVM��vg
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CLvm::ImportVg_HLVM(vector<mp_string>& vecPriPvName, mp_string& strVgName, mp_string& strMapInfo, vector<mp_string>& vecWWN)
{
    LOGGUARD("");
#ifndef WIN32
    mp_int32 iRet = MP_SUCCESS;
    mp_string strLegacyDisk;
    mp_string strRealVgName;
    mp_string strSecPvName;
    mp_string strPersistentDisk;
    mp_string strParam;
    mp_string strMapInfoFile;
    mp_string strWWN;
    mp_string strAllSecPvName;
    mp_int32 iSuccessTimes = 0;
    vector<mp_string> ::iterator it = vecPriPvName.begin();
    vector<mp_string> ::iterator iter = vecWWN.begin();  


    mp_string strTmpPath = CPath::GetInstance().GetTmpPath();
    strMapInfoFile = "\"" + strTmpPath + PATH_SEPARATOR + strVgName + ".map\"";
    iRet = WriteVgMapInfo(strMapInfo, strMapInfoFile);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_LVM_IMPORT_VG_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Write vg mapinfo failed, iRet = %d.", iRet);
        return ERROR_DEVICE_LVM_IMPORT_VG_FAILED;
    }
    
    for(; it != vecPriPvName.end(); it++)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, 
            "Begin to import vg, vgname(%s), pripvname(%s), mapinfo(%s), wwn(%s).", 
            strVgName.c_str(), it->c_str(), strMapInfo.c_str(), iter->c_str());
            
        iRet = CDisk::GetDevNameByWWN(strLegacyDisk, *iter);
        TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_NOT_HUAWEI_LUN);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get device name by wwn failed, iRet = %d.", iRet);
            return iRet;
        }

        iRet = CDisk::GetSecPvName(*it, strLegacyDisk, strSecPvName);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_LVM_GET_PV_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sec pv failed, iRet=%d.", iRet);
            return iRet;
        }

        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Secpvname is %s.", strSecPvName.c_str());
        
        //����disk name��ȡvg name
        GetVgName_HLVM(strSecPvName, strRealVgName);

        if (strRealVgName != "")
        {
            mp_bool bIsExported = MP_FALSE;
            iRet = IsVgExported(strRealVgName, bIsExported);
            TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_LVM_QUERY_VG_STATUS_FAILED);
            if (MP_SUCCESS != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Vg exported status failed, iRet = %d.", iRet);
                return iRet;
            }

            //δ������ֱ�ӷ��سɹ�
            if (!bIsExported)
            {
                COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Vg \"%s\" is already imported, input Vg is %s",
                          strRealVgName.c_str(), strVgName.c_str());
                iSuccessTimes++;
            }
            else if (strRealVgName != strVgName)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Vg \"%s\" of hard disk %s is not the expected Vg %s.",
                                strRealVgName.c_str(), strSecPvName.c_str(), strVgName.c_str());
                return ERROR_COMMON_INVALID_PARAM;
            }
        }
        else
        {
            strAllSecPvName = strAllSecPvName + " " + strSecPvName;
        }
        iter++;
    }

    if (vecPriPvName.size() == iSuccessTimes)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Vg \"%s\" is already imported, input Vg is \"%s\"",
                  strRealVgName.c_str(), strVgName.c_str());
        return MP_SUCCESS;
    }
        
    //vgimport -m vgname.map -v vgname /dev/disk/diskxxx
    strParam = " -m " + strMapInfoFile + " -v " + strVgName + " " + strAllSecPvName;
    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_VGIMPORT, strParam, NULL, ERROR_DEVICE_LVM_IMPORT_VG_FAILED);
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Import vg(%s) succ.", strVgName.c_str());
        
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ImportVg_ALVM
Description  : �����ļ�ϵͳ����Ϊaix lvm��vg
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::ImportVg_ALVM(mp_string& strVgName, vector<mp_string>& vecWWN)
{
    LOGGUARD("");
#ifndef WIN32
    //���Ƚ��л������
    //����WWN��ȡdisk name
    //����disk name��ȡ������vg���ƣ����vg����Ϊ"none"��ֱ�ӷ��سɹ�
    //�жϸ�vg�Ƿ��ѵ���
    //���Ϊ���룬�����ƺ�strVgName��һ�£�����ʧ��
    //����WWN��ȡdisk name
    mp_string strDeviceName, strRealVgName;
    mp_int32 iSuccessTimes = 0;
    for (vector<mp_string>::iterator it = vecWWN.begin(); it != vecWWN.end(); it++)
    {
        mp_int32 iRet = CDisk::GetDevNameByWWN(strDeviceName, *it);
        TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_NOT_HUAWEI_LUN);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get device name by wwn failed, iRet = %d.", iRet);
            return iRet;
        }

        iRet = GetVgName_ALVM(strDeviceName, strRealVgName);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_LVM_QUERY_VG_STATUS_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Vg name failed, iRet=%d.", iRet);
            return iRet;
        }
        if ("None" != strRealVgName)
        {
            if (strRealVgName != strVgName)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Vg \"%s\" of hdisk \"%s\" is not the expected Vg \"%s\".",
                        strRealVgName.c_str(), strDeviceName.c_str(), strVgName.c_str());
                return ERROR_COMMON_INVALID_PARAM;
            }
            else
            {
                COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Vg \"%s\" is already imported.", strVgName.c_str());
                 iSuccessTimes++;
            }
        }
    }
    if (vecWWN.size() == iSuccessTimes)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Vg \"%s\" is already imported, input Vg is \"%s\"",
                  strRealVgName.c_str(), strVgName.c_str());
        return MP_SUCCESS;
    }
    mp_string strParam = "-y " + strVgName + " -n " + strDeviceName;
    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_IMPORTVG, strParam, NULL, ERROR_DEVICE_LVM_IMPORT_VG_FAILED);
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ImportVg_VXVM
Description  : ���������Ϊvxvm��dg
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::ImportVg_VXVM(mp_string& strVgName, mp_string& strMapInfo)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bIsExported = MP_FALSE;
    mp_string strParam;

    iRet = IsVgExported_VXVM(strVgName, bIsExported);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_VXVM_QUERY_DG_STATUS_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Vg exported status failed, iRet = %d.", iRet);
        return iRet;
    }

    if(bIsExported)
    {
        ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_VXDG_IMPORT, strVgName, NULL, ERROR_DEVICE_VXVM_IMPORT_DG_FAILED);
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Vg \"%s\" is already imported.", strVgName.c_str());
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ExportVg_LLVM
Description  : �����ļ�ϵͳ����Ϊllvm��vg
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::ExportVg_LLVM(mp_string& strVgName)
{
    LOGGUARD("");
#ifndef WIN32
    mp_int32 cycle = 3;
    /* ����ѭ��,���������г��ֲ�һ�µ�����; */
    while(--cycle)
    {
        //�ж�vg�Ƿ���ڣ�������ֱ�ӷ��سɹ�
        if (!IsVgExist(strVgName))
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Vg %s is not exist.", strVgName.c_str());
            return MP_SUCCESS;
        }

        //�ж�vg�Ƿ񵼳�������ѵ��룬ֱ�ӷ��سɹ�
        mp_bool bExported = MP_FALSE;
        mp_int32 iRet = IsVgExported(strVgName, bExported);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_LVM_EXPORT_VG_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Vg exmported status failed, iRet=%d.", iRet);
            return iRet;
        }
        if (bExported)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Vg \"%s\" is exported.", strVgName.c_str());
            return MP_SUCCESS;
        }
        
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" with param \"%s\" will be executed by root.", "ROOT_COMMAND_VGEXPORT", strVgName.c_str());
        iRet = CRootCaller::Exec(ROOT_COMMAND_VGEXPORT, strVgName, NULL);
        if (MP_SUCCESS == iRet)
        {
            break;
        }
        else
        {
            DoSleep(1000);
        }
    }

    if (0 == cycle)
    {
        return ERROR_DEVICE_LVM_EXPORT_VG_FAILED;
    }
#endif
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : �����ļ�ϵͳ����ΪHLVM��vg
Input        : strVgName -- ��������vg����
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CLvm::ExportVg_HLVM(mp_string& strVgName)
{
    LOGGUARD("");
#ifndef WIN32
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begint to export vg(%s).", strVgName.c_str());
    //�ж�vg�Ƿ񵼳�������ѵ��룬ֱ�ӷ��سɹ�
    mp_bool bExported = MP_FALSE;
    mp_int32 iRet = IsVgExported(strVgName, bExported);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_LVM_EXPORT_VG_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Vg exmported status failed, iRet=%d.", iRet);
        return iRet;
    }
	//CodeDex�󱨣�Dead Code	
    if (bExported)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Vg \"%s\" is exported.", strVgName.c_str());
        return MP_SUCCESS;
    }

    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_VGEXPORT, strVgName, NULL, ERROR_DEVICE_LVM_EXPORT_VG_FAILED);

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Export vg(%s) succ.", strVgName.c_str());
        
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ExportVg_ALVM
Description  : �����ļ�ϵͳ����Ϊaix lvm��vg
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::ExportVg_ALVM(mp_string& strVgName)
{
    LOGGUARD("");
#ifndef WIN32
    //�ж�vg�Ƿ����
    vector<mp_string> vecRlt;
    mp_bool IsVgExist = MP_FALSE;
	mp_bool IsPvofVgExist = MP_FALSE;
	
    mp_string strCmd = "lsvg | awk '$1==\"" + strVgName + "\"'";
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" will be executed.", strCmd.c_str());
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Exec lsvg \"%s\" failed, iRet = %d.", strVgName.c_str(), iRet);
        return ERROR_DEVICE_LVM_EXPORT_VG_FAILED;
    }

    if(vecRlt.empty())
    {
        IsVgExist = MP_TRUE;
    }
    vecRlt.clear();
    //Ԥ�Ȳ�ѯpv��Ϣ
    strCmd = "lspv | awk '$3==\"" + strVgName + "\" {print $1}'";
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" will be executed.", strCmd.c_str());

    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Exec lspv of vg \"%s\" failed, iRet = %d.", strVgName.c_str(), iRet);
        return ERROR_DEVICE_LVM_EXPORT_VG_FAILED;
    }

    if(vecRlt.empty())
    {
        IsPvofVgExist = MP_TRUE;
    }

    if(IsVgExist&&IsPvofVgExist)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Vg \"%s\" has been exported.", strVgName.c_str());
    }
    else
    {
        ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_EXPORTVG, strVgName, NULL, ERROR_DEVICE_LVM_EXPORT_VG_FAILED);
    }
    for (vector<mp_string>::iterator it = vecRlt.begin(); it != vecRlt.end(); it++)
    {
        mp_string strParam = "-dl " + *it;
        iRet = CRootCaller::Exec(ROOT_COMMAND_RMDEV, strParam, NULL);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Exec rmdev of disk \"%s\" failed, iRet = %d.", it->c_str(), iRet);
        }
    }
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ExportVg_VXVM
Description  : ������Ϊvxvm��vg����δʵ��
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::ExportVg_VXVM(mp_string& strVgName)
{
    LOGGUARD("");
    //�ж�vg�Ƿ����
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bIsExported = MP_FALSE;
    mp_string strParam;

    iRet = IsVgExported_VXVM(strVgName, bIsExported);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_VXVM_QUERY_DG_STATUS_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Vg exported status failed, iRet = %d.", iRet);
        return iRet;
    }

    if(bIsExported)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Vg \"%s\" is already exported.", strVgName.c_str());
    }
    else
    {
    #if defined(SOLARIS)
        /* ���Ҿ���strVgName�µ�����vxvm��disk; */
        std::vector<mp_string> VecVgName;
        mp_string strParam = mp_string(" list | grep ") + mp_string("\"") + strVgName + mp_string("\"") + " | nawk -F ' ' '{print $1}'";
        ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_VXDISK, strParam, &VecVgName, ERROR_COMMON_OPER_FAILED);
    #endif
    
        ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_VXDG_DEPORT, strVgName, NULL, ERROR_DEVICE_VXVM_EXPORT_DG_FAILED);

    #if defined(SOLARIS)
        /* ����strVgName�µ�����vxvm��disk; */
        for (std::vector<mp_string>::iterator iter = VecVgName.begin();
            iter != VecVgName.end();
            ++iter)
        {
            strParam = " rm " + *iter;
            ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_VXDISK, strParam, NULL, ERROR_COMMON_OPER_FAILED);
        }
        
        strParam = "  -vf sd";
        ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_UPDATE_DRV, strParam, NULL, ERROR_COMMON_OPER_FAILED);
    #endif
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ActivateVg_LLVM
Description  : �����ļ�ϵͳ����Ϊllvm��vg
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::ActivateVg_LLVM(mp_string& strVgName)
{
    LOGGUARD("");
#ifndef WIN32
    mp_string strParam = "-a y " + strVgName;
    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_VGCHANGE, strParam, NULL, ERROR_DEVICE_LVM_ACTIVE_VG_FAILED);
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ActivateVg_ALVM
Description  : �����ļ�ϵͳ����Ϊaix lvm��vg
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::ActivateVg_ALVM(mp_string& strVgName)
{
    LOGGUARD("");
#ifndef WIN32
    //�ж�vg�Ƿ񼤻�
    mp_string strCmd = "lsvg " + strVgName;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" will be executed.", strCmd.c_str());
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (iRet == MP_SUCCESS)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Vg \"%s\" is already active.", strVgName.c_str());
        return MP_SUCCESS;
    }

    ROOT_EXEC(ROOT_COMMAND_VARYONVG, strVgName, NULL);
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ActivateVg_HLVM
Description  : �����ļ�ϵͳ����ΪHlvm��vg
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::ActivateVg_HLVM(mp_string& strVgName, mp_string strVgActiveMode, mp_int32 iRecoverType)
{
    LOGGUARD("");
#ifndef WIN32
    mp_string strParam;
    mp_string strSpace(" ");
    mp_string strAloneActive(" -a y ");
    mp_string strSetClusterFlag(" -c y ");
    mp_string strUnsetClusterFlag(" -c n ");

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Vg name is (%s), vg active mode is (%s).", strVgName.c_str(), strVgActiveMode.c_str());
    
    if (ALONE_TO_ALONE == iRecoverType)
    {
        strParam = strAloneActive + strVgName;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The recovertype is alone to alone, strParam(%s).", strParam.c_str());
    }
    else if (ALONE_TO_CLUSTER == iRecoverType)
    {
        strParam = strSetClusterFlag + strVgName;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The recovertype is alone to cluster, strParam(%s).", strParam.c_str());
        ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_VGCHANGE, strParam, NULL, ERROR_DEVICE_LVM_ACTIVE_VG_FAILED);
        strParam = strSpace + strVgActiveMode + strSpace + strVgName;
    }
    else if (CLUSTER_TO_CLUSTER == iRecoverType)
    {
        strParam = strSpace + strVgActiveMode + strSpace + strVgName;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The recovertype is cluster to cluster, strParam(%s).", strParam.c_str());
    }
    else if (CLUSTER_TO_ALONE == iRecoverType)
    {    
        strParam = strUnsetClusterFlag + strVgName;
        
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The recovertype is cluster to alone, strParam(%s).", strParam.c_str());
        
        ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_VGCHANGE, strParam, NULL, ERROR_DEVICE_LVM_ACTIVE_VG_FAILED);
        strParam = strAloneActive + strVgName;
    }
    
    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_VGCHANGE, strParam, NULL, ERROR_DEVICE_LVM_ACTIVE_VG_FAILED);
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ActivateVg_VXVM
Description  : �����ļ�ϵͳ����Ϊvxvm��vg����δʵ��
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::ActivateVg_VXVM(mp_string& strVgName)
{
    LOGGUARD("");
    mp_string strParam;
    strParam= "-g " + strVgName + " startall";
    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_VXVOL, strParam, NULL, ERROR_DEVICE_VXVM_ACTIVE_DG_FAILED);

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: DeActivateVg_LLVM
Description  : ȥ�����ļ�ϵͳ����Ϊllvm��vg
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::DeActivateVg_LLVM(mp_string& strVgName)
{
    LOGGUARD("");
#ifndef WIN32
    mp_int32 cycle = 3;
    /* ����ѭ��,���������г��ֲ�һ�µ�����; */
    while (--cycle)
    {
        //�ж�vg�Ƿ���ڣ�������ֱ�ӷ��سɹ�
        if (!IsVgExist(strVgName))
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Vg \"%s\" is not exist.", strVgName.c_str());
            return MP_SUCCESS;
        }

        //�ж�vg�Ƿ񵼳�������ѵ��룬ֱ�ӷ��سɹ�
        mp_bool bExported = MP_FALSE;
        mp_int32 iRet = IsVgExported(strVgName, bExported);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_LVM_QUERY_VG_STATUS_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Vg emported status failed, iRet=%d.", iRet);
            return iRet;
        }
        if (bExported)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Vg \"%s\" is exported.", strVgName.c_str());
            return MP_SUCCESS;
        }
        
        mp_string strParam = "-a n " + strVgName;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" with param \"%s\" will be executed by root.", "ROOT_COMMAND_VGCHANGE", strParam.c_str());
        iRet = CRootCaller::Exec(ROOT_COMMAND_VGCHANGE, strParam, NULL);
        if (MP_SUCCESS == iRet)
        {
            break;
        }
        else
        {
            DoSleep(1000);
        }
    }

    if (0 == cycle)
    {
        return ERROR_DEVICE_LVM_DEACTIVE_VG_FAILED;
    }
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: DeActivateVg_ALVM
Description  : ȥ�����ļ�ϵͳ����Ϊaix lvm��vg
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::DeActivateVg_ALVM(mp_string& strVgName)
{
    LOGGUARD("");
#ifndef WIN32
    //�ж�vg�Ƿ񼤻�
    mp_string strCmd = "lsvg " + strVgName;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" will be executed.", strCmd.c_str());
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Vg \"%s\" is already deactive.", strVgName.c_str());
        return MP_SUCCESS;
    }

    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_VARYOFFVG, strVgName, NULL, ERROR_DEVICE_LVM_DEACTIVE_VG_FAILED);
#endif
    return MP_SUCCESS;

}

/*------------------------------------------------------------
Function Name: DeActivateVg_VXVM
Description  : ȥ���������Ϊvxvm��vg����δʵ��
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::DeActivateVg_VXVM(mp_string& strVgName)
{
    LOGGUARD("");
    mp_string strParam;
    strParam= "-g " + strVgName + " stopall";
    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_VXVOL, strParam, NULL, ERROR_DEVICE_VXVM_DEACTIVE_DG_FAILED);
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: DeActivateVg_HLVM
Description  : �����ļ�ϵͳ����ΪHlvm��vg
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::DeActivateVg_HLVM(mp_string& strVgName)
{
    LOGGUARD("");
#ifndef WIN32
    //�ж�vg�Ƿ���ڣ�������ֱ�ӷ��سɹ�
    mp_string strCmd = "vgdisplay " + strVgName;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" will be executed.", strCmd.c_str());
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Vg \"%s\" is not exist.", strVgName.c_str());
        return MP_SUCCESS;
    }

    //�ж�vg�Ƿ񵼳�������ѵ��룬ֱ�ӷ��سɹ�
    mp_bool bExported = MP_FALSE;
    iRet = IsVgExported(strVgName, bExported);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Vg emported status failed, iRet=%d.", iRet);
        return iRet;
    }
    if (bExported)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Vg \"%s\" is exported.", strVgName.c_str());
        return MP_SUCCESS;
    }

    mp_string strParam;
    strParam = " -a n " + strVgName;
    ROOT_EXEC(ROOT_COMMAND_VGCHANGE, strParam, NULL);
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: IsVgExported
Description  : �ж�vg�Ƿ��Ѿ�����
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::IsVgExported(mp_string& strVgName, mp_bool& bIsExported)
{
    LOGGUARD("");
#ifdef LINUX
    vector <mp_string> vecEcho;
    mp_string strParam = "'" + strVgName + "' 2>/dev/null | grep 'VG Status' | awk '{print $3}'";
    ROOT_EXEC(ROOT_COMMAND_VGDISPLAY, strParam, &vecEcho);

    if (vecEcho.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Echo is empty.");
        return MP_FAILED;
    }
    bIsExported = MP_FALSE;
    for(vector <mp_string>::iterator iter = vecEcho.begin();iter != vecEcho.end();++iter)
    {
        mp_string strState = *iter;
        if (strState.find("exported") != mp_string::npos)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Vg \"%s\" is exported", strVgName.c_str());
            bIsExported = MP_TRUE;
            break;
        }
    }
#endif
#ifdef HP_UX_IA
    mp_int32 iRet = MP_SUCCESS;
    vector <mp_string> vecLvmTab;
    mp_string strParam;
    mp_string strVgPath;

    strParam = " /etc/lvmtab";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_STRINGS, strParam, &vecLvmTab);
    if (MP_SUCCESS != iRet || vecLvmTab.size() == 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get lvmtab info failed, iRet %d.", iRet);
        return ERROR_DEVICE_LVM_QUERY_VG_STATUS_FAILED;
    }

    strVgPath = "/dev/" + strVgName;
    for (vector<mp_string>::iterator it = vecLvmTab.begin(); it != vecLvmTab.end(); it++)
    {
        if (strVgPath == *it)
        {
            bIsExported = MP_FALSE;
            return MP_SUCCESS;
        }
    }
    
    bIsExported = MP_TRUE;
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :��ȡLLVM Vg����
Input        :    strDevice---�豸��
Output       :  strVgName---Vg������
Return       :   MP_SUCCESS---��ȡ�ɹ�
                 MP_FAILED---��ȡʧ�ܣ�iRet---��Ӧ������
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CLvm::GetVgName_LLVM(mp_string& strDevice, vector<mp_string> &vecVgName)
{
#ifndef WIN32
    LOGGUARD("");
    mp_bool bIsSdisk = CDisk::IsSdisk(strDevice);
    //vector<mp_string> vecVgName;
    mp_string strCmd;
    mp_int32 iRet;
    if (bIsSdisk)
    {
        strCmd = "cat /proc/partitions  | grep -e '" + strDevice + "'|awk '{print $4}'";
        vector<mp_string> vecDiskName;
        iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecDiskName);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CSystemExec::ExecSystemWithEcho failed, strCmd is %s, iRet is %d.",
                     strCmd.c_str(), iRet);
            return iRet;
        }

        if (vecDiskName.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get disk partitions:%s", strDevice.c_str());
            return MP_FAILED;
        }
        for (vector<mp_string>::iterator it = vecDiskName.begin(); it != vecDiskName.end(); it++)
        {  
            strCmd = mp_string("'") + "/dev/" + *it + "'| grep -v 'PV' | awk '{print $2}'";
            iRet = CRootCaller::Exec(ROOT_COMMAND_PVS, strCmd, &vecVgName);
        }
    }
    else
    {
        strCmd = "'" + strDevice + "'| grep -v 'LV' | awk '{print $2}'";
        iRet = CRootCaller::Exec(ROOT_COMMAND_LVS, strCmd, &vecVgName);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CRootCaller::Exec failed, strCmd is %s, iRet is %d.",
                     strCmd.c_str(), iRet);
            return iRet;
        }
    }

    if (vecVgName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "vecVgName is empty.");
        return MP_FAILED;
    }
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :HP��LVM-�����豸����ȡVG����
Input        :strDevice -- �豸��
Output       : strVgName --VG����
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CLvm::GetVgName_HLVM(mp_string& strDevice, mp_string& strVgName)
{
#ifdef HP_UX_IA
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_bool bIsDskdisk = CDisk::IsDskdisk(strDevice);
    mp_string strParam;
    vector<mp_string> vecVgName;
    
    if (bIsDskdisk)
    {
        strParam = " -d " + strDevice + " | grep 'VG Name' | awk -F '/' '{print $NF}'";
        iRet = CRootCaller::Exec(ROOT_COMMAND_PVDISPLAY, strParam, &vecVgName);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get vg name of pv(%s) failed, iRet = %d.", 
                strDevice.c_str(), iRet);
            return iRet;
        }
    }

    if(vecVgName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get vg name of pv(%s) is empty.", 
            strDevice.c_str());
        return MP_FAILED;
    }
    strVgName= vecVgName.front();
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :��ȡALVM  Vg����
Input        :    strDevice---�豸��
Output       :  strVgName---Vg������
Return       :   MP_SUCCESS---��ȡ�ɹ�
                 MP_FAILED---��ȡʧ�ܣ�iRet---��Ӧ������
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CLvm::GetVgName_ALVM(mp_string& strDevice, mp_string& strVgName)
{
    vector<mp_string> vecVgName, vecDevName;
    mp_string strCmd = "lspv | awk '$1==\"" + strDevice + "\" {print $1}'";
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecDevName);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get device name failed, iRet = %d.", iRet);
        return iRet;
    }
    if (vecDevName.empty()) //empty means pv name not eq strDevice
    {
        strCmd = "lslv -L '" + strDevice + "' | grep 'VOLUME GROUP:' | awk '{print $6}'";
    }
    else
    {
        strCmd = "lspv | awk '$1==\"" + strDevice + "\" {print $3}'";
    }
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecVgName);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CSystemExec::ExecSystemWithEcho failed, strCmd is %s, iRet is %d.",
                     strCmd.c_str(), iRet);
        return iRet;
    }

    if (vecVgName.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "vecVgName is empty.");
        return MP_FAILED;
    }
    else
    {
        strVgName = vecVgName[0];
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: IsVgExported
Description  : �ж�VXVM��vg�Ƿ��Ѿ�����
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CLvm::IsVgExported_VXVM(mp_string& strVgName, mp_bool& bIsExported)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecVgInfo;

    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_VXDG_LIST, "", &vecVgInfo);
    if (MP_SUCCESS != iRet || vecVgInfo.size() == 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get vg status info failed, iRet %d.", iRet);
        return ERROR_DEVICE_VXVM_QUERY_DG_STATUS_FAILED;
    }

    size_t idxSep;
    const mp_string SEPARATOR = STR_SPACE;
    mp_string strVolumeGroupName;
    vector<mp_string>::iterator it = vecVgInfo.begin();

    for (; it != vecVgInfo.end(); it++)
    {
        idxSep = it->find(SEPARATOR);
        if (mp_string::npos == idxSep)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get vg info failed when find 1nd separator, vg info is [%s].", (*it).c_str());;
            return ERROR_DEVICE_VXVM_QUERY_DG_STATUS_FAILED;
        }
        strVolumeGroupName = it->substr(0, idxSep);
        if (strVolumeGroupName == strVgName)
        {
            bIsExported = MP_FALSE;
            break;
        }
    }

    if(it == vecVgInfo.end())
    {
        bIsExported = MP_TRUE;
    }
      
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: IsVgExist
Description  : �ж�vg�Ƿ����
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_bool CLvm::IsVgExist(mp_string& strVgName)
{
    LOGGUARD("");

    mp_int32 iRet = CRootCaller::Exec(ROOT_COMMAND_VGS, strVgName, NULL);
    if (iRet == MP_SUCCESS)
    {
        return MP_TRUE;
    }
    else
    {
        return MP_FALSE;
    }
}
/*------------------------------------------------------------ 
Description  :��vg��Ϣд���ļ�
Input        : strMapInfo -- vg��Ϣ
                  strMapInfoFile--��д����ļ�
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_bool CLvm::WriteVgMapInfo(mp_string& strMapInfo, mp_string& strMapInfoFile)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    size_t idxSep;
    mp_string strParam;
    mp_string strVgInfo;
    mp_string strLvInfo;

    idxSep = strMapInfo.find("\n");
    if (mp_string::npos == idxSep)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
          "Get vg and lv info failed when find 1nd separator, vg map info is [%s].", strMapInfo.c_str());
        return MP_FAILED;
    }
    strVgInfo = strMapInfo.substr(0, idxSep);
    strLvInfo = strMapInfo.substr(idxSep + 1);
    
    strParam = " \"" + strVgInfo + "\">" + strMapInfoFile;
    iRet = CRootCaller::Exec(ROOT_COMMAND_ECHO, strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        return iRet;
    }

    strParam = " \"" + strLvInfo + "\">>" + strMapInfoFile;
    iRet = CRootCaller::Exec(ROOT_COMMAND_ECHO, strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        return iRet;
    }
    return MP_SUCCESS;
}

#endif //WIN32

