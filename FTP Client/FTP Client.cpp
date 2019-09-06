// FTP Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FTP Client.h"
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

FTPClient::FTPClient()
{
	AfxSocketInit(NULL);
	ClientSocket.Create();
	memset(buf, 0, sizeof buf);
	//default active
	actOrPas = 1;
	//default port
	port = 21;
}

FTPClient::~FTPClient()
{

}

//ham lay tu dau tien roi xoa
string getword(string& str)
{
	string res;
	res = str.substr(0, str.find_first_of(" "));//chuoi con cho den ky tu trang dau tien
	str.erase(0, res.length() + 1);//xoa chu dau cung voi ky tu trang
	return res;
}

void FTPClient::handleCMD()//processing from cmd
{
	string cmd, tmp;
	bool isLogin = 0;
	while (true)
	{
		if (isLogin == 1)
			cout << "ftp> ";
		getline(cin, cmd);//nhap lenh 
		cin.sync();
		tmp = getword(cmd);//lay lenh
		if (tmp == "ftp")
		{
			tmp = getword(cmd);//lay ip
			if (tmp == "")
				getline(cin, tmp);
			login(tmp.c_str());
			isLogin = 1;
		}

		else if (tmp == "ls" || tmp == "dir")
		{
			list(tmp.c_str(), getword(cmd).c_str());
		}

		else if (tmp == "put")
		{
			tmp = getword(cmd);//lay ten file
			if (tmp == "")
				getline(cin, tmp);
			upload(tmp.c_str());
		}

		else if (tmp == "get")
		{
			tmp = getword(cmd);//lay ten file
			if (tmp == "")
				getline(cin, tmp);
			download(tmp.c_str());
		}

		else if (tmp == "mput")
		{
			tmp = getword(cmd);
			if (tmp == "")
				getline(cin, tmp);
			uploadMFile(tmp.c_str());
		}
		else if (tmp == "mget" || tmp == "mdele" || tmp == "mdelete")
		{
			dowMFile_mget_AndDelMFile_mdele(tmp.c_str());
		}
		else if (tmp == "cd")
		{
			tmp = getword(cmd);//lay duong dan
			if (tmp == "")
				getline(cin, tmp);
			changeCurtDictServer(tmp.c_str());
		}

		else if (tmp == "lcd")
		{
			tmp = getword(cmd);//lay duong dan
			if (tmp == "")
				getline(cin, tmp);
			changeCurtDictClient(tmp.c_str());
		}

		else if (tmp == "delete")
		{
			tmp = getword(cmd);//lay ten file
			if (tmp == "")
				getline(cin, tmp);
			delFile(tmp.c_str());
		}

		else if (tmp == "mkdir")
		{
			tmp = getword(cmd);//lay ten thu muc
			if (tmp == "")
				getline(cin, tmp);
			createFolder(tmp.c_str());
		}

		else if (tmp == "rmdir")
		{
			tmp = getword(cmd);//lay ten thu muc 
			if (tmp == "")
				getline(cin, tmp);
			delEmptyFolder(tmp.c_str());
		}

		else if (tmp == "pwd")
			currentDirectory();
		else if (tmp == "quote" && getword(cmd) == "pasv")
		{
			passiveMode();
			actOrPas = 2;//passive mode is on
		}
		else if (tmp == "quote" && getword(cmd) == "actv")
		{
			actOrPas = 1;//passive mode is on
		}
		else if (tmp == "quit")
		{
			quit();
			isLogin = 0;
		}
		else if (tmp == "exit")
			return;
		else
			cout << "Invalid commnad...\n";
	}
}

void FTPClient::replyLogCode(int code)
{
	switch (code)
	{
	case 200:
		printf("Command okay");
		break;
	case 500:
		printf("Syntax error, command unrecognized.");
		printf("This may include errors such as command line too long.");
		break;
	case 501:
		printf("Syntax error in parameters or arguments.");
		break;
	case 202:
		printf("Command not implemented, superfluous at this site.");
		break;
	case 502:
		printf("Command not implemented.");
		break;
	case 503:
		printf("Bad sequence of commands.");
		break;
	case 530:
		printf("Not logged in.");
		break;
	}
	printf("\n");
}

