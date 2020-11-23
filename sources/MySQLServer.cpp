
#include "../headers/MySQLServer.h"
#include <fstream>
// #define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#pragma warning(disable : 4996)		// bogus deprecation warning
// #pragma warning(disable: 4996)

MySQLServer::MySQLServer(void)
{
	field = NULL;
	result_id = 1;
	result = NULL;
}

MySQLServer::~MySQLServer(void)
{

}

int MySQLServer::initConnection()
{
  if (!(mysql=mysql_init(0)))
  {
    fprintf(stderr,"Couldn't initialize mysql struct\n");
    connectionOK = 0;
    return 0;
  }
	    
    connectionOK = 1;
    return 1;
}

int MySQLServer::databaseConnection(SystemSettings& ClientSettings)
{
	char use[] = "Use ";
	char useAmaDatabase[64] ={0};
	strcpy(useAmaDatabase,use);
	strcat(useAmaDatabase,ClientSettings.database.c_str());

    if ( (mysql = mysql_init((MYSQL*) 0)) && 
		mysql_real_connect( mysql, ClientSettings.host.c_str() ,
		ClientSettings.user.c_str(),ClientSettings.passwd.c_str(),
		ClientSettings.database.c_str(), ClientSettings.port,NULL, 0 ) )
	//    mysql_real_connect( mysql, NULL, "root","", NULL, MYSQL_PORT,NULL, 0 ) )
    {
     if (!mysql_query(mysql,useAmaDatabase))
	 {
       databaseConnectionOK = 1;
	   cout << " Ok! Connecting to the server has been successfully installed! " << endl;
	 }
	  databaseConnectionOK = 1;
      return 1;      
	}
	else
	{
    printf( "Can't connect to the mysql server on port %d !\n",MYSQL_PORT );
	fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
    //mysql_close( myData );
	databaseConnectionOK = 0;
    return 0;
    }
}

int MySQLServer::serverLogin(SystemSettings& ClientSettings)
{
 int find = 0;
 char registerOnServer[256] ={0};
 char get_ID[128] ={0};
 get_ID[0]='\0';
 registerOnServer[0]='\0';
 strcpy(registerOnServer,"SELECT COUNT(client_id) from clients where ip_address='"); 
 strcat(registerOnServer,ClientSettings.ip_address);              // here
 strcat(registerOnServer,"';");
 if (mysql_query(mysql,registerOnServer))
  {
       fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
	   return 0;
       //mysql_close(mysql);
  }
 else
 {
	 result = mysql_store_result(mysql);
	 row = mysql_fetch_row(result);
	 find = atoi(row[0]);
	 if (find == 0)
	 {
	   registerOnServer[0]='\0';
	   strcat(registerOnServer,"INSERT INTO clients(status_id,ip_address,lastVisit) VALUES(1,'");
	   strcat(registerOnServer,ClientSettings.ip_address);
	   strcat(registerOnServer,"',NOW());");
	     if (mysql_query(mysql,registerOnServer))
          {
           fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
           //mysql_close(mysql);
          }
		 else // Регистрация прошла успешна
		 {  
		   if (mysql_query(mysql,"SELECT LAST_INSERT_ID()"))
            {
             fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
             //mysql_close(mysql);
            }
		   else // Успешно получен уникальный идентификатор машины в базе клиентов  
		   {
			 result = mysql_store_result(mysql);
	         row = mysql_fetch_row(result);
			 ClientSettings.client_id = atoi(row[0]);
			 return 1;
		   }
		 }
	 }
	 else // Клиент уже зарегистрирован, считываем его ID на всякий случай 
	 {
		 strcat(get_ID,"SELECT client_id from clients where ip_address='");
		 strcat(get_ID,ClientSettings.ip_address);
		 strcat(get_ID,"';");
		 if (mysql_query(mysql,get_ID))
          {
             fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
             //mysql_close(mysql);
          }
		  else // Успешно получен уникальный идентификатор машины в базе клиентов  
		  {
			 result = mysql_store_result(mysql);
	         row = mysql_fetch_row(result);
			 ClientSettings.client_id = atoi(row[0]);
			 return 1;
		  }
	 }
 }
 return 1;
}

