/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __HANATEST_H__
#define __HANATEST_H__

#define private public

#include "apps/hana/Hana.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/String.h"
#include "common/RootCaller.h"
#include "common/SystemExec.h"
#include "array/Array.h"
#include <sstream>
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(mp_int32, const mp_int32, mp_uint64, const mp_char*, const mp_char*, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class CHanaTest : public testing::Test
{
    public:
    static mp_void SetUpTestCase()
    {
        m_stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        delete m_stub;
    }
private:
    static Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* m_stub;
};
Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CHanaTest::m_stub;

//���庯������
/*���Ա����������������Class��+������+Type��׺
 *���Ա������Stub����������������Stubǰ׺+Class��+������+Type��׺
 *���Ա������Stub������������void* pthis + ԭ������������ôд����Ϊ�����ܻ��ԭ��������������������̬������ԭ��������һ�¡�
 *��̬������ȫ�ֺ������⺯��������������������+Type��׺��
 *��̬������ȫ�ֺ������⺯����stub����������������Stubǰ׺+������+Type��׺��
 *��̬������ȫ�ֺ������⺯����stub�����Ĳ�������ԭ��������һ�£���ôд����Ϊ�����ܻ��ԭ������������������
*/

#endif