char* FTPClient::sendCommand(const char* cmd, const char* info, const int& code)
{
	memset(buf, 0, BUFSIZ);
	sprintf(buf, "%s %s\r\n", cmd, info);
	ClientSocket.Send(buf, strlen(buf), 0);
	memset(buf, 0, BUFSIZ);
	ClientSocket.Receive(buf, BUFSIZ, 0);
	sscanf(buf, "%d", &codeftp);
	if (codeftp != code)
	{
		replyLogCode(codeftp);
	}
	return buf;
}

bool FTPClient::sendActPort()
{
	int actport;// actport = p1 * 256 + p2
	//1024 <= actport <= 65550
	srand((unsigned)time(NULL));
	int p1 = rand() % 252 + 4;// 4 <= p1 <= 255 
	int p2 = rand() % 271;//0 <= p2 <= 270
	actport = p1 * 256 + p2;

	char ip[20];
	strcpy(ip, IP);
	//convert ip to form: 127,0,0,1
	for (int i = 0; i < strlen(ip); i++)
	{
		if (ip[i] == '.')
			ip[i] = ',';
	}
	//send port to server
	memset(buf, 0, BUFSIZ);
	sprintf(buf, "PORT %s,%d,%d\r\n", ip, p1, p2);
	ClientSocket.Send(buf, strlen(buf), 0);
	//recieve reply
	memset(buf, 0, BUFSIZ);
	ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf;
	//create socket
	if (!ClientSocketAct.Create(actport))
	{
		cout << "Can't initialize socket...\n";
		return 0;
	}
	ClientSocketAct.Listen();

	return 1;
}

bool FTPClient::openPASV()
{
	//send commad to open passive mode
	memset(buf, 0, BUFSIZ);
	strcpy(buf, "PASV\r\n");
	ClientSocket.Send(buf, strlen(buf), 0);
	//recieve reply
	memset(buf, 0, BUFSIZ);
	ClientSocket.Receive(buf, BUFSIZ, 0);
	sscanf(buf, "%d", &codeftp);
	if (codeftp != 227)
	{
		cout << "Can't enter passive mode...\n";
		return 0;
	}
	int nameserver[4], port1, port2;
	sscanf_s(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &nameserver[0], &nameserver[1],
		&nameserver[2], &nameserver[3], &port1, &port2);
	//get port
	int port = port1 * 256 + port2;
	//get IP
	wstring sserver = to_wstring(nameserver[0]) + L"."
		+ to_wstring(nameserver[1]) + L"."
		+ to_wstring(nameserver[2]) + L"."
		+ to_wstring(nameserver[3]);
	if (!ClientSocketPas.Create())
	{
		cout << "Can't initialize socket...\n";
		return 0;
	}
	ClientSocketPas.Connect(sserver.c_str(), port);
	return 1;
}

bool FTPClient::login(const char* ip)
{
	//get ip from cmd
	strcpy(IP, ip);

	//connect to server
	if (!ClientSocket.Connect(CString(IP), port))
	{
		cout << "Can't connect...\n";
		return 0;
	}
	printf("Connection established, waiting for welcome message...\n");
	//How to know the end of welcome message: http://stackoverflow.com/questions/13082538/how-to-know-the-end-of-ftp-welcome-message
	memset(buf, 0, sizeof buf);
	while ((tmpres = ClientSocket.Receive(buf, BUFSIZ, 0)) > 0)
	{
		sscanf(buf, "%d", &codeftp);
		printf("%s", buf);
		if (codeftp != 220) //120, 240, 421: something wrong
		{
			replyLogCode(codeftp);
			exit(1);
		}

		char* str = strstr(buf, "220");//to break
		if (str != NULL)
			break;
		memset(buf, 0, tmpres);
	}
	cout << "Welcom...\n\n";

	//LOGIN
	string info;
	//send username
	cout << "User: ";
	getline(cin, info);
	cin.sync();

	cout << sendCommand("USER", info.c_str(), 331);

	//send pass
	cout << "Pass: ";
	getline(cin, info);
	cin.sync();

	cout << sendCommand("PASS", info.c_str(), 331);

	return 1;
}