int MySQLServer::checkAllClients(SystemSettings ClientSettings)
{
  const char *_checkAllClients = "UPDATE clients SET status_id = '%d'\
 WHERE (unix_timestamp(now()) - unix_timestamp(lastVisit)) > 90;";
  char checkAllClients[128] = {0};
  sprintf(checkAllClients,_checkAllClients,ClientSettings.status_offline);

  if (mysql_query(mysql,checkAllClients))
  {
	  fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
	  return -1;
  }
  return 1;
}

int MySQLServer::setStatusFreeForAllSubtasksClientsOfWichInOffline(SystemSettings ClientSettings)
{
  const char *_setAllSubtasksClientsWhichIsOffline = "UPDATE subtasks SET status_id = '%d'\
WHERE (takenBy_id IN ( SELECT client_id FROM clients WHERE unix_timestamp(now()) - unix_timestamp(lastVisit) > 90)\
 AND status_id = '%d');";
  char setAllSubtasks[256] = {0};
  sprintf(setAllSubtasks,_setAllSubtasksClientsWhichIsOffline,ClientSettings.status_free,
 ClientSettings.status_in_progress);

  if (mysql_query(mysql,setAllSubtasks))
  {
	  fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
	  return -1;
  }
  return 1;

}

int MySQLServer::getSubtaskForSearch(SystemSettings& ClientSettings,SearchOptions& SearchArgs)
{
   // Самым высоким приоритетом обладает задача выбора только той подзадачи, у задачи которой
   // имееется меньшего всего дней для поиска
   // из всех имеющихся необработанных задач в таблице tasks. 
   // Следующим условием для хранения файлов  в кэше является:
   // непересекаемость кэшей клиентов при отсутствии на начальном этапе необработанных файлов
   // которые взяли клиенты, но не обработали из за аварийной ситуации ( ушел в оффлайн )
   // Следующим главным условием является то, что клиенты должны обработать в первую очередь
  //  только те файлы, которые имеются на жестком диске
   // Только потом нужно брать на поиск те файлы, которые не были обработаны 
   // из-за аварийной ситуации
   const char *prefixForDataPath = ClientSettings.remotePrefixForDataPath.c_str();
   // Блокируем на время таблицу с подзадачами, чтобы другие клиенты не смогли сделать SELECT-UPDATE
   // подробнее на странице: http://www.mysql.ru/docs/man/LOCK_TABLES.html :
   // Без использования LOCK TABLES существует вероятность того, что какой-либо иной поток управления
   // может вставить новую строку в таблицу subtasks между выполнением операций SELECT и UPDATE.
   const char *lockTablesWrite ="LOCK TABLES subtasks WRITE, subtasks as s WRITE, subtasks as s1 WRITE, subtasks as sb WRITE, tasks as t WRITE,\
 cache WRITE, clients WRITE;"; 

   // 0.5) быстрый селект лимит 1 для определения, есть ли задачи. если нет, то 30 сек и пункт 0.5
   const char *quicklySelectLimit1ForGetSubtask = "SELECT subtask_id,filename from subtasks\
 WHERE (status_id = 1) LIMIT 1;";

   const char *resetReservedSubtasksByAnotherClients = "UPDATE subtasks SET status_id = '%d'\
 WHERE takenBy_id IN (SELECT client_id from clients WHERE unix_timestamp(now()) - lastVisit > 900);";

 /*
   // Берем ту подзадачу из той задачи, в которой меньше всего дней для поиска
   const char *selectShortestFreeSubtaskNoCache = "SELECT subtask_id, count(subtask_id) AS cc from subtasks WHERE status_id ='%d'\
 GROUP BY subtask_id ORDER BY cc ASC LIMIT 1;";
   // Берем ту подзадачу(которую не удалось обработать клиентом) из той задачи, в которой меньше всего дней для поиска
   // Условие unix_timestamp(now()) - time_Start > 180 означает, 
   // что прошло уже 3 минуты как была взята подзадача на поиск 
   // и клиент не откликнулся ( выключился, перезагрузился , повис )

   const char *selectShortestIdleSubtaskNoCache = "SELECT subtask_id, filename, count(subtask_id) AS cc from subtasks \
 WHERE ( status_id ='%d' AND unix_timestamp(now()) - time_Progress > 180 ) GROUP BY subtask_id ORDER BY cc ASC LIMIT 1;";

   // Получаем информацию о первой попавшейся подзадаче на поиск, удовлетворяющей условию: selectShortestFreeSubtask
   const char *_getInfoAboutFreeSubtaskNoCache = "SELECT subtask_id,fileName from subtasks\
 where ( status_id = '%d' AND subtask_id = '%d') LIMIT 1;";

  // Бронируем новую подзадачу ( т.е. ту, которую еще не выполняли ) на поиск 
   const char *updateNewSubtask = "UPDATE subtasks SET status_id='%d',takenBy_id='%d',time_Start = unix_timestamp(now())\
 where ( status_id ='%d' AND subtask_id = '%d') LIMIT 1;";

  // Бронируем старую подзадачу ( т.е. ту, которую не выполнял клиент ) на поиск
   const char *updateOldSubtask = "UPDATE subtasks SET status_id='%d',takenBy_id='%d',\
 time_Start = unix_timestamp(now()),time_Progress = unix_timestamp(now())\
 where ( status_id ='%d' AND filename = '%s')";

   
   const char *selectFileWhichInCacheTableFromShortestFreeSubtasks = "SELECT t.filename FROM\
 (SELECT subtask_id, count(subtask_id) AS st, filename FROM subtasks\
 GROUP BY subtask_id ORDER BY st ASC) t WHERE EXISTS \
 (SELECT * FROM cache WHERE (cache.filename = t.filename AND cache_id = '%d')) LIMIT 1;";
   
   const char *selectFileWhichInCacheTableFromShortestIdleSubtasks = "SELECT t.filename FROM\
 (SELECT subtask_id, count(subtask_id) AS st, filename FROM subtasks\
 GROUP BY subtask_id ORDER BY st ASC) t WHERE EXISTS \
 (SELECT * FROM cache WHERE (cache.filename = t.filename AND cache_id = '%d' )) LIMIT 1;";



    const char *selectShortestFreeSubTasks = "SELECT s.subtask_id,s.filename\
 FROM (SELECT subtask_id, count(*) cnt FROM subtasks \
 GROUP BY subtask_id ORDER BY cnt) AS s1\
 LEFT JOIN subtasks as s ON (s1.subtask_id = s.subtask_id)\
 WHERE (status_id = '%d')"; 

   const char *selectShortestIdleSubTasks = "SELECT s.subtask_id,s.filename\
 FROM (SELECT subtask_id, count(*) cnt FROM subtasks\
 GROUP BY subtask_id ORDER BY cnt) AS s1\
 LEFT JOIN subtasks as s ON (s1.subtask_id = s.subtask_id)\
 WHERE (status_id = '%d' AND unix_timestamp(now()) - time_Start > 90 )"; 


    */
   const char *setStatusBusyForAllSubtasksFileNameWichInCache = "UPDATE subtasks\
 SET status_id='%d',takenBy_id='%d',timeStart = now(),time_Start = unix_timestamp(now())\
 where ( subtask_id ='%d' AND filename = '%s');";

  
   const char *selectShortestFreeSubTasksWithCache = "SELECT sb.subtask_id,sb.filename FROM subtasks sb,tasks t\
 WHERE sb.filename IN ( SELECT filename from cache WHERE client_id = '%d' )\
 AND (t.task_id = sb.subtask_id) AND (sb.status_id = '%d') ORDER BY subtasks_count LIMIT 1;";

   const char *selectShortestFreeSubTasksWithoutCache = "SELECT sb.subtask_id,sb.filename FROM subtasks sb,tasks t\
 WHERE sb.filename NOT IN ( SELECT filename from cache )\
 AND (t.task_id = sb.subtask_id) AND (sb.status_id = '%d') ORDER BY subtasks_count LIMIT 1;";

   const char *selectShortestFreeSubTasksWithoutDeadCache = "SELECT sb.subtask_id,sb.filename FROM subtasks sb,tasks t\
 WHERE sb.filename IN ( SELECT filename from cache where client_id IN (select client_id from clients\
 where status_id = '%d')) AND (t.task_id = sb.subtask_id) AND (sb.status_id = '%d')\
 ORDER BY t.subtasks_count LIMIT 1;";

    
   // Разблокируем таблицу с подзадачами, чтобы другие клиенты смогли обратиться к таблице подзадач
   // и тоже поставить блокировку на чтение-запись
   const char *unlockTables = "UNLOCK TABLES;";

   // Начальная инициализация, которая дает гарантию на то, что в переменные типа char[] не попадет мусор
   // char str[0]= '\0'; не дает гарантии, что строка будет пустой в режиме RELEASE, нужно также использовать
   // функцию memset.
   // Есть еще другой вариант , использовать только переменные типа std::string вместо char * , char[]

   char catchSubtask[512] = {0};
   char getShortestFreeSubtasks[512] = {0};
   char getShortestIdleSubtasks[512] = {0};
   char getShortestDeadSubtasks[512] = {0};
   char getInfoAboutFreeSubtask[512] = {0};
   char getInfoAboutTakenSubtask[512] = {0};
   char firstFileNameforSearchWichInCache[32] = {0};

   firstFileNameforSearchWichInCache[0] = '\0';
   catchSubtask[0] = '\0';
   getShortestFreeSubtasks[0] = '\0';
   getShortestIdleSubtasks[0] = '\0';
   getInfoAboutFreeSubtask[0] = '\0';
   getInfoAboutTakenSubtask[0] = '\0';

   int num_fields = 0;
   unsigned long long num_rows = 0;

   // sprintf() делает почти тоже самое что и printf(), только результат помещает в переменную типа const char


if (( connectionOK == 1 ) && ( databaseConnectionOK == 1))
 {
   
   if (mysql_query(mysql,lockTablesWrite))
   {
	  fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
	  return -1;
   }

     if (mysql_query(mysql,quicklySelectLimit1ForGetSubtask))
     {
		 fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
		 return -1;
     }

		result = mysql_store_result(mysql);
        num_rows = mysql_num_rows(result);

		if ( num_rows == 0 ) // Задач для поиска не обнаружено
		{
		    mysql_free_result(result);  

			if (mysql_query(mysql,unlockTables))
            {
				fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
				return -1;
			}

			return 0;
		}

		mysql_free_result(result); 
		sprintf(getShortestFreeSubtasks,selectShortestFreeSubTasksWithCache,ClientSettings.client_id,ClientSettings.status_free);

			if (mysql_query(mysql,getShortestFreeSubtasks))
			{
				fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
				return -1;
			}

       	result = mysql_store_result(mysql);
        num_rows = mysql_num_rows(result);

       if ( num_rows != 0 ) // Есть новые подзадачи ( никем не взятые) для поиска
	   {
         // Проверим, есть ли хотя бы один файл в кэше
          while ( row = mysql_fetch_row(result))
		  {
			  SearchArgs.allSubtaskIds.push_back(atoi(row[0]));
			  SearchArgs.allFileNames.push_back(row[1]);
		  }
           mysql_free_result(result); 
	   }
	   else
	   {
			  sprintf(getShortestIdleSubtasks,selectShortestFreeSubTasksWithoutCache,ClientSettings.status_free);

			  if (mysql_query(mysql,getShortestIdleSubtasks))
			  {
				  fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
				  return -1;
			  }

        result = mysql_store_result(mysql);
        num_rows = mysql_num_rows(result);

	  if ( num_rows != 0 ) // Нет новых подзадач для поиска, но есть необработанные другим клиентом!
	  {
			   while ( row = mysql_fetch_row(result))
		       {
			       SearchArgs.allSubtaskIds.push_back(atoi(row[0]));
			       SearchArgs.allFileNames.push_back(row[1]);
		       }
               mysql_free_result(result); 
	  }
	  else
	  {
		  sprintf(getShortestDeadSubtasks,selectShortestFreeSubTasksWithoutDeadCache,ClientSettings.status_offline,
		   ClientSettings.status_free);
		  if (mysql_query(mysql,getShortestDeadSubtasks))
		  {
		    fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
			return -1;
		  }
		result = mysql_store_result(mysql);
        num_rows = mysql_num_rows(result);

		 if ( num_rows != 0 ) // Нет новых подзадач для поиска, но есть необработанные другим клиентом!
	     {
			   while ( row = mysql_fetch_row(result))
		       {
			       SearchArgs.allSubtaskIds.push_back(atoi(row[0]));
			       SearchArgs.allFileNames.push_back(row[1]);
		       }
               mysql_free_result(result); 
	     }

	  }

	   }

	  if (!SearchArgs.getNeedSubtasks(ClientSettings))
	  {
		   if (mysql_query(mysql,unlockTables))
		   {
			  fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
			  return -1;
		   }
		  return 0;
	  }
	  else
	  {
		  
		  for ( unsigned int i = 0; i < SearchArgs.allNeedSubtasksIds.size() ; i++)
		  {
             memset(catchSubtask,'\0',512); 
			 sprintf(catchSubtask,setStatusBusyForAllSubtasksFileNameWichInCache,
				 ClientSettings.status_in_progress,ClientSettings.client_id,
				 SearchArgs.allNeedSubtasksIds[i],SearchArgs.allNeedFileNamesForSearch[i].c_str());

		   if (mysql_query(mysql,catchSubtask))
		   {
			  fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
			  return -1;
		   }

		  }

		   if (mysql_query(mysql,unlockTables))
		   {
			  fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
			  return -1;
		   }
		  return 1;

	  }

}
   return 1;
}

