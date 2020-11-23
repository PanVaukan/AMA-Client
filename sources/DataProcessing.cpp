
#include "../headers/DataProcessing.h"
#pragma warning(disable : 4996)		// bogus deprecation warning

DataProcessing::DataProcessing(void)
{
	sizeOfBuffer = 2097152;
	phoneNumberA = 0;
	phoneNumberB = 0;
	memset(phoneNumberAchar,'\0',64);
	memset(phoneNumberBchar,'\0',64);
	phoneNumberAchar[0]='\0';
	phoneNumberBchar[0]='\0';

	answer = -1;
	year=0;
	month=0;
	day=0;
	hour=0;
	minute=0;
	second=0;
	callDuration=0;

	memset(date,'\0',11);
	memset(time,'\0',11);
	memset(yearChar,'\0',5);
	memset(monthChar,'\0',3);
	memset(dayChar,'\0',3);
	memset(hourChar,'\0',3);
	memset(minuteChar,'\0',3);
	memset(secondChar,'\0',3);

	date[0]='\0';
	time[0]='\0';
	yearChar[0]='\0';
	monthChar[0]='\0';
	dayChar[0]='\0';
	hourChar[0]='\0';
	minuteChar[0]='\0';
	secondChar[0]='\0';
}


DataProcessing::~DataProcessing(void)
{
}

void DataProcessing::resetDataParametrs()
{
	sizeOfBuffer = 2097152;
	phoneNumberA = 0;
	phoneNumberB = 0;
	phoneNumberAchar[0]='\0';
	phoneNumberBchar[0]='\0';

	year=0;
	month=0;
	day=0;
	hour=0;
	minute=0;
	second=0;
	callDuration=0;

	date[0]='\0';
	time[0]='\0';
	yearChar[0]='\0';
	monthChar[0]='\0';
	dayChar[0]='\0';
	hourChar[0]='\0';
	minuteChar[0]='\0';
	secondChar[0]='\0';

}

DataProcessing& DataProcessing::getCurrData()
{
	return *this;
}

unsigned long long DataProcessing::setterPhoneNumber(unsigned long long &i,int phoneLength, char* buffer)
{
int odd = 0;
unsigned long long phoneNumber = 0;
if ( phoneLength % 2 == 0 )
{
phoneLength = (phoneLength / 2);
odd =0;
}
else { phoneLength = (phoneLength / 2) +1;
odd = 1;
}
i++;


for (unsigned long long j = i; j < i + phoneLength; j++)
{
phoneNumber *=10;
phoneNumber += int(unsigned char(buffer[j])>>4) ;

if ((j != i+phoneLength-1 ))
{
phoneNumber *=10;
phoneNumber += int(unsigned char(buffer[j]) & 15) ;
}
else
{
if (odd ==0)
{
phoneNumber *=10; 
phoneNumber += int(unsigned char(buffer[j]) & 15);
}

}

} // end for

i+=phoneLength;

return phoneNumber;

}

string DataProcessing::charSetterPhoneNumber(int &i,int phoneLength, char* buffer)
{
int odd = 0;
string phoneNumber = "";
if ( phoneLength % 2 == 0 )
{
phoneLength = (phoneLength / 2);
odd =0;
}
else { phoneLength = (phoneLength / 2) +1;
odd = 1;
}
i++;

for (int j = i; j < i + phoneLength; j++)
{
phoneNumber += char('0'+ int(unsigned char(buffer[j])>>4)) ;

if ((j != i+phoneLength-1 ))
{
phoneNumber += char('0'+ int(unsigned char(buffer[j]) & 15)) ;
}
else
{
if (odd ==0)
{
phoneNumber += char('0' + int(unsigned char(buffer[j]) & 15));
}

}

} // end for

i+=phoneLength;
return phoneNumber;
}

