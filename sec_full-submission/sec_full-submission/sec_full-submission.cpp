// sec_full-submission.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "windows.h"
#include <set>
#include<string>
#include<vector>
#include "json/json.h"
#include "rapidxml/rapidxml.hpp"
#include <fstream>
#pragma warning(disable:4996)
struct gaapInfo
{
	char keyName[128];    
	char contextRef[512];  //截止日期或指定特定日期
	char unitRef[32];      //货币单位
	char value[128];      //内容
	char decimals[128];
};

//struct gaapInfo
//{
//	std::string keyName;
//	std::string contextRef;
//	std::string unitRef;
//	std::string value;
//};

int nMode =0;
HANDLE hSyFile = 0;
std::vector<std::string> symbolList;
char keyString[][128]=
{
	"us-gaap:CashAndCashEquivalentsAtCarryingValue",      //现金或现金等价物
	"us-gaap:AllowanceForDoubtfulAccountsReceivableCurrent", //坏账,
	"us-gaap:Liabilities",//负债总额
	"us-gaap:LiabilitiesAndStockholdersEquity",//负债和股东权益合计
	"us-gaap:StockholdersEquity",//母公司股东权益合计
	"us-gaap:StockholdersEquityIncludingPortionAttributableToNoncontrollingInterest",//股东权益合计
	"us-gaap:SharesOutstanding",   //普通股
	"us-gaap:CommonStockSharesOutstanding",//普通股,  
	"dei:EntityCommonStockSharesOutstanding",//普通股, 即时
	"dei:TradingSymbol"  ,//交易代码,
	"dei:EntityRegistrantName"  ,//公司注册名字
	"context"
};

char JsonKey[][128]=
{
    "EntityCommonStockSharesOutstanding"
};

//子内容关键字
char subkeyString[][64]=
{
	"contextRef",
	"unitRef",
	"decimals"
};
void parse_node_tree(const rapidxml::xml_node<>* node, Json::Value& parent)
{
	Json::Value obj(Json::objectValue);

	std::multiset<std::string> array;
	for (rapidxml::xml_node<> *child = node->first_node(); child != nullptr; child = child->next_sibling())
	{
		if (child->type() != rapidxml::node_element)
			continue;

		array.insert(child->name());
	}

	if (node->value_size() > 0)
	{
		obj["#text"] = node->value();
	}

	bool hasChilds = false;

	for (rapidxml::xml_attribute<> *attr = node->first_attribute(); attr != nullptr; attr = attr->next_attribute())
	{
		hasChilds = true;
		obj[attr->name()] = attr->value();
	}

	for (rapidxml::xml_node<> *child = node->first_node(); child != nullptr; child = child->next_sibling())
	{
		if (child->type() != rapidxml::node_element)
			continue;

		hasChilds = true;

		Json::Value& next = obj[child->name()];
		if (array.count(child->name()) > 1 && !next.isArray())
		{
			next = Json::arrayValue;
		}

		parse_node_tree(child, next);
	}

	// set result.
	if (parent.isArray())
	{
		parent.append(obj);
	}
	else
	{
		if (obj.isArray() || hasChilds)
			parent = obj;
		else
			parent = node->value();
	}
}


// convert xml string to json
std::string xmltojson(const std::string& xml)
{
	/*char xml_text[xml.size() + 1];

	memset(xml_text, 0, xml.size());
	strncpy(xml_text, xml.c_str(), xml.size());
	xml_text[xml.size()] = '\0';*/

	std::string xml_text = xml;
	rapidxml::xml_document<> doc;

	try
	{
		doc.parse<0>((char *)xml_text.data());
	}
	catch (rapidxml::parse_error& exp)
	{
		std::cout << exp.what() << std::endl;
		return std::string();
	}

	rapidxml::xml_node<> *node = doc.first_node();
	if (node == nullptr)
		return std::string();

	Json::Value jdoc;
	Json::Value& jroot = jdoc[node->name()];

	parse_node_tree(node, jroot);

	Json::FastWriter fast_writer;
	return fast_writer.write(jdoc);
}

