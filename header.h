#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <time.h>

#define MSEC 1000
#define PAUSE system("pause");

struct info_sig {
    std::string L0Name;
    std::string signalName;
    int readCycle;
    std::vector<double> values;
    info_sig():L0Name(""),signalName(""),readCycle(0),values(){}
};
typedef std::vector<info_sig> allConf;

allConf create_DB(void);
