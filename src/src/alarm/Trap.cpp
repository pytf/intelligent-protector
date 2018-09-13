/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "alarm/Trap.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "common/Mac.h"
#include "common/Path.h"
#include "rest/Interfaces.h"

/*------------------------------------------------------------
Function Name:SendAlarm
Description  :���͸澯
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CTrapSender::SendAlarm(alarm_param_t &alarmParam)
{
    LOGGUARD("");
    //ͨ���澯������ѯdb���Ƿ����ͬ���澯
    alarm_Info_t alarmInfo;

    mp_int32 iRet = CAlarmDB::GetAlarmInfoByParam(alarmParam.iAlarmID, alarmParam.strAlarmParam, alarmInfo);
    if (MP_SUCCESS != iRet)
    {
        //��¼��־
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetAlarmInfoByParam failed, alarmID = %d, alarmParam = %s",
             alarmParam.iAlarmID, alarmParam.strAlarmParam.c_str());
        return iRet;
    }

    //���db��û����ͬ�澯��������һ��
    if (alarmInfo.iAlarmSN == -1)
    {
        iRet = NewAlarmRecord(alarmParam, alarmInfo);
        if (MP_SUCCESS != iRet)
        {
            //��¼��־
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "NewAlarmRecord failed, alarmID = %d, alarmParam=%s",
                 alarmParam.iAlarmID, alarmParam.strAlarmParam.c_str());
            return iRet;
        }
        else
        {
            //��¼��־
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "newAlarmRecord success, alarmID = %d, alarmParam=%s",
                 alarmParam.iAlarmID, alarmParam.strAlarmParam.c_str());
        }
    }

    //��ȡtrap server��ַ��Ϣ
    vector<trap_server> vecServerInfo;
    iRet = CAlarmDB::GetAllTrapInfo(vecServerInfo);
    if (MP_SUCCESS != iRet)
    {
        //��¼��־
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get TRAP server info from database failed.");
        return iRet;
    }

    Pdu pdu;
    ConstructPDU(alarmInfo, pdu);
    SendTrap(pdu, vecServerInfo);

    //��¼��־
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Send alarm(ID = %d, Param= \"%s\") to server sucess.",
                 alarmInfo.iAlarmID, alarmInfo.strAlarmParam.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:SendAlarm
Description  :���ͻָ��澯
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CTrapSender::ResumeAlarm(alarm_param_t &stAlarm)
{
    LOGGUARD("");
    alarm_Info_t alarmInfo;
    alarmInfo.iAlarmID = stAlarm.iAlarmID;
    alarmInfo.strAlarmParam = stAlarm.strAlarmParam;
    //ͨ���澯������ѯdb���Ƿ����ͬ���澯
    mp_int32 iRet = CAlarmDB::GetAlarmInfoByParam(stAlarm.iAlarmID, stAlarm.strAlarmParam, alarmInfo);
    if (MP_SUCCESS != iRet)
    {
        //��¼��־
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetAlarmInfoByParam failed, alarmID = %d, alarmParam = %s",
             stAlarm.iAlarmID, stAlarm.strAlarmParam.c_str());
        return iRet;
    }

    //�ҵ��˸澯��¼��ֱ��ɾ��
    if (alarmInfo.iAlarmSN != -1)
    {
        iRet = CAlarmDB::DeleteAlarmInfo(alarmInfo.iAlarmSN, alarmInfo.iAlarmID);
        if (MP_SUCCESS != iRet)
        {
            //��¼��־
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Delete alarm failed, alarmID = %d, alarmSN = %d",
                 alarmInfo.iAlarmID, alarmInfo.iAlarmSN);
            return iRet;
        }
        else
        {
            //��¼��־
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Delete alarm success, alarmID = %d, alarmSN = %d",
                 alarmInfo.iAlarmID, alarmInfo.iAlarmSN);
        }
    }

    alarmInfo.iAlarmCategoryType = ALARM_CATEGORY_RESUME;
    //��ȡtrap server��ַ��Ϣ
    vector<trap_server> vecServerInfo;
    iRet = CAlarmDB::GetAllTrapInfo(vecServerInfo);
    if (MP_SUCCESS != iRet)
    {
        //��¼��־
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get TRAP server info from database failed.");
        return iRet;
    }

    Pdu pdu;
    ConstructPDU(alarmInfo, pdu);
    SendTrap(pdu, vecServerInfo);

    //��¼��־
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Resume alarm(ID = %d, Param = \"%s\") to server sucess.",
                 stAlarm.iAlarmID, stAlarm.strAlarmParam.c_str());
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : ���͵���Trap��Trap server
Input        : 
Output       : 
Return       :  
               
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_void CTrapSender::SendSingleTrap(Pdu &pdu, trap_server& trapServer, mp_int32 securityModel, OctetStr& securityName)
{
	//CodeDex��,KLOCWORK.RH.LEAK
    UdpAddress address(trapServer.strServerIP.c_str() );
    address.set_port((unsigned short)trapServer.iPort);
    Snmp *snmp;
    mp_int32 status = 0;

    try
    {
        if (address.get_ip_version() == Address::version_ipv4)
        {
            if (trapServer.strListenIP.compare(UNKNOWN) == 0)
            {
                snmp = new Snmp(status, "0.0.0.0");
            }
            else   //�ų���ȡAgent����IPʧ�ܵ����
            {
                snmp = new Snmp(status, trapServer.strListenIP.c_str());
            }
        }
        else
        {
            snmp = new Snmp(status, "::");
        }
    }
    catch(...)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New Snmp failed.");
        return;
    }

    if (NULL == snmp)
    {
        return;
    }

    if (SNMP_CLASS_SUCCESS != status)
    {
        delete(snmp);
        return;
    }

    UTarget utarget(address);
    utarget.set_version((snmp_version)trapServer.iVersion);    //Ŀǰֻʵ��SNMPV3����д��
    utarget.set_security_model(securityModel);
    utarget.set_security_name(securityName);

    COMMLOG(OS_LOG_INFO,LOG_COMMON_INFO,"Send Trap to:%s with %s.", trapServer.strServerIP.c_str(), trapServer.strListenIP.c_str());
    status = snmp->trap(pdu,utarget);
    delete(snmp);
}

/*------------------------------------------------------------
Function Name:SendTrap
Description  :����trap��trap server
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CTrapSender::SendTrap(Pdu &pdu, vector<trap_server> &vecServerInfo)
{
    ////CodeDex��,KLOCWORK.RH.LEAK
    LOGGUARD("");
    Snmp::socket_startup();
    snmp_v3_param stSnmpV3Param;
    CAlarmConfig::GetSnmpV3Param(stSnmpV3Param);

    //V3����
    OctetStr privPassword(stSnmpV3Param.strPrivPassword.c_str());
    OctetStr authPassword(stSnmpV3Param.strAuthPassword.c_str());
    OctetStr securityName(stSnmpV3Param.strSecurityName.c_str());
    mp_int32 securityModel = stSnmpV3Param.iSecurityModel;
    mp_int64 authProtocol = stSnmpV3Param.iAuthProtocol;
    mp_int64 privProtocol = stSnmpV3Param.iPrivProtocol;
    v3MP *v3_MP;

    mp_string strTmpEngineId = GetLocalNodeCode();
    //���������д��Ϊ1�������ļ���������
    mp_uint32 snmpEngineBoots = 1;

    mp_int32 construct_status = 0;
    try
    {
        v3_MP = new v3MP(strTmpEngineId.c_str(), snmpEngineBoots, construct_status);
    }
    catch(...)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New v3MP failed.");
        v3_MP = NULL;
    }
    
    if (!v3_MP)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "v3MP is NULL.");
        return;
    }
    if (SNMPv3_MP_OK != construct_status)
    {
        delete(v3_MP);
        return;
    }

    //��v3_MP��ָ���жϲ���Ҫ��coverity�����ʾ�������룬���ζ�Ӧpclint�澯��
    USM *usm = v3_MP->get_usm();  //lint !e613
    usm->add_usm_user(securityName,
        (long)authProtocol, (long)privProtocol,
        authPassword, privPassword);

    vector<trap_server>::iterator it = vecServerInfo.begin();
    for (;vecServerInfo.end() != it;  ++it)
    {
        SendSingleTrap(pdu, *it, securityModel, securityName);
    }
    delete(v3_MP);
    Snmp::socket_cleanup();
}//lint !e429

/*------------------------------------------------------------
Function Name:ConstructPDUCommon
Description  :���ݸ澯��Ϣ����pdu���ݣ���ConstructPDU����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CTrapSender::ConstructPDUCommon(Pdu &pdu)
{
    Vb vb;
    vb.set_oid(OID_ISM_ALARM_REPORTING_NODECODE);
    vb.set_value(GetLocalNodeCode().c_str());
    pdu += vb;
    //vb.clear();
    //vb.set_oid(OID_ISM_ALARM_REPORTING_LOCATIONINFO);
    //vb.set_value("");
    //pdu += vb;
    //vb.clear();
    //vb.set_oid(OID_ISM_ALARM_REPORTING_RESTOREADVICE);
    //vb.set_value("");
    //pdu += vb;
    //vb.clear();
    //vb.set_oid(OID_ISM_ALARM_REPORTING_FAULTTITLE);
    //vb.set_value("");
    //pdu += vb;
    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_FAULTTYPE);
    vb.set_value(ALARM_TYPE_EQUPMENTFAULT);
    pdu += vb;
    //vb.clear();
    //vb.set_oid(OID_ISM_ALARM_REPORTING_FAULTLEVEL);
    //vb.set_value(ALARM_LEVEL_MAJOR);
    //pdu += vb;
    
    vb.clear();
    mp_time time;
    CMpTime::Now(&time);
    mp_string strNowTime = CMpTime::GetTimeString(&time);
    vb.set_oid(OID_ISM_ALARM_REPORTING_FAULTTIME);
    vb.set_value(strNowTime.c_str());
    pdu += vb;
}

/*------------------------------------------------------------
Function Name:ConstructPDU
Description  :���ݸ澯��Ϣ����pdu����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CTrapSender::ConstructPDU(alarm_Info_t &stAlarm, Pdu &pdu)
{
    LOGGUARD("");
    //��Ϊtrap�ϱ�ģ��OID
    Oid oid( OID_HUAWEI_TRAP_MODEL );
    pdu_security_info stPduSecurInfo;
    (mp_void)GetPduSecurInfo(stPduSecurInfo);

    int securityLevel = stPduSecurInfo.iSecurityLevel;
    OctetStr contextName(stPduSecurInfo.strContextName.c_str());
    OctetStr contextEngineID(stPduSecurInfo.strContextEngineID.c_str());
    Vb vb;
    
    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_ALARMID);
    vb.set_value(stAlarm.iAlarmID);
    pdu += vb;
    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_SERIALNO);
    vb.set_value(stAlarm.iAlarmSN);
    pdu += vb;
    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_ADDITIONINFO);
    vb.set_value(stAlarm.strAlarmParam.c_str());
    pdu += vb;
    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_FAULTCATEGORY);
    vb.set_value(stAlarm.iAlarmCategoryType);
    pdu += vb;
    //vb.clear();
    //vb.set_oid(OID_ISM_ALARM_REPORTING_LOCATIONID);
    //vb.set_value("");
    //pdu += vb;
    ConstructPDUCommon(pdu);
    pdu.set_notify_id(oid);
    pdu.set_notify_enterprise(oid);
    pdu.set_security_level(securityLevel);
    pdu.set_context_name (contextName);
    pdu.set_context_engine_id(contextEngineID);
    return MP_TRUE;
}

/*------------------------------------------------------------
Function Name:ConstructPDU
Description  :�������ļ��ж�ȡsnmp��ذ�ȫ����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CTrapSender::GetPduSecurInfo(pdu_security_info &stPduSecurInfo)
{
    LOGGUARD("");
    //�������ļ��л�ȡ��ȫ����
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SNMP_SECTION, CFG_SECURITY_LEVEL, stPduSecurInfo.iSecurityLevel);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SNMP:security_level value from xml config failed");
        stPduSecurInfo.iSecurityLevel = 3;
    }
    //�������ļ��л�ȡcontext name
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, CFG_CONTEXT_NAME, stPduSecurInfo.strContextName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SNMP:context_name value from xml config failed");
        stPduSecurInfo.strContextName = "";
    }
    //�������ļ��л�ȡcontext engine id
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, CFG_ENGINE_ID, stPduSecurInfo.strContextEngineID);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SNMP:engine_id value from xml config failed");
        stPduSecurInfo.strContextEngineID = "";
    }

    //COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "TrapSender::GetPduSecurInfo Get secritylevel is %d, contextname is %s,contextengineid is %s",
        //stPduSecurInfo.iSecurityLevel, stPduSecurInfo.strContextName.c_str(), stPduSecurInfo.strContextEngineID.c_str());
    return MP_TRUE;
}

/*------------------------------------------------------------
Function Name:GetLocalNodeCode
Description  :��ȡ������mac��ַ
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_string CTrapSender::GetLocalNodeCode()
{
    LOGGUARD("");
    vector<mp_string> vecMacs; 
    mp_string strHostsnFile = CPath::GetInstance().GetConfFilePath(HOSTSN_FILE);

    mp_int32 iRet = CMpFile::ReadFile(strHostsnFile, vecMacs);
    if (MP_SUCCESS != iRet || 0 == vecMacs.size())
    {
         COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get host sn failed, iRet = %d, size of vecMacs is %d", iRet, vecMacs.size());
         return "";
    }
    return vecMacs.front();
}

/*------------------------------------------------------------
Function Name:NewAlarmRecord
Description  :�²���һ���澯��¼�������뵽sqlite���ݿ�
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CTrapSender::NewAlarmRecord(alarm_param_t &alarmParam, alarm_Info_t &alarmInfo)
{
    //��ȡ�µ�sn
    LOGGUARD("");
    mp_int32 iCurrSN = 0;
    mp_int32 iRet = CAlarmDB::GetSN(iCurrSN);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get CurSN Failed iRet is: %d", iRet);
        return iRet;
    }

    alarmInfo.iAlarmSN = (iCurrSN == MAX_ALARM_ID) ? 0 : iCurrSN + 1; //����ˮ��
    alarmInfo.iAlarmID = alarmParam.iAlarmID;
    //alarmInfo.iAlarmLevel = ALARM_LEVEL_MAJOR;
    alarmInfo.strAlarmParam = alarmParam.strAlarmParam;
    alarmInfo.iAlarmType = ALARM_TYPE_EQUPMENTFAULT;
    alarmInfo.strEndTime = "";
    mp_time time;
    CMpTime::Now(&time);
    mp_string strNowTime = CMpTime::GetTimeString(&time);
    alarmInfo.strStartTime = strNowTime;
    alarmInfo.iAlarmCategoryType = ALARM_CATEGORY_FAULT;
    iRet = CAlarmDB::InsertAlarmInfo(alarmInfo);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "InsertAlarmInfo failed");
        return iRet;
    }

    //������ˮ��
    iRet = CAlarmDB::SetSN(alarmInfo.iAlarmSN);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "SetSN Failed iRet is: %d",iRet);
        return iRet;
    }

    return MP_SUCCESS;
}


