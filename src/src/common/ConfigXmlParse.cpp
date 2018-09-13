/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/ConfigXmlParse.h"
#include "common/Types.h"
#include "common/File.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"

CConfigXmlParser CConfigXmlParser::m_instance;


/*------------------------------------------------------------
Function Name:Init
Description  :��ʼ������
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CConfigXmlParser::Init(mp_string strCfgFilePath)
{
    m_strCfgFilePath = strCfgFilePath;
    m_lastTime = 0;
    return Load();
}

/*------------------------------------------------------------
Function Name:Load
Description  :����xml�����ļ�
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CConfigXmlParser::Load()
{
    if (!CMpFile::FileExist(m_strCfgFilePath.c_str()))
    {
        printf("Config file is not exist, path is \"%s\".\n", BaseFileName(m_strCfgFilePath.c_str()));
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    if (m_pDoc != NULL)
    {
        delete m_pDoc;
        m_pDoc = NULL;
    }

    try
    {
    	//CodeDex�󱨣�Memory Leak
        m_pDoc = new TiXmlDocument(m_strCfgFilePath.c_str());
    }
    catch(...)
    {
        m_pDoc = NULL;
    }

    if (!m_pDoc)
    {
        printf("New TiXmlDocument failed.\n");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    if (!m_pDoc->LoadFile())
    {
        printf("Load config xml file failed.\n");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    mp_time tLastMoidfyTime;
    mp_int32 iRet = CMpFile::GetlLastModifyTime(m_strCfgFilePath.c_str(), tLastMoidfyTime);
    if (MP_SUCCESS != iRet)
    {
        return MP_FALSE;
    }
    m_lastTime = tLastMoidfyTime;
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:GetChildElement
Description  :���ݸ��ڵ��ȡ�ӽڵ�����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
TiXmlElement* CConfigXmlParser::GetChildElement(TiXmlElement* pParentElement, mp_string strSection)
{
    if (pParentElement == NULL)
    {
        return NULL;
    }

    TiXmlElement *pCfgSec = pParentElement->FirstChildElement();
    if (NULL == pCfgSec)
    {
        return NULL;
    }

    while (pCfgSec)
    {
        //Coverity&Fortify��:FORTIFY.Null_Dereference
        const char* sectionName = pCfgSec->Value();
        if (sectionName == 0 || *sectionName == 0)
        {
            pCfgSec = pCfgSec->NextSiblingElement();
            continue;
        }

        if (0 == strcmp(sectionName, strSection.c_str()))
        {
            return pCfgSec;
        }
        else
        {
            pCfgSec = pCfgSec->NextSiblingElement();
        }
    }

    return NULL;
}

/*------------------------------------------------------------
Function Name:IsModified
Description  :�ж������ļ�������Ƿ��޸Ĺ�
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CConfigXmlParser::IsModified()
{
    mp_int32 iRet = MP_SUCCESS;

    if(m_strCfgFilePath.empty())
    {
        return MP_FALSE;
    }

    mp_time tLastMoidfyTime;
    iRet = CMpFile::GetlLastModifyTime(m_strCfgFilePath.c_str(), tLastMoidfyTime);
    if (MP_SUCCESS != iRet)
    {
        return MP_FALSE;
    }

    return (tLastMoidfyTime != m_lastTime);
}

/*------------------------------------------------------------
Function Name:ParseNodeValue
Description  :����xml����ĳ���ڵ��ֵ
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CConfigXmlParser::ParseNodeValue(TiXmlElement *pCfgSec, NodeValue& nodeValue)
{
    if (NULL == pCfgSec)
    {
        return;
    }
    TiXmlElement * pChildItem = pCfgSec->FirstChildElement();
    if (NULL == pChildItem)
    {
        return;
    }

    while(pChildItem)
    {
        //Coverity&Fortify��:FORTIFY.Null_Dereference
        const char* nodeName = pChildItem->Value();
        if (nodeName == NULL || *nodeName == 0)
        {
            pChildItem = pChildItem->NextSiblingElement();
            continue;
        }
        //�˴���������ж�����¼��xml���ã���Ҫ�����޸�
        TiXmlAttribute * pAttr = pChildItem->FirstAttribute();
        if (pAttr != NULL)
        {
            nodeValue[nodeName] = pAttr->Value();
        }
        pChildItem = pChildItem->NextSiblingElement();
    }
}

/*------------------------------------------------------------
Function Name:ParseNodeValue
Description  :������������ֵת�����ַ�����ֻ�ʺ�root��ֻ����һ����Ŀ¼�ĳ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CConfigXmlParser::GetValueString(mp_string strSection, mp_string strKey, mp_string& strValue)
{
    CThreadAutoLock lock(&m_cfgValueMutexLock);
    if (IsModified())
    {
        Load();
    }

    //rootElement��tinyxml��֤�ǿ�
    TiXmlElement *rootElement = m_pDoc->RootElement();

    TiXmlElement *pCfgSec = GetChildElement(rootElement, strSection);
    if (NULL == pCfgSec)
    {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    NodeValue nodeValue;
    ParseNodeValue(pCfgSec, nodeValue);

    NodeValue::iterator itNode = nodeValue.find(strKey);
    if (itNode == nodeValue.end())
    {
        printf("Section Name \"%s\" Key \"%s\" is not exist.\n", strSection.c_str(), strKey.c_str());
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    strValue = nodeValue[strKey];
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:ParseNodeValue
Description  :������������ֵת����boolֵ��ֻ�ʺ�root��ֻ����һ����Ŀ¼�ĳ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CConfigXmlParser::GetValueBool(mp_string strSection, mp_string strKey, mp_bool& bValue)
{
    mp_string strValue;
    mp_int32 iRet = GetValueString(strSection, strKey, strValue);
    if (iRet != MP_SUCCESS)
    {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    mp_int32 iValue = atoi(strValue.c_str());
    iValue != 0 ? (bValue = MP_TRUE) : (bValue = MP_FALSE);
    return MP_SUCCESS;

}

/*------------------------------------------------------------
Function Name:GetValueInt32
Description  :������������ֵת����int��ֻ�ʺ�root��ֻ����һ����Ŀ¼�ĳ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CConfigXmlParser::GetValueInt32(mp_string strSection, mp_string strKey, mp_int32& iValue)
{
    mp_string strValue;
    mp_int32 iRet = GetValueString(strSection, strKey, strValue);
    if (iRet != MP_SUCCESS)
    {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    iValue = atoi(strValue.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:GetValueInt32
Description  :������������ֵת����float��ֻ�ʺ�root��ֻ����һ����Ŀ¼�ĳ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CConfigXmlParser::GetValueFloat(mp_string strSection, mp_string strKey, mp_float& fValue)
{
    mp_string strValue;
    mp_int32 iRet = GetValueString(strSection, strKey, strValue);
    if (iRet != MP_SUCCESS)
    {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    fValue = (mp_float)atof(strValue.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:GetValueInt64
Description  :������������ֵת���ɳ�int��ֻ�ʺ�root��ֻ����һ����Ŀ¼�ĳ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CConfigXmlParser::GetValueInt64(mp_string strSection, mp_string strKey, mp_int64& lValue)
{
    mp_string strValue;
    mp_int32 iRet = GetValueString(strSection, strKey, strValue);
    if (iRet != MP_SUCCESS)
    {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    lValue = atol(strValue.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:SetValue
Description  :���������ļ�ĳ���ڵ��ֵ��ֻ�ʺ�root��ֻ����һ����Ŀ¼�ĳ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CConfigXmlParser::SetValue(mp_string strSection, mp_string strKey, mp_string strValue)
{
    CThreadAutoLock lock(&m_cfgValueMutexLock);
    if (!CMpFile::FileExist(m_strCfgFilePath.c_str()))
    {
        printf("Config file is not exist, path is \"%s\".\n", BaseFileName(m_strCfgFilePath.c_str()));
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    //rootElement��tinyxml��֤�ǿ�
    TiXmlElement *rootElement = m_pDoc->RootElement();

    mp_bool bChanged = MP_FALSE;

    TiXmlElement *pCfgSec = GetChildElement(rootElement, strSection);
    if (pCfgSec == NULL)
    {
        printf("GetChildElement failed.\n");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    //�ҵ�Section
    TiXmlElement *pChildItem = pCfgSec->FirstChildElement();
    while(pChildItem)
    {
        //Coverity&Fortify��:FORTIFY.Null_Dereference
        const mp_char* nodeName = pChildItem->Value();
        mp_bool bIsKeyEqual =  ( nodeName == NULL || *nodeName == 0 || (0 != strcmp(nodeName, strKey.c_str())) );
        if ( bIsKeyEqual )
        {
            pChildItem = pChildItem->NextSiblingElement();
            continue;
        }

        //�ҵ�node
        TiXmlAttribute *pAttr = pChildItem->FirstAttribute();
        if (NULL == pAttr)
        {
            bChanged = MP_FALSE;
        }
        else
        {
            pAttr->SetValue(strValue.c_str());
            bChanged = MP_TRUE;
        }
        break;
    }

    if (bChanged)
    {
        m_pDoc->SaveFile(m_strCfgFilePath.c_str());
    }
    else
    {
        printf("attribute can not be find, Key is %s.", strKey.c_str());
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    mp_time tLastMoidfyTime;
    mp_int32 iRet = CMpFile::GetlLastModifyTime(m_strCfgFilePath.c_str(), tLastMoidfyTime);
    if (MP_SUCCESS != iRet)
    {
        return MP_FALSE;
    }

    m_lastTime = tLastMoidfyTime;
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:SetValue
Description  :���������ļ�ĳ���ڵ��ֵ��ֻ�ʺ�root��ֻ����2����������Ŀ¼�ĳ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CConfigXmlParser::SetValue(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_string strValue)
{
    CThreadAutoLock lock(&m_cfgValueMutexLock);
    if (!CMpFile::FileExist(m_strCfgFilePath.c_str()))
    {
        printf("Config file is not exist, path is \"%s\".\n", BaseFileName(m_strCfgFilePath.c_str()));
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    //rootElement��tinyxml��֤�ǿ�
    TiXmlElement *rootElement = m_pDoc->RootElement();

    mp_bool bChanged = MP_FALSE;

    TiXmlElement *pCfgParentSec = GetChildElement(rootElement, strParentSection);
    if (NULL == pCfgParentSec)
    {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    TiXmlElement *pCfgSec = GetChildElement(pCfgParentSec, strChildSection);
    if (NULL == pCfgSec)
    {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    //�ҵ�Section
    TiXmlElement * pChildItem = pCfgSec->FirstChildElement();
    while(pChildItem)
    {
        //Coverity&Fortify��:FORTIFY.Null_Dereference
        const mp_char* nodeName = pChildItem->Value();
        mp_bool bIsKeyEqual =  ( nodeName == NULL || *nodeName == 0 || (0 != strcmp(nodeName, strKey.c_str())) );
        if ( bIsKeyEqual )
        {
            pChildItem = pChildItem->NextSiblingElement();
            continue;
        }

        //�ҵ�node
        TiXmlAttribute * pAttr = pChildItem->FirstAttribute();
        if (NULL == pAttr)
        {
            bChanged = MP_FALSE;
        }
        else
        {
            pAttr->SetValue(strValue.c_str());
            bChanged = MP_TRUE;
        }
        break;
    }

    if (bChanged)
    {
        m_pDoc->SaveFile(m_strCfgFilePath.c_str());
    }
    else
    {
        printf("attribute can not be find, Key is %s", strKey.c_str());
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    mp_time tLastMoidfyTime;
    mp_int32 iRet = CMpFile::GetlLastModifyTime(m_strCfgFilePath.c_str(), tLastMoidfyTime);
    if (MP_SUCCESS != iRet)
    {
        return MP_FALSE;
    }

    m_lastTime = tLastMoidfyTime;
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:GetValueString
Description  :��ȡ�����ļ�ĳ���ڵ��ֵ����ת����string��ֻ�ʺ�root��ֻ����2����������Ŀ¼�ĳ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CConfigXmlParser::GetValueString(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_string& strValue)
{
    CThreadAutoLock lock(&m_cfgValueMutexLock);
    if (IsModified())
    {
        Load();
    }

    //tinyxml��֤rootElementΪ�ǿ�
    TiXmlElement *rootElement = m_pDoc->RootElement();
    //CodeDex��,KLOCWORK.NPD.FUNC.MUST
    TiXmlElement *pCfgParentSec = GetChildElement(rootElement, strParentSection);
    if (NULL == pCfgParentSec)
    {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    TiXmlElement *pCfgSec = GetChildElement(pCfgParentSec, strChildSection);
    if (NULL == pCfgSec)
    {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    NodeValue nodeValue;
    ParseNodeValue(pCfgSec, nodeValue);

    NodeValue::iterator itNode = nodeValue.find(strKey);
    if (itNode == nodeValue.end())
    {
        printf("Parent section name \"%s\", child section name \"%s\", key \"%s\" is not exist.",
            strParentSection.c_str(), strChildSection.c_str(), strKey.c_str());
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    strValue = nodeValue[strKey];
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:GetValueBool
Description  :��ȡ�����ļ�ĳ���ڵ��ֵ����ת����bool��ֻ�ʺ�root��ֻ����2����������Ŀ¼�ĳ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CConfigXmlParser::GetValueBool(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_bool& bValue)
{
    mp_string strValue;
    mp_int32 iRet = GetValueString(strParentSection, strChildSection, strKey, strValue);
    if (iRet != MP_SUCCESS)
    {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    mp_int32 iValue = atoi(strValue.c_str());
    iValue != 0 ? (bValue = MP_TRUE) : (bValue = MP_FALSE);
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:GetValueInt32
Description  :��ȡ�����ļ�ĳ���ڵ��ֵ����ת����int32��ֻ�ʺ�root��ֻ����2����������Ŀ¼�ĳ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CConfigXmlParser::GetValueInt32(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_int32& iValue)
{
    mp_string strValue;
    mp_int32 iRet = GetValueString(strParentSection, strChildSection, strKey, strValue);
    if (iRet != MP_SUCCESS)
    {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    iValue = atoi(strValue.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:GetValueInt64
Description  :��ȡ�����ļ�ĳ���ڵ��ֵ����ת����int64��ֻ�ʺ�root��ֻ����2����������Ŀ¼�ĳ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CConfigXmlParser::GetValueInt64(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_int64& lValue)
{
    mp_string strValue;
    mp_int32 iRet = GetValueString(strParentSection, strChildSection, strKey, strValue);
    if (iRet != MP_SUCCESS)
    {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    lValue = atol(strValue.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:GetValueFloat
Description  :��ȡ�����ļ�ĳ���ڵ��ֵ����ת����float��ֻ�ʺ�root��ֻ����2����������Ŀ¼�ĳ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CConfigXmlParser::GetValueFloat(mp_string strParentSection, mp_string strChildSection, mp_string strKey, mp_float& fValue)
{
    mp_string strValue;
    mp_int32 iRet = GetValueString(strParentSection, strChildSection, strKey, strValue);
    if (iRet != MP_SUCCESS)
    {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    fValue = (mp_float)atof(strValue.c_str());
    return MP_SUCCESS;

}


