
#pragma once
#include <winsock2.h> 
#include "../includes/include_MySQL/mysql.h"
#pragma comment(lib, "libmysql.lib")
#include "../headers/DataProcessing.h"
#include "../headers/SearchOptions.h"
#include "../headers/SystemSettings.h"


class MySQLServer
{

private:

	MYSQL *mysql; // Дескриптор соединения
	MYSQL_ROW row; // Массив полей текущей строки
	MYSQL_RES *result; // Дескриптор результирующей таблицы
    MYSQL_FIELD *field; // Дескриптор таблицы
	int connectionOK;  // Соединение проинициализировано
	int databaseConnectionOK; // Установлено соединение с базой данных
	unsigned int* timeout;  // Таймаут обращения к базе данных
	char *partOfFileName;
    int result_id; 

public:

	MySQLServer(void);
   ~MySQLServer(void);

    friend class SearchOptions;
    friend class SystemSettings;
	friend class DataProcessing;

	void resetParametrs();
	int initConnection(); // Инициализируем дескриптор соединения
	int databaseConnection(SystemSettings& ClientSettings); // Устанавливаем соединение с базой данных
	int serverLogin(SystemSettings& ClientSettings); // Регистрируем свой IP адрес на сервере
	int checkAllClients(SystemSettings ClientSettings);
    int setStatusFreeForAllSubtasksClientsOfWichInOffline(SystemSettings ClientSettings);
	int getSubtaskForSearch(SystemSettings& ClientSettings,SearchOptions& SearchArgs); // Получение подзадачи 
	int getSearchArgsFromTask(SystemSettings ClientSettings,SearchOptions& SearchArgs); // Получение данных для поиска
	int setStatusDoneForCurrSubtask(SystemSettings ClientSettings,SearchOptions SearchArgs); // Подзадача выполнена: status = done
	int trySetStatusDoneForCurrTask(SystemSettings ClientSettings,SearchOptions SearchArgs); // Задача выполнена: status = done
	int updateStatusOnlineOfClient(SystemSettings ClientSettings);
    int fileExistsInCacheTable(SystemSettings ClientSettings,SearchOptions SearchArgs); // Проверка на существование файла в кэше
	int insertFileNameInCacheTable(SystemSettings ClientSettings,SearchOptions SearchArgs);
	bool insertRecordsInResultTable(DataProcessing& ResultData); // Сохранение найденных записей в MySQL

};


