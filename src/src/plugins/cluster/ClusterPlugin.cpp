/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "plugins/cluster/ClusterPlugin.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/RootCaller.h"
#include "rest/Interfaces.h"
#include "rest/MessageProcess.h"

REGISTER_PLUGIN(CClusterPlugin); //lint !e19
CClusterPlugin::CClusterPlugin()
{
    //cluster
    REGISTER_ACTION(REST_CLUSTER_QUERY_CLUSTERINFO, REST_URL_METHOD_GET, &CClusterPlugin::QueryClusterInfo);
    REGISTER_ACTION(REST_CLUSTER_QUERY_ACTIVEHOST, REST_URL_METHOD_GET, &CClusterPlugin::QueryActiveHost);
    REGISTER_ACTION(REST_CLUSTER_START_CLUSTER, REST_URL_METHOD_PUT, &CClusterPlugin::StartCluster);
    REGISTER_ACTION(REST_CLUSTER_START_RESOURCEGROUP, REST_URL_METHOD_PUT, &CClusterPlugin::StartResouceGroup);
    REGISTER_ACTION(REST_CLUSTER_STOP_RESOURCEGROUP, REST_URL_METHOD_PUT, &CClusterPlugin::StopResouceGroup);
}

CClusterPlugin::~CClusterPlugin()
{
}
/*------------------------------------------------------------ 
Description  :��Ⱥ�����ͳһ�ӿ���ڣ��ڴ˷ַ����þ���Ľӿ�
Input        : req -- ������Ϣ
Output       : rsp -- ��Ӧ��Ϣ
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CClusterPlugin::DoAction(CRequestMsg* req, CResponseMsg* rsp)
{
    DO_ACTION(CClusterPlugin, req, rsp);
}
/*------------------------------------------------------------ 
Description  :��ѯ�����
Input        : req -- ������Ϣ
Output       : rsp -- ��Ӧ��Ϣ
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CClusterPlugin::QueryActiveHost(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strResGrpName;
    mp_string strClusterType;
    CRequestURL& vrequrl= req->GetURL();
    map<mp_string, mp_string>& vreqal= vrequrl.GetQueryParam();
    map<mp_string, mp_string>::iterator iter=vreqal.begin();

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to query active Cluster.");

    for (; iter != vreqal.end(); ++iter)
    {
        if (REST_PARAM_CLUSTER_RESGRPNAME == iter->first)
        {
            strResGrpName = iter->second;
        }
        
        if (REST_PARAM_CLUSTER_CLUSTERTYPE == iter->first)
        {
            strClusterType = iter->second;
        }
    }

    mp_bool bIsActive = MP_TRUE;
    mp_int32 iClusterType = atoi(strClusterType.c_str());
    //����У��
    mp_string strPre;
    mp_string strExclude;
    mp_string strInclude("");
    mp_int32 lenEnd = 1;
    mp_int32 lenBeg = 1;
    CHECK_FAIL_EX(CheckParamString(strResGrpName, 1, 254, strInclude, strExclude));
    strInclude = "1234567";
    CHECK_FAIL_EX(CheckParamString(strClusterType, lenBeg, lenEnd, strInclude, strExclude));
    iRet = m_cluster.IsActiveNode(strResGrpName, iClusterType, bIsActive);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query active Cluster failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& jValue= rsp->GetJsonValueRef(); 
    jValue[REST_PARAM_CLUSTER_ISACTIVE] = bIsActive; 

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query active Cluster succ.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :��ѯ��Ⱥ��Ϣ
Input        : req -- ������Ϣ
Output       : rsp -- ��Ӧ��Ϣ
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CClusterPlugin::QueryClusterInfo(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    db_info_t stdbInfo;
    mp_string strDBType;
    mp_string strClusterType;
    vector<cluster_info_t> vecClusterInfo;
    CRequestURL& vrequrl= req->GetURL();
    map<mp_string, mp_string>& vreqal= vrequrl.GetQueryParam();

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to query cluster info.");

	GetDBInfo(vreqal,stdbInfo,strDBType,strClusterType);
    //��ȡ��Ϣͷ�е��û�������
    (mp_void)GetDBAuthParam(req, stdbInfo.strdbUsername, stdbInfo.strdbPassword);
 
    //����У��
    mp_string strInclude("1234");
    mp_string strExclude;

//    CHECK_FAIL_EX(CheckParamString(strDBType, 1, 1, strInclude, strExclude));
    mp_int32 iDbType = atoi(strDBType.c_str());
    if ((1 != iDbType) && (2 != iDbType) && (3 != iDbType) && (4 != iDbType) && (15 != iDbType))
    {
        return MP_FAILED;
    }
    strInclude = "1234567";
    CHECK_FAIL_EX(CheckParamString(strClusterType, 1, 1, strInclude, strExclude));
        
    iRet = m_cluster.QueryClusterInfo(stdbInfo, strClusterType, strDBType, vecClusterInfo);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get cluster info failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& val= rsp->GetJsonValueRef();
    Json::Value jValue;
    vector<cluster_info_t>::iterator itertmp;
    for(itertmp = vecClusterInfo.begin(); itertmp != vecClusterInfo.end(); ++itertmp)
    {
        jValue[REST_PARAM_CLUSTER_CLUSTERNAME] = itertmp->strClusterName;
        jValue[REST_PARAM_CLUSTER_RESGRPNAME] = itertmp->strResGrpName;
        jValue[REST_PARAM_CLUSTER_VGACTIVEMODE] = itertmp->strVgActiveMode;      
        jValue[REST_PARAM_CLUSTER_NETWORKNAME] = itertmp->strNetWorkName;
        
        vector<mp_string> vecRes = itertmp->vecResourceName;
        mp_size vecSize = vecRes.size();
        Json::Value vecResource;
        mp_uint32 i = 0;
        for (; i < vecSize; ++i)
        {
            Json::Value valueResource = vecRes[i];
            vecResource.append(valueResource);
        }
        jValue[REST_PARAM_CLUSTER_RESOURCENAME] = vecResource;

        vector<mp_string> vecDevGrp = itertmp->vecDevGrpName;
        vecSize = vecDevGrp.size();
        Json::Value vecDeviceGroup;
        for (i = 0; i < vecSize; ++i)
        {
            Json::Value valueDeviceGroup = vecDevGrp[i];
            vecDeviceGroup.append(valueDeviceGroup);
        }
        jValue[REST_PARAM_CLUSTER_DEVGRPNAME] = vecDeviceGroup;
        val.append(jValue);  
    }

    stdbInfo.strdbPassword.replace(0, stdbInfo.strdbPassword.length(), "");
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Query cluster info succ.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :������Ⱥ
Input        : req -- ������Ϣ
Output       : rsp -- ��Ӧ��Ϣ
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CClusterPlugin::StartCluster(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strResGrpName;
    mp_string strClusterType;
    mp_int32 iClusterType;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to start cluster.");

    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jReqValue, REST_PARAM_CLUSTER_CLUSTERTYPE, strClusterType);
    GET_JSON_STRING(jReqValue, REST_PARAM_CLUSTER_RESGRPNAME, strResGrpName);

    //����У��
    mp_string strExclude;
    mp_string strPre;
    mp_string strInclude("1234567");
    CHECK_FAIL_EX(CheckParamString(strClusterType, 1, 1, strInclude, strExclude));
    strInclude = "";
    CHECK_FAIL_EX(CheckParamString(strResGrpName, 1, 254, strInclude, strExclude));

    iClusterType = atoi(strClusterType.c_str());
    iRet = m_cluster.StartCluster(iClusterType, strResGrpName);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start cluster failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start cluster succ.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :������Դ��
Input        : req -- ������Ϣ
Output       : rsp -- ��Ӧ��Ϣ
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CClusterPlugin::StartResouceGroup(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS; 
    mp_string strResGrpName;
    mp_string strClusterType;
    mp_string strDBType;
    vector<mp_string> vecMSFCRes;
    vector<mp_string> vecDevGrp;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to start resourcegroup.");

    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jReqValue, REST_PARAM_CLUSTER_CLUSTERTYPE, strClusterType);
    GET_JSON_STRING(jReqValue, REST_PARAM_CLUSTER_RESGRPNAME, strResGrpName);
    GET_JSON_STRING(jReqValue, REST_PARAM_CLUSTER_APPTYPE, strDBType);
    CHECK_JSON_VALUE(jReqValue, REST_PARAM_CLUSTER_RESOURCENAME);
    //����У��
    mp_string strExclude;
    mp_string strPre;
    mp_string strInclude("1234567");
    CHECK_FAIL_EX(CheckParamString(strClusterType, 1, 1, strInclude, strExclude));
    strInclude = "1234";
    CHECK_FAIL_EX(CheckParamString(strDBType, 1, 1, strInclude, strExclude));
    strInclude = "";
    CHECK_FAIL_EX(CheckParamString(strResGrpName, 1, 254, strInclude, strExclude));
    
    if (jReqValue[REST_PARAM_CLUSTER_RESOURCENAME].isArray())
    {
        mp_uint32 uiSize = jReqValue[REST_PARAM_CLUSTER_RESOURCENAME].size();
        for (mp_uint32 i = 0; i < uiSize; i++)
        {
            mp_string strResName = jReqValue[REST_PARAM_CLUSTER_RESOURCENAME][i].asString();
            CHECK_FAIL_EX(CheckParamString(strResName, 1, 254, strInclude, strExclude));
            vecMSFCRes.push_back(strResName);
        }
    }

    CHECK_JSON_VALUE(jReqValue, REST_PARAM_CLUSTER_DEVGRPNAME);
    if (jReqValue[REST_PARAM_CLUSTER_DEVGRPNAME].isArray())
    {
        mp_uint32 uiSize = jReqValue[REST_PARAM_CLUSTER_DEVGRPNAME].size();
        for (mp_uint32 i = 0; i < uiSize; i++)
        {
            mp_string strDevGrpName = jReqValue[REST_PARAM_CLUSTER_DEVGRPNAME][i].asString();
            CHECK_FAIL_EX(CheckParamString(strDevGrpName, 1, 254, strInclude, strExclude));
            vecDevGrp.push_back(strDevGrpName);
        }
    }
    
    iRet = m_cluster.StartResGrp(strResGrpName, vecDevGrp, strClusterType, strDBType, vecMSFCRes);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Start resourcegroup failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start resourcegroup succ.");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :ֹͣ��Դ��
Input        : req -- ������Ϣ
Output       : rsp -- ��Ӧ��Ϣ
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CClusterPlugin::StopResouceGroup(CRequestMsg* req, CResponseMsg* rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_string strResGrpName;
    mp_string strClusterType;
    mp_string strDBType;
    vector<mp_string> vecMSFCRes;
    vector<mp_string> vecDevGrp;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to stop resourcegroup.");
    const Json::Value& jReqValue = req->GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jReqValue, REST_PARAM_CLUSTER_CLUSTERTYPE, strClusterType);
    GET_JSON_STRING(jReqValue, REST_PARAM_CLUSTER_RESGRPNAME, strResGrpName);
    GET_JSON_STRING(jReqValue, REST_PARAM_CLUSTER_APPTYPE, strDBType);
    CHECK_JSON_VALUE(jReqValue, REST_PARAM_CLUSTER_RESOURCENAME);
    //����У��
    mp_string strExclude;
    mp_string strPre;
    mp_string strInclude("1234567");
    CHECK_FAIL_EX(CheckParamString(strClusterType, 1, 1, strInclude, strExclude));
    strInclude = "1234";
    CHECK_FAIL_EX(CheckParamString(strDBType, 1, 1, strInclude, strExclude));
    strInclude = "";
    CHECK_FAIL_EX(CheckParamString(strResGrpName, 1, 254, strInclude, strExclude));
    
    if (jReqValue[REST_PARAM_CLUSTER_RESOURCENAME].isArray())
    {
        mp_uint32 uiSize = jReqValue[REST_PARAM_CLUSTER_RESOURCENAME].size();
        for (mp_uint32 i = 0; i < uiSize; i++)
        {
            mp_string strResName = jReqValue[REST_PARAM_CLUSTER_RESOURCENAME][i].asString();
            CHECK_FAIL_EX(CheckParamString(strResName, 1, 254, strInclude, strExclude));
            vecMSFCRes.push_back(strResName);
        }
    }

    CHECK_JSON_VALUE(jReqValue, REST_PARAM_CLUSTER_DEVGRPNAME);
    if (jReqValue[REST_PARAM_CLUSTER_DEVGRPNAME].isArray())
    {
        mp_uint32 uiSize = jReqValue[REST_PARAM_CLUSTER_DEVGRPNAME].size();
        for (mp_uint32 i = 0; i < uiSize; i++)
        {
            mp_string strDevGrpName = jReqValue[REST_PARAM_CLUSTER_DEVGRPNAME][i].asString();
            CHECK_FAIL_EX(CheckParamString(strDevGrpName, 1, 254, strInclude, strExclude));
            vecDevGrp.push_back(strDevGrpName);
        }
    }
    
    iRet = m_cluster.StopResGrp(strResGrpName, vecDevGrp, strClusterType, strDBType, vecMSFCRes);
    if (MP_SUCCESS != iRet)
    {
        //rsp->SetRetCode(iRet);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Stop resourcegroup failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Stop resourcegroup succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :��ȡ�·������ݿ���֤��Ϣ
Input        : req -- rest����ṹ��
               strdbUsername -- ���ݿ��û���
               strdbPassword -- ���ݿ�����
Output       : strdbUsername -- ���ݿ��û���
               strdbPassword -- ���ݿ�����
Return       : MP_SUCCESS -- �ɹ�
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CClusterPlugin::GetDBAuthParam(CRequestMsg* req, mp_string &strdbUsername, mp_string &strdbPassword)
{
    const mp_char *pHeadStr = NULL;
    pHeadStr = req->GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBUSERNAME);
    if (pHeadStr)
    {
        strdbUsername = mp_string(pHeadStr);
        (mp_void)CMpString::Trim((mp_char *)strdbUsername.c_str());
    }
    else
    {
        strdbUsername = "";
    }

    pHeadStr = req->GetHttpReq().GetHeadNoCheck(HTTPPARAM_DBPASSWORD);
    if (pHeadStr)
    {
        strdbPassword = mp_string(pHeadStr);
        (mp_void)CMpString::Trim((mp_char *)strdbPassword.c_str());
    }
    else
    {
        strdbPassword = "";
    }
    
    return MP_SUCCESS;
}

mp_void CClusterPlugin::GetDBInfo(map<mp_string, mp_string>& vreqal, db_info_t &stdbInfo, mp_string &strDBType, mp_string &strClusterType)
{
	map<mp_string, mp_string>::iterator iter=vreqal.begin();
	
    for (; iter != vreqal.end(); ++iter)
    {
        if (REST_PARAM_CLUSTER_INSTNAME == iter->first)
        {
            stdbInfo.strinstName = iter->second;
        }
        if (REST_PARAM_CLUSTER_DBNAME == iter->first)
        {
            stdbInfo.strdbName = iter->second;
        }
        if (REST_PARAM_CLUSTER_CLUSTERTYPE == iter->first)
        {
            strClusterType = iter->second;
        }
        if (REST_PARAM_CLUSTER_APPTYPE == iter->first)
        {
            strDBType = iter->second;
        }
    }
}
