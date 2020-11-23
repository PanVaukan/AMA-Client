
#include "../headers/SystemSettings.h"
#include <direct.h>
#include <conio.h>
#include <tchar.h>
#include <crtdbg.h>
#include <tchar.h>


#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4996)		// bogus deprecation warning
//#define _UNICODE

SystemSettings::SystemSettings(void)
{
	SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
	
	client_id = 0;
	ip_address[0] ='\0';
	database="";
	localPrefixForDataPath="";
	remotePrefixForDataPath="";
	memset(ip_address,'\0',128);
	memset(fileNameFromSubtasksTable,'\0',32);
	memset(fullPathForDownload,'\0',512);
	fileNameFromSubtasksTable[0] ='\0';
	fullPathForDownload[0] = '\0';
	passwd = "";
	memset(strTime,'\0',255);
	strTime[0] = '\0';
	user ="";
	

    status_free = 1;
    status_download = 2;
    status_in_progress = 3;
    status_pause = 4;
    status_done = 5;
    status_offline = 6;
	status_reserved = 7;

	sizeOfBuffer = 10485760;
	currSizeOfCache = 0;
	volumeOfCache = 0;
	freeSpaceMb = 0;
	
}


SystemSettings::~SystemSettings(void)
{
	// delete []ip_address;
}

void SystemSettings::wait(long seconds)
{
	seconds = seconds * 1000;
	Sleep(seconds);
	std::cout << "Waiting for new task..." << endl;
	GetLocalTime(&systime);
	cout << "Current Time: ";
    sprintf(strTime, "%d:%d:%02d", systime.wHour, systime.wMinute, systime.wSecond);
    std::cout << strTime << endl;
}

void SystemSettings::resetSettings()
{
  sizeOfBuffer = 2097152;
  fileNameFromSubtasksTable[0] ='\0';
  fullPathForDownload[0] = '\0';
  strTime[0] = '\0';
}

void SystemSettings::getIPaddress()
{
 const int WSVer = 0x101;
 hostent *h = NULL;
 char Buf[128]={0};
 Buf[0] = '\0';
  if (WSAStartup(WSVer, &wsaData) == 0) 
   {
    if (gethostname(&Buf[0], 128) == 0)
     { 
       h = gethostbyname(&Buf[0]);
        if (h != NULL)
		{
        cout << "Your ip address: ";
		cout << inet_ntoa (*(reinterpret_cast<in_addr *>(*(h->h_addr_list)))) << endl;
		memset(ip_address,'\0',128);
		strcpy(ip_address,inet_ntoa (*(reinterpret_cast<in_addr *>(*(h->h_addr_list)))));
		ip_address[strlen(ip_address)+1] = '\0';
		
		
	  //return inet_ntoa (*(reinterpret_cast<in_addr *>(*(h->h_addr_list))));
		}
		else
		{
		cout <<"You aren't online and you haven't ip-adress";
	 //return NULL;
		}
     }
    WSACleanup();
  }
}

int SystemSettings::getPathPrefixesForData()
{
 char str[256]={0};

 ifstream inFile("config.txt",ios::in);
 inFile.getline(str,255,'\n');
 memset(str,'\0',255);
 inFile.getline(str,255,'\n');
 localPrefixForDataPath = str;
 memset(str,'\0',255);
 inFile.getline(str,255,'\n');
 memset(str,'\0',255);
 inFile.getline(str,255,'\n');
 sscanf(str,"%lld",&volumeOfCache);
 memset(str,'\0',255);
 inFile.getline(str,255,'\n');
 memset(str,'\0',255);
 inFile.getline(str,255,'\n');
 remotePrefixForDataPath = str;
 memset(str,'\0',255);
 inFile.getline(str,255,'\n');
 memset(str,'\0',255);
 inFile.getline(str,255,'\n');
 host = str;
 memset(str,'\0',255);
 inFile.getline(str,255,'\n');
 memset(str,'\0',255);
 inFile.getline(str,255,'\n');
 sscanf(str,"%d",&port);
 memset(str,'\0',255);
 inFile.getline(str,255,'\n');
 memset(str,'\0',255);
 inFile.getline(str,255,'\n');
 database = str;
 memset(str,'\0',255);
 inFile.getline(str,256,'\n');
 memset(str,'\0',255);
 inFile.getline(str,255,'\n');
 user = str;
 memset(str,'\0',255);
 inFile.getline(str,255,'\n');
 memset(str,'\0',255);
 inFile.getline(str,255,'\n');
 passwd = str;
 memset(str,'\0',255);

 inFile.close();

 return 1;
}

