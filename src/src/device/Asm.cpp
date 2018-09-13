/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "device/Asm.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"

#ifndef WIN32
CAsm::CAsm()
{
}

CAsm::~CAsm()
{
}
/*------------------------------------------------------------ 
Description  : ɨ��ASMLIB ����
Input        : ��
Output       : ��
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CAsm::AsmLibScan()
{
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin elevated privileges to scan asmlib disks.");
    
    //ʹ��oracleasmɨ�����ڱ�����root��ִ��
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCANASMLIB, "", NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DEVICE_ASM_SCAN_ASMLIB_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Elevated privileges to scan asmlib disks failed, iRet %d", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Elevated privileges to scan asmlib disks succ.");
    return MP_SUCCESS;
}
#endif //WIN32

