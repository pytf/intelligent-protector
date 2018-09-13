/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef WIN32
#include "device/Raw.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"
#include "array/Array.h"

#include <sys/stat.h>
#include <sstream>

CRaw::CRaw()
{
}

CRaw::~CRaw()
{
}

#if defined LINUX
/*------------------------------------------------------------
Function Name: Create
Description  : �������豸
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Create(raw_info_t& rawInfo)
{
    LOGGUARD("");
    //���ж�raw�����Ƿ�����
    mp_int32 iRet = StartRawService();
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_START_FAILED);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check raw service failed.");
        return iRet;
    }

    //�ж����豸�Ƿ����
    if(CMpFile::FileExist(rawInfo.strRawDevPath.c_str()))
    {
        mp_string strUsedDevByRaw;
        iRet = GetDeviceUsedByRaw(rawInfo.strRawDevPath, strUsedDevByRaw);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_FILESYS_GET_DEV_FAILED);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get block device \"%s\" for raw failed.",
                     rawInfo.strRawDevPath.c_str());
            return iRet;
        }
        if (rawInfo.strDevName == strUsedDevByRaw)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The raw dev \"%s\" is bounded to the right block dev.",
                rawInfo.strDevName.c_str());
            return MP_SUCCESS;
        }
        else if (!strUsedDevByRaw.empty())
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The raw dev is not bounded to the expected block dev, expected dev %s, curr dev %s.",
                rawInfo.strDevName.c_str(), strUsedDevByRaw.c_str());
            return ERROR_DEVICE_RAW_USED_BY_OTHER_DEV;
        }
        //��strUsedDevByRawΪ���ַ����Ǵ������豸�ļ������Ƕ�Ӧ�Ŀ��豸�ļ��Ѿ�������
        //δɾ�����豸ʱȥӳ����ܴ��������ĳ���
        else
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The bounded dev is not exist, bound raw %s to dev %s.",
                rawInfo.strRawDevPath.c_str(), rawInfo.strDevName.c_str());

            //ɾ�������豸�Ժ��ٴ���
            iRet = Delete(rawInfo);
            TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_DELETE_FAILED);
            if (MP_SUCCESS != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete raw device before recreate it again failed, raw device %s.",
                    rawInfo.strRawDevPath.c_str());
                return iRet;
            }
        }
    }

    //�ж������豸�Ƿ��Ƿ���
    mp_string::size_type pos = rawInfo.strDevName.find_first_of(SECTION_NUM_TAG);
    mp_bool bSection = MP_FALSE;
    mp_string strSectionNum;
    if (mp_string::npos != pos)
    {
        bSection = MP_TRUE;
        strSectionNum = rawInfo.strDevName.substr(pos, pos + 1);
    }

    //����LUN WWN��ȡ�����ڵ��豸����
    mp_string strDevName;
    iRet = CDisk::GetDevNameByWWN(strDevName, rawInfo.strLunWWN);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_NOT_HUAWEI_LUN);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get device name by wwn faild, wwn is \"%s\".",
                    rawInfo.strLunWWN.c_str());
        return iRet;
    }
    if (bSection)
    {
        strDevName = strDevName + strSectionNum;
    }

    //server�ܱ�֤���豸������ȷ����������ֻ�������
    mp_string strParam = rawInfo.strRawDevPath + " " + rawInfo.strDevName;
    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_RAW, strParam, NULL, ERROR_DEVICE_RAW_CREATE_FAILED);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create raw device, raw device name \"%s\", block device name \"%s\".",
               rawInfo.strRawDevPath.c_str(), rawInfo.strDevName.c_str());

    //�������豸�Ժ��Ӧ��/dev/raw/rawx�ļ������������ɣ�һ���ܿ����ɣ�ԭ�����ں˷���Ϣ��udevd��Ȼ������raw�豸�ļ�
    //���������豸�ļ��Ժ����syncϵͳ����ˢ���ļ�ϵͳ���浽����
    sync();

    mp_bool bIsRawExist = CMpFile::WaitForFile(rawInfo.strRawDevPath.c_str(), CHECK_RAW_DEVICE_INTERVAL, CHECK_RAW_DEVICE_COUNT);
    if (!bIsRawExist)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Raw device file \"%s\" dose not exist.", rawInfo.strRawDevPath.c_str());
        return ERROR_COMMON_DEVICE_NOT_EXIST;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create raw device successful.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: Delete
Description  : ɾ�����豸
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Delete(raw_info_t& rawInfo)
{
    LOGGUARD("");
#if defined LINUX
    //�ж����豸�Ƿ����
    if(!CMpFile::FileExist(rawInfo.strRawDevPath.c_str()))
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The raw device \"%s\" dose not exist.",
            rawInfo.strRawDevPath.c_str());
        return MP_SUCCESS;

    }

    mp_string strParam = rawInfo.strRawDevPath + " 0 0";
    ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_RAW, strParam, NULL, ERROR_DEVICE_RAW_DELETE_FAILED);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Delete raw device successful.");
    return MP_SUCCESS;
#endif
#if defined AIX
    mp_string strDevName;
    mp_int32 iRet = CDisk::GetDevNameByWWN(strDevName, rawInfo.strLunWWN);
    //���ͨ��wwn��ѯ�����豸����ֱ�ӷ��سɹ�
    if (ERROR_COMMON_DEVICE_NOT_EXIST == iRet)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Dev of WWN(%s) is not exist.", rawInfo.strLunWWN.c_str());
        return MP_SUCCESS;
    }

    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_NOT_HUAWEI_LUN);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Excute GetDevNameByWWN failed, iRet %d.", iRet);
        return iRet;
    }

    if (strDevName != rawInfo.strDevName.substr(strlen("/dev/")))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Dev name(%s) of WWN(%s) is not equal to %s.",
            strDevName.c_str(), rawInfo.strLunWWN.c_str(), rawInfo.strDevName.c_str());
        
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_string strParam = "-dl " + strDevName;
    return CRootCaller::Exec(ROOT_COMMAND_RMDEV, strParam, NULL);
#endif
}

/*------------------------------------------------------------
Function Name: StartRawService
Description  : �������豸����
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::StartRawService()
{
#ifdef SUSE
    //�ж�raw service�Ƿ��Ѿ�����
    mp_string strParam = "|grep '^raw'";
    mp_int32 iRet = CRootCaller::Exec(ROOT_COMMAND_LSMOD, strParam, NULL);
    if (iRet == MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Raw service is already started.");
        return MP_SUCCESS;
    }

    strParam = "raw start";
    ROOT_EXEC(ROOT_COMMAND_SERVICE, strParam, NULL);
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetDeviceUsedByRaw
Description  : ��ȡ���豸��ʹ�õ��豸
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::GetDeviceUsedByRaw(mp_string& strRawDevPath, mp_string& strUsedDevName)
{
    mp_int32 iBoundMajor, iBoundMinor;
    mp_int32 iRet = GetBoundedDevVersions(strRawDevPath, iBoundMajor, iBoundMinor);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get device version for raw device \"%s\" failed.",
              strRawDevPath.c_str());
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get device version succ, raw device \"%s\", bound major %d, bound minor %d.",
            strRawDevPath.c_str(), iBoundMajor, iBoundMinor);

    //��ȡ/devĿ¼�������豸�ļ��б�
    vector<mp_string> vecFileNames;
    mp_string strDir = "/dev";
    iRet = CMpFile::GetFolderFile(strDir, vecFileNames);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get file list in dir \"%s\" failed.", strDir.c_str());
        return iRet;
    }

    if (0 == vecFileNames.size())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Dir \"%s\" is empty.", strDir.c_str());
        return MP_SUCCESS;
    }

    mp_string strFilePath;
    mp_int32 iMajorTmp, iMinorTmp;
    for (vector<mp_string>::iterator iter = vecFileNames.begin(); iter != vecFileNames.end(); iter++)
    {
        strFilePath = strDir + PATH_SEPARATOR + *iter;
        iRet = GetDeviceNumber(strFilePath, iMajorTmp, iMinorTmp);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get major and minor version for \"%s\" failed",
                strFilePath.c_str());
            return iRet;
        }
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get dev version succ, dev name \"%s\", major %d, minor %d.",
            strFilePath.c_str(), iMajorTmp, iMinorTmp);

        if (iMajorTmp == iBoundMajor && iMinorTmp == iBoundMinor)
        {
            strUsedDevName = strFilePath;
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Device \"%s\" was already bounded to the raw device %s.",
                strUsedDevName.c_str(), strRawDevPath.c_str());
            break;
        }
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetBoundedDevVersions
Description  : ��ȡbounded�汾
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::GetBoundedDevVersions(mp_string& strRawDevPath, mp_int32& iBoundMajor, mp_int32& iBoundMinor)
{
    struct stat sb;
    mp_int32 iMajor = 0;
    mp_int32 iMinor = 0;

    mp_int32 iRet = stat(strRawDevPath.c_str(), &sb);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stat raw device failed, raw device \"%s\", errno %d.",
            strRawDevPath.c_str(), errno);
        return MP_FAILED;
    }

    if (!S_ISCHR(sb.st_mode))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "\"%s\" is not a character device.",
            strRawDevPath.c_str());
        return MP_FAILED;
    }

    //major��minor��ͷ�ļ�<sys/sysmacros.h>�ж���
    iMajor = major(sb.st_rdev); //lint !e747
    iMinor = minor(sb.st_rdev); //lint !e747
    if (iMajor != RAW_MAJOR)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "\"%s\" is not a raw device.",
            strRawDevPath.c_str());
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get raw device version succ, raw device \"%s\", major %d, minor %d.",
        strRawDevPath.c_str(), iMajor, iMinor);

    //��iMinor��Ϊ��������rootcaller
    ostringstream oss;
    oss << iMinor;
    mp_string strParam = oss.str();
    vector<mp_string> vecRlt;
    ROOT_EXEC(ROOT_COMMAND_RAW_MINOR_MAJOR, strParam, &vecRlt);

    if (vecRlt.size() != 2)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Size of vecRlt is not 2.");
        return MP_FAILED;
    }
    iBoundMajor = atoi(vecRlt[0].c_str());
    iBoundMinor = atoi(vecRlt[1].c_str());
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get raw device bounded info succ, raw device \"%s\", bound major %d, bound minor %d.",
           strRawDevPath.c_str(), iBoundMajor, iBoundMinor);
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetDeviceNumber
Description  : ��ȡ���豸���к�
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::GetDeviceNumber(mp_string& rstrDeviceName, mp_int32& iMajor, mp_int32& iMinor)
{
    struct stat sb;
    mp_int32 iRet = stat(rstrDeviceName.c_str(), &sb);
    if (0 != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "stat raw dev \"%s\" failed.", rstrDeviceName.c_str());
        return MP_FAILED;
    }

    iMajor = major(sb.st_rdev); //lint !e747
    iMinor = minor(sb.st_rdev); //lint !e747

    return MP_SUCCESS;
}
#endif

#if defined AIX
/*------------------------------------------------------------
Function Name: Create
Description  : �������豸
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Create(raw_info_t& rawInfo)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strDev;
    mp_string strRawDevPath;
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_DEBUG, 
        "Begin to create raw device, device name is %s, rawdevice name is %s.", 
        rawInfo.strDevName.c_str(), rawInfo.strRawDevPath.c_str());
    strDev = rawInfo.strDevName.substr(strlen("/dev/"));

    // �ж����豸�´洢�����豸�Ƿ���豸������ͬ
    strRawDevPath = rawInfo.strRawDevPath.substr(0,5) + rawInfo.strRawDevPath.substr(6);
    //����ͬ��˵����mknod�������豸����Ҫ����һ��
    if (rawInfo.strDevName != strRawDevPath)
    {
        // ͨ��mknod�����豸�ļ�
        iRet = CreateRAWDevice(rawInfo);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CreateDeviceByMknod failed.");
            return iRet;
        }
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create raw device succ.");
        return MP_SUCCESS;
    }
    
    //��AIX�£����Ȱ�������·���dev�����Ƿ�Ϊhdisk,Ӧ�ÿ��������ĳ���:
    //��hdisk2����Ϊoraasm2,����ȥӳ��֮������ӳ������,�����ֱ��hdiskxx��������Ҫ����www�ҵ�hdiskȻ��������.
    mp_bool bIsHdisk = CDisk::IsHdisk(strDev);
    if (!bIsHdisk)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Raw device name(%s) isn't hdisk, need to rename.", strDev.c_str());
        mp_string strHdiskName;
        mp_bool bIsDevExist = MP_FALSE;
        mp_bool bRet = MP_FALSE;
        iRet = CDisk::GetDevNameByWWN(strHdiskName, rawInfo.strLunWWN);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get device name failed, WWN: %s.", rawInfo.strLunWWN.c_str());
            return iRet;
        }
    
        mp_string strParam = "\"" + strHdiskName + "\" -n \"" + strDev + "\"";
        vector<mp_string> vecResult;
        
        iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_RENDEV, strParam, &vecResult);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Change dev name failed, param: rendev -l %s, iRet: %d.", strParam.c_str(), iRet);
            return iRet;
        }
    
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Raw device rename success, before rename: %s, after rename: %s.", strHdiskName.c_str(), strDev.c_str());
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create raw device succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: Delete
Description  : ɾ�����豸
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Delete(raw_info_t& rawInfo)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strDev;
    mp_string strRawDevPath;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_DEBUG, 
        "Begin to delete raw device, device name is %s, rawdevice name is %s.", 
        rawInfo.strDevName.c_str(), rawInfo.strRawDevPath.c_str());
    strDev = rawInfo.strDevName.substr(strlen("/dev/"));
    //�ж����豸�Ƿ����
    if(CMpFile::FileExist(rawInfo.strRawDevPath.c_str()))
    {

        // �ж����豸�´洢�����豸�Ƿ���豸������ͬ
        strRawDevPath = rawInfo.strRawDevPath.substr(0,5) + rawInfo.strRawDevPath.substr(6);
        //����ͬ��˵����mknod�������豸����Ҫɾ��
        if (rawInfo.strDevName != strRawDevPath)
        {
            // ɾ��ͨ��mknod�������豸�ļ�
            ROOT_EXEC_TRANS_RETCODE(ROOT_COMMAND_RM, rawInfo.strRawDevPath, NULL, ERROR_DEVICE_RAW_DELETE_FAILED);
        }
    }
    
    mp_string strParam = "-dl " + strDev;
    return CRootCaller::Exec(ROOT_COMMAND_RMDEV, strParam, NULL);
}

/*------------------------------------------------------------
Description  : ͨ��mknod�����豸
Input        : rstMountDiskInfo -- ������Ϣ
Output       :
Return       : MP_SUCCESS -- �����ɹ�
               ��ISSP_RTN_OK -- ����ʧ��
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CRaw::CreateRAWDevice(raw_info_t& rawInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string majorNumDevice = "", minorNumDevice = "", majorNumMount = "", minorNumMount = "";
    mp_bool isExists = MP_FALSE;
    mp_string strDeviceName = "";

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to create Device handle by mknod, device name %s, to create %s.",
        rawInfo.strDevName.c_str(), rawInfo.strRawDevPath.c_str());

    strDeviceName = rawInfo.strDevName.insert(5,"r");
    // ��ȡԭʼ�豸�豸��
    iRet = GetDeviceNumber(strDeviceName, majorNumDevice, minorNumDevice);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_CREATE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get AIX rhisk device number failed, device name %s",
            strDeviceName.c_str());

        return iRet;
    }

    // �ж���Ҫ�����豸�Ƿ����
    iRet = IsDeviceExists(rawInfo.strRawDevPath, isExists);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_CREATE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check raw device failed, raw device name %s",
            rawInfo.strRawDevPath.c_str());

        return iRet;
    }

    // �����ǰ�豸�Ѿ�������Ҫ�ж��Ƿ���׼���������豸
    if (MP_TRUE == isExists)
    {
        iRet = CheckDeviceByDeviceNumber(rawInfo, strDeviceName, majorNumMount, minorNumMount, majorNumDevice, minorNumDevice);
        if(MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Compare raw device(%s) and mknod device(%s) by device number failed .", 
            rawInfo.strRawDevPath.c_str(), strDeviceName.c_str());
            return iRet;
        }
    }
     // ��Ҫ�������豸�����ڣ�ֱ�Ӵ���
    else
    {
        iRet = CreateDeviceByMknod(rawInfo.strRawDevPath, majorNumDevice, minorNumDevice);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_CREATE_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create device(%s) by mknod(%s) failed.", 
                rawInfo.strRawDevPath.c_str(), strDeviceName.c_str());
            return iRet;
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to create Device handle by mknod, device name %s, to create %s.",
        strDeviceName.c_str(), rawInfo.strRawDevPath.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :  ����DFS�����豸��Ϣ
Input        :
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CRaw::CheckDeviceByDeviceNumber(raw_info_t& rawInfo, mp_string strDeviceName, mp_string &majorNumMount, mp_string &minorNumMount, 
    mp_string majorNumDevice, mp_string minorNumDevice)
{
        mp_int32 iRet = MP_SUCCESS;
        iRet = GetDeviceNumber(rawInfo.strRawDevPath, majorNumMount, minorNumMount);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_CREATE_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get AIX device number of raw device failed, device name %s.",
                rawInfo.strRawDevPath.c_str());
            return iRet;
        }

        // �ж�ͨ��mknod���豸�Ƿ���׼���������豸
        mp_bool bEqual = majorNumDevice != majorNumMount || minorNumDevice != minorNumMount;
        if (bEqual)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The device (%s) is created by device (major %s, minor %s), do nothing do mknod.",
                rawInfo.strRawDevPath.c_str(), majorNumMount.c_str(), minorNumMount.c_str());
            return ERROR_DEVICE_RAW_USED_BY_OTHER_DEV;
        }
        else
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The device (%s) is created by device (%s), do nothing do mknod.",
                rawInfo.strRawDevPath.c_str(), strDeviceName.c_str());
        }

        return MP_SUCCESS;
}
#endif

#ifdef HP_UX_IA
/*------------------------------------------------------------
Function Name: Create
Description  : �������豸(֮ǰ�豸�������豸����Ϊ���������ƣ�
               �־��Ż�Ϊ�ֱ�������)
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Create(raw_info_t& rawInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strRawDevice;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, 
		"Begin to create raw device, device name is (%s), raw device name is (%s).", 
		rawInfo.strDevName.c_str(), rawInfo.strRawDevPath.c_str());
    //��������豸��ȡ����hdisk����Ȩ�޸�ֵ�����lv�����������ֲ���仯
    if (CDisk::IsDskdisk(rawInfo.strDevName))
    {
        // �ж����豸�´洢�����豸�Ƿ���豸������ͬ
        // �����ͬ���豸Ϊ/dev/dsk/cxxdxxtxx��/dev/rdsk/cxxdxxtxx
        mp_string strRawDevPath;
        strRawDevPath = rawInfo.strRawDevPath.substr(0,5) + rawInfo.strRawDevPath.substr(6);

        //����ͬ��˵����mknod�������豸����Ҫ����һ��
        if (rawInfo.strDevName != strRawDevPath)
        {
            // ͨ��mknod�����豸�ļ�
            iRet = CreateRAWDevice(rawInfo);
            if (MP_SUCCESS != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CreateDeviceByMknod failed.");
                return iRet;
            }
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create raw device succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: Delete
Description  : ɾ�����豸
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Delete(raw_info_t& rawInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strPersistentDSF;
    mp_string strRawDevPath;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to delete raw device, raw device %s.",
            rawInfo.strRawDevPath.c_str());

    //�ж����豸�Ƿ����
    if(!CMpFile::FileExist(rawInfo.strRawDevPath.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The raw device \"%s\" dose not exist.",
            rawInfo.strRawDevPath.c_str());
        return MP_SUCCESS;

    }

    // �ж����豸�´洢�����豸�Ƿ���豸������ͬ
    // �����ͬ���豸Ϊ/dev/dsk/cxxdxxtxx��/dev/rdsk/cxxdxxtxx
    strRawDevPath = rawInfo.strRawDevPath.substr(0,5) + rawInfo.strRawDevPath.substr(6);
    //����ͬ��˵����mknod�������豸����Ҫɾ��
    if (rawInfo.strDevName!= strRawDevPath)
    {
        iRet = DeleteDevice(rawInfo.strRawDevPath);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_DELETE_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete mount path (%s) failed.",
                rawInfo.strRawDevPath.c_str());
            return iRet;
        }
    }

    // 2014-11-20:
    //����ɾ��persistent disk�����ɾ����Ҳ���Զ����ɣ�������IO�޷�ȥӳ��
    //
    // 2014-11-19:
    //ɾ�� persistent disk�����ʹ��disk mknod���豸��������deleteDevice��ͻ�ֱ��
    // ɾ����Ӧdisk�豸���˴���ɾ��ʧ�ܣ����жϲ������
    //iRet = DeletePersistentDSF(strPersistentDSF);
    //if (ISSP_RTN_OK != iRet)
    //{
    //    COMMLOG(OS_LOG_ERROR, LOG_AGENT_INTERNAL_ERROR, "Delete persistent DSF (%s) failed.",
    //        strPersistentDSF.c_str());
    //    //return ISSP_RTN_FAIL;
    //}
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Delete raw device %s succ.", rawInfo.strRawDevPath.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : ͨ��mknod�����豸
Input        : rstMountDiskInfo -- ������Ϣ
Output       :
Return       : MP_SUCCESS -- �����ɹ�
               ��ISSP_RTN_OK -- ����ʧ��
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CRaw::CreateRAWDevice(raw_info_t& rawInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string majorNumDevice = "", minorNumDevice = "", majorNumMount = "", minorNumMount = "";
    mp_bool isExists = MP_FALSE;
    mp_bool isPDMk = MP_TRUE;
    mp_string strDeviceName = "";

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to create Device handle by mknod, device name %s, to create %s.",
        rawInfo.strDevName.c_str(), rawInfo.strRawDevPath.c_str());
	
    iRet = CDisk::GetHPRawDiskName(rawInfo.strDevName, strDeviceName);
	if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get raw disk name of disk(%s) failed, ret %d.",
                rawInfo.strDevName.c_str(), iRet);

        return ERROR_DISK_GET_RAW_DEVICE_NAME_FAILED;
    }

    // ��ȡԭʼ�豸�豸��
    iRet = GetDeviceNumber(strDeviceName, majorNumDevice, minorNumDevice);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_CREATE_FAILED);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get HP Disk SN failed, device name %s",
            rawInfo.strDevName.c_str());

        return iRet;
    }

    // �ж���Ҫ�����豸�Ƿ����?
    iRet = IsDeviceExists(rawInfo.strRawDevPath, isExists);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check device failed, device name %s",
            rawInfo.strRawDevPath.c_str());

        return iRet;
    }

    // �����ǰ�豸�Ѿ�������Ҫ�ж��Ƿ���׼���������豸
    if (MP_TRUE == isExists)
    {
        iRet = UpdateDeviceByDsf(rawInfo, majorNumMount, minorNumMount, majorNumDevice, minorNumDevice);
        if(MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "UpdateDeviceByDsf failed.");
            return iRet;
        }
    }
     // ��Ҫ�������豸�����ڣ�ֱ�Ӵ���
    else
    {
        iRet = CreateDeviceByMknod(rawInfo.strRawDevPath, majorNumDevice, minorNumDevice);
        TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_RAW_CREATE_FAILED);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CreateDeviceByMknod failed");
            return iRet;
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to create Device handle by mknod, device name %s, to create %s.",
        rawInfo.strDevName.c_str(), rawInfo.strRawDevPath.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : ��ȡDevice info
Input        : strMajor -- Device ��major
                 strMinor -- Device ��minor
Output       : deviceStatus -- �豸״̬ 1������ 2NO_HW״̬ 3����״̬
Return       : MP_SUCCESS -- �����ɹ�
               ��ISSP_RTN_OK -- ����ʧ��
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CRaw::GetDeviceStatus(mp_string &strMajor, mp_string &strMinor, mp_char &deviceStatus)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecResult1;
    vector<mp_string> vecResult2;
    mp_bool bRet;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to get device status(m %s, n %s).",
        strMajor.c_str(), strMinor.c_str());

    //ls -l /dev/rdisk/* | awk '{if ($6=="0x550600") print $NF}'; ls -l /dev/rdsk/* | awk '{if ($6=="0x550600") print $NF}'

    strParam = " /dev/rdisk/* | awk '{if ($5== \"" + strMajor + "\") print $NF\" \"$6}' | awk '{if ($2== \""
        + strMinor + "\") print $1}'";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_LS, strParam, &vecResult1);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Search device failed m %s, n %s.", strMajor.c_str(), strMinor.c_str());
        return iRet;
    }

    strParam = " /dev/rdsk/* | awk '{if ($5== \"" + strMajor + "\") print $NF\" \"$6}' | awk '{if ($2== \""
        + strMinor + "\") print $1}'";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_LS, strParam, &vecResult2);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Search device failed m %s, n %s.", strMajor.c_str(), strMinor.c_str());
        return iRet;
    }

    // �豸������
    bRet = (vecResult1.empty()&&vecResult2.empty());
    if (bRet)
    {
        deviceStatus = HP_DEVICE_STATUS_NOEXISTS;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to get device status(m %s, n %s), Device is not exists.",
            strMajor.c_str(), strMinor.c_str());
        return MP_SUCCESS;
    }

    // ��ȡ�豸״̬
    mp_string strDevice = vecResult1.empty()?vecResult2.front():vecResult1.front();
    vecResult1.clear();
    vecResult2.clear();

    // ����disk�豸���ڣ�����ͨ��ioscan�޷���ѯ���������
    //ioscan -funC disk -N | grep /dev/rdisk/disk421; ioscan -funC disk | grep /dev/rdisk/disk421

    strParam = " -funC disk -N | grep " + strDevice;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCAN, strParam, &vecResult1);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Search device by ioscan failed, device %s.", strDevice.c_str());
        return iRet;
    }

    strParam = " -funC disk | grep " + strDevice;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCAN, strParam, &vecResult2);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Search device by ioscan failed, device %s.", strDevice.c_str());
        return iRet;
    }

    // �豸������
    bRet = (vecResult1.empty()&&vecResult2.empty());
    if (bRet)
    {
        deviceStatus = HP_DEVICE_STATUS_NOEXISTS;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to get device status by ioscan(device %s), device is not valid.",
            strDevice.c_str());
        return MP_SUCCESS;
    }
    vecResult1.clear();
    vecResult2.clear();

    // �жϸ��豸�Ƿ���NO_HW״?
    //ioscan -funC disk -N /dev/rdisk/disk421 | grep NO_HW; ioscan -funC disk /dev/rdisk/disk421 | grep NO_HW

    strParam = " -funC disk -N " + strDevice + " | grep NO_HW";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCAN, strParam, &vecResult1);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Search device status by ioscan failed, device %s.", strDevice.c_str());
        return iRet;
    }

    strParam = " -funC disk " + strDevice + " | grep NO_HW";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_IOSCAN, strParam, &vecResult2);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Search device status by ioscan failed, device %s.", strDevice.c_str());
        return iRet;
    }

    // �豸ʱ��NO_HW״̬
    bRet = (!vecResult1.empty()||!vecResult2.empty());
    if (bRet)
    {
        deviceStatus = HP_DEVICE_STATUS_NOHW;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to get device status by ioscan(device %s), device S/W status is NO_HW.",
            strDevice.c_str());
    }
    else
    {
        deviceStatus = HP_DEVICE_STATUS_NORMAL;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to get device status by ioscan(device %s), device S/W status is normal.",
            strDevice.c_str());
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to get device status(m %s, n %s).",
        strMajor.c_str(), strMinor.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : ɾ���豸
Input        : strDeviceName -- ��Ҫɾ���豸���豸����
Output       :
Return       : MP_SUCCESS -- �����ɹ�
               ��ISSP_RTN_OK -- ����ʧ��
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CRaw::DeleteDevice(mp_string& strDeviceName)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    mp_bool bDeviceExists = MP_FALSE;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to delete device %s.", strDeviceName.c_str());

    iRet = IsDeviceExists(strDeviceName, bDeviceExists);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check device %s exists failed.", strDeviceName.c_str());
        return MP_FAILED;
    }

    if (MP_FALSE == bDeviceExists)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Device %s is not exists.", strDeviceName.c_str());
        return MP_SUCCESS;
    }

    strParam = " -a " + strDeviceName;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_RMSF, strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete device %s failed.", strDeviceName.c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to delete device %s.", strDeviceName.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :  ����DFS�����豸��Ϣ
Input        :
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CRaw::UpdateDeviceByDsf(raw_info_t& rawInfo, mp_string &majorNumMount, mp_string &minorNumMount, 
    mp_string majorNumDevice, mp_string minorNumDevice)
{
        mp_int32 iRet = MP_SUCCESS;
        iRet = GetDeviceNumber(rawInfo.strRawDevPath, majorNumMount, minorNumMount);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get HP Device SN failed, device name %s",
                rawInfo.strRawDevPath.c_str());

            return iRet;
        }

        // �ж�ͨ��mknod���豸�Ƿ���׼���������豸
        // ���������Ҫ�ж��豸״̬
        //1.����mk���豸�Ѿ�������
        //2.����mk���豸�Ѿ���NO_HW״̬
        //3.����mk���豸ʱ����״̬
        mp_bool bEqual = majorNumDevice != majorNumMount || minorNumDevice != minorNumMount;
        if (bEqual)
        {
            mp_char deviceStatus = 0;
            iRet = GetDeviceStatus(majorNumMount, minorNumMount, deviceStatus);
            if (MP_SUCCESS != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetDeviceStatus failed");
                return iRet;
            }

            // ����������豸״̬�쳣����ɾ��mountpath
            mp_bool needDel = (HP_DEVICE_STATUS_NOEXISTS == deviceStatus || HP_DEVICE_STATUS_NOHW == deviceStatus);
            if (needDel)
            {
                iRet = DeleteDevice(rawInfo.strRawDevPath);
                if (MP_SUCCESS != iRet)
                {
                    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CreateDeviceByMknod failed");
                    return iRet;
                }
            }
            // �豸�����Ѿ�������ռ�ã����ش���
            else
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The device (%s) is created by device (major %s, minor %s), do nothing do mknod.",
                    rawInfo.strRawDevPath.c_str(), majorNumMount.c_str(), minorNumMount.c_str());
                return ERROR_DEVICE_RAW_USED_BY_OTHER_DEV;
            }
        }
        else
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "The device (%s) is created by device (%s), do nothing do mknod.",
                rawInfo.strRawDevPath.c_str(), rawInfo.strDevName.c_str());
        }

        return MP_SUCCESS;
}
#endif

#if defined SOLARIS
/*------------------------------------------------------------
Function Name: Create
Description  : �������豸
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Create(raw_info_t& rawInfo)
{
    //solaris�����豸���贴��ֱ�ӷ��سɹ�
    LOGGUARD("");
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: Delete
Description  : ɾ�����豸
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::Delete(raw_info_t& rawInfo)
{
    LOGGUARD("");
    return MP_SUCCESS;
}
#endif

#if (defined HP_UX_IA) || (defined AIX)
/*------------------------------------------------------------
Function Name: GetDeviceNumber
Description  : ��ȡ���豸�����豸��
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRaw::GetDeviceNumber(mp_string& rstrDeviceName, mp_string& majorNum, mp_string& minorNum)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecResult;
    mp_string strRes;
    mp_string strParam;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to get Device number, device name %s.",
        rstrDeviceName.c_str());

    // ll /dev/rdsk/c84t0d3  | awk '{print $5,$6}'"
#ifdef HP_UX_IA
    strParam = " " + rstrDeviceName + "| awk '{print $5,$6}'";
#elif AIX
    strParam = " " + rstrDeviceName + "| awk '{print $5,$6}' | tr -d ','";
#endif
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_LS, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Device number failed, device name %s.",
            rstrDeviceName.c_str());

        return MP_FAILED;
    }

    if(!vecResult.empty())
    {
        strRes = vecResult.front();
    }
    vecResult.clear();

    // �������ؽ��188 0x540300
    size_t blackIndex = strRes.find(" ");
    if (mp_string::npos == blackIndex)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Device number failed, device name %s, find %s.",
            rstrDeviceName.c_str(), strRes.c_str());
        return MP_FAILED;
    }
    majorNum = strRes.substr(0, blackIndex);
    minorNum = strRes.substr(blackIndex + 1);

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get Device number succ, device name %s, major %s, minor %s.",
        rstrDeviceName.c_str(), majorNum.c_str(), minorNum.c_str());
    return MP_SUCCESS;
}
#endif

/*------------------------------------------------------------
Description  : �ж�disk�Ƿ����
Input        : rstrDeviceName -- �豸����
Output       : isDeviceExist -- ISSP_TRUE �豸���ڡ�ISSP_FALSE �豸������
Return       : MP_SUCCESS -- �����ɹ�
               ��ISSP_RTN_OK -- ����ʧ��
Create By    :
Modification :
-------------------------------------------------------------*/
mp_int32 CRaw::IsDeviceExists(mp_string& rstrDeviceName, mp_bool& isDeviceExist)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecResult;
    mp_string strRes;
    mp_string strParam;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to check Device exists, device name %s.",
        rstrDeviceName.c_str());

    strParam = " " + rstrDeviceName + " | wc -l";
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_LS, strParam, &vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check Device exists failed, device name %s.",
            rstrDeviceName.c_str());

        return MP_FAILED;
    }

    if(!vecResult.empty())
    {
        strRes = vecResult.front();
        (void)CMpString::Trim((mp_char*)strRes.c_str());
    }
    vecResult.clear();

    if (0 == strcmp(strRes.c_str(), "0"))
    {
        isDeviceExist = MP_FALSE;
    }
    else
    {
        isDeviceExist = MP_TRUE;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Check Device exists succ, device name %s.",
        rstrDeviceName.c_str());

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :  ͨ��mknod�����豸
Input        : strDeviceName -- Ҫ�������豸
                  strMajorNum--ԭ�豸�����豸��
                  strMinorNum--ԭ�豸�Ĵ��豸��
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CRaw::CreateDeviceByMknod(mp_string &strDeviceName, mp_string strMajorNum,
    mp_string strMinorNum)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to create device %s by mknod with major %s, minor %s.",
            strDeviceName.c_str(), strMajorNum.c_str(), strMinorNum.c_str());

    // like this "mknod ora_data_05 c 188 0x540300"

    strParam = strDeviceName + " c " + strMajorNum + " " +strMinorNum;
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_MKNOD, strParam, NULL);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "mknod device failed, major %s, minor %s, to create %s.",
            strMajorNum.c_str(), strMinorNum.c_str(), strDeviceName.c_str());

        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Succ to create device %s by mknod with major %s, minor %s.",
            strDeviceName.c_str(), strMajorNum.c_str(), strMinorNum.c_str());
    return MP_SUCCESS;
}

#endif //WIn32

