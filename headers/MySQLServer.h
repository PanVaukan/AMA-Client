
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

	MYSQL *mysql; // ���������� ����������
	MYSQL_ROW row; // ������ ����� ������� ������
	MYSQL_RES *result; // ���������� �������������� �������
    MYSQL_FIELD *field; // ���������� �������
	int connectionOK;  // ���������� �������������������
	int databaseConnectionOK; // ����������� ���������� � ����� ������
	unsigned int* timeout;  // ������� ��������� � ���� ������
	char *partOfFileName;
    int result_id; 

public:

	MySQLServer(void);
   ~MySQLServer(void);

    friend class SearchOptions;
    friend class SystemSettings;
	friend class DataProcessing;

	void resetParametrs();
	int initConnection(); // �������������� ���������� ����������
	int databaseConnection(SystemSettings& ClientSettings); // ������������� ���������� � ����� ������
	int serverLogin(SystemSettings& ClientSettings); // ������������ ���� IP ����� �� �������
	int checkAllClients(SystemSettings ClientSettings);
    int setStatusFreeForAllSubtasksClientsOfWichInOffline(SystemSettings ClientSettings);
	int getSubtaskForSearch(SystemSettings& ClientSettings,SearchOptions& SearchArgs); // ��������� ��������� 
	int getSearchArgsFromTask(SystemSettings ClientSettings,SearchOptions& SearchArgs); // ��������� ������ ��� ������
	int setStatusDoneForCurrSubtask(SystemSettings ClientSettings,SearchOptions SearchArgs); // ��������� ���������: status = done
	int trySetStatusDoneForCurrTask(SystemSettings ClientSettings,SearchOptions SearchArgs); // ������ ���������: status = done
	int updateStatusOnlineOfClient(SystemSettings ClientSettings);
    int fileExistsInCacheTable(SystemSettings ClientSettings,SearchOptions SearchArgs); // �������� �� ������������� ����� � ����
	int insertFileNameInCacheTable(SystemSettings ClientSettings,SearchOptions SearchArgs);
	bool insertRecordsInResultTable(DataProcessing& ResultData); // ���������� ��������� ������� � MySQL

};