int MySQLServer::setStatusDoneForCurrSubtask(SystemSettings ClientSettings,SearchOptions SearchArgs)
{
	const char* _updateSubtask = "UPDATE subtasks SET status_id = '%d',timeEnd = now(),time_End = unix_timestamp(now())\
 where ( subtask_id ='%d' AND filename = '%s' )";
	const char* _updateTask = "UPDATE tasks SET subtasks_ready_count = subtasks_ready_count + 1 WHERE task_id = '%d'";
	char updateSubtask[256] = {0};
	char updateTask[256] = {0};
	updateSubtask[0] = '\0';
	updateTask[0] = '\0';
	sprintf(updateSubtask,_updateSubtask,ClientSettings.status_done,SearchArgs.subtask_id,SearchArgs.fileNameFromSubtasksTable);
	sprintf(updateTask,_updateTask,SearchArgs.subtask_id);

	if (mysql_query(mysql,updateSubtask))
    {
	    fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
		return -1;
    }

	if (mysql_query(mysql,updateTask))
    {
	    fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
		return -1;
    }
	
    return 1;
}

int MySQLServer::trySetStatusDoneForCurrTask(SystemSettings ClientSettings,SearchOptions SearchArgs)
{
 const char* _checkTask = "SELECT subtask_id FROM subtasks\
 WHERE (subtask_id = '%d' AND status_id < '%d' ) LIMIT 1;";
 const char* _updateTask = "UPDATE tasks SET status_id = '%d',done_at = now()\
 WHERE task_id ='%d';";
 char checkTask[256] = {0};
 char updateTask[256] = {0};
 unsigned long long num_rows = 0;
 sprintf(checkTask,_checkTask,SearchArgs.subtask_id,ClientSettings.status_done);

 if (mysql_query(mysql,checkTask))
 {
   fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
   return -1;
 }

 result = mysql_store_result(mysql);
 num_rows = mysql_num_rows(result);

 if (num_rows == 0)
 {
	sprintf(updateTask,_updateTask,ClientSettings.status_done,SearchArgs.subtask_id);
	if (mysql_query(mysql,updateTask))
	{
		fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
		return -1;
	}
 }
 else
 {
	 mysql_free_result(result);  	
	 return 0;
 }
	 mysql_free_result(result);  
	 return 1;
}