bool SystemSettings::checkFreeSpace()
{
  DWORD dwSecPerClus = 0;
  DWORD dwBytesPerSec = 0;
  DWORD dwFreeClus = 0; 
  DWORD dwTotalClus = 0;

  if (!GetDiskFreeSpace(localPrefixForDataPath.c_str(),&dwSecPerClus,&dwBytesPerSec,&dwFreeClus,&dwTotalClus))
  return false;
  else
  {
  freeSpaceMb = (((long long)dwSecPerClus * (long long)dwBytesPerSec * (long long)dwFreeClus) / 1048576);
  return true;
  }
  
}

bool SystemSettings::GetFileSize64(LPCTSTR lpszPath, PLARGE_INTEGER lpFileSize)
{
  BOOL rc = FALSE;

  _ASSERTE(lpszPath);
  _ASSERTE(lpFileSize);

 if (lpszPath && lpFileSize)
 {
  lpFileSize->QuadPart = 0;
  HANDLE hFile = NULL;

  hFile = CreateFile(lpszPath, READ_CONTROL, 0, NULL, OPEN_EXISTING, 0, NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			rc = GetFileSizeEx(hFile, lpFileSize);
			CloseHandle(hFile);
		}
	}
	return rc;
}

bool SystemSettings::GetFolderSize(LPCTSTR lpszStartFolder, 
				       BOOL bRecurse, 
				       BOOL bQuickSize,
				       PLARGE_INTEGER lpFolderSize,
				       LPDWORD lpFolderCount /*= NULL*/,
				       LPDWORD lpFileCount /*= NULL*/)
{

	BOOL rc = FALSE;

	_ASSERTE(lpszStartFolder);
	_ASSERTE(lpFolderSize);

	if (lpszStartFolder || lpFolderSize)
	{
		lpFolderSize->QuadPart = 0;
		if (lpFolderCount)
			*lpFolderCount = 0;
		if (lpFileCount)
			*lpFileCount = 0;

		if (bQuickSize && (_tcslen(lpszStartFolder) < 4))
		{
			// get used space for entire volume

			ULARGE_INTEGER ulFreeCaller;
			ulFreeCaller.QuadPart = 0;
			ULARGE_INTEGER ulTotal;
			ulTotal.QuadPart = 0;
			ULARGE_INTEGER ulFree;
			ulFree.QuadPart = 0;

			rc = SHGetDiskFreeSpace(lpszStartFolder, &ulFreeCaller, &ulTotal, &ulFree);
			lpFolderSize->QuadPart = ulTotal.QuadPart - ulFree.QuadPart;

			return rc;
		}

		HANDLE hFF = 0;
		WIN32_FIND_DATA wfd = { 0 };
		TCHAR szPath[2000];
		LARGE_INTEGER li_total;
		li_total.QuadPart = 0;
		DWORD dwTotalFolderCount = 0;
		DWORD dwTotalFileCount = 0;

		_tcsncpy(szPath, lpszStartFolder, sizeof(szPath)/sizeof(TCHAR)-1);
		_tcscat(szPath, _T("\\*"));
		
		hFF = FindFirstFile(szPath, &wfd);
		
		if (hFF != INVALID_HANDLE_VALUE)
		{
			rc = TRUE;

			do
			{
				_tcsncpy(szPath, lpszStartFolder, sizeof(szPath)/sizeof(TCHAR)-1);
				_tcscat(szPath, _T("\\"));
				_tcscat(szPath, wfd.cFileName);

				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (_tcscmp(wfd.cFileName, _T(".")) &&
						_tcscmp(wfd.cFileName, _T("..")) &&
						bRecurse)
					{
						// recurse into the subdirectory
						
						LARGE_INTEGER li;
						li.QuadPart = 0;
						DWORD dwFolderCount = 0;
						DWORD dwFileCount = 0;
						rc = GetFolderSize(szPath, bRecurse, bQuickSize, &li, 
									&dwFolderCount, &dwFileCount);
						li_total.QuadPart += li.QuadPart;
						dwTotalFolderCount += dwFolderCount + 1;
						dwTotalFileCount += dwFileCount;
					}
				}
				else
				{
					LARGE_INTEGER li;
					li.QuadPart = 0;
					rc = GetFileSize64(szPath, &li);
					li_total.QuadPart += li.QuadPart;
					dwTotalFileCount += 1;
				}

			} while (FindNextFile(hFF, &wfd));
			// CloseHandle(hFF);
			
		}

		*lpFolderSize = li_total;
		currSizeOfCache = ( li_total.QuadPart / 1024 ) / 1024; 
		if (lpFolderCount)
			*lpFolderCount = dwTotalFolderCount;
		if (lpFileCount)
			*lpFileCount = dwTotalFileCount;
		
	}

	
	return rc;
}

