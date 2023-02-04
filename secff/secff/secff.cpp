// secff.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>

#include "windows.h"
#include<string>
#include<vector>
#include "json/json.h"
#include <fstream>
using namespace std;
#pragma warning(disable:4996)
HANDLE hShFile;
#pragma comment (lib, "User32.lib")
//vector<string> CompList;
std::map<std::string,long long> maplist;
void ReadJsonData(char *pFileName, char *pName)
{
	char pszWirte[256];
	Json::Reader reader;
	Json::Value root;
	ifstream in(pFileName, ios::binary);
	long long llShares = 0;
	long long llTmp;
	long long llEquity = 0;
	Json::Value va;
	
	if (!in.is_open())
	{
		printf("open error file%s\n", pFileName);
		return;
	}
	if (!reader.parse(in, root))
		goto t1;
	if (!root.isMember("EntityCommonStockSharesOutstanding"))
		goto t1;
	if (!root.isMember("StockholdersEquity"))
		goto t1;
	if (!root.isMember("TradingSymbol"))
		goto t1;
	if (!root.isMember("EntityRegistrantName"))
		goto t1;
//	printf("%s\n", root.toStyledString().data());
	va = root["EntityCommonStockSharesOutstanding"];
	for (int k = 0; k < va.size(); k++)
	{
		llTmp= atoll(va[k]["value"].asCString());
		llShares += llTmp;
	}
	printf("%s=%lld\n", pFileName, llShares);
	//printf("%lld\n", llShares);

	llEquity= atoll(root["StockholdersEquity"][0]["value"].asCString());

	sprintf_s(pszWirte, 256, "%s|%s|%lld|%lld\n", root["TradingSymbol"].asCString(), root["EntityRegistrantName"].asCString(), llShares, llEquity);
	//sprintf_s(pszWirte, 256, "%s\n", pszSymbloName);
	int k;
	/*for (k = 0; k < CompList.size(); k++)
	{
		if (CompList[k] == root["EntityRegistrantName"].asCString())
			break;
	}*/
	if (maplist.find(root["EntityRegistrantName"].asCString()) == maplist.end())
	{
		if (llShares > 3000000)
		{
			DWORD dwWrite;
			WriteFile(hShFile, pszWirte, strlen(pszWirte), &dwWrite, 0);
			maplist.insert(std::pair<std::string, long long>(root["EntityRegistrantName"].asCString(), llShares));
		}
	}
	/*if (k == CompList.size() && llShares>3000000)
	{
		DWORD dwWrite;
		WriteFile(hShFile, pszWirte, strlen(pszWirte), &dwWrite, 0);
		CompList.push_back(root["EntityRegistrantName"].asCString());
	}*/
	//root["EntityCommonStockSharesOutstanding"].size()
t1:
	in.close();


}

DWORD ListAllFileInDirectory(LPSTR szPath)
{
	char szFilePath[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	HANDLE hListFile;
	char szFullPath[MAX_PATH];

	// 构造代表子目录和文件夹路径的字符串，使用通配符“*”
	lstrcpy(szFilePath, szPath);
	lstrcat(szFilePath, "\\*");

	// 查找第一个文件目录，获得查找句柄
	hListFile = FindFirstFile(szFilePath, &FindFileData);
	if (hListFile == INVALID_HANDLE_VALUE)
	{
		printf("错误： %d", GetLastError());
		return 1;
	}
	else
	{
		do {
			// 过滤"."和".."，不需要遍历
			if (lstrcmp(FindFileData.cFileName, TEXT(".")) == 0 ||
				lstrcmp(FindFileData.cFileName, TEXT("..")) == 0)
			{
				continue;
			}

			// 构造成全路径
			//wsprintf(szFullPath, "%s\\%s", szPath, FindFileData.cFileName);
			lstrcpy(szFullPath, szPath);
			lstrcat(szFullPath, "\\");
			lstrcat(szFullPath, FindFileData.cFileName);

			//dwTotalFileNum++;

			// 打印
		//	printf("\n%s\t", szFullPath);

			//AddTreeItem()
			// 如果是目录，则递归调用，列举下级目录
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				//	printf("<DIR>");

				ListAllFileInDirectory(szFullPath);
			}
			else
			{
				//printf("%s\n", szFullPath);

				ReadJsonData(szFullPath, FindFileData.cFileName);

			}
		} while (FindNextFile(hListFile, &FindFileData));
	}
	return 0;
}
int main()
{

	
	//ReadSecData((char *)"C:\\sec10q\\roku.txt", (char *)"roku.txt");
	hShFile = CreateFile("C:\\bighouse\\SharesA.txt", GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hShFile == INVALID_HANDLE_VALUE)
		return 0;
//	ListAllFileInDirectory((char *)"C:\\sec20f\\");
//	ListAllFileInDirectory((char *)"C:\\sec10q\\");
	ListAllFileInDirectory((char *)"C:\\bighouse\\sec10json\\");
	//return 0;
	/*char pszFileName[] = "C:\\bighouse\\sec10json\\ROKU, INC.json";
	ReadJsonData(pszFileName, (char *)"");*/
    std::cout << "Hello World!\n"; 
}

