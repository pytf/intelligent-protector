/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/IpTest.h"
#include "common/ErrorCode.h"
#include "common/Ip.h"
/*------------------------------------------------------------ 
 Description :����CIPCheck::IsIPV4(mp_string&)
 Input         :   
 Output       :    
 Return       : 
 Create By    :
Modification : 
-------------------------------------------------------------*/

TEST_F(IPTest, IsIPV4TEST){
  mp_string strIpAddr = "";
  mp_bool ret = true;

  //���ַ���
  ret = CIPCheck::IsIPV4(strIpAddr);
  EXPECT_EQ(MP_FALSE, ret);

  //ȫ���ַ���
  strIpAddr = "0.0.0.0";
  ret = CIPCheck::IsIPV4(strIpAddr);
  EXPECT_EQ(MP_FALSE, ret);

  //ĳ���γ���255
  strIpAddr = "127.0.3.256";
  ret = CIPCheck::IsIPV4(strIpAddr);
  EXPECT_EQ(MP_FALSE, ret);

  //���������ַ�
  strIpAddr = "1o9.25.31.4l";
  ret = CIPCheck::IsIPV4(strIpAddr);
  EXPECT_EQ(MP_FALSE, ret);

  //��ipv4���͸�ʽ
  strIpAddr = "10:2:3:5";
  ret = CIPCheck::IsIPV4(strIpAddr);
  EXPECT_EQ(MP_FALSE, ret);
  
  //�����ַ���
  strIpAddr = "10.2.3.5";
  ret = CIPCheck::IsIPV4(strIpAddr);
  EXPECT_EQ(MP_TRUE, ret);
}
