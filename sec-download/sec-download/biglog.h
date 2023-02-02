#pragma once
namespace gamelog {
	extern HANDLE OpenLogFile(char *pszFileName, int nFlag);// nFlag =2:从零读取的方式打开, 1 追加的方式打开，0表示重新写入的方式打开
	extern HANDLE OpenAppPathLogFile(char *pszFileName, int nFlag);
	extern void   WriteLogWithHandle(HANDLE hFile, char *pszBuffer);
	extern void   GetAppPath(char *pPath);
	extern void   WriteLogWithDateTime(HANDLE hFile, char *pszBuffer);
	extern void   WriteGameLog(char *pszFileName, char *pszBuffer, int nFlag);
	extern void   WriteLog(char *pszFileName, char *pszBuffer, int nFlag=1);
};

#define ADD_FLAG 1