
#include "../headers/SystemSettings.h"
#include "../headers/MySQLServer.h"
#include "../headers/DataProcessing.h"
#include "../headers/SearchOptions.h"
#include <conio.h>

int main()
{
	SystemSettings ClientSettings;
	MySQLServer MySQLProvider;	
	SearchOptions SearchArgs;
	DataProcessing Search;

	ClientSettings.getIPaddress();
	ClientSettings.getPathPrefixesForData();

	if (!ClientSettings.DirecoryExistsInCache())
	ClientSettings.CreateLocalDirectoryForCache();
	
	MySQLProvider.initConnection();

   if (MySQLProvider.databaseConnection(ClientSettings))
 if (MySQLProvider.serverLogin(ClientSettings))
{
MySQLProvider.updateStatusOnlineOfClient(ClientSettings);
while (true)
{

  MySQLProvider.checkAllClients(ClientSettings);
  MySQLProvider.setStatusFreeForAllSubtasksClientsOfWichInOffline(ClientSettings);

  if ( ClientSettings.checkSizeOfCache() )
  {
	
    if (MySQLProvider.getSubtaskForSearch(ClientSettings,SearchArgs))
    {
		while (SearchArgs.getNextSubtask())
		{	
				MySQLProvider.getSearchArgsFromTask(ClientSettings,SearchArgs);

				ClientSettings.setPathForSearch(SearchArgs);
				ClientSettings.prepareCacheForDownload(SearchArgs);

			  if (!ClientSettings.FileExistsInCache(SearchArgs))
			  {  
				ClientSettings.CreateFoldersForCaching(SearchArgs);
				ClientSettings.CopyFileFromNetwork(SearchArgs);
				MySQLProvider.insertFileNameInCacheTable(ClientSettings,SearchArgs);
			  }
			  else
			  {
				  if ( ! ClientSettings.CheckExistingFile(SearchArgs))
				  {
					  ClientSettings.CopyFileFromNetwork(SearchArgs);
					  MySQLProvider.insertFileNameInCacheTable(ClientSettings,SearchArgs);
				  }
			  }

			  ClientSettings.ShowInformationAboutCurrSubtask(SearchArgs);
			  Search.searchData(SearchArgs,MySQLProvider);
			  MySQLProvider.setStatusDoneForCurrSubtask(ClientSettings,SearchArgs);

			  Search.resetDataParametrs();
			  ClientSettings.resetSettings();
			  SearchArgs.resetSearchArgs();

			  MySQLProvider.trySetStatusDoneForCurrTask(ClientSettings,SearchArgs);
			  MySQLProvider.updateStatusOnlineOfClient(ClientSettings);

		}
		// MySQLProvider.trySetStatusDoneForCurrTask(ClientSettings,SearchArgs);					
	}
  
    else
    {
	  ClientSettings.wait(10);  // <---- Must be 30 seconds 
	  MySQLProvider.updateStatusOnlineOfClient(ClientSettings);
	}
  }
}
 }
getch();
return 0;
}