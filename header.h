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
#include <thread>
#include <future>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <functional>
#include <chrono>

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

struct stat_pred{
    std::string sig_name;
    double mean;
    double std_dev;
    stat_pred():sig_name(""),mean(0.0),std_dev(0.0){}
    stat_pred(std::string sig_name, double mean, double std_dev):sig_name(sig_name),mean(mean),std_dev(std_dev){}
};
typedef std::vector<stat_pred> vectPred;

allConf create_DB(void);
void ecriture_thread(allConf);
int ecriture(info_sig);
void delete_stuff(std::string);
void sparse_db(std::ifstream&,allConf&);
allConf first_steps(void);
void make_predictor_steps(allConf);
allConf filtering_db(const allConf&);
void save_and_plot_predictors(const vectPred&,const allConf&);
void fan_prediction(const allConf&);
void temp_motor_prediction(const allConf&);

