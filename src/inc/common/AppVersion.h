/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_APP_VERSION_
#define _AGENT_APP_VERSION_

#include "common/Types.h"

#define AGENT_PACKAGE_VERSION  "V100R001C00"
#define AGENT_VERSION          "V100R001C00"
#define RD_PROVIDER_VERSION    L"1.0.0.0"
#define COMPILE_TIME           "compile"

//�������"1.0.0" :
//1.ÿ��V����R�汾�ŵı������һ�����ּ�1.
//2.ÿ��C�汾�ŵı�����ڶ������ּ�1.  
//3.ÿ�ν����汾ʱ�����������ּ�1.
#define AGENT_BUILD_NUM        "1.0.0"

inline void AgentVersion()
{
    printf("Copyright 2017-2018 Huawei Technologies Co., Ltd.\n");
    printf("Version     : %s\n", AGENT_VERSION);
    printf("Build Number: %s\n", AGENT_BUILD_NUM);
}

#endif //_AGENT_APP_VERSION_

