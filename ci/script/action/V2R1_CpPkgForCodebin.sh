#!/bin/sh
OSTYPE=`uname`
if [ "${OSTYPE}" = "SunOS" ]
then
    export ICP_ROOT=/export/home/icp/Agent
else
    export ICP_ROOT=/home/icp/Agent
fi

export PKG_PATH=${ICP_ROOT}/RD_V200R001C00_Agent_Codebin/code/current/Package

[ ! -d "${PKG_PATH}" ] && mkdir "${PKG_PATH}"

sudo umount /mnt
sudo mount -t cifs -o username=Administrator,password=Huawei@123 //10.183.192.206/RDV2R1C00_AgentPackage_Codebin /mnt
if [ $? -ne 0 ]
then
    echo "ERROR:mount //10.183.192.206/RDV2R1C00_AgentPackage_Codebin to /mnt failed!"
    exit 1
fi

PKG_VERSION=`cat /home/icp/Agent/V2R1C10VersionNo.txt`

rm ${PKG_PATH}/*
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-AIX53-ppc_64.tar.gz"  ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-AIX-ppc_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-HP-UX_11.23-ia_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-HP-UX_11.31-ia_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-iSoft3-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-OL5-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-OL6-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-RedHat5-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-RedHat6-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-RedHat7-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-Rocky4-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-Rocky6-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-Solaris-sparc_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-SuSE10-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-SuSE11-x86_64.tar.gz" ${PKG_PATH}/
sudo cp "/mnt/pkg/OceanStor BCManager ${PKG_VERSION}_Agent-WIN64.zip" ${PKG_PATH}/

cd ${PKG_PATH}
rm "${ICP_ROOT}/RD_V200R001C00_Agent_Codebin/code/current/${PKG_VERSION}_eReplication_Agent.zip"
zip "OceanStor BCManager ${PKG_VERSION}_eReplication_Agent.zip" *.tar.gz *.zip

# copy to release pkg
cp "OceanStor BCManager ${PKG_VERSION}_eReplication_Agent.zip" /mnt/
# copy to from upload vmp
mv "OceanStor BCManager ${PKG_VERSION}_eReplication_Agent.zip" ../

sudo umount /mnt