void FTPClient::list(const char* cmd, const char* fname)
{
	if (actOrPas == 1)
	{
		if (!sendActPort())
			return;
	}
	else if (actOrPas == 2)
	{
		if (!openPASV())
			return;
	}

	//send command
	if (strcmp(cmd, "dir") == 0)
		sendCommand("NLST", "", 150);
	else if (strcmp(cmd, "ls") == 0)
		sendCommand("LIST", fname, 150);
	//get code to check
	sscanf(buf, "%d", &codeftp);
	if (codeftp != 150)
		return;

	//list
	if (actOrPas == 1)
	{
		if (!ClientSocketAct.Accept(Data))
		{
			cout << "Can't get data...\n";
			return;
		}
		memset(buf, 0, sizeof buf);
		Data.Receive(buf, BUFSIZ, 0);
		cout << buf;
		Data.Close();
		ClientSocketAct.Close();
	}
	else if (actOrPas == 2)
	{
		memset(buf, 0, sizeof buf);
		ClientSocketPas.Receive(buf, BUFSIZ, 0);
		cout << buf;
		ClientSocketPas.Close();
	}

	//cout reply
	memset(buf, 0, sizeof buf);
	ClientSocket.Receive(buf, sizeof buf);
	cout << buf;
}

//get extension
string getFileExtension(string fname)
{
	int tmp = -1;//check "." is exists
	tmp = fname.find('.');
	if (tmp == -1)//this is folder
		return "";
	fname = fname.substr(fname.find_last_of("."));//ex: get extension .txt
	fname.insert(0, "*");//ex: *.txt
	return fname;
}

void FTPClient::upload(const char* path)
{
	//file exists??
	if (access(path, 0))
	{
		cout << "File not found...\n";
		return;
	}
	string fname(path);
	char buf[BUFSIZ];
	memset(buf, 0, sizeof buf);
	//get file name from the path
	fname = fname.substr(fname.find_last_of("\\") + 1);

	if (actOrPas == 1)
	{
		if (!sendActPort())
			return;
	}
	else if (actOrPas == 2)
	{
		if (!openPASV())
			return;
	}
	//send type to put file image
	if (getFileExtension(fname) == ".jpg")
		cout << sendCommand("TYPE I", "", 200);

	//send commnad
	cout << sendCommand("STOR", fname.c_str(), 150);
	if (codeftp != 150 && codeftp != 125)
		return;

	//put data to server
	ifstream fin(fname, ios::binary);
	if (fin.is_open())
	{
		if (actOrPas == 1)
		{
			if (!ClientSocketAct.Accept(Data))
			{
				cout << "Can't get data...\n";
				return;
			}
		}
		while (!fin.eof())
		{
			fin.read(buf, sizeof buf);
			int sizeReaded = fin.gcount();
			if (actOrPas == 1)
			{
				Data.Send(buf, sizeReaded);
			}
			else if (actOrPas == 2)
			{
				ClientSocketPas.Send(buf, sizeReaded);
			}
			memset(buf, 0, sizeof buf);
		}
	}
	else
	{
		cout << "File not found..." << endl;
		//550
		ClientSocket.Receive(buf, sizeof buf);
		cout << buf;
		return;
	}

	//close socket
	if (actOrPas == 1)
	{
		Data.Close();
		ClientSocketAct.Close();
	}
	else if (actOrPas == 2)
		ClientSocketPas.Close();

	//cout reply
	ClientSocket.Receive(buf, sizeof buf);
	cout << buf;

}

void FTPClient::download(const char* fname)
{
	if (actOrPas == 1)
	{
		if (!sendActPort())
			return;
	}
	else if (actOrPas == 2)
	{
		if (!openPASV())
			return;
	}

	//send type to put file image
	if (getFileExtension(fname) == ".jpg")
		cout << sendCommand("TYPE I", "", 200);
	//send commnad 
	cout << sendCommand("RETR", fname, 150);
	sscanf(buf, "%d", &codeftp);
	if (codeftp != 150)
		return;
	memset(buf, 0, sizeof buf);

	//get data from server
	ofstream fout(fname, ios::binary);
	if (fout.is_open())
	{
		int len;
		if (actOrPas == 1)
		{
			if (!ClientSocketAct.Accept(Data))
			{
				cout << "Can't get data...\n";
				return;
			}
			len = Data.Receive(buf, sizeof buf);
			Data.Close();
			ClientSocketAct.Close();
		}
		else if (actOrPas == 2)
		{
			len = ClientSocketPas.Receive(buf, sizeof buf);
			ClientSocketPas.Close();
		}
		fout.write(buf, len);
	}
	else
	{
		cout << "File not found..." << endl;
		//550
		memset(buf, 0, sizeof buf);
		ClientSocket.Receive(buf, sizeof buf);
		cout << buf;
		return;
	}

	//cout reply
	memset(buf, 0, sizeof buf);
	ClientSocket.Receive(buf, sizeof buf);
	cout << buf;
}

