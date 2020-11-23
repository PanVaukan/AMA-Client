
#include "../headers/SearchOptions.h"

#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4996)		// bogus deprecation warning

SearchOptions::SearchOptions(void)
{ 
	subtask_id = 0;
	memset(fileNameFromSubtasksTable,'\0',32);
	memset(fullPathToFileForSearch,'\0',512);
	fileNameFromSubtasksTable[0] = '\0';
	fullPathToFileForSearch[0] ='\0';

     answer = -1; 
	 firstSearchYear = 0;
	 firstSearchMonth = 0;
	 firstSearchDay = 0;
	 lastSearchYear = 0;
	 lastSearchMonth = 0;
	 lastSearchDay = 0;
	 phoneNumberA = 0;
	 phoneNumberB = 0;

	 memset(_firstSearchYear, '\0',4);
	 memset(_firstSearchMonth, '\0',2);
	 memset(_firstSearchDay, '\0',2);
	 memset(_lastSearchDay, '\0',4);
	 memset(_lastSearchDay, '\0',2);
	 memset(_lastSearchDay, '\0',2);
	 memset(_phoneNumberA, '\0',64);
	 memset(_phoneNumberB, '\0',64);
	 memset(firstSearchDate, '\0',11);
	 memset(lastSearchDate, '\0',11);

	 _firstSearchYear[0]='\0';
	 _firstSearchMonth[0] ='\0';
	 _firstSearchDay[0] ='\0';
	 _lastSearchYear[0] ='\0';
	 _lastSearchMonth[0] ='\0';
	 _lastSearchDay[0]='\0';
	 _phoneNumberA[0]='\0';
	 _phoneNumberB[0]='\0';
	  firstSearchDate[0]='\0';
	  lastSearchDate[0]='\0';

	 allFileNames.clear();
}


SearchOptions::~SearchOptions(void)
{
}

void SearchOptions::resetSearchArgs()
{
 //subtask_id = 0;
 fullPathToFileForSearch[0] ='\0';

 answer = -1; 
 firstSearchYear = 0;
 firstSearchMonth = 0;
 firstSearchDay = 0;
 lastSearchYear = 0;
 lastSearchMonth = 0;
 lastSearchDay = 0;
 phoneNumberA = 0;
 phoneNumberB = 0;
 sizeOfCurrFile = 0;

 memset(_firstSearchYear,'\0',4);
 memset(_firstSearchMonth,'\0',2);
 memset(_firstSearchDay,'\0',2);
 memset(_lastSearchYear,'\0',4);
 memset(_lastSearchMonth,'\0',2);
 memset(_lastSearchDay,'\0',2);
 memset(_phoneNumberA,'\0',64);
 memset(_phoneNumberB,'\0',64);
 memset(firstSearchDate,'\0',11);
 memset(lastSearchDate,'\0',11);

 _firstSearchYear[0]='\0';
 _firstSearchMonth[0] = '\0';
 _firstSearchDay[0] = '\0';
 _lastSearchYear[0] = '\0';
 _lastSearchMonth[0] = '\0';
 _lastSearchDay[0]='\0';
 _phoneNumberA[0]='\0';
 _phoneNumberB[0]='\0';
 firstSearchDate[0]='\0';
 lastSearchDate[0]='\0';
}

bool SearchOptions::getNeedSubtasks(SystemSettings ClientSettings)
{
  bool findSubtaskForSearch = false;
			 
  for ( unsigned int i = 0 ; i < allFileNames.size() ; i++)
  {
	strcpy(fileNameFromSubtasksTable, allFileNames[i].c_str()); 
    ClientSettings.setPathForSearch(*this);

	if (ClientSettings.FileExistsInCache(*this))
	{
		allNeedSubtasksIds.push_back(allSubtaskIds[i]);
		allNeedFileNamesForSearch.push_back(allFileNames[i]);
		findSubtaskForSearch = true;
	}

  }


  if (( !findSubtaskForSearch ) && ( !allFileNames.empty()))
	{
		
	 allNeedSubtasksIds.push_back(allSubtaskIds[0]);
	 allNeedFileNamesForSearch.push_back(allFileNames[0]);
	 findSubtaskForSearch = true;

	}

	allSubtaskIds.clear();
	allFileNames.clear();

  return findSubtaskForSearch;
}

bool SearchOptions::getNextSubtask()
{
	if (!allNeedSubtasksIds.empty())
	{
		subtask_id = allNeedSubtasksIds[0];
		strcpy(fileNameFromSubtasksTable,allNeedFileNamesForSearch[0].c_str());

		allNeedSubtasksIds.erase(allNeedSubtasksIds.begin());
		allNeedFileNamesForSearch.erase(allNeedFileNamesForSearch.begin());
		return true;

	}
  return false;
}

  char* SearchOptions::getfullPathToFileForSearch()
 {
	 return fullPathToFileForSearch;
 }

  bool SearchOptions::checkRecord(DataProcessing& DataFromBinFile)
  {

	  if ( phoneNumberA != 0)
	  if ( DataFromBinFile.phoneNumberA != phoneNumberA )

		   return false;

	  if ( phoneNumberB != 0)
	  if ( DataFromBinFile.phoneNumberB != phoneNumberB )

		   return false;

	  
	  if ( answer == -1 ) return true;
      if ( DataFromBinFile.answer != answer )
      return false;
      

		return true;
  }
  