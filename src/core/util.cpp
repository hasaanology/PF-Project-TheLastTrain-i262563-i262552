#include<iostream>
#include <cctype>  
#include "util.h"
#include<string>
using namespace std;
int clampInt(int val, int minVal, int maxVal)
{
    if(val>maxVal)
    {
        return maxVal;
    }
   else  if(val<minVal)
    {
        return minVal;
    }
    else 
    return val;
}
void trimWhitespace(char* str)
{
if (str == nullptr)
{return;}

int length=0;

while (str[length] != '\0')
{
    length++;
}
int start=0;
while (start<length && isspace((unsigned char)str[start]))
{
    start++;
}
if (start ==length)
{
str[0]='\0';
return;   
}
int end=length-1;
while(end>start && isspace((unsigned char)str[end]))
{
    end--;
}

int index=0;
for (int i = start; i <= end; i++)
{
    str[index]=str[i];
    index++;
}
str[index]='\0';
}

bool stringEquals(const char* a, const char* b)
{
  if ( a==nullptr || b==nullptr)
  {return false;}

  int i=0;
  while(a[i] != '\0'&& b[i] !='\0')
  {
    if(a[i] != b[i])
    {
        return false;
    }
    i++;
  }
    if (a[i] == '\0' && b[i] == '\0') 
    {
    return true;
    }
    return false;
}
void printHeader(const char* title)
{
    if (title == nullptr)
    {
    return;
    }
    cout<<"========================"<<endl;
    cout<<"    "<<title<<endl;
    cout<<"========================"<<endl;
}


void clearScreen()
{
    cout << string(100, '\n');
}






// Primitive helper utilities, clamping, and screen control