int MySQLServer::updateStatusOnlineOfClient(SystemSettings ClientSettings)
{
 const char *_updateStatus = "UPDATE clients SET lastVisit = NOW()\
 WHERE client_id = '%d';";
 char refreshStatusClient[256]={0};
 sprintf(refreshStatusClient,_updateStatus,ClientSettings.client_id);

 if (mysql_query(mysql,refreshStatusClient))
 {
    fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
	return -1;
 }

 return 1;

}

int MySQLServer::getSearchArgsFromTask(SystemSettings ClientSettings,SearchOptions& SearchArgs)
{
 char SELECT_QUERY[256] ={0};
 char getArgsQuery[256] = "SELECT answer,phoneNumberA,phoneNumberB,first_search_date,last_search_date";
 strcat(getArgsQuery," from tasks where (task_id='%d' AND status_id='%d');");
 sprintf(SELECT_QUERY,getArgsQuery,SearchArgs.subtask_id,ClientSettings.status_free);

 char StrArg[64] = {0};
 StrArg[0] = '\0';
 char *partOfDateValue = NULL;

 int i_date = 0;
 int num_fields = 0;
 unsigned long long num_rows = 0;

 if (mysql_query(mysql,SELECT_QUERY))
 {
   fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
   //mysql_close(mysql);
 }
  else
  {
      result = mysql_store_result(mysql);
      num_fields = mysql_num_fields(result);
      num_rows = mysql_num_rows(result);

   if ( num_rows == 0 ) return 0;

   while ( row = mysql_fetch_row(result))
   {        
    for (int i = 0; i < num_fields; i++)
    {
	//	 if ( i > 0 ) printf("[%s]", row[i]); // comment
		 if ( i == 0 )
	     {   
           sprintf(StrArg,"%s",row[i]);
           if ( strcmp("(null)",StrArg) == 0 )
		   {
			 SearchArgs.answer = -1;	    	
		   }
		  else SearchArgs.answer = atoi(row[i]);
		 }
	     if ( i == 1)
	     {
           sprintf(StrArg,"%s",row[i]);
	       if (strcmp("(null)",StrArg) == 0 )
	       {
            SearchArgs.phoneNumberA = 0;
		    strcpy(SearchArgs._phoneNumberA,"");			
		   }
	      else
	      {
            sscanf(StrArg,"%lld",&SearchArgs.phoneNumberA); 
            strcpy(SearchArgs._phoneNumberA, row[i]);  
			
    	  }
		 }
         if ( i == 2)
	     {
            sprintf(StrArg,"%s",row[i]);
    	    if (strcmp("(null)",StrArg) == 0 )
	        {
            SearchArgs.phoneNumberB = 0;
		    strcpy(SearchArgs._phoneNumberB,"");				
		    }
		   else
		   {
	        SearchArgs.phoneNumberB = atoi(StrArg); 
		    sscanf(StrArg,"%lld",&SearchArgs.phoneNumberB);
            strcpy(SearchArgs._phoneNumberB, row[i]);  
		   }
		   }
	       if ( i == 3 )
		   {
            sprintf(StrArg,"%s",row[i]);
		   if (strcmp("(null)",StrArg) == 0 )
    	   {
            SearchArgs.firstSearchYear=0;
	        strcpy(SearchArgs._firstSearchYear,"");
		    SearchArgs.firstSearchMonth=0;
	        strcpy(SearchArgs._firstSearchMonth,"");
		    SearchArgs.firstSearchDay=0;
	        strcpy(SearchArgs._firstSearchDay,"");
		   }
		   else
		   {
            i_date = 0;
			strcpy(SearchArgs.firstSearchDate,StrArg);
		    partOfDateValue = strtok(StrArg,"-");

		while (partOfDateValue != NULL)
		{

            if ( i_date == 0 )
			{
             strcpy(SearchArgs._firstSearchYear,partOfDateValue);
			 sscanf(partOfDateValue+2,"%d",&SearchArgs.firstSearchYear);
		  // SearchArgs.firstSearchYear = SearchArgs.firstSearchYear - 2000;
			}

			if ( i_date == 1 )
			{
             strcpy(SearchArgs._firstSearchMonth,partOfDateValue); 
			 sscanf(partOfDateValue,"%d",&SearchArgs.firstSearchMonth);
			}

			if ( i_date == 2 )
			{
             strcpy(SearchArgs._firstSearchDay,partOfDateValue); 
			 sscanf(partOfDateValue,"%d",&SearchArgs.firstSearchDay);          
			}
			
            partOfDateValue = strtok(NULL, "-");
			i_date++;

		}
		   }
		   }
	  
       if ( i == 4)
  	    {
          sprintf(StrArg,"%s",row[i]);
		  if (strcmp("(null)",StrArg) == 0 )
		  {
              SearchArgs.lastSearchYear=0;
	          strcpy(SearchArgs._lastSearchYear,"");
			  SearchArgs.lastSearchMonth=0;
	          strcpy(SearchArgs._lastSearchMonth,"");
			  SearchArgs.lastSearchDay=0;
	          strcpy(SearchArgs._lastSearchDay,"");		
		  }
	else
	{
		   i_date = 0;
		   strcpy(SearchArgs.lastSearchDate,StrArg);
		   partOfDateValue = strtok(StrArg,"-");

	 while (partOfDateValue != NULL)
	 {
           if ( i_date == 0 )
		   {
             strcpy(SearchArgs._lastSearchYear,partOfDateValue); 
			 sscanf(partOfDateValue+2,"%d",&SearchArgs.lastSearchYear);
		   }

        	if ( i_date == 1 )
			{
             strcpy(SearchArgs._lastSearchMonth,partOfDateValue); 
			 sscanf(partOfDateValue,"%d",&SearchArgs.lastSearchMonth);
			}

			if ( i_date == 2 )
			{
             strcpy(SearchArgs._lastSearchDay,partOfDateValue);
		     sscanf(partOfDateValue,"%d",&SearchArgs.lastSearchDay);           
			}
			
            partOfDateValue = strtok (NULL, "-");
			i_date++;

	 }
		  }	  			  
	   }
	   }
          //  printf("\n");
	  }  			  
	 }

    result_id = SearchArgs.subtask_id;
	return 1;
}