void SystemSettings::getFileDateTime(string filename)
{
  
  HANDLE fH;
  FILETIME creationTime;
  SYSTEMTIME sysTime;
  fH = CreateFile(filename.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if(fH != INVALID_HANDLE_VALUE)
  {
        GetFileTime(fH, &creationTime, 0, 0);
        FileTimeToSystemTime(&creationTime, &sysTime);
        std::cout<<sysTime.wDay<<"."<<sysTime.wMonth<<"."<<sysTime.wYear<<
			" "<<sysTime.wHour<<":"<<sysTime.wMinute<<std::endl;
        CloseHandle(fH);
 
  }
 }

bool SystemSettings::deleteOldFilesForDownloadNewFile(LPCTSTR lpszStartFolder, 
				       BOOL &bRecurse, 				   
				       PLARGE_INTEGER lpFolderSize,long long sizeOfFileForDownload)     
{
	BOOL rc = FALSE;
   _ASSERTE(lpszStartFolder);
  	HANDLE hFF = 0;
	WIN32_FIND_DATA wfd = { 0 };
	TCHAR szPath[2000];
	LARGE_INTEGER li_total;
	li_total.QuadPart = 0;

   _tcsncpy(szPath, lpszStartFolder, sizeof(szPath)/sizeof(TCHAR)-1);
   _tcscat(szPath, _T("\\*"));

   	hFF = FindFirstFile(szPath, &wfd);
		
		if (hFF != INVALID_HANDLE_VALUE)
		{
			rc = TRUE;

			do
			{
				_tcsncpy(szPath, lpszStartFolder, sizeof(szPath)/sizeof(TCHAR)-1);
				_tcscat(szPath, _T("\\"));
				_tcscat(szPath, wfd.cFileName);

				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (_tcscmp(wfd.cFileName, _T(".")) &&
						_tcscmp(wfd.cFileName, _T("..")) &&
						bRecurse)
					{
						// recurse into the subdirectory		
						LARGE_INTEGER li;
     					rc = deleteOldFilesForDownloadNewFile(szPath, bRecurse,&li,sizeOfFileForDownload);
					}
				}
				else
				{
					LARGE_INTEGER li;
					li.QuadPart = 0;
					rc = GetFileSize64(szPath, &li);
					currSizeOfCache -= (li.QuadPart / 1024) / 1024;
					DeleteFile(szPath);
					if ( volumeOfCache - currSizeOfCache -1 >= sizeOfFileForDownload )
					{
					 	bRecurse = FALSE;
			   		}
					
				}

			} while (FindNextFile(hFF, &wfd) && (bRecurse));
		}

		return rc;
}

void SystemSettings::getVolumeCache()
{
	LARGE_INTEGER lpFolderSize;
	DWORD lpFolderCount = 0 ;
	DWORD lpFileCount = 0 ; 

	GetFolderSize(localPrefixForDataPath.c_str(), TRUE, TRUE, &lpFolderSize, &lpFolderCount, &lpFileCount);

}

void SystemSettings::deleteOldFiles()
{	
	BOOL bRecurse = TRUE;
	LARGE_INTEGER li;
	long long sizeOfFileForDownload;
	GetFileSize64(fullPathForDownload,&li);
	sizeOfFileForDownload = (li.QuadPart / 1024) /1024;
	deleteOldFilesForDownloadNewFile(localPrefixForDataPath.c_str(), bRecurse , &li , sizeOfFileForDownload);

}

bool SystemSettings::checkSizeOfCache()
{

  checkFreeSpace();
  getVolumeCache();
 
  if ( freeSpaceMb < 2048 )
  {
    currSizeOfCache = 0;
    return false;
  }
  else
  {
  
  if ( freeSpaceMb - volumeOfCache < 1024 )
  {
	  return false;
  }

  return true;

  }
  
}

void SystemSettings::prepareCacheForDownload(SearchOptions& SearchArgs)
{
  LARGE_INTEGER li;
  long long sizeOfFileForDownload;
  GetFileSize64(fullPathForDownload,&li);
  sizeOfFileForDownload = (li.QuadPart / 1024) /1024;
  SearchArgs.sizeOfCurrFile = sizeOfFileForDownload;

  if ( volumeOfCache - currSizeOfCache < sizeOfFileForDownload )
  {
     deleteOldFiles();
  }

}

