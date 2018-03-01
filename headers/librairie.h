#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <time.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <future>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <functional>
#include <chrono>
#include <list>
#include <boost/filesystem.hpp>

#define MSEC 1000
#define PAUSE system("pause");
#define CUR_DIR std::string(_getcwd(NULL,0))
#define ACTIVE_SIGNALS_FILE std::string("MTLINKi_Active_Signals.txt")
#define TEMP_MAX_MOT_WINDING 140 //Celsius degrees
#define TEMP_MAX_PULSECODER 100  //Celsius degrees
