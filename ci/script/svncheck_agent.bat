echo off
set currentpath=%cd%
set repopath_source="D:\pom\ReplicationDirector_V1R3C00_Agent\code\current\Agent"

D:
rem cd %repopath%

set yy=%date:~0,4%&set month=%date:~5,2%&set day=%date:~8,2%
set hh=%time:~0,2%&set mm=%time:~3,2%
if /i %hh% LSS 10 (set hh=0%time:~1,1%)
rem ���Сʱ��С��10������ֿո����г�����һ�в�0

svn status --depth infinity %repopath_source% > %currentpath%\Agent_%yy%_%month%_%day%_%hh%_%mm%_������.txt
svn info %repopath_source% > %currentpath%\Agent_%yy%_%month%_%day%_%hh%_%mm%_����״̬.txt

pause