int MySQLServer::fileExistsInCacheTable(SystemSettings ClientSettings,SearchOptions SearchArgs)
{
	const char *findFileInCacheTable ="SELECT COUNT(*) from cache where (cache_id='%d' AND filename='%s') LIMIT 1;";
	char findQuery[256] = {0};
	findQuery[0] = '\0';
	sprintf(findQuery,findFileInCacheTable,ClientSettings.client_id,SearchArgs.fileNameFromSubtasksTable);

    int countRecords =0;
    unsigned long long num_rows = 0;


	if (mysql_query(mysql,findQuery))
    {
       fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
       //mysql_close(mysql);
	   return 0;
    }
	else
	{ 
	    result = mysql_store_result(mysql);
	    num_rows = mysql_num_rows(result);

		if ( row = mysql_fetch_row(result))
        countRecords = atoi(row[0]);

		if (countRecords == 0 ) return 0;
		else return 1;

	}
}

int MySQLServer::insertFileNameInCacheTable(SystemSettings ClientSettings,SearchOptions SearchArgs)
{
	const char* insertFileNameInTable="INSERT INTO cache(client_id,filename) VALUES(%d,'%s');";
	char insertQuery[256] ={0};
	insertQuery[0] = '\0';
	sprintf(insertQuery,insertFileNameInTable,ClientSettings.client_id,SearchArgs.fileNameFromSubtasksTable);

	if (mysql_query(mysql,insertQuery))
    {
       fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
       //mysql_close(mysql);
	   return 0;
    }
	else
	{ 
      return 1;
	}
}

