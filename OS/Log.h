#pragma once
#ifndef LOG
#define LOG

#include <sys/stat.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace std;

class Log
{
private:
    vector<string> commandStack;

public:
    Log();
    ~Log();
    bool init();

    bool readFromFile();
    bool writeToFile();

    bool push(string command);
    bool pop();
    void printLog();
    string checkLog();
};

#endif