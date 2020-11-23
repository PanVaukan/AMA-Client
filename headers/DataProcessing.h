
#pragma once
#include "../headers/SearchOptions.h"
#include "../headers/MySQLServer.h"

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <vector>

using std::ifstream;
using std::ofstream;
using std::string;
using std::cout;
using std::ios;
using std::endl;
using std::vector;

class DataProcessing
{
private:

unsigned long long sizeOfBuffer;

unsigned __int64 phoneNumberA;
unsigned __int64 phoneNumberB;

char phoneNumberAchar[64];
char phoneNumberBchar[64];

int answer;
int year;
int month;
int day;
int hour;
int minute;
int second;
int callDuration;

char date[11];
char time[11];

char yearChar[5];
char monthChar[3];
char dayChar[3];

char hourChar[3];
char minuteChar[3];
char secondChar[3];

public:
	DataProcessing(void);
	~DataProcessing(void);

friend class MySQLServer;
friend class SearchOptions;

void resetDataParametrs();
DataProcessing& getCurrData();
void CopyFileFromNework();
unsigned long long setterPhoneNumber(unsigned long long &i,int phoneLength, char* buffer);
string charSetterPhoneNumber(int &i,int phoneLength, char* buffer);
void searchData(SearchOptions SearchArgs,MySQLServer& Provider);

   

};