bool MySQLServer::insertRecordsInResultTable(DataProcessing& ResultData)
{
	const char* insertFoundRecordInResultTable ="INSERT INTO results"; 
	char insertQuery[1024] ={0};
	char insertResultData[1024] ={0};
	insertQuery[0]='\0';
	insertResultData[0]='\0';
	strcat(insertQuery,insertFoundRecordInResultTable);
	strcat(insertQuery,"(result_id,answer,date,time,phoneNumberA,phoneNumberB,callDuration) ");
	strcat(insertQuery,"VALUES (%d,%d,'%s','%s',%lld,%lld,%d);");
	
    ResultData.yearChar[0]='\0';
	if ( (ResultData.year / 10) !=0)
	sprintf(ResultData.yearChar,"20%d",ResultData.year);
	else sprintf(ResultData.yearChar,"200%d",ResultData.year);	
	
    ResultData.monthChar[0]='\0';
    if ( (ResultData.month / 10) !=0)
    sprintf(ResultData.monthChar,"%d",ResultData.month);
	else sprintf(ResultData.monthChar,"0%d",ResultData.month);

	ResultData.dayChar[0]='\0';
	if ( (ResultData.day / 10) !=0)
	sprintf(ResultData.dayChar,"%d",ResultData.day);
	else sprintf(ResultData.dayChar,"0%d",ResultData.day);

	ResultData.date[0]='\0';
	sprintf(ResultData.date,"%s-%s-%s",ResultData.yearChar,ResultData.monthChar,ResultData.dayChar);
		
	ResultData.hourChar[0]='\0';
	if ( (ResultData.hour / 10) !=0)
	sprintf(ResultData.hourChar,"%d",ResultData.hour);
	else sprintf(ResultData.hourChar,"0%d",ResultData.hour);
		
	ResultData.minuteChar[0]='\0';
	if ( (ResultData.minute / 10) !=0)
	sprintf(ResultData.minuteChar,"%d",ResultData.minute);
	else sprintf(ResultData.minuteChar,"0%d",ResultData.minute);

	ResultData.secondChar[0]='\0';
	if ( (ResultData.second / 10) !=0)
	sprintf(ResultData.secondChar,"%d",ResultData.second);	
	else sprintf(ResultData.secondChar,"0%d",ResultData.second);
	sprintf(ResultData.time,"%s:%s:%s",ResultData.hourChar,ResultData.minuteChar,ResultData.secondChar);

	sprintf(insertResultData,insertQuery,result_id,ResultData.answer,ResultData.date,ResultData.time,
	ResultData.phoneNumberA,ResultData.phoneNumberB,ResultData.callDuration);

	if (mysql_query(mysql,insertResultData))
    {
     fprintf(stderr,"%d %s\n",mysql_errno(mysql),mysql_error(mysql));
	 return false;
     //mysql_close(mysql);
    }
	else 
	return true;

}


	  

	  