void DataProcessing::searchData(SearchOptions SearchArgs,MySQLServer& Provider)
{
unsigned long long fileSize = 0;
unsigned long long countNumbers = 0;
unsigned long long currentShift = 0;
unsigned long long totalShift = 0;
long long shiftForFile = 0;
int phoneLength = 0;
int bug = 0;
int odd = 0;
unsigned long long blockSize = 0;
int sizeOfTotalBuffer = 0;
unsigned char FirstByteCallDuration = 0x00;
unsigned char SecondByteCallDuration = 0x00;
unsigned char ThirdByteCallDuration = 0x00;

int Byte100 = 0;
int Byte101 = 0;
int Byte168 = 0;

int countZero = 0;
int countDurations = 0;
int coutByte168 = 0;

SYSTEMTIME systime;
char str[256];


for (int i = 0; i < 1 ; i++ )
{

 ifstream source(SearchArgs.getfullPathToFileForSearch(), ios::in | ios::binary);
if (!source)
{ 
source.close(); 
std::cout << "Can\'t open file: " << source << endl; 
return;
}

source.seekg(0,ios::end);
fileSize = source.tellg();
source.seekg(0,ios::beg);


// cout << " File Number --> " << i << endl << endl; 

char* buffer = new  char[sizeOfBuffer];

cout << endl;
cout <<" Begin SearchTime: " << endl; 
GetLocalTime(&systime);
sprintf(str, "%d:%d:%02d", systime.wHour, systime.wMinute, systime.wSecond);
std::cout << str;


while (!source.eof())
{

if ( (sizeOfBuffer + source.tellg()) > fileSize )   // if ( (sizeOfBuffer + source.tellg()) - 1 > fileSize )
{
sizeOfBuffer = fileSize - source.tellg();
source.read(buffer,sizeOfBuffer);
}
else
source.read(buffer,sizeOfBuffer);

if ( sizeOfBuffer == 0)
break;


for (unsigned long long i = 0; i < sizeOfBuffer; i++)
{

if (i > totalShift ) // Пропуск нулевых заполнителей
{
totalShift = i;
countZero++;
}

if ((unsigned char(buffer[i]) == 132) && (i == totalShift))
{
i++;
currentShift = unsigned char(buffer[i]);


if ((sizeOfBuffer - totalShift ) < currentShift)   // if ((sizeOfBuffer - totalShift - 1 ) < currentShift)
{
//if ( currentShift < sizeOfBuffer ) 
//{
shiftForFile = sizeOfBuffer - totalShift;
source.seekg( -shiftForFile,ios::cur);
totalShift = 0;
break;
//}
//else
//bug++;
}

else 

{

totalShift += currentShift; 
blockSize +=currentShift;

phoneNumberA = 0;
phoneNumberB = 0;
callDuration = 0;
Byte100 = 0;
Byte101 = 0;
Byte168 = 0;

i++; // working byte
i++; //
i++; // 
answer = int(unsigned char(buffer[i]) & 4);
i++; // total 24 bits
i++;
i++;

phoneLength = unsigned char(buffer[i]);


if (phoneLength > 32)
phoneLength = phoneLength & 15;

// cout << charSetterPhoneNumber(i,phoneLength,buffer);

if (phoneLength !=0)
phoneNumberA = setterPhoneNumber(i,phoneLength,buffer);
else
{
phoneNumberA = 0;
i++;
}

} // end else

countNumbers++;

}

if ((unsigned char(buffer[i]) == 100) && (Byte100 == 0))
{
Byte100 = 1;
i++;
year = unsigned char(buffer[i]);
i++;

/*
yearChar[0]='\0';
if ( (year / 10) !=0)
sprintf(yearChar,"%d",year);
else sprintf(yearChar,"0%d",year);
*/

month = unsigned char(buffer[i]);
i++;

/*
monthChar[0]='\0';
if ( (month / 10) !=0)
sprintf(monthChar,"%d",month);
else sprintf(monthChar,"0%d",month);
*/

day = unsigned char(buffer[i]);
i++;

/*
dayChar[0]='\0';
if ( (day / 10) !=0)
sprintf(dayChar,"%d",day);
else sprintf(dayChar,"0%d",day);

date[0]='\0';
sprintf(date,"%s-%s-%s",dayChar,monthChar,yearChar);
//cout << date << endl;
*/

hour = unsigned char(buffer[i]);
i++;

/*
hourChar[0]='\0';
if ( (hour / 10) !=0)
sprintf(hourChar,"%d",hour);
else sprintf(hourChar,"0%d",hour);
*/

minute = unsigned char(buffer[i]);
i++;

/*
minuteChar[0]='\0';
if ( (minute / 10) !=0)
sprintf(minuteChar,"%d",minute);
else sprintf(minuteChar,"0%d",minute);
*/

second = unsigned char(buffer[i]);
i++;

/*
secondChar[0]='\0';
if ( (second / 10) !=0)
sprintf(secondChar,"%d",second);
else sprintf(secondChar,"0%d",second);
sprintf(time,"%s:%s:%s",hourChar,minuteChar,secondChar);
//cout << time << endl;
*/

i++;
FirstByteCallDuration = unsigned char(buffer[i++]);
SecondByteCallDuration = unsigned char(buffer[i++]);
ThirdByteCallDuration = unsigned char(buffer[i++]);
callDuration = ((ThirdByteCallDuration << 16) | (SecondByteCallDuration << 8) | FirstByteCallDuration);

countDurations++;

}


if ((unsigned char(buffer[i]) == 101) && (Byte101 == 0))
{

Byte101 = 1;
i++;
phoneLength = unsigned char(buffer[i]);
if (phoneLength > 32)
phoneLength = phoneLength & 15;

if (phoneLength !=0)
phoneNumberB = setterPhoneNumber(i,phoneLength,buffer);
else
{
phoneNumberB = 0;
i++;

}
}


if ((unsigned char(buffer[i]) == 168) && (Byte168 == 0))
{
Byte168 = 1;
coutByte168++;

}

/*
if ( SearchArgs.checkRecord(*this) )
{
	Provider.insertRecordsInResultTable(*this);
}
*/

if (i < totalShift )
{
if ( SearchArgs.checkRecord(*this) )
{
	Provider.insertRecordsInResultTable(*this);
}
i = totalShift-1;
}

} // end main loop
totalShift = 0;

// if ( source.eof() )
// break;


}

cout << endl;
cout <<" End SearchTime: " << endl; 
GetLocalTime(&systime);
sprintf(str, "%d:%d:%02d", systime.wHour, systime.wMinute, systime.wSecond);
cout << str << endl;

blockSize = 0;
totalShift = 0;
delete []buffer;
//getche();
source.close();
}


}