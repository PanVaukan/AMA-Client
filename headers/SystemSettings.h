
#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <winsock2.h>
#include <tchar.h>
#pragma comment (lib, "ws2_32.lib")
#include "../headers/SearchOptions.h"

using namespace std;

class SystemSettings
{
private:

int client_id;
int status_free;
int status_download;
int status_in_progress;
int status_pause;
int status_done;
int status_offline;
int status_reserved;

unsigned long long sizeOfBuffer; 

char ip_address[128];
char fileNameFromSubtasksTable[32];
char fullPathForDownload[512];
string localPrefixForDataPath;
string remotePrefixForDataPath;
long long  volumeOfCache;
long long currSizeOfCache;
long long  freeSpaceMb;
string host;
int port;
string database;
string user;
string passwd;

WSADATA wsaData;
SYSTEMTIME systime;
char strTime[256];

public:
	SystemSettings(void);
	~SystemSettings(void);
	
	friend class MySQLServer;
    friend class SearchOptions;		

	void wait(long seconds);
	void resetSettings();
	void getIPaddress();
	int getPathPrefixesForData();
	bool checkFreeSpace();
	bool GetFileSize64(LPCTSTR lpszPath, PLARGE_INTEGER lpFileSize);
	bool GetFolderSize(LPCTSTR lpszStartFolder, 
				       BOOL bRecurse, 
				       BOOL bQuickSize,
				       PLARGE_INTEGER lpFolderSize,
				       LPDWORD lpFolderCount /*= NULL*/,
				       LPDWORD lpFileCount /*= NULL*/);
	void getFileDateTime(string filename);
	bool deleteOldFilesForDownloadNewFile(LPCTSTR lpszStartFolder, 
				           BOOL &bRecurse, 				   
				           PLARGE_INTEGER lpFolderSize, long long sizeOfFileForDownload); 	  
	void deleteOldFiles();
	void getVolumeCache();
	bool checkSizeOfCache();
	void prepareCacheForDownload(SearchOptions& SearchArgs);
	void setPathForSearch(SearchOptions& SearchArgs); 
	bool FileExistsInCache(SearchOptions SearchArgs);
	bool CheckExistingFile(SearchOptions& SearchArgs);
	bool DirecoryExistsInCache();
	bool CreateLocalDirectoryForCache();
	bool CreateFoldersForCaching(SearchOptions SearchArgs);
	bool CopyFileFromNetwork(SearchOptions SearchArgs);
	void ShowInformationAboutCurrSubtask(SearchOptions SearchArgs);
   //bool FileExists(const char *fname);

};

