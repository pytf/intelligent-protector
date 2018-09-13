/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/File.h"
#include "common/Defines.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "securec.h"
#include "common/CryptAlg.h"
#include "common/Path.h"

#ifndef WIN32
#include <dirent.h>
#endif

/*------------------------------------------------------------
Function Name: OpenFile
Description  : ���ļ�

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CMpFile::OpenFile()
{
    //to do
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: FileExist
Description  : �ж�ָ��·���ļ��Ƿ����

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_bool CMpFile::FileExist(const mp_char* pszFilePath)
{
#ifdef WIN32
    struct _stat fileStat;
    if (0 != _stat(pszFilePath, &fileStat))
#else
    struct stat fileStat;
    if (0 != stat(pszFilePath, &fileStat))
#endif
    {
        return MP_FALSE;
    }

    //Ŀ¼����false
    if (S_ISDIR(fileStat.st_mode))
    {
        return MP_FALSE;
    }

    return MP_TRUE;
}

/*------------------------------------------------------------
Function Name: DirExist
Description  : �ж�ָ��·���ļ����Ƿ����

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_bool CMpFile::DirExist(const mp_char* pszDirPath)
{
#ifdef WIN32
    struct _stat fileStat;
    if (0 != _stat(pszDirPath, &fileStat))
#else
    struct stat fileStat;
    if (0 != stat(pszDirPath, &fileStat))
#endif
    {
        return MP_FALSE;
    }

    //Ŀ¼����false
    if (!S_ISDIR(fileStat.st_mode))
    {
        return MP_FALSE;
    }

    return MP_TRUE;
 }

/*------------------------------------------------------------
Function Name: CreateDir
Description  : �����ļ���

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CMpFile::CreateDir(const mp_char* pszDirPath)
{
    //to do
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: FileSize
Description  : ��ȡ�ļ���С����λ���ֽ�

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CMpFile::FileSize(const mp_char* pszFilePath, mp_uint32& uiSize)
{
    if (NULL == pszFilePath)
    {
        return MP_FAILED;
    }

#ifdef WIN32
    struct _stat fileStat;
    if (0 != _stat(pszFilePath, &fileStat))
#else
    struct stat fileStat;
    if (0 != stat(pszFilePath, &fileStat))
#endif
    {
        return MP_FAILED;
    }

    uiSize = fileStat.st_size;

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetlLastModifyTime
Description  : ��ȡ�ļ���һ�θ���ʱ��

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CMpFile::GetlLastModifyTime(const mp_char* pszFilePath, mp_time& tLastModifyTime)
{
    if (NULL == pszFilePath)
    {
        return MP_FAILED;
    }

#ifdef WIN32
    struct _stat fileStat;
    if (0 != _stat(pszFilePath, &fileStat))
#else
    struct stat fileStat;
    if (0 != stat(pszFilePath, &fileStat))
#endif
    {
        return MP_FAILED;
    }

    tLastModifyTime = fileStat.st_mtime;

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ReadFile
Description  : ���ļ��ж���ÿ�����ݵ�string��vector�У������ɾ�����ļ���
               �ú��������ӽ��ܲ���

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CMpFile::ReadFile(mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    //CodeDex�󱨣�ZERO_LENGTH_ALLOCATIONS
    //CodeDex�󱨣�FORTIFY.Path_Manipulation
    //codedex��, CANONICAL_FILEPATH��FilePath·���������������
    LOGGUARD("");
    mp_int32 iRet = MP_FAILED;
    mp_int32 iTmp = strFilePath.find_last_of(PATH_SEPARATOR);
    mp_string strFileName = strFilePath.substr(iTmp + 1);

    if (!CMpFile::FileExist(strFilePath.c_str()))
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Can't find file %s.", strFileName.c_str());

        return MP_FAILED;
    }

    FILE* pFile = fopen(strFilePath.c_str(), "r");
    if (NULL == pFile)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open file %s failed, errno[%d]:%s.", strFileName.c_str(),
            errno, strerror(errno));

        return MP_FAILED;
    }

    if(fseek(pFile, 0L, SEEK_END) != 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call fseek failed, errno[%d]:%s.", errno, strerror(errno));
        iRet = fclose(pFile);
        GETOSERRORCODE(0 != iRet);
        return MP_FAILED;
    }

    mp_int32 iFileLen = (mp_int32)ftell(pFile);
    //����ļ�Ϊ�գ�ֱ�ӷ���
    if (iFileLen == 0)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "File %s to be read is empty.", strFileName.c_str());
        vecOutput.clear();
        iRet = fclose(pFile);
        GETOSERRORCODE(0 != iRet);
        return MP_SUCCESS;
    }

    if(fseek(pFile, 0L, SEEK_SET) != 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call fseek failed, errno[%d]:%s.", errno, strerror(errno));
        iRet = fclose(pFile);
        GETOSERRORCODE(0 != iRet);
        return MP_FAILED;
    }

    mp_char *pBuff;
    //CodeDex�󱨣�Memory Leak
    NEW_ARRAY_CATCH(pBuff, mp_char, iFileLen + 1);
    if (!pBuff)
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "New failed.");
        iRet = fclose(pFile);
        GETOSERRORCODE(0 != iRet);
        return MP_FAILED;
    }
    iRet = memset_s(pBuff, iFileLen + 1, 0, iFileLen + 1);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iRet);
        delete[] pBuff;
        iRet = fclose(pFile);
        GETOSERRORCODE(0 != iRet);
        return MP_FAILED;
    }
    while(NULL != fgets(pBuff, iFileLen + 1, pFile))
    {
        //д�ļ�ʱ��ÿ��ĩβ�����˻��з�����ȡʱȥ�����з�'\n'
        //���һ�п���û�л��з�����Ҫ���⴦��
        if (pBuff[strlen(pBuff) - 1] == '\n')
        {
            pBuff[strlen(pBuff) - 1] = 0;
        }
        vecOutput.push_back(pBuff);
        iRet = memset_s(pBuff, iFileLen + 1, 0, iFileLen + 1);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iRet);
            delete[] pBuff;
            fclose(pFile);
            return MP_FAILED;
        }
    }

    delete[] pBuff;
    iRet = fclose(pFile);
    GETOSERRORCODE(0 != iRet);
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetFolderFile
Description  : ����ĳ���ļ����������ļ�����

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CMpFile::GetFolderFile(mp_string& strFolder, vector<mp_string>& vecFileList)
{
#ifdef WIN32
    WIN32_FIND_DATA fd = {0};
    HANDLE hSearch;
    mp_string strFilePathName;
    mp_int32 iRet = EOK;

    strFilePathName = strFolder + "\\*";
    //CodeDex�󱨣�Missing Check against Null
    hSearch = FindFirstFile(strFilePathName.c_str(), &fd);
    if (INVALID_HANDLE_VALUE  == hSearch)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "hSearch is INVALID_HANDLE_VALUE.");
        return MP_FAILED;
    }

    if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, ".."))
        {
            vecFileList.push_back(fd.cFileName);
        }
    }

    for (;;)
    {
        iRet = memset_s(&fd, sizeof(fd), 0, sizeof(fd));
        if (EOK != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "memset_s failed, iRet = %d.", iRet);
            FindClose(hSearch);
            return MP_FAILED;
        }

        if (!FindNextFile(hSearch, &fd))
        {
            if (ERROR_NO_MORE_FILES == GetLastError())
            {
                break;
            }
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "FindNextFile failed.");
            FindClose(hSearch);
            return MP_FAILED;
        }

        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, ".."))
            {
                vecFileList.push_back(fd.cFileName);
            }
        }
    }

    FindClose(hSearch);

    return MP_SUCCESS;
#else
    DIR *dir;
    struct dirent *ptr = NULL;
    struct dirent dp;
    struct stat strFileInfo = {0};
    mp_char acFullName[MAX_PATH_LEN] = {0};

    dir = opendir(strFolder.c_str());
    if (NULL == dir)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "opendir failed.");
        return MP_FAILED;
    }
    
    while ((MP_SUCCESS == readdir_r(dir, &dp, &ptr)) && (NULL != ptr))
    {
        if(MP_FAILED == SNPRINTF_S(acFullName, MAX_PATH_LEN, MAX_PATH_LEN - 1, \
            "%s/%s", strFolder.c_str(), ptr->d_name))
        {
            closedir(dir);
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "SNPRINTF_S failed.");
            return MP_FAILED;
        }


        if (lstat(acFullName, &strFileInfo) < 0)
        {
            closedir(dir);
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "lstat failed.");
            return MP_FAILED;
        }

        if (!S_ISDIR(strFileInfo.st_mode))
        {
            if (strcmp(ptr->d_name, ".") && strcmp(ptr->d_name, ".."))
            {
                vecFileList.push_back(ptr->d_name);
            }
        }
    }

    closedir(dir);
    return MP_SUCCESS;
#endif
}

mp_bool CMpFile::WaitForFile(const mp_char* pszFilePath, mp_int32 iSleepInterval, mp_int32 iSleepCount)
{
    mp_bool bIsExist = MP_FALSE;
    mp_int32 iCount = iSleepCount;

    while(iCount > 0)
    {
        if (FileExist(pszFilePath))
        {
            bIsExist = MP_TRUE;
            break;
        }

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "File \"%s\" dose not exist, wait for it.", pszFilePath);

        DoSleep(iSleepInterval);
        iCount--;
    }

    return bIsExist;
}

mp_int32 CMpFile::DelFile(const mp_char* pszFilePath)
{
    if (NULL == pszFilePath)
    {
        return MP_SUCCESS;
    }
    if (!FileExist(pszFilePath))
    {
        return MP_SUCCESS;
    }
#ifdef WIN32
    DeleteFile(pszFilePath);
    return MP_SUCCESS;
#else
    //Coverity&Fortify��:FORTIFY.Race_Condition--File_System_Access 
    //DelFile������û����setuid������rootʱ����
    CHECK_NOT_OK(remove(pszFilePath));
#endif

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ReadFile
Description  : ���ļ������Ķ���ÿ�����ݵ�string��vector�У������ɾ�����ļ���
               �ú��������ӽ��ܲ���

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CIPCFile::ReadFile(mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    //CodeDex�󱨣�ZERO_LENGTH_ALLOCATIONS
    //codedex��CANONICAL_FILEPATH��FilePath·���������������
    mp_int32 iTmp = strFilePath.find_last_of(PATH_SEPARATOR);
    mp_string strFileName = strFilePath.substr(iTmp + 1);

    mp_int32 iRet = CMpFile::ReadFile(strFilePath, vecOutput);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read file failed.");
        return iRet;
    }

    //ɾ��Դ�ļ�
    iRet = remove(strFilePath.c_str());
    if (0 != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Remove file %s failed, errno[%d]:%s.", strFileName.c_str(),
            errno, strerror(errno));
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: WriteFile
Description  : ��string��vector�е�Ԫ������ȡ��������д���ļ���
               �ú��������ӽ��ܲ���

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CIPCFile::WriteFile(mp_string& strFilePath, vector<mp_string>& vecInput)
{
//CodeDex�󱨣�TOCTOU
#ifndef WIN32
    struct stat fileStat;
    mp_int32 iRet1 = MP_FAILED;
    iRet1 = memset_s(&fileStat, sizeof(fileStat), 0, sizeof(fileStat));
    if ( iRet1 != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret = %d.", iRet1);
        return MP_FAILED;
    }
    mp_int32 fileExist = 0;
#endif 
    mp_int32 iRet = MP_FAILED;
    //codedex��CANONICAL_FILEPATH��FilePath·���������������
    mp_int32 iTmp = strFilePath.find_last_of(PATH_SEPARATOR);
    mp_string strFileName = strFilePath.substr(iTmp + 1);
    
    const mp_char* strBaseFileName = NULL;
    //CodeDex�󱨣�Unchecked Return Value����BaseFileName�ķ���ֵ����Ϊ��
    strBaseFileName = BaseFileName(strFilePath.c_str());
    
    if (CMpFile::FileExist(strFilePath.c_str()))
    {
        //��ȡ�ļ�Ȩ��
#ifndef WIN32
        if( 0 != stat(strFilePath.c_str(), &fileStat))
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get file stat of %s failed, errno[%d]:%s.", strBaseFileName, 
                errno, strerror(errno));
            return MP_FAILED;
        }
        fileExist = 1;
#endif 
        //ɾ���������ļ�
        iRet = remove(strFilePath.c_str());
        if (0 != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Remove file %s failed, errno[%d]:%s.", strBaseFileName,
                errno, strerror(errno));
            return MP_FAILED;
        }
    }
    //codedex�󱨣� Race Condition:File System Access
    FILE* pFile = fopen(strFilePath.c_str(), "a+");
    if (NULL == pFile)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open file %s failed, errno[%d]:%s.", strBaseFileName,
            errno, strerror(errno));
        return MP_FAILED;
    }

    for (vector<mp_string>::iterator iter = vecInput.begin(); iter != vecInput.end(); iter++)
    {
        fprintf(pFile, "%s\n", iter->c_str());
    }

    iRet = fflush(pFile);
    if (0 != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call fflush failed, errno[%d]:%s.", errno, strerror(errno));
        iRet = fclose(pFile);
        GETOSERRORCODE(0 != iRet);
        return MP_FAILED;
    }
    iRet = fclose(pFile);
    GETOSERRORCODE(0 != iRet);
#ifndef WIN32
    //���ļ��޸�Ϊɾ��֮ǰȨ��
    if ( 1 == fileExist)
    {
        if (chmod(strFilePath.c_str(), fileStat.st_mode) == -1)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "chmod hostsn %s failed, errno[%d]:%s.", strBaseFileName,
                errno, strerror(errno));
            CMpFile::DelFile(strFilePath.c_str());
            return MP_FAILED;
        }
    }
#endif
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Write file succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: WriteInput
Description  : ��strInputд���ļ�

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CIPCFile::WriteInput(mp_string& strUniqueID, mp_string& strInput)
{
    mp_string strFileName = INPUT_TMP_FILE + strUniqueID;
    //������ʱ�ļ���·��
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);
    
    const mp_char* strBaseFileName = NULL;
    strBaseFileName = BaseFileName(strFileName.c_str());
    
    vector<mp_string> vecInput;
    vecInput.push_back(strInput);

    mp_int32 iRet = WriteFile(strFilePath, vecInput);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Write input info file failed, file %s.", strBaseFileName);
    }

    return iRet;
}

/*------------------------------------------------------------
Function Name: ReadInput
Description  : ���ļ��ж�ȡ��������

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CIPCFile::ReadInput(mp_string& strUniqueID, mp_string& strInput)
{
    mp_string strFileName = INPUT_TMP_FILE + strUniqueID;
    //������ʱ�ļ���·��
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);
    vector<mp_string> vecOutput;
    
    const mp_char* strBaseFileName = NULL;
    strBaseFileName = BaseFileName(strFileName.c_str());
    
    mp_int32 iRet = ReadFile(strFilePath, vecOutput);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Write input info file failed, file %s.", strBaseFileName);
        return iRet;
    }

    if (vecOutput.size() != 1)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invalid file content, file %s, line count %d.",
            strBaseFileName, vecOutput.size());
        return MP_FAILED;
    }

    strInput = vecOutput[0];
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Read input info file succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: WriteResult
Description  : ��ִ�н����vecRlt��������ȡ������д�뵽��ʱ����ļ���

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CIPCFile::WriteResult(mp_string& strUniqueID, vector<mp_string>& vecRlt)
{
    if (vecRlt.empty())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "vecRlt is empty, no need to write result file.");
        return MP_SUCCESS;
    }
    mp_string strFileName = RESULT_TMP_FILE + strUniqueID;
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);
    
    const mp_char* strBaseFileName = NULL;
    strBaseFileName = BaseFileName(strFileName.c_str());
    
    mp_int32 iRet = WriteFile(strFilePath, vecRlt);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Write result file failed, file %s.", strBaseFileName);
    }
    else
    {
        mp_int32 iChownRet = ChownResult(strUniqueID);
        if (MP_SUCCESS != iChownRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Chown result file failed, file %s.", strBaseFileName);
        }
    }
    
    return iRet;
}

/*------------------------------------------------------------
Function Name: ReadResult
Description  : ����ʱ����ļ��ж�ȡ���ݣ�д�뵽vecRlt��

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CIPCFile::ReadResult(mp_string& strUniqueID, vector<mp_string>& vecRlt)
{
    mp_string strFileName = RESULT_TMP_FILE + strUniqueID;
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);
    
    const mp_char* strBaseFileName = NULL;
    strBaseFileName = BaseFileName(strFileName.c_str());
    
    //����������ļ������ڣ��п�����û���������������ݣ�����������سɹ�
    if (!CMpFile::FileExist(strFilePath.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Can't find file %s.", strBaseFileName);
        return MP_SUCCESS;
    }

    mp_int32 iRet = ReadFile(strFilePath, vecRlt);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read result file failed, file %s.", strBaseFileName);
    }

    return iRet;
}

/*------------------------------------------------------------
Function Name: ReadOldResult
Description  : ����ʱ����ļ��ж�ȡ���ݣ�д�뵽vecRlt�У���ʱ�ļ���ʽ����V1R3���

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CIPCFile::ReadOldResult(mp_string& strUniqueID, vector<mp_string>& vecRlt)
{
    mp_string strFileName = "RST" + strUniqueID + ".txt";
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);
    
    const mp_char* strBaseFileName = NULL;
    strBaseFileName = BaseFileName(strFileName.c_str());
    
    //����������ļ������ڣ��п�����û���������������ݣ�����������سɹ�
    if (!CMpFile::FileExist(strFilePath.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Can't find file %s.", strBaseFileName);
        return MP_SUCCESS;
    }

    mp_int32 iRet = ReadFile(strFilePath, vecRlt);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read result file failed, file %s.", strBaseFileName);
    }

    return iRet;
}


/*------------------------------------------------------------
Function Name: ChownResult
Description  : �޸���ʱ����ļ���Ȩ�ޣ��ĳ�rdadmin�û�
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CIPCFile::ChownResult(mp_string& strUniqueID)
{
#ifndef WIN32
    mp_int32 iRet = MP_SUCCESS;
    //��ȡrdadmin�û���uid
    mp_int32 uid(-1), gid(-1);

    mp_string strFileName = RESULT_TMP_FILE + strUniqueID;
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(strFileName);
    
    const mp_char* strBaseFileName = NULL;
    strBaseFileName = BaseFileName(strFileName.c_str());
    
    //����������ļ������ڣ��п�����û���������������ݣ�����������سɹ�
    //rdagent�ڶ�ȡ����ļ�ʱ���жϲ����Ľ��
    if (!CMpFile::FileExist(strFilePath.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Can't find file %s.", strBaseFileName);
        return MP_SUCCESS;
    }

    iRet = GetUidByUserName(AGENT_RUNNING_USER, uid, gid);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get user(%s) uid and gid failed.", AGENT_RUNNING_USER);
        return MP_FAILED;
    }

    // ����rdadmin��uid��gid
    iRet = ChownFile(strFilePath, uid, gid);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Chown file failed, file %s.", strBaseFileName);
        return MP_FAILED;
    }
    
    return MP_SUCCESS;
#else
    return MP_SUCCESS;
#endif
}

