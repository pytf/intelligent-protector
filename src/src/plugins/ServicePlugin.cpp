/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/ServicePlugin.h"
#include "common/Log.h"
#include "common/ErrorCode.h"

////////////////////////////////////////////////////////////////////////////////////////////////
//�����ܶ�̬��������Ԥ��
////////////////////////////////////////////////////////////////////////////////////////////////
mp_void CServicePlugin::Initialize(IPluginManager* pMgr)
{
    return;
}

mp_int32 CServicePlugin::Destroy()
{
    return MP_SUCCESS;
}

mp_void CServicePlugin::SetOption(const mp_char* pszName, mp_void* pvVal)
{
    return;
}

mp_bool CServicePlugin::GetOption(const mp_char* pszName, mp_void* pvVal, mp_int32 sz)
{
    return MP_TRUE;
}

mp_void* CServicePlugin::CreateObject(const mp_char* pszName)
{
    return NULL;
}

mp_int32 CServicePlugin::GetClasses(IPlugin::DYN_CLASS *pClasses, mp_int32 sz)
{
    return MP_SUCCESS;
}

const mp_char* CServicePlugin::GetName()
{
    return NULL;
}

const mp_char* CServicePlugin::GetVersion()
{
    return NULL;
}

mp_size CServicePlugin::GetSCN()
{
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////


/*------------------------------------------------------------ 
Description  : ���ò����ִ������
Input        : req -- ������Ϣ
Output       : rsp -- ��Ӧ��Ϣ
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CServicePlugin::Invoke(CRequestMsg* req, CResponseMsg* rsp)
{
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin invoke service plugin.");
    iRet = DoAction(req, rsp);
    if (MP_SUCCESS != iRet)
    {
        rsp->SetRetCode((mp_int64)iRet);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Do rest action failed, iRet %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Invoke service plugin succ");
    return MP_SUCCESS;
}

