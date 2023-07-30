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

//struct StockInfo {
//	double pe;
//	double nprice;
//	long long llshares;
//	char pszName[128];
//};

//struct ReportInfo {
//	long long llCommonShares;//总股本
//	double fEquity;//总权益
//};



struct FinInfo
{
	double fCommonShares;//总股本
	double fEquity;//总权益
	double fCash;//现金
	double fCashEqu;//现金等价物
	double fCashShortInv;//现金和短期投资
	double fAccReceivable;//应收账款
	double fTotalReceivables;//总应收账款
	double fTotalCurrentAssets;//流动资产合计
	double fLongTermInvestments;//长期投资

	double fTotalCurrentLiabilities;//流动负债总计
	double fTotalLiabilities;//总负债
};



struct FiscalPeriodInfo
{
	std::string pszStatementDate;  //财报截止日期
	std::string pszSourceData;     //披露日期
	std::string pszReportType;     //报告类型,如10k,10Q
	std::string pszStatementType;  //陈述类型

	FinInfo fin;//财务信息
	/*union
	{
		IncInfo incinfo;
		BalInfo bainfo;
		CasInfo casinfo;
	}sc;*/
};

struct StockInfo
{
	std::string name;
	std::vector<FiscalPeriodInfo> Finlist;

};
HANDLE hLogFile = 0;
std::vector<StockInfo> infoList;

