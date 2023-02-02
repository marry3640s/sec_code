#include "pch.h"
#include "stdio.h"
#include "windows.h"
#include "biglog.h"

void gamelog::GetAppPath(char *pPath)
{
	DWORD dwLength = GetModuleFileNameA(GetModuleHandle(NULL), pPath,MAX_PATH);
	char *p = strrchr(pPath, '\\');
	if (p != NULL)	
		*p = '\0';
}

void gamelog::WriteLog(char *pszFileName, char *pszBuffer, int nFlag)
{
	HANDLE hLog = OpenLogFile(pszFileName, nFlag);
	WriteLogWithHandle(hLog, pszBuffer);
	CloseHandle(hLog);
}

void gamelog::WriteGameLog(char *pszFileName, char *pszBuffer, int nFlag)
{
	HANDLE hLog = OpenAppPathLogFile(pszFileName, 1);
	WriteLogWithDateTime(hLog, pszBuffer);
	CloseHandle(hLog);
}

HANDLE gamelog::OpenAppPathLogFile(char *pszFileName, int nFlag)
{
	char pszName[MAX_PATH];
	char pszAppPath[MAX_PATH];
	gamelog::GetAppPath(pszAppPath);
	sprintf_s(pszName, MAX_PATH, "%s\\%s", pszAppPath, pszFileName);
	return OpenLogFile(pszName, nFlag);
}

// nFlag = 1 追加的方式打开，0表示重新写入的方式打开,nFlag =2:从零读取的方式打开
HANDLE gamelog::OpenLogFile(char *pszFileName, int nFlag)
{
	DWORD dFileFlag;
	if (nFlag == 1 || nFlag==2)
		dFileFlag = OPEN_ALWAYS;
	else
		dFileFlag = CREATE_ALWAYS;
	int nFileSize;
	HANDLE hLogFile = 0;
	hLogFile = CreateFile(pszFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, dFileFlag, FILE_ATTRIBUTE_NORMAL, 0);
	if (hLogFile == INVALID_HANDLE_VALUE)
	{
		hLogFile = 0;
		return 0;
	}
	if (nFlag == 1)
	{
		nFileSize = GetFileSize(hLogFile, 0);
		SetFilePointer(hLogFile, nFileSize, 0, FILE_BEGIN);
	}
	return hLogFile;
}


void gamelog::WriteLogWithDateTime(HANDLE hFile, char *pszBuffer)
{
	DWORD dwWrite;
	

	if (hFile == 0 || pszBuffer == 0)
		return;
	if (strlen(pszBuffer)>2000)
		return;
	char pszWriteBuf[2048];
	char szTime[16];
	char szDate[16];
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	GetTimeFormatA(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &sysTime, "HH:mm:ss", szTime, 16);
	GetDateFormatA(LOCALE_USER_DEFAULT, LOCALE_USE_CP_ACP, &sysTime, "yyyy-MM-dd", szDate, 16);
	sprintf_s(pszWriteBuf, 2048, "[%s %s]--%s", szDate, szTime, pszBuffer);
	WriteFile(hFile, pszWriteBuf, strlen(pszWriteBuf), &dwWrite, 0);
}


////写入信息
void gamelog::WriteLogWithHandle(HANDLE hFile, char *pszBuffer)
{
	DWORD dwWrite;
	if (hFile == 0 || pszBuffer == 0)
		return;
	if (strlen(pszBuffer)>2048)
		return;
	WriteFile(hFile, pszBuffer, strlen(pszBuffer), &dwWrite, 0);
}