bool SystemSettings::CheckExistingFile(SearchOptions& SearchArgs)
{

  LARGE_INTEGER li;
  long long sizeOfFileForDownload = 0;
  long long sizeOfExistingFileInCache = 0;
  GetFileSize64(fullPathForDownload,&li);
  sizeOfFileForDownload = (li.QuadPart / 1024) / 1024;
  GetFileSize64(SearchArgs.fullPathToFileForSearch,&li);
  sizeOfExistingFileInCache = (li.QuadPart / 1024) / 1024;
  SearchArgs.sizeOfCurrFile = sizeOfFileForDownload;

  if ( sizeOfExistingFileInCache = sizeOfFileForDownload )
  return true;
  else
  {
    DeleteFile(SearchArgs.fullPathToFileForSearch);
    return false;
  }

}

void SystemSettings::setPathForSearch(SearchOptions& SearchArgs)
{
	int inc = 0;
	char *partOfFileName = NULL;
	char tempFileNameFromSubtasksTable[32] = {0};
	char yy[3]={0};

	tempFileNameFromSubtasksTable[0] = '\0';
	memset(fullPathForDownload,'\0',512);
	memset(SearchArgs.fullPathToFileForSearch,'\0',512);
	SearchArgs.fullPathToFileForSearch[0]='\0';
	yy[0] = '\0';   
	
	strcat(tempFileNameFromSubtasksTable,SearchArgs.fileNameFromSubtasksTable);
	strcat(SearchArgs.fullPathToFileForSearch,localPrefixForDataPath.c_str());
	//SearchArgs.fullPathToFileForSearch[localPrefixForDataPath.size()] = '\0';
	strcat(fullPathForDownload,remotePrefixForDataPath.c_str());
	
	partOfFileName = strtok(tempFileNameFromSubtasksTable,"-");

    while (partOfFileName != NULL)
    {
	 if ( inc == 0 )
	  {
		  yy[0]=partOfFileName[2];
		  yy[1]=partOfFileName[3];
		  yy[2]='\0';
		  strcat(SearchArgs.fullPathToFileForSearch,"\\");
		  strcat(fullPathForDownload,"\\");
	  }
      strcat(SearchArgs.fullPathToFileForSearch,partOfFileName);
	  strcat(fullPathForDownload,partOfFileName);
	  if  ( inc == 1)
	  {
	    strcat(SearchArgs.fullPathToFileForSearch,yy);
		strcat(fullPathForDownload,yy);
	  }
      partOfFileName = strtok (NULL, "-");
	  if ( partOfFileName != NULL)
	  {
        strcat(SearchArgs.fullPathToFileForSearch,"\\");
		strcat(fullPathForDownload,"\\");
	  }
	  else
	  {
       strcat(SearchArgs.fullPathToFileForSearch,".ama");
	   strcat(fullPathForDownload,".ama");

	  }
	  inc++;
	}

	  //position = strstr(fullPathToFile,strtok(_fileNameFromSubtasksTable,"-"));
	  //int offset = (int) (position-fullPathToFile);
	  /*
	       Как можно было легче сделать это через переменную типа std::string:
	  fileNameForSearch = prefixForDataPath + fileNameFromSubtasksTable.substr(0,4)+"\\";
	  fileNameForSearch += fileNameFromSubtasksTable.substr(5,2);
	  fileNameForSearch += fileNameFromSubtasksTable.substr(2,2)+"\\";
	  fileNameForSearch += fileNameFromSubtasksTable.substr(8,2)+".ama";   
	  */	
}

bool SystemSettings::FileExistsInCache(SearchOptions SearchArgs)
{
	return  std::ifstream(SearchArgs.fullPathToFileForSearch,ios::in | ios::_Nocreate) !=NULL;
}

bool SystemSettings::DirecoryExistsInCache()
{
	if (chdir(localPrefixForDataPath.c_str()) !=0)
	{
		return false;
	}
	else return true;
}

