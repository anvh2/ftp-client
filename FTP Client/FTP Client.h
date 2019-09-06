#pragma once
#define _CRE_SECURE_NO_WARNNINGS
#include "resource.h"
#include <afxsock.h>
#include <string>
#include <io.h>
#include "dirent.h"
#include <vector>
#include <conio.h>

using namespace std;

class FTPClient
{
private:
	char buf[BUFSIZ + 1];
	int codeftp, tmpres;
	CSocket ClientSocket;
	CSocket Data;
	CSocket ClientSocketAct;
	CSocket ClientSocketPas;
	int actOrPas;//1 is active, 2 is passive
	char IP[20];
	UINT port;
public:
	void handleCMD();//processing from command line
	void replyLogCode(int);
	char* sendCommand(const char* str, const char* info, const int& code);
	bool sendActPort();
	bool openPASV();
	//cau 1
	bool login(const char*);
	//cau 2
	void list(const char*, const char*);
	//cau 3
	void upload(const char*);
	//cau 4
	void download(const char*);
	//cau 5
	void uploadMFile(const char*);
	//cau 6, 10
	void FTPClient::dowMFile_mget_AndDelMFile_mdele(string cmd);
	//cau 7
	void changeCurtDictServer(const char*);//change the current directory on the server //ex: folder1/folder2/...
	//cau 8
	void changeCurtDictClient(const char*);
	//cau 9
	void delFile(const char*);
	//cau 11
	void createFolder(const char*);
	//cau 12
	void delEmptyFolder(const char*);
	//cau 13
	void currentDirectory();
	//cau 14
	void passiveMode();//cmd: quote pasv
	//cau 15
	void quit();

	FTPClient();
	~FTPClient();
};