void GetReportInfo(char *szPath)
{

	StockInfo bb;
	struct FiscalPeriodInfo info;
	rapidxml::file<>xmlFile(szPath);
	rapidxml::xml_document<>doc;
	doc.parse<0>(xmlFile.data());
	rapidxml::xml_node<> *root = doc.first_node("ReportFinancialStatements");
	rapidxml::xml_node<> *fin = root->first_node("FinancialStatements");
	rapidxml::xml_node<> *ann = fin->first_node("AnnualPeriods");
	if (ann == NULL)
		return;

	for (rapidxml::xml_node<> * pp = root->first_node("Issues")->first_node("Issue")->first_node("IssueID"); pp; pp = pp->next_sibling())
	{
		if (memcmp(pp->first_attribute("Type")->value(), "Ticker", strlen("Ticker")) == 0)
		{
			bb.name = pp->value();
			break;
		}
	}
	//root->first_node("Issues")->first_node("IssueID");
	for (rapidxml::xml_node<> * fiscal = ann->first_node("FiscalPeriod"); fiscal; fiscal = fiscal->next_sibling())
	{
		memset(&info.fin, 0x00, sizeof(info.fin));
		for (rapidxml::xml_node<> * pp = fiscal->first_node("Statement"); pp; pp = pp->next_sibling())
		{
			rapidxml::xml_node<> *head= pp->first_node("FPHeader");
			info.pszStatementDate = head->first_node("StatementDate")->value();
			info.pszReportType= head->first_node("Source")->value();
			if (info.pszReportType == "PROSPECTUS" || info.pszReportType =="ARS")//招股说明书
			{
				goto nt;
			}
		//	info.pszSourceData = head->first_node("Source")->first_attribute("Date")->value();
			if (memcmp(pp->first_attribute("Type")->value(), "INC", strlen("INC")) == 0)
			{
				info.pszStatementType = "INC";
			}
			else if (memcmp(pp->first_attribute("Type")->value(), "BAL", strlen("BAL")) == 0)
			{
				info.pszStatementType = "BAL";
				for (rapidxml::xml_node<> * item = pp->first_node("lineItem"); item; item = item->next_sibling())
				{
					if (memcmp(item->first_attribute("coaCode")->value(), "QTCO", strlen("QTCO")) == 0)
					{
						info.fin.fCommonShares = atof(item->value());
					}
					if (memcmp(item->first_attribute("coaCode")->value(), "QTLE", strlen("QTLE")) == 0)
					{
						info.fin.fEquity = atof(item->value());
					}
					if (memcmp(item->first_attribute("coaCode")->value(), "ACSH", strlen("ACSH")) == 0)
					{
						info.fin.fCash = atof(item->value());
					}
					if (memcmp(item->first_attribute("coaCode")->value(), "ACAE", strlen("ACAE")) == 0)
					{
						info.fin.fCashEqu = atof(item->value());
					}
					if (memcmp(item->first_attribute("coaCode")->value(), "SCSI", strlen("SCSI")) == 0)
					{
						info.fin.fCashShortInv = atof(item->value());
					}
					if (memcmp(item->first_attribute("coaCode")->value(), "AACR", strlen("AACR")) == 0)
					{
						info.fin.fAccReceivable = atof(item->value());
					}
					if (memcmp(item->first_attribute("coaCode")->value(), "ATRC", strlen("ATRC")) == 0)
					{
						info.fin.fTotalReceivables = atof(item->value());
					}
					if (memcmp(item->first_attribute("coaCode")->value(), "ATCA", strlen("ATCA")) == 0)
					{
						info.fin.fTotalCurrentAssets = atof(item->value());
					}

					if (memcmp(item->first_attribute("coaCode")->value(), "SINV", strlen("SINV")) == 0)
					{
						info.fin.fLongTermInvestments = atof(item->value());
					}

					if (memcmp(item->first_attribute("coaCode")->value(), "LTCL", strlen("LTCL")) == 0)
					{
						info.fin.fTotalCurrentLiabilities = atof(item->value());
					}

					if (memcmp(item->first_attribute("coaCode")->value(), "LTLL", strlen("LTLL")) == 0)
					{
						info.fin.fTotalLiabilities = atof(item->value());
					}
				}
			}
			else if (memcmp(pp->first_attribute("Type")->value(), "CAS", strlen("CAS")) == 0)
			{
				info.pszStatementType = "CAS";
			}
		}

		bb.Finlist.push_back(info);
	}
nt:
	infoList.push_back(bb);
	//公司上市小于3年不统计
	if (bb.Finlist.size() <= 3 )
		return;

	double fInitShares = 0;
	double fSharesRatioList[5] = { 1,1,1,1,1 };
	
	for (int k = 0; k < bb.Finlist.size(); k++)
	{
		if (k == 0)
		{
			fInitShares = bb.Finlist[0].fin.fCommonShares;
			if (fInitShares <= 0)
				return;
		}
		if (bb.Finlist[k].fin.fCommonShares <= 0)
			break;
		else if (k >= 1)
		{
			fSharesRatioList[k - 1] =  fInitShares/ bb.Finlist[k].fin.fCommonShares;
			for (int j = k; j < 5; j++)
				fSharesRatioList[j] = fSharesRatioList[k - 1];
		}
	}

	char pszWrite[1024];
	DWORD dwWrite;
	sprintf_s(pszWrite, 1024, "%s,%0.3f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f\n", bb.name.data(), bb.Finlist[0].fin.fCommonShares, fSharesRatioList[0], fSharesRatioList[1], fSharesRatioList[2], fSharesRatioList[3]
	, fSharesRatioList[4]);
	WriteFile(hLogFile, pszWrite, strlen(pszWrite), &dwWrite, 0);
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
				GetReportInfo(szFullPath);
				printf("%s\n", FindFileData.cFileName);
			//	StockInfo  info= GetSotckInfo(szFullPath);
			//	memset(info.pszName, 0x00, sizeof(info.pszName));
			//	memcpy(info.pszName, FindFileData.cFileName, strlen(FindFileData.cFileName) - 4);
				/*printf("%s=%f\n", info.pszName, info.pe);
				if (info.pe > 0)
				{
					nTick++;
					fAllpe += info.pe;
					infolist.push_back(info);
				}*/
			//	char pszSy[MAX_PATH] = "";
			
				
				
			}
		} while (FindNextFile(hListFile, &FindFileData));
	}
	return 0;
}
int main()
{
	
	hLogFile = CreateFile("c:\\bighouse\\shares.txt", GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	//GetReportInfo((char *)"C:\\bighouse\\美股财务数据\\ReportsFinStatements\\NASDAQ\\BWAC.txt");
	//GetReportInfo((char *)"C:\\bighouse\\美股财务数据\\ReportsFinStatements\\NASDAQ\\AAPL.txt");
	ListAllFileInDirectory((char *)"C:\\bighouse\\美股财务数据\\ReportsFinStatements\\");
	//ListAllFileInDirectory((char *)"C:\\bighouse\\美股财务数据\\ReportsFinStatements\\NASDAQ\\");
	/*ListAllFileInDirectory((char *)"C:\\bighouse\\财务数据\\快照\\20230723\\");
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
	printf("纳斯达克100指数 总盈利数=%d,加权平均市盈率=%f\n", nTick, llRatio);*/
	return 1;
	
	
}