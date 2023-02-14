// secff.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>

#include "windows.h"
#include<string>
#include<vector>

#include <map>

struct gainfo{
	char pszTicker[64];
	char pszCompanyName[256];
	double fPrice;
	long long llShares;//总股本
	long long llEquity;
	long long llAssets;
	long long llMarketCap;
	long long llLabs;
	double rate;
	double rateAssets;
};

struct tickerCompanyT {
	char pszTicker[64];
	char pszCompanyName[256];
};

std::map<std::string,struct gainfo> maplist;

std::map<std::string, struct tickerCompanyT> Companylist;

int main()
{

	FILE *fp;
	int nRet = fopen_s(&fp, "C:\\bighouse\\symPrice.txt", "r");
	if (nRet != NULL)
		return 0;
	char pszTemp[1024] = "";
	gainfo info;
	for (;;)
	{

		if (fgets(pszTemp, 1024, fp) == NULL)
			break;
		pszTemp[strlen(pszTemp) - 1] = 0x00;
		//printf("%s\n", pszTemp);

		memset(&info, 0x00, sizeof(gainfo));
		char *end=strchr(pszTemp, ',');
		memcpy(info.pszTicker, pszTemp, end - pszTemp);
		info.fPrice = atof(end + 1);
		maplist.insert(std::pair<std::string, struct gainfo>(info.pszTicker, info));
	}
	fclose(fp);


	
	// nRet = fopen_s(&fp, "C:\\bighouse\\ticker.txt", "r");
	//if (nRet != NULL)
	//	return 0;
	////char pszTemp[1024] = "";
	//tickerCompanyT tt;
	//for (;;)
	//{

	//	if (fgets(pszTemp, 1024, fp) == NULL)
	//		break;
	//	pszTemp[strlen(pszTemp) - 1] = 0x00;
	//	//printf("%s\n", pszTemp);

	//	memset(&tt, 0x00, sizeof(tickerCompanyT));
	//	char *end = strchr(pszTemp, '|');
	//	memcpy(tt.pszTicker, pszTemp, end - pszTemp);
	//	memcpy(tt.pszCompanyName, end+1, pszTemp+strlen(pszTemp)-end);
	//	std::map<std::string, struct gainfo>::iterator it;
	//	it = maplist.find(tt.pszTicker);
	//	if (it != maplist.end())
	//	{
	//		memcpy(it->second.pszCompanyName, tt.pszCompanyName, strlen(tt.pszCompanyName));
	//	}
	//	//Companylist.insert(std::pair<std::string, struct tickerCompanyT>(tt.pszTicker, tt));
	//}
	//fclose(fp);


	nRet = fopen_s(&fp, "C:\\bighouse\\SharesA.txt", "r");
	if (nRet != NULL)
		return 0;
	
	for (;;)
	{
		memset(&info, 0x00, sizeof(gainfo));
		memset(pszTemp, 0x00, sizeof(pszTemp));
		if (fgets(pszTemp, 1024, fp) == NULL)
			break;
		pszTemp[strlen(pszTemp) - 1] = 0x00;
		printf("%s\n", pszTemp);

		char *end;
		char *pp = pszTemp;
		char pszBuf[256] = "";
		for (int m = 0; m < 6; m++)
		{
			memset(pszBuf, 0x00, sizeof(pszBuf));
			end= strchr(pp, '|');
			if (m == 0)
			{
				memcpy(info.pszTicker, pp, end - pp);
				if (memcmp(info.pszTicker, "INPX", 4) == 0)
				{
					int c;
					c = 5;
				}
			}
			if (m ==1)
				memcpy(info.pszCompanyName, pp, end - pp);
			if (m == 2)
			{
				memcpy(pszBuf, pp, end - pp);
				info.llShares = atoll(pszBuf);
			}
			if (m == 3)
			{
				memcpy(pszBuf, pp, end - pp);
				info.llEquity = atoll(pszBuf);
			}
			if (m == 4)
			{
				memcpy(pszBuf, pp, end - pp);
				info.llAssets = atoll(pszBuf);
			}
			if (m == 5)
			{
				info.llLabs = atoll(pp);
			}
			pp = pp + (end - pp) + 1;
		}
		std::map<std::string, struct gainfo>::iterator it;
		it = maplist.find(info.pszTicker);
		if (it != maplist.end())
		{
			it->second.llShares = info.llShares;
			it->second.llEquity = info.llEquity;
			it->second.llAssets = info.llAssets;
			it->second.llLabs = info.llLabs;
			long long llMarketCap = info.llShares* it->second.fPrice;
			it->second.llMarketCap = llMarketCap;

			memcpy(it->second.pszCompanyName, info.pszCompanyName, strlen(info.pszCompanyName));
			if (llMarketCap != 0 && info.llEquity > 0)
			{
				it->second.rate = (double)info.llEquity / (double)llMarketCap;
			}
			else
			{
				it->second.rate = 0;
			}
			long long lbb = info.llAssets - info.llLabs;
			if (llMarketCap != 0 && lbb > 0)
			{
				it->second.rateAssets = (double)lbb / (double)llMarketCap;
			}
			else
			{
				it->second.rateAssets = 0;
			}
		}

	}

	fclose(fp);

	std::map<std::string, struct gainfo>::iterator it;
	HANDLE hFile = CreateFile("C:\\bighouse\\now.txt", GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	for (it = maplist.begin(); it != maplist.end(); it++)
	{
		char pszWirte[1024];
		sprintf_s(pszWirte, 1024, "%s|%s|%lld|%lld|%lld|%g|%g|%lld|%lld|%g\n", it->second.pszTicker,it->second.pszCompanyName, it->second.llShares
			, it->second.llEquity,it->second.llMarketCap,it->second.fPrice,
			it->second.rate,it->second.llAssets,it->second.llLabs,it->second.rateAssets);
		//sprintf_s(pszWirte, 256, "%s\n", pszSymbloName);
		DWORD dwWrite;
		WriteFile(hFile, pszWirte, strlen(pszWirte), &dwWrite, 0);
	}
	CloseHandle(hFile);
	return 0;
	//ReadSecData((char *)"C:\\sec10q\\roku.txt", (char *)"roku.txt");
	/*hShFile = CreateFile("C:\\bighouse\\Shares.txt", GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hShFile == INVALID_HANDLE_VALUE)
		return 0;*/

	return 0;
	char pszFileName[] = "C:\\sec-test\\sec-edgar-filings\\TDOC\\10-Q\\0001558370-22-015977\\full-submission.txt";
	//ReadSecData(pszFileName);
	std::cout << "Hello World!\n";
}

