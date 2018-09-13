/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/FileSearcher.h"
#include "common/Defines.h"
#include "common/File.h"
#include "common/String.h"
#include "common/Log.h"
#include "securec.h"
/*------------------------------------------------------------ 
Description  :����·��
Input        :     pszPath---·��
Output       :   
Return       :    
                 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CFileSearcher::SetPath(const mp_char* pszPath)
{
    m_paths.clear();
    const mp_char* ptr = pszPath;
    const mp_char* p = NULL;
    mp_size sz = 0;
    while(*ptr)
    {
        while(' ' == *ptr) 
        {
            ptr++;
        }

        p = ptr;
        sz = 0;
        while(*ptr && *ptr != PATH_SEPCH)
        {
            ptr++;
            sz++;
        }

        if(*ptr)
        {
            ptr++;
        }

        AddPath(p, sz);
    }
    RebuildPathString();
}
/*------------------------------------------------------------ 
Description  :��ȡ·��
Input        :      
Output       :   
Return       :   m_strPath ---��õ�·��
                 
Create By    :
Modification : 
-------------------------------------------------------------*/
const mp_string& CFileSearcher::GetPath()
{
    return m_strPath;
}
/*------------------------------------------------------------ 
Description  :���·����Ϣ
Input        :      
Output       :   
Return       :    
                 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CFileSearcher::Clear()
{
    m_paths.clear();
    m_strPath = "";
}
/*------------------------------------------------------------ 
Description  :����·�������ļ��Ƿ����
Input        :      pszFile---�ļ�����strPath---·��
Output       :   
Return       :    MP_TRUE---���ҳɹ�
                 MP_FALSE---����ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CFileSearcher::Search(const mp_char* pszFile, mp_string& strPath)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin search file, name %s.", BaseFileName(pszFile));

    for(mp_size i = 0; i < m_paths.size(); i++)
    {
        if(IsExists(pszFile, m_paths[i], strPath))
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Search file succ.");
            return MP_TRUE;
        }
    }

    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Search file failed.");
    
    return MP_FALSE;
}
/*------------------------------------------------------------ 
Description  :����·�����Ҷ���ļ��Ƿ����
Input        :      pszFile---�ļ�����flist---�ļ���Ϣ�б�
Output       :   
Return       :  true---���ҳɹ�
                  false---����ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CFileSearcher::Search(const mp_char* pszFile, vector<mp_string>& flist)
{
    mp_size sz = 0;
    mp_string strPath;
    
    for(mp_size i = 0; i< m_paths.size(); i++)
    {
        if(IsExists(pszFile, m_paths[i], strPath))
        {
            flist.push_back(strPath);
            sz++;
        }
    }

    return (sz > 0);
}
/*------------------------------------------------------------ 
Description  :���·�� 
Input        :      pszPath---·��
Output       :   
Return       :   
                  
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CFileSearcher::AddPath(const mp_char* pszPath)
{
    if(NULL == pszPath)
    {
        return;
    }

    AddPath(pszPath, strlen(pszPath));
    RebuildPathString();
}
/*------------------------------------------------------------ 
Description  :��� ·����Ϣ
Input        :      pszDir---·����sz---·���ַ�����
Output       :   
Return       :   
                  
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CFileSearcher::AddPath(const mp_char* pszDir, mp_size sz)
{
    if('\0' == pszDir[0] || 0 == sz)
    {
        return;
    }

    const mp_char* p = pszDir;
    while(sz > 0 && ' ' == *p)
    {
        p++;
        sz--;
    }

    while(sz > 0 && ' ' == p[sz-1])
    {
        sz--;
    }

    if(0 == sz)
    {
        return;
    }
    
    while(sz > 0 && IS_DIR_SEP_CHAR(p[sz-1]))
    {
        if(sz == 1)
        {
            break;
        }
        
        sz--;
    }
    
    m_paths.push_back(mp_string(p, sz));
}
/*------------------------------------------------------------ 
Description  :�ж��ļ��Ƿ����
Input        :      pszFile---�ļ���strDir---Ŀ¼
Output       :   strPath---·��
Return       :   
                  
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CFileSearcher::IsExists(const mp_char* pszFile, const mp_string& strDir, mp_string& strPath)
{
    mp_char szFile[1024] = {0};
    mp_int32 iRet = MP_SUCCESS;
    
    iRet = SNPRINTF_S(szFile, sizeof(szFile), sizeof(szFile) - 1, "%s%s%s", strDir.c_str(), 
        PATH_SEPARATOR, pszFile);
    if (MP_FAILED == iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Snprintfs failed.");
        return MP_FALSE;
    }

    if (!CMpFile::FileExist(szFile))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "File not exist, file %s.", BaseFileName(szFile));
        return MP_FALSE;
    }

    strPath = szFile;
    return MP_TRUE;
}
/*------------------------------------------------------------ 
Description  :�ؽ�·��Ϊ�ַ�����ʽ
Input        :      
Output       :    
Return       :   
                  
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CFileSearcher::RebuildPathString()
{
    mp_string strPath;
    for(mp_size i = 0; i < m_paths.size(); i++)
    {
        if(i > 0)
        {
            strPath.push_back(PATH_SEPCH);
        }
        strPath += m_paths[i];
    }
    
    m_strPath = strPath;
}