void FTPClient::uploadMFile(const char* ftype)
{
	DIR *pDir;
	struct dirent *entry;
	//get the current directory
	wchar_t current_path[1024];
	GetCurrentDirectory(1024, current_path);
	//convert lpwstr to char*
	char path[1024];
	wcstombs(path, current_path, 1024);
	//get file list
	string sel;//upload this file if enter y or yes
	//read all file name and upload
	if (pDir = opendir(path))
	{
		while (entry = readdir(pDir))
		{
			if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
			{
				string tmp = getFileExtension((string)entry->d_name);//get file extension
				if (strcmp(tmp.c_str(), ftype) == 0)//compare with ftype
				{
					cout << "mput " << entry->d_name << "? ";
					cin >> sel;
					if (sel == "y" || sel == "yes")
					{
						char tmp_path[255];
						sprintf(tmp_path, "%s\\%s", path, entry->d_name);//create path
						upload(tmp_path);
					}
				}
			}
		}
	}
}

void FTPClient::dowMFile_mget_AndDelMFile_mdele(string cmd)
{
	if (actOrPas == 1)
	{
		if (!sendActPort())
			return;
		//send commnad
		sendCommand("NLST", "", 150);
		if (!ClientSocketAct.Accept(Data))
		{
			cout << "Can't get data...\n";
			return;
		}
		//get data
		memset(buf, 0, BUFSIZ);
		Data.Receive(buf, BUFSIZ, 0);
		Data.Close();
		ClientSocketAct.Close();
	}
	else if (actOrPas == 2)
	{
		if (!openPASV())
			return;
		//send commad
		sendCommand("NLST", "", 150);
		//get data
		memset(buf, 0, sizeof buf);
		ClientSocketPas.Receive(buf, BUFSIZ, 0);
		ClientSocketPas.Close();
	}
	string str(buf), sel;//download this file if enter y or yes

	//get all reply from server
	memset(buf, 0, BUFSIZ);
	ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf;


	while (str.length() > 0)
	{
		string fname = str.substr(0, str.find_first_of("\r\n"));
		str.erase(0, fname.length() + 2);//delete file name and 2 characters: \r\n
		if (getFileExtension(fname) == "")
			continue;
		cout << cmd << " " << fname << "? ";
		cin >> sel;
		if (sel == "y" || sel == "yes")
		{
			if (cmd == "mget")
				download(fname.c_str());
			else if (cmd == "mdele" || cmd == "mdelete")
				delFile(fname.c_str());
		}
	}
}

void FTPClient::changeCurtDictServer(const char* path)
{
	cout << sendCommand("CWD", path, 1024);
}

void FTPClient::changeCurtDictClient(const char* path)
{
	wchar_t dest[1024];
	mbstowcs(dest, path, 1024);//convert char* to LPWSTR
	//set the current directory under the client
	if (!SetCurrentDirectory(dest))
	{
		cout << "Can't set current directory under the client...\n";
		return;
	}
	wchar_t current_path[1024];
	//get the current directory to print on cmd
	GetCurrentDirectory(1024, current_path);
	wprintf(L"Local directory now: %s.\n", current_path);
}

void FTPClient::delFile(const char* fname)
{
	cout << sendCommand("DELE", fname, 250);
}

void FTPClient::createFolder(const char* fdname)
{
	cout << sendCommand("XMKD", fdname, 257);
}

void FTPClient::delEmptyFolder(const char* fdname)
{
	cout << sendCommand("XRMD", fdname, 250);
}

void FTPClient::currentDirectory()
{
	cout << sendCommand("XPWD", "", 257);
}

void FTPClient::passiveMode()
{
	cout << sendCommand("pasv", "", 221);
}

void FTPClient::quit()
{
	cout << sendCommand("QUIT", "", 221);
}