bool SystemSettings::CreateLocalDirectoryForCache()
{
 char *tempPath;
 char *tempPartOfPath;
 char discTitle[4]={0};
 char Path[512]= {0};
 char checkPaths[512]={0};
 checkPaths[0] = '\0';
 discTitle[0] = '\0';
 Path[0] = '\0';
 strncpy(discTitle,localPrefixForDataPath.c_str(),3);
 discTitle[3] = '\0';

  if (chdir(discTitle) == -1 )
   return false;
 
 strcpy(Path,localPrefixForDataPath.c_str()); 
 tempPath = Path; 

 tempPartOfPath = strtok(tempPath,"\\");
  while (tempPartOfPath != NULL)
  { 
   strcat(checkPaths,tempPartOfPath);

    if (chdir(checkPaths) != 0 )
     mkdir(checkPaths);
 
   strcat(checkPaths,"\\");
   tempPartOfPath = strtok (NULL, "\\");

  }
 
 return true;
}

bool SystemSettings::CreateFoldersForCaching(SearchOptions SearchArgs)
{
 char *tempPath;
 char *tempPartOfPath;
 char setPaths[512] = {0};
 setPaths[0] = '\0';

 tempPath = SearchArgs.fullPathToFileForSearch;

 tempPartOfPath = strtok(tempPath,"\\");
  while (tempPartOfPath != NULL)
  { 
   strcat(setPaths,tempPartOfPath);

    if ((int)strstr(setPaths,"ama")!=0)
    break;

    if (chdir(setPaths) != 0 )
     mkdir(setPaths);
 
   strcat(setPaths,"\\");
   tempPartOfPath = strtok (NULL, "\\");

  }

 return true;
}

bool SystemSettings::CopyFileFromNetwork(SearchOptions SearchArgs)
{
 unsigned long long fileSize = 0;
 char* buffer = new char[sizeOfBuffer];

 ifstream source(fullPathForDownload, ios::in | ios::binary);
  if (!source)
  { 
   source.close(); 
   std::cout << "Can\'t open file: " << source << endl; 
   return false;
  }

 ofstream destination(SearchArgs.fullPathToFileForSearch, ios::out | ios::binary);
  if (!destination)
  {  
   destination.close(); 
   std::cout << "Can\'t open file: " << destination << endl; 
   return false;
  }

   source.seekg(0,ios::end);
   fileSize = source.tellg();
   source.seekg(0,ios::beg);

   cout << " StartTime download file: " << endl;
   GetLocalTime(&systime);
   sprintf(strTime, "%d:%d:%02d", systime.wHour, systime.wMinute, systime.wSecond);
   std::cout << strTime << endl;

   while (!source.eof())
   {

    if ( (sizeOfBuffer + source.tellg()) > fileSize )
    {
     sizeOfBuffer = fileSize - source.tellg();

     if ( sizeOfBuffer == 0)
     break;

     source.read(buffer,sizeOfBuffer); 
     destination.write(buffer,sizeOfBuffer);

	}
    else
    {
      source.read(buffer,sizeOfBuffer);
      destination.write(buffer,sizeOfBuffer);
    }

    if ( sizeOfBuffer == 0) 
    break;

   }

    source.close();
    destination.close();
	delete []buffer; 

	cout << " EndTime download file: " << endl;
	GetLocalTime(&systime);
    sprintf(strTime, "%d:%d:%02d", systime.wHour, systime.wMinute, systime.wSecond);
    std::cout << strTime << endl;

	return true;
}

void SystemSettings::ShowInformationAboutCurrSubtask(SearchOptions SearchArgs)
{
	system("cls");
	cout << "___________________________________ " << endl;
	cout << " Information about current task: " << endl;
	cout << "___________________________________ " << endl;
	cout << " Number of Task: " << SearchArgs.subtask_id << endl;
	cout << " Name of user: " << endl;
	cout << " Registration time of task: " << endl;
	if ( SearchArgs.phoneNumberA != 0)
	cout << " PhoneNumber A: " << SearchArgs.phoneNumberA << endl;
	else cout << " PhoneNumber A: " << "-" << endl;
	if ( SearchArgs.phoneNumberB != 0)
	cout << " PhoneNumber B: " << SearchArgs.phoneNumberB << endl;
	else cout << " PhoneNumber B: " << "-" << endl;
	cout << " Answer: " << SearchArgs.answer << endl;
	cout << " First Search Date: " << SearchArgs.firstSearchDate << endl;
	cout << " Last Search Date: " << SearchArgs.lastSearchDate << endl;
	cout << "___________________________________ " << endl;
	cout << " Information about current subtask: " << endl;
	cout << "___________________________________ " << endl;
	cout << " Filename: " << SearchArgs.fileNameFromSubtasksTable << endl;
	cout << " Size of File: " << SearchArgs.sizeOfCurrFile << "MB" << endl;
	cout << "___________________________________ ";
	

}
