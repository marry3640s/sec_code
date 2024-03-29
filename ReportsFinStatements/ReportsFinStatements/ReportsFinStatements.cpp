﻿// ReportsFinStatements.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "windows.h"
#include <set>
#include<string>
#include<vector>
#include <map>
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
	double fValue; //市值
	double fNPrice;//现价
	std::vector<FiscalPeriodInfo> Finlist;

};
HANDLE hLogFile = 0;
//std::vector<StockInfo> infoList;
std::map<std::string, StockInfo> infoList;
void GetSotckInfo(char *szPath,char *pszSym)
{
	StockInfo info;
	
	rapidxml::file<>xmlFile(szPath);
	rapidxml::xml_document<>doc;
	doc.parse<0>(xmlFile.data());
	rapidxml::xml_node<> *root = doc.first_node("ReportSnapshot");
	rapidxml::xml_node<> *cogen = root->first_node("CoGeneralInfo");
	rapidxml::xml_node<> * shares = cogen->first_node("SharesOut");
	if (shares == NULL)
		return;
	double fdr = 1.0;
	rapidxml::xml_node<> *glo=root->first_node("Issues")->first_node("Issue")->first_node("GlobalListingType");
	if (glo != NULL)
	{
		rapidxml::xml_attribute<> *sharesPer = glo->first_attribute("SharesPerListing");
		if (sharesPer != NULL)
		{
			fdr = atof(sharesPer->value());
		}
		//fdr=atof(glo->first_attribute("SharesPerListing")->value());
	}

	double fExchangeRate = 1.0; //汇率
	/*rapidxml::xml_node<> *mos = root->first_node("Issues")->first_node("Issue")->first_node("MostRecentSplit");
	if (mos != NULL)
	{
		fExchangeRate = atof(mos->value());
	}*/
	info.name = pszSym;

	/*for (rapidxml::xml_node<> * pp = root->first_node("Issues")->first_node("Issue")->first_node("IssueID"); pp; pp = pp->next_sibling())
	{
		if (memcmp(pp->first_attribute("Type")->value(), "Ticker", strlen("Ticker")) == 0)
		{
			info.name = pp->value();
			break;
		}
	}*/
	/*std::cout << shares->value() << std::endl;
	std::cout << shares->first_attribute("Date")->value() << std::endl;
	std::cout << shares->first_attribute("TotalFloat")->value() << std::endl;*/
	long long llshares = atoll(shares->value());

	rapidxml::xml_node<> *ratio = root->first_node("Ratios");

	for (rapidxml::xml_node<> * group = ratio->first_node("Group"); group; group = group->next_sibling())
	{
		if (memcmp(group->first_attribute("ID")->value(), "Price and Volume", strlen("Price and Volume")) == 0)
		{
			for (rapidxml::xml_node<> * pp = group->first_node("Ratio"); pp; pp = pp->next_sibling())
			{
				if (memcmp(pp->first_attribute("FieldName")->value(), "NPRICE", strlen("NPRICE")) == 0)
				{
					info.fNPrice = atof(pp->value());
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
				/*	info.pe = atof(pp->value());
					return info;*/
				}
			}
		}
	}
	info.fValue = llshares/fdr * info.fNPrice/1000000/ fExchangeRate;
	infoList.insert(std::pair<std::string, StockInfo >(info.name, info));
	/*infoList.push_back(info);*/
	//return info;
	//return "0";
}

void GetReportInfo(char *szPath , char *pszSym)
{

	//StockInfo bb;
	std::string name;
	struct FiscalPeriodInfo info;
	rapidxml::file<>xmlFile(szPath);
	rapidxml::xml_document<>doc;
	doc.parse<0>(xmlFile.data());
	rapidxml::xml_node<> *root = doc.first_node("ReportFinancialStatements");
	rapidxml::xml_node<> *fin = root->first_node("FinancialStatements");
	rapidxml::xml_node<> *ann = /*fin->first_node("AnnualPeriods")*/ fin->first_node("InterimPeriods") ;
	if (ann == NULL)
		return;
	rapidxml::xml_node<> *regcur= root->first_node("CoGeneralInfo")->first_node("ReportingCurrency");

	//<ReportingCurrency Code = "KRW">Won< / ReportingCurrency>

	/*for (rapidxml::xml_node<> * pp = root->first_node("Issues")->first_node("Issue")->first_node("IssueID"); pp; pp = pp->next_sibling())
	{
		if (memcmp(pp->first_attribute("Type")->value(), "Ticker", strlen("Ticker")) == 0)
		{
			name = pp->value();
			break;
		}
	}*/
	//if (name == "LI")
	//{
	//	int c;
	//	c = 5;
	//}
	name = pszSym;
	std::string szCurcode = "USD";
	double fExchangeRate = 1.0; //汇率
	if (regcur != NULL)
	{
		rapidxml::xml_attribute<> *curcode = regcur->first_attribute("Code");
		if (curcode != NULL)
		{
			szCurcode = curcode->value();
			fExchangeRate = atof(root->first_node("CoGeneralInfo")->first_node("MostRecentExchange")->value());
			
		}
	}
	
	std::map<std::string, StockInfo>::iterator it;
	it = infoList.find(name);
	if (it == infoList.end())
		return;
	
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

		//bb.Finlist.push_back(info);
		it->second.Finlist.push_back(info);
	}
