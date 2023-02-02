// sec-download.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "biglog.h"
#include "json/json.h"
#include <map>
#pragma warning(disable : 4996)

//DEL
//QWEST CORP|QVC INC

//ERROR SharesOustanding
//UHS  LBTYA CAF PYXS ELDN

//dr比例 CDLX SNPX
using namespace std;
struct SecInfo
{
	std::string cik;       //cik ,唯一标识
	std::string company;   //公司名称
	std::string type;      //类型
	std::string date;      //日期
	std::string downpath;  //下载路径
};

std::vector<std::string> pathList;
std::vector<struct SecInfo> InfoList;



char downdir[] = "c:\\bighouse\\sec_10Q\\";

struct SecInfo GetLineInfo(char *linebuf)
{
	struct SecInfo info;
	int m = 0;
	int pp = 0;
	int nTick = 0;
	char pszTemp[256] = "";
	for (;;)
	{
		if (linebuf[m] == NULL)
		{
			if (nTick == 4)
				info.downpath = pszTemp;
			break;
		}
		if (linebuf[m] == '|')
		{
			if (nTick == 0)
				info.cik = pszTemp;
			else if (nTick == 1)
				info.company = pszTemp;
			else if (nTick == 2)
				info.type = pszTemp;
			else if (nTick == 3)
				info.date = pszTemp;
			nTick++;
			pp = 0;
			memset(pszTemp, 0x00, sizeof(pszTemp));
		}
		else
		{
			pszTemp[pp] = linebuf[m];
			pp++;
		}
		m++;
	}
	return info;
}

