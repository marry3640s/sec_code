// ReportsFinStatements.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "windows.h"
#include <set>
#include<string>
#include<vector>
#include <sstream>
//#include "json/json.h"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"
using namespace std;
#include <fstream>
#pragma warning(disable:4996)

#pragma comment (lib, "User32.lib")

struct StockInfo {
	double pe;
	double nprice;
	long long llshares;
	char pszName[128];
};
std::vector<StockInfo> infolist;
StockInfo GetSotckInfo(char *szPath)
{
	StockInfo info;
	memset(&info, 0x00, sizeof(info));
	rapidxml::file<>xmlFile(szPath);
	rapidxml::xml_document<>doc;
	doc.parse<0>(xmlFile.data());
	rapidxml::xml_node<> *root = doc.first_node("ReportSnapshot");
	rapidxml::xml_node<> *cogen = root->first_node("CoGeneralInfo");
	rapidxml::xml_node<> * shares = cogen->first_node("SharesOut");
	/*std::cout << shares->value() << std::endl;
	std::cout << shares->first_attribute("Date")->value() << std::endl;
	std::cout << shares->first_attribute("TotalFloat")->value() << std::endl;*/
	info.llshares = atoll(shares->value());

	rapidxml::xml_node<> *ratio = root->first_node("Ratios");

	for (rapidxml::xml_node<> * group = ratio->first_node("Group"); group; group = group->next_sibling())
	{
		if (memcmp(group->first_attribute("ID")->value(), "Price and Volume", strlen("Price and Volume")) == 0)
		{
			for (rapidxml::xml_node<> * pp = group->first_node("Ratio"); pp; pp = pp->next_sibling())
			{
				if (memcmp(pp->first_attribute("FieldName")->value(), "NPRICE", strlen("NPRICE")) == 0)
				{
					info.nprice=atof(pp->value());
				}
			}
		}
		if (memcmp(group->first_attribute("ID")->value(), "Other Ratios", strlen("Other Ratios")) == 0)
		{
			for (rapidxml::xml_node<> * pp = group->first_node("Ratio"); pp; pp = pp->next_sibling())
			{
				if (memcmp(pp->first_attribute("FieldName")->value(), "PEEXCLXOR", strlen("PEEXCLXOR")) == 0)
				{
					/*std::cout << pp->value() << std::endl;
					break;*/
					info.pe= atof(pp->value());
					return info;
				}
			}
		}
	}
	return info;
	//return "0";
}
int nTick = 0;
double fAllpe = 0;
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
				StockInfo  info= GetSotckInfo(szFullPath);
				memset(info.pszName, 0x00, sizeof(info.pszName));
				memcpy(info.pszName, FindFileData.cFileName, strlen(FindFileData.cFileName) - 4);
				printf("%s=%f\n", info.pszName, info.pe);
				if (info.pe > 0)
				{
					nTick++;
					fAllpe += info.pe;
					infolist.push_back(info);
				}
			//	char pszSy[MAX_PATH] = "";
			
				
				
			}
		} while (FindNextFile(hListFile, &FindFileData));
	}
	return 0;
}
int main()
{
	ListAllFileInDirectory((char *)"C:\\bighouse\\财务数据\\快照\\20230723\\");
	printf("纳斯达克100指数 总盈利数=%d,平均市盈率=%f\n", nTick,fAllpe / (double)nTick);
	double fAllVaule=0;
	for (int k = 0; k < infolist.size(); k++)
	{
		fAllVaule += infolist[k].llshares*infolist[k].nprice;
	}
	double llRatio = 0;
	for (int k = 0; k < infolist.size(); k++)
	{
		printf("%s=%f\n", infolist[k].pszName,(infolist[k].llshares*infolist[k].nprice) / fAllVaule * infolist[k].pe);
		llRatio += (infolist[k].llshares*infolist[k].nprice) / fAllVaule * infolist[k].pe;
	}
	printf("纳斯达克100指数 总盈利数=%d,加权平均市盈率=%f\n", nTick, llRatio);
	return 1;
	
	
}