nt:
	if (it->second.Finlist.size() < 1)
		return;
	if (it->second.fValue < 50)
		return;
	double ratA = 0.001;
	double ratB = 0.001;
	if ( it->second.Finlist[0].fin.fTotalCurrentAssets> it->second.Finlist[0].fin.fTotalLiabilities)
        ratA=  ((it->second.Finlist[0].fin.fTotalCurrentAssets-it->second.Finlist[0].fin.fTotalLiabilities)/ fExchangeRate)/it->second.fValue;
	if (it->second.Finlist[0].fin.fTotalCurrentAssets + it->second.Finlist[0].fin.fLongTermInvestments > it->second.Finlist[0].fin.fTotalLiabilities)
        ratB = ((it->second.Finlist[0].fin.fTotalCurrentAssets+it->second.Finlist[0].fin.fLongTermInvestments - it->second.Finlist[0].fin.fTotalLiabilities)/ fExchangeRate)/it->second.fValue;



	char pszWrite[1024];
	DWORD dwWrite;
	sprintf_s(pszWrite, 1024, "%s,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f\n", it->second.name.data(), it->second.fValue, it->second.Finlist[0].fin.fTotalCurrentAssets,
		it->second.Finlist[0].fin.fLongTermInvestments, it->second.Finlist[0].fin.fTotalLiabilities, ratA, ratB);
	WriteFile(hLogFile, pszWrite, strlen(pszWrite), &dwWrite, 0);
	//infoList.push_back(bb);
	//公司上市小于3年不统计
	/*if (it->second.Finlist.size() <= 3 )
		return;

	double fInitShares = 0;
	double fSharesRatioList[5] = { 1,1,1,1,1 };
	
	for (int k = 0; k < it->second.Finlist.size(); k++)
	{
		if (k == 0)
		{
			fInitShares = it->second.Finlist[0].fin.fCommonShares;
			if (fInitShares <= 0)
				return;
		}
		if (it->second.Finlist[k].fin.fCommonShares <= 0)
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
	WriteFile(hLogFile, pszWrite, strlen(pszWrite), &dwWrite, 0);*/
}

int nTick = 0;
double fAllpe = 0;
DWORD ListAllFileInDirectory(LPSTR szPath,int nType)
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

				ListAllFileInDirectory(szFullPath,nType);
			}
			else
			{
				char pszSym[256];
				memset(pszSym, 0x00, sizeof(pszSym));
				memcpy(pszSym, FindFileData.cFileName, strlen(FindFileData.cFileName) - 4);
				if(nType==0)
				   GetSotckInfo(szFullPath,pszSym);
				else if(nType==1)
				  GetReportInfo(szFullPath, pszSym);
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
	
	hLogFile = CreateFile("c:\\bighouse\\ReportsFins.txt", GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	//GetSotckInfo((char *)"C:\\bighouse\\美股财务数据\\快照\\NASDAQ\\BDRX.txt");
	/*GetSotckInfo((char *)"C:\\bighouse\\美股财务数据\\快照\\NASDAQ\\LI.txt", (char *)"LI");
	GetReportInfo((char *)"C:\\bighouse\\美股财务数据\\ReportsFinStatements\\NASDAQ\\LI.txt", (char *)"LI");*/
	//GetReportInfo((char *)"C:\\bighouse\\美股财务数据\\ReportsFinStatements\\NASDAQ\\AAPL.txt");
	//ListAllFileInDirectory((char *)"C:\\bighouse\\美股财务数据\\ReportsFinStatements\\");
	ListAllFileInDirectory((char *)"C:\\bighouse\\美股财务数据\\快照\\",0);
	ListAllFileInDirectory((char *)"C:\\bighouse\\美股财务数据\\ReportsFinStatements\\",1);
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