void DownloadMasterTable(int nYear,int nQth)
{
	char pszHttpAddr[256]="";
	sprintf_s(pszHttpAddr, 256, "/Archives/edgar/full-index/%04d/QTR%d/master.idx", nYear, nQth);
	std::string body;
	httplib::SSLClient cli("www.sec.gov");
	auto res = cli.Get(pszHttpAddr, [&](const char *data, size_t data_length) {
		body.append(data, data_length);
		return true;
	});
	std::string dir = "c:\\bighouse\\";
	dir += "SecMasterTable";
	CreateDirectory(dir.data(), NULL);

	
	char pszFileName[256];
	sprintf_s(pszFileName, "%s\\%04d_%d_master.idx",dir.data(), nYear, nQth);
	HANDLE hFile;
	hFile = CreateFile(pszFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	DWORD dwWrite;
	WriteFile(hFile, body.data(), body.length(), &dwWrite, 0);
	CloseHandle(hFile);

	return;
}

void ReadMasterIdx(char *pFileName,char *pTypeField)
{
	HANDLE hFile;
	hFile = CreateFile(pFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	DWORD nSize = GetFileSize(hFile, 0);
	DWORD dwRead;
	char *pFileBuf = (char *)malloc(nSize + 1);
	ReadFile(hFile, pFileBuf, nSize, &dwRead, 0);
	int k = 0;
	int j = 0;
	
	char linebuf[1024]="";
	char *pBuf=strstr(pFileBuf, "|Filename");
	if (pBuf == NULL)
		goto t1;
	pBuf += strlen("|Filename");
	pBuf = strstr(pBuf, "-\n");
	if (pBuf == NULL)
		goto t1;
	pBuf += 2;
	for (;;)
	{
		if (*(pBuf + k) == NULL)
			break;
		if (*(pBuf + k) == 0x0d || *(pBuf + k) == 0x0a)
		{
			if (strlen(linebuf) > 5)
			{
				struct SecInfo info = GetLineInfo(linebuf);
				if (memicmp(pTypeField, info.type.data(), strlen(pTypeField)) == 0 && info.type.length()== strlen(pTypeField))
				{
					InfoList.push_back(info);
				}

			}
			memset(linebuf, 0x00, sizeof(linebuf));
			j = 0;

		}
		else
		{
			linebuf[j]= *(pBuf + k);
			j++;
		}
		k++;
	}
t1:
	free(pFileBuf);
	CloseHandle(hFile);
}

void downsecfile(struct SecInfo info)
{
	std::string path = "/Archives/";
	path += info.downpath;
	int nReFlag = false;
	//去年\字符，不然不能创建文件
	info.company.erase(std::remove(info.company.begin(), info.company.end(), '/'), info.company.end());
	info.company.erase(std::remove(info.company.begin(), info.company.end(), '\\'), info.company.end());
	for (int k = 0; k < pathList.size(); k++)
	{
		if (memcmp(pathList[k].data(), info.downpath.data(), pathList[k].length()) == 0)
		{
			std::string dir = downdir + info.company;
			std::string filepath = dir;
			filepath += "\\";
			filepath += "full-submission.txt";
			HANDLE hFile;
			hFile = CreateFile(filepath.data(), GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if (hFile == INVALID_HANDLE_VALUE)
				break;
			int nSize = GetFileSize(hFile, 0);
			if (nSize == 0)
			{
				CloseHandle(hFile);
				printf("%s %s 文件大小是0，要重新下载\n", info.company.data(), info.downpath.data());
				nReFlag = true;
				break;
			}
			CloseHandle(hFile);
			printf("%s %s 已经下载过了，请不要重复下载\n",info.company.data(),info.downpath.data());
			return;
		}
	}
	httplib::SSLClient cli("www.sec.gov");
	std::string body;
	auto res = cli.Get(path.data(), [&](const char *data, size_t data_length) {
		body.append(data, data_length);
		return true;
	});

	
	std::string dir=downdir+info.company;
	CreateDirectory(dir.data(), NULL);

	printf("下载%s....中\n", info.company.data());
	std::string filepath = dir;
	filepath += "\\";
	filepath += "full-submission.txt";
	HANDLE hFile;
	hFile = CreateFile(filepath.data() , GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return ;
	DWORD dwWrite;
	WriteFile(hFile, body.data(), body.length(), &dwWrite, 0);
	CloseHandle(hFile);

	//DWORD dwWrite;
	std::string logname;
	logname = downdir;
	logname += "log.txt";

	if (nReFlag == false)
	{
		HANDLE hLog = gamelog::OpenLogFile((char *)logname.data(), 1);
		WriteFile(hLog, info.downpath.data(), info.downpath.length(), &dwWrite, 0);
		WriteFile(hLog, "\n", strlen("\n"), &dwWrite, 0);
		CloseHandle(hLog);
	}

	
}

void PreLogFile()
{
	std::string logname;
	logname = downdir;
	logname += "log.txt";

	HANDLE hLog = gamelog::OpenLogFile((char *)logname.data(), 2);
	DWORD nSize = GetFileSize(hLog, 0);
	if (nSize == 0)
		return;
	DWORD dwRead;
	char *pBuf = (char *)malloc(nSize + 1);
	ReadFile(hLog, pBuf, nSize, &dwRead, 0);
	int k = 0;
	int j = 0;

	char linebuf[1024] = "";
	
	for (;;)
	{
		if (*(pBuf + k) == NULL)
			break;
		if (*(pBuf + k) == 0x0d || *(pBuf + k) == 0x0a)
		{
			if (strlen(linebuf) > 2)
			{
				pathList.push_back(linebuf);
			}
			memset(linebuf, 0x00, sizeof(linebuf));
			j = 0;

		}
		else
		{
			linebuf[j] = *(pBuf + k);
			j++;
		}
		k++;
	}

	free(pBuf);
	CloseHandle(hLog);
}
struct TInfo {
	int nBegin;
	int nEnd;
};

DWORD WINAPI ThreadDownFile(LPVOID lpvoid)
{
	struct TInfo *pInfo = (struct TInfo *)lpvoid;

	for (int k = pInfo->nBegin; k < pInfo->nEnd; k++)
	{
		printf("down index=%d\n", k);
		if (k == 11)
		{
			int j;
			j = 0;
		}
		downsecfile(InfoList[k]);
	}
	return true;
}
#define MAX_THREAD 20

void GetTicker()
{

	HANDLE hFile;
	hFile = CreateFile("c:\\bighouse\\ticker.txt", GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	DWORD dwWrite;
	
	std::ifstream ifs;
	ifs.open("C:\\bighouse\\stock_symblo.json"); // Windows自己注意路径吧
	assert(ifs.is_open());

	Json::Reader reader;
	Json::Value root;
	// 解析到root，root将包含Json里所有子元素
	if (!reader.parse(ifs, root, false))
	{
		return ;
	}
	Json::Value StockList = root["data"];
	printf("%d\n", StockList.size());
	int nSam = 0;
	std::map<std::string,int> tickerList;
	for (int j = 0; j < InfoList.size(); j++)
	{
		//if (/*memicmp("HENRY SCHEIN", pName, 12) == 0 &&*/ memicmp("HENRY SCHEIN", InfoList[j].company.data(), 12) == 0)
		//{
		//	int c;
		//	c = 5;
		//}
		int nIndex = InfoList[j].company.length();
		if (memicmp(InfoList[j].company.data() + nIndex - 3, "INC", 3) == 0 ||
			memicmp(InfoList[j].company.data() + nIndex - 4, "INC.", 4) == 0)
		{
			for (;;)
			{
				if (nIndex == 0 || InfoList[j].company[nIndex] == ' ')
				{
					if (InfoList[j].company[nIndex - 1] == ',')
						nIndex--;
					break;
				}
				nIndex--;
			}
		}
		char pszTemp[256] = "";
		memcpy(pszTemp, InfoList[j].company.data(), nIndex);

		
		int k = 0;
		for (k = 0; k < StockList.size(); k++)
		{
			const char *pName = StockList[k]["name"].asCString();
			
			
			
			if (memicmp(pszTemp, pName, strlen(pszTemp)) == 0)
			{
				std::map<std::string,int>::iterator it;
				it = tickerList.find(StockList[k]["symbol"].asCString());
				if (it == tickerList.end())
				{
					char pszWrite[1024];
					sprintf(pszWrite, "%s|%s\n", StockList[k]["symbol"].asCString(), InfoList[j].company.data());
					WriteFile(hFile, pszWrite, strlen(pszWrite), &dwWrite, 0);

					tickerList.insert(std::pair<std::string, int >(StockList[k]["symbol"].asCString(), 0));
				}
			
				nSam++;
				break;
			}
		}
		if (k == StockList.size())
		{
			printf("%s\n", InfoList[j].company.data());
		}
	}
	//for (int k = 0; k < StockList.size(); k++)
	//{
	//	const char *pName = StockList[k]["name"].asCString();
	//	int j = 0;
	//	for (j = 0; j < InfoList.size(); j++)
	//	{
	//		if (memicmp(InfoList[j].company.data(), pName, InfoList[j].company.length()) == 0)
	//		{
	//			char pszWrite[1024];
	//			sprintf(pszWrite,"%s,%s\n", StockList[k]["symbol"].asCString(),InfoList[j].company.data());
	//			WriteFile(hFile, pszWrite, strlen(pszWrite), &dwWrite, 0);
	//			nSam++;
	//			break;
	//		}
	//	}
	//	
	//	//if(memcmp(pName,""))

	//}

	printf("have same=%d\n", nSam);
	CloseHandle(hFile);
}
int main()
{

	
	/*ReadMasterIdx((char *)"c:\\bighouse\\master.idx", (char *)"10-Q");
	printf("%d\n", InfoList.size());*/
	struct TInfo param[MAX_THREAD];
	PreLogFile();
	ReadMasterIdx((char *)"c:\\bighouse\\master.idx", (char *)"10-Q");
	ReadMasterIdx((char *)"c:\\bighouse\\master.idx", (char *)"10-K");


	printf("%d\n", InfoList.size());

	//DownloadMasterTable(2008, 4);
	//GetTicker();
	//return 0;

	int nThreadCount =1;  //线程数量
	memset(&param, 0x00, sizeof(param));
	int nEach=InfoList.size() / nThreadCount;
	DWORD dwThread;
	for (int k = 0; k < nThreadCount; k++)
	{
		param[k].nBegin = nEach * k;
		param[k].nEnd = param[k].nBegin + nEach - 1;
		if (k == nThreadCount - 1)
			param[k].nEnd = InfoList.size();
		CreateThread(0, 0, &ThreadDownFile, &param[k], 0, &dwThread);
	}

	for (;;)
	{
		Sleep(100);
	}
	/*for (int k = 0; k < InfoList.size(); k++)
	{
		downsecfile(InfoList[k]);
	}*/

	return 0;
	httplib::SSLClient cli("www.sec.gov");
	std::string body;

	//auto res = cli.Get("/Archives/edgar/data/1428439/0001428439-22-000036.txt", [](uint64_t len, uint64_t total) {
	//	printf("%lld / %lld bytes => %d%% complete\n",
	//		len, total,
	//		(int)(len * 100 / total));
	//	return true; // return 'false' if you want to cancel the request.
	//});
	auto res = cli.Get("/Archives/edgar/data/1428439/0001428439-22-000036.txt", [&](const char *data, size_t data_length) {
		body.append(data, data_length);
		return true;
	});
	
	/*httplib::Client cli("www.sec.gov");
	auto res = cli.Get("/");
	std::cout << res->body << std::endl;*/
	
    std::cout << "Hello World!\n"; 
}

