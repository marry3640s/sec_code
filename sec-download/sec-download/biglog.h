#pragma once
namespace gamelog {
	extern HANDLE OpenLogFile(char *pszFileName, int nFlag);// nFlag =2:�����ȡ�ķ�ʽ��, 1 ׷�ӵķ�ʽ�򿪣�0��ʾ����д��ķ�ʽ��
	extern HANDLE OpenAppPathLogFile(char *pszFileName, int nFlag);
	extern void   WriteLogWithHandle(HANDLE hFile, char *pszBuffer);
	extern void   GetAppPath(char *pPath);
	extern void   WriteLogWithDateTime(HANDLE hFile, char *pszBuffer);
	extern void   WriteGameLog(char *pszFileName, char *pszBuffer, int nFlag);
	extern void   WriteLog(char *pszFileName, char *pszBuffer, int nFlag=1);
};

#define ADD_FLAG 1