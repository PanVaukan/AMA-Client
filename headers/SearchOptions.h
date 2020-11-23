
#pragma once
#include <string>
#include <vector>
#include "../headers/DataProcessing.h"
#include "../headers/MySQLServer.h"

using std::string;
using std::vector;

class SearchOptions
{

private:

	int subtask_id;
	string localPrefixForDataPath;
    string remotePrefixForDataPath;
	char fileNameFromSubtasksTable[32];
	char  fullPathToFileForSearch[512];

    int answer;
	int firstSearchYear;
	char _firstSearchYear[4];
	int firstSearchMonth;
	char _firstSearchMonth[2];
	int firstSearchDay;
	char _firstSearchDay[2];
	int lastSearchYear;
	char _lastSearchYear[4];
	int lastSearchMonth;
	char _lastSearchMonth[2];
	int lastSearchDay;
	char _lastSearchDay[2];
	unsigned long long phoneNumberA;
	char _phoneNumberA[64];
	unsigned long long phoneNumberB;
	char _phoneNumberB[64];

	char firstSearchDate[11];
	char lastSearchDate[11];
	long long sizeOfCurrFile;

	vector <string> allFileNames;
    vector <string> allNeedFileNamesForSearch;
    vector <int> allSubtaskIds;
	vector <int> allNeedSubtasksIds;

public:

	SearchOptions(void);
	~SearchOptions(void);

	friend class SystemSettings;
	friend class MySQLServer;
	friend class DataProcessing;
	
	void resetSearchArgs();
	bool getNeedSubtasks(SystemSettings ClientSettings);
	bool getNextSubtask(); 
	char* getfullPathToFileForSearch();
	bool checkRecord(DataProcessing& DataFromBinFile);
		
};