std::vector<std::string> InfoList;
std::vector<gaapInfo>    gaapInfoList;

bool IsKeyHead(char *pBuf)
{
	if (memcmp("<us-gaap:", pBuf, 9) == 0 || memcmp("<dei:", pBuf, 5) == 0 || memcmp("<context", pBuf, 8) == 0)
	     return true;
	else
		 return false;
}

bool IsKeyHeadEnd(char *pBuf)
{
	if (memcmp("</us-gaap:", pBuf , 10)==0 || memcmp("</dei:", pBuf, 6) == 0 || memcmp("</context", pBuf, 9) == 0)
		return true;
	else
		return false;
}

void ReadSecData(char *pFileName)
{
	InfoList.clear();
	HANDLE hFile;
	hFile = CreateFile(pFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	int nSize = GetFileSize(hFile, 0);
	if (nSize == 0)
	{
		CloseHandle(hFile);
		printf("%s size=0\n", pFileName, nSize);
		return;
	}
	DWORD dwRead;
	char *pBuf = (char *)malloc(nSize + 1);
	ReadFile(hFile, pBuf, nSize, &dwRead, 0);
	int nKeyCount = sizeof(keyString) / 128;
	int k = 0;
	int m;
	std::string pszKeyContent;
	for (;;)
	{
		if (k >= nSize-strlen("<us-gaap:"))
			break;
		if (IsKeyHead(pBuf + k))
		{
		//	if(memcmp("<us-gaap:CommonStockSharesIssued"))
			for (int j = 0; j < nKeyCount; j++)
			{
				if (memcmp(keyString[j], pBuf + k + 1, strlen(keyString[j])) == 0)
				{
					
					m = 0; 
					pszKeyContent.clear();
					if (!(*(pBuf + k + strlen(keyString[j]) + 1) == 0x0a || *(pBuf + k + strlen(keyString[j]) + 1) == ' '))
						continue;
					for (;;)
					{
						if (k >= nSize-strlen("</us-gaap:"))
							goto nt;
						if ((IsKeyHead(pBuf + k) && m != 0 )|| m > 1500)
						{
							if (m > 1500)
							{
								int dk;
								dk = 6;
								goto nt;
							}
						}

						if (IsKeyHeadEnd(pBuf + k))
						{
							char pszTemp[128] = "</";

							memcpy(pszTemp + strlen(pszTemp), keyString[j], strlen(keyString[j]));
							pszTemp[strlen(pszTemp)] = '>';
							if (memcmp(pszTemp, pBuf + k, strlen(pszTemp)) == 0)
							{

								k += strlen(pszTemp);
								pszKeyContent += pszTemp;

								if (memcmp(pszKeyContent.data() + 1, "context", 7) != 0)
								{
									int nPos =pszKeyContent.find("id=\"");
									
									if (nPos != std::string::npos)
									{
										int nEnd= pszKeyContent.find("\"",nPos+strlen("id=\""));
										if (nEnd != std::string::npos)
										{
											pszKeyContent.erase(nPos, nEnd - nPos);
										}
									}
								}
								//memcpy(pszKeyContent + m, pszTemp, strlen(pszTemp));
								//printf("%s\n", pszKeyContent.data());
								//InfoList.push_back(pszKeyContent);
								int nFlag = true;
								//去掉重复的内容
								for (int bb = 0; bb < InfoList.size(); bb++)
								{
									if (memcmp(InfoList[bb].c_str(), pszKeyContent.c_str(), pszKeyContent.length()) == 0)
									{
										nFlag = false;
									}
								}
								if (nFlag == true)
									InfoList.push_back(pszKeyContent);
								else
								{
									printf("differ!\n");
								}
								break;
							}
						}
					
						pszKeyContent += pBuf[k];
					//	memcpy((char *)pszKeyContent.at(m), (char *)pBuf+k, 1);
						k++;
						m++;
					}

				}
			}
		}
nt:
		k++;
	}
	CloseHandle(hFile);
	free(pBuf);
}
//读取sec文件，并提取关键字内容


struct gaapInfo AnalyseGaapInfo(char *pBuf)
{
	struct gaapInfo info;
	memset(&info, 0x00, sizeof(gaapInfo));
	int k = 0;
	int nSize = strlen(pBuf);
	int nSubCount = sizeof(subkeyString) / 64;
	char pszKeyContent[2048];
	int nIndex = 0;
	bool keyFlagNoGet = false;

	for (;;)
	{
		if (k >= nSize - 1)
			break;
		if (keyFlagNoGet == false && (pBuf[k] == ' ' || pBuf[k] == 0x0a))
		{
			memcpy(info.keyName, pBuf + 1, k - 1);
			keyFlagNoGet = true;
		}
		for (int j = 0; j < nSubCount; j++)
		{
			if (memcmp(">", pBuf + k, 1) == 0)
			{
				nIndex = k + 1;
				memset(info.value, 0x00, sizeof(info.value));
				for (;;)
				{
					if (memcmp("<", pBuf + k, 1) == 0)
					{
						memcpy(info.value, pBuf + nIndex, k - nIndex);
						break;
					}
					k++;
				}
			}
			else if (memcmp(subkeyString[j], pBuf + k, strlen(subkeyString[j])) == 0)
			{

				nIndex = 0;
				for (;;)
				{

					if (memcmp("\"", pBuf + k, 1) == 0 || memcmp("\'", pBuf + k, 1) == 0)
					{
						if (nIndex != 0)
						{
							if (memcmp(subkeyString[j], "contextRef", strlen(subkeyString[j])) == 0)
							{
								memcpy(info.contextRef, pBuf + nIndex, k - nIndex);
							}
							if (memcmp(subkeyString[j], "unitRef", strlen(subkeyString[j])) == 0)
							{
								memcpy(info.unitRef, pBuf + nIndex, k - nIndex);
							}
							if (memcmp(subkeyString[j], "decimals", strlen(subkeyString[j])) == 0)
							{
								memcpy(info.decimals, pBuf + nIndex, k - nIndex);
							}
							/*if (memcmp(subkeyString[j], "unitRef", strlen(subkeyString[j])) == 0)
							{
								memcpy(info.keyName, "unitRef", strlen(subkeyString[j]));
								memcpy(info.unitRef, pBuf + nIndex, k - nIndex);
							}*/
							break;

						}
						else
							nIndex = k + 1;
					}
					//	pszKeyContent[m] = pszKeyContent[k];
					k++;
				}
			}
		}
		k++;
	}
	return info;
}


struct EntSharesInfo
{
	//std::string value;
	std::vector<std::string> dimensionList;
	std::vector<std::string> valueList;
	std::string instant;
};

struct CashInfo
{
	std::string instant;
};

EntSharesInfo AnalyseEntContextInfo(char *pBuf)
{
	EntSharesInfo info;
	std::string jtxt =xmltojson(pBuf);
	printf("%s\n", jtxt.data());
	Json::Reader reader;
	Json::Value root;
	//从字符串中读取数据
	reader.parse(jtxt, root);
	info.instant =root["context"]["period"]["instant"].asString();

    Json::Value va = root["context"]["entity"]["segment"]["xbrldi:explicitMember"];
	if (va.isArray())
	{
		for (int j = 0; j < va.size(); j++)
		{
			info.dimensionList.push_back(va[j]["dimension"].asString());
			info.valueList.push_back(va[j]["#text"].asString());
		}
	}

	else
	{
		info.dimensionList.push_back(va["dimension"].asString());
		info.valueList.push_back(va["#text"].asString());
		//info.dimension = root["context"]["entity"]["segment"]["xbrldi:explicitMember"]["dimension"].asString();
		//info.value = root["context"]["entity"]["segment"]["xbrldi:explicitMember"]["#text"].asString();
	}
	return info;
}

CashInfo AnalyseCashContextInfo(char *pBuf)
{
	CashInfo info;
	std::string jtxt = xmltojson(pBuf);
	//printf("%s\n", jtxt.data());
	Json::Reader reader;
	Json::Value root;
	//从字符串中读取数据
	reader.parse(jtxt, root);
	info.instant = root["context"]["period"]["instant"].asString();
	return info;
}
//struct EntSharesOutstandingInfo
//{
//
//};
void WriteInfoToJson(char *szFullPath)
{
	Json::Value root;
	std::vector<std::string> dataList[20];
	for (int k = 0; k < InfoList.size(); k++)
	{

		//CashAndCashEquivalentsAtCarryingValue
		for (int m = 0; m < 4; m++)
		{
			if (memcmp(InfoList[k].data() + 1, keyString[m], strlen(keyString[m])) == 0
				&& (InfoList[k].data()[strlen(keyString[m])+1]==' ' || InfoList[k].data()[strlen(keyString[m])+1] == 0x0a) )
			{
				
				struct gaapInfo info = AnalyseGaapInfo((char *)InfoList[k].data());
				for (int j = 0; j < InfoList.size(); j++)
				{
					std::string cmpstr = "<context id=\"";
					cmpstr += info.contextRef;
					if (memcmp(InfoList[j].data(), cmpstr.data(), cmpstr.length()) == 0)
					{
						CashInfo ent = AnalyseCashContextInfo((char *)InfoList[j].data());

						Json::Value node_ent;
						node_ent["decimals"] = Json::Value(info.decimals);
						node_ent["unitRef"] = Json::Value(info.unitRef);
						node_ent["period"]["instant"] = Json::Value(ent.instant);
						node_ent["value"] = Json::Value(info.value);
						std::string tmp = keyString[m] + 8;
						//这些字段里要去掉重复的日期
						int pp;
						for (pp = 0; pp < dataList[m].size(); pp++)
						{
							if (dataList[m][pp] == ent.instant)
								break;
						}
						if (pp == dataList[m].size())
						{
							dataList[m].push_back(ent.instant);
							root[tmp].append(node_ent);
						}

						
					}
				}
			}
		}

		for (int m = 4; m < 8; m++)
		{
			if (memcmp(InfoList[k].data() + 1, keyString[m], strlen(keyString[m])) == 0
				&& (InfoList[k].data()[strlen(keyString[m]) + 1] == ' ' || InfoList[k].data()[strlen(keyString[m]) + 1] == 0x0a))
			{

				struct gaapInfo info = AnalyseGaapInfo((char *)InfoList[k].data());
				for (int j = 0; j < InfoList.size(); j++)
				{
					std::string cmpstr = "<context id=\"";
					cmpstr += info.contextRef;
					if (memcmp(InfoList[j].data(), cmpstr.data(), cmpstr.length()) == 0)
					{
						EntSharesInfo ent = AnalyseEntContextInfo((char *)InfoList[j].data());

						Json::Value node_ent;
						node_ent["decimals"] = Json::Value(info.decimals);
						node_ent["unitRef"] = Json::Value(info.unitRef);
						node_ent["period"]["instant"] = Json::Value(ent.instant);
						if (ent.dimensionList.size() == 1)
						{
							node_ent["segment"]["dimension"] = Json::Value(ent.dimensionList[0]);
							node_ent["segment"]["value"] = Json::Value(ent.valueList[0]);
						}
						else
						{
							for (int pp = 0; pp < ent.dimensionList.size(); pp++)
							{
								Json::Value tmp;
								tmp["dimension"] = Json::Value(ent.dimensionList[pp]);
								tmp["value"] = Json::Value(ent.valueList[pp]);
								node_ent["segment"].append(tmp);
							}
						}
						node_ent["value"] = Json::Value(info.value);
						std::string tmp = keyString[m] + 8;
						root[tmp].append(node_ent);
					}
				}
			}
		}
		//"EntityCommonStockSharesOutstanding"
		if (memcmp(InfoList[k].data()+1 , keyString[8],strlen(keyString[8]))==0)
		{
			struct gaapInfo info = AnalyseGaapInfo((char *)InfoList[k].data());
			for (int j = 0; j < InfoList.size(); j++)
			{
				std::string cmpstr="<context id=\"";
				cmpstr += info.contextRef;
				if (memcmp(InfoList[j].data(), cmpstr.data(), cmpstr.length()) == 0)
				{
					EntSharesInfo ent= AnalyseEntContextInfo((char *)InfoList[j].data());
					
					Json::Value node_ent;
					node_ent["decimals"] = Json::Value(info.decimals);
					node_ent["unitRef"] = Json::Value(info.unitRef);
					node_ent["period"]["instant"] = Json::Value(ent.instant);
					if (ent.dimensionList.size() == 1)
					{
						node_ent["segment"]["dimension"] = Json::Value(ent.dimensionList[0]);
						node_ent["segment"]["value"] = Json::Value(ent.valueList[0]);
					}
					else
					{
						for (int pp = 0; pp < ent.dimensionList.size(); pp++)
						{
							Json::Value tmp;
							tmp["dimension"] = Json::Value(ent.dimensionList[pp]);
							tmp["value"] = Json::Value(ent.valueList[pp]);
							node_ent["segment"].append(tmp);
						}
					}
					node_ent["value"] = Json::Value(info.value);
					root["EntityCommonStockSharesOutstanding"].append(node_ent);
				}
			}
		}


		if (memcmp(InfoList[k].data() + 1, keyString[9], strlen(keyString[9])) == 0)
		{
			struct gaapInfo info = AnalyseGaapInfo((char *)InfoList[k].data());
			root["TradingSymbol"] = Json::Value(info.value);
		}
		if (memcmp(InfoList[k].data() + 1, keyString[10], strlen(keyString[10])) == 0)
		{
			struct gaapInfo info = AnalyseGaapInfo((char *)InfoList[k].data());
			root["EntityRegistrantName"] = Json::Value(info.value);
		}
	}
	printf("%s\n", root.toStyledString().data());

	char pszWritePath[MAX_PATH] = "C:\\bighouse\\sec10json";
	char pszSymbloName[MAX_PATH] = "";
	char pszWriteFile[MAX_PATH];
	char *p = strstr(szFullPath, "sec_10Q\\\\");
	//	char *p=strstr(szFullPath, "sec-edgar-filings\\\\");
	if (p == NULL)
		return;
	//	p += strlen("sec-edgar-filings\\\\");
	p += strlen("sec_10Q\\\\");
	char *end = strstr(p, "\\");
	if (end == NULL)
		return;
	memcpy(pszSymbloName, p, end - p);
	sprintf_s(pszWriteFile, MAX_PATH, "%s\\%s.json", pszWritePath, pszSymbloName);
	Json::StyledWriter sw;
	std::ofstream os;
	os.open(pszWriteFile);
	os << sw.write(root);
	os.close();
}
void WriteInfoToFile(char *szFullPath)
{
	HANDLE hFile = 0;
	/*int nLen = strlen("C:\\bighouse\\sec_10Q\\\\FULL HOUSE RESORTS INC\\full-submission.txt");
	if (memcmp(szFullPath, "C:\\bighouse\\sec_10Q\\\\FULL HOUSE RESORTS INC\\full-submission.txt", nLen) == 0)
	{
		printf("Write File %s\n", szFullPath);
	}*/
	
	printf("Write File %s\n", szFullPath);
	char pszWritePath[MAX_PATH] = "C:\\bighouse\\test10";
	//char pszWritePath[MAX_PATH] = "C:\\sec20f";
	char pszSymbloName[MAX_PATH]="";
	char pszWriteFile[MAX_PATH];
	char *p = strstr(szFullPath, "sec_10Q\\\\");
//	char *p=strstr(szFullPath, "sec-edgar-filings\\\\");
	if (p == NULL)
		return;
//	p += strlen("sec-edgar-filings\\\\");
	p += strlen("sec_10Q\\\\");
	char *end=strstr(p, "\\");
	if (end == NULL)
		return;
	memcpy(pszSymbloName, p, end - p);
	sprintf_s(pszWriteFile, MAX_PATH, "%s\\%s.txt", pszWritePath, pszSymbloName);

	
	hFile = CreateFile(pszWriteFile, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	for (int k = 0; k < InfoList.size(); k++)
	{
		DWORD dwWrite;
		WriteFile(hFile, InfoList[k].c_str(), InfoList[k].length(), &dwWrite, 0);
		WriteFile(hFile, "\n", strlen("\n"),  &dwWrite, 0);
		//AnalyseGaapInfo((char *)InfoList[k].c_str());
	}
	CloseHandle(hFile);
}

#pragma comment (lib, "User32.lib")

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
				if (nMode == 1 && memcmp("full-submission.txt", FindFileData.cFileName, strlen("full-submission.txt")) == 0)
				{
					char pszSymbloName[MAX_PATH] = "";
					char *p = strstr(szFullPath, "sec-edgar-filings\\\\");
					if (p == NULL)
						return 0;
					p += strlen("sec-edgar-filings\\\\");
					char *end = strstr(p, "\\");
					memcpy(pszSymbloName, p, end - p);

					int nRepFlag = false;
					for (int k = 0; k < symbolList.size(); k++)
					{
						if (memcmp(symbolList[k].c_str(), pszSymbloName, strlen(pszSymbloName)) == 0 && symbolList[k].length()== strlen(pszSymbloName))
						{
							nRepFlag = true;
							if (strchr(pszSymbloName, '$') == NULL)
							   printf("%s\n", pszSymbloName);
							break;
						}
					}
					if (nRepFlag == false)
					{
						symbolList.push_back(pszSymbloName);
						if (strchr(pszSymbloName, '$') == NULL)
						{

							DWORD dwWrite;
							WriteFile(hSyFile, pszSymbloName, strlen(pszSymbloName), &dwWrite, 0);
							WriteFile(hSyFile, "\n", strlen("\n"), &dwWrite, 0);
						}
					}

					
				}
			
				//if(strchr())
				if (memcmp("full-submission.txt", FindFileData.cFileName, strlen("full-submission.txt")) == 0 && nMode!=1)
				{
					
					ReadSecData(szFullPath);
					WriteInfoToJson(szFullPath);
					//WriteInfoToFile(szFullPath);

					//for (int k = 0; k < InfoList.size(); k++)
					//	AnalyseGaapInfo((char *)InfoList[k].c_str());
				}
			}
		} while (FindNextFile(hListFile, &FindFileData));
	}
	return 0;
}
int main()
{
	if (nMode == 1)
	{
		
		hSyFile = CreateFile("C:\\bighouse\\Symblo_chinese.txt", GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hSyFile == INVALID_HANDLE_VALUE)
			return 0;
	}
	int nLen;
//	AnalyseGaapInfo((char *)"<us-gaap:CashAndCashEquivalentsAtCarryingValue unitRef=\"USD\" contextRef=\"E11\" decimals=\"INF\">84845</us-gaap:CashAndCashEquivalentsAtCarryingValue>");
//	ListAllFileInDirectory((char *)"C:\\sec-test\\sec-edgar-filings\\");
	//ListAllFileInDirectory((char *)"C:\\sec_chinesestock\\sec-edgar-filings\\");
    ListAllFileInDirectory((char *)"C:\\bighouse\\sec_10Q\\");
	return 0;
	/*char pszFileName[] = "C:\\bighouse\\sec_10Q\\ROKU, INC\\full-submission.txt";
	ReadSecData(pszFileName);
	WriteInfoToJson((char *)"c:\\bighouse\\roku.txt");*/
	/*for(int k=0;k<InfoList.size();k++)
	   AnalyseGaapInfo((char *)InfoList[k].c_str());*/

    std::cout << "Hello World!\n"; 
}

