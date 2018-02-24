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

struct info_sig {
    std::string L0Name;
    std::string signalName;
    int readCycle;
    std::vector<double> values;
    double q;
    info_sig():L0Name(""),signalName(""),readCycle(0),values(),q(0.0){}
};
typedef std::vector<info_sig> allConf;

struct info_active_sig {
    std::string L1Name;
    std::string signalName;
    std::string time1;//separated by : each timer
    std::string time2;
    std::string time3;
    double value;
    info_active_sig():L1Name(""),signalName(""),time1(""),time2(""),time3(""),value(0){}
};
typedef std::vector<info_active_sig> allConf_active;

struct stat_pred{
    std::string sig_name;
    double mean;
    double std_dev;
    double q;
    stat_pred():sig_name(""),mean(0.0),std_dev(0.0),q(0.0){}
    stat_pred(std::string sig_name, double mean, double std_dev, double q):sig_name(sig_name),mean(mean),std_dev(std_dev),q(q){}
};
typedef std::vector<stat_pred> vectPred;

struct Predictors{
    vectPred fan_pred;

    Predictors():fan_pred(){}
};

typedef std::vector<std::string> csv_filename;
typedef std::vector<std::vector<double>> all_dist;
typedef std::vector<double> total_dist_vect;

allConf create_DB(void);
void ecriture_thread(const allConf&);
int ecriture(const info_sig&);
void delete_stuff(std::string&);
void sparse_db(std::ifstream&,allConf&);
allConf first_steps(void);
void make_predictor_steps(const allConf&);
allConf filtering_db(const allConf&);
void save_and_plot_predictors(const vectPred&,const allConf&);
void fan_prediction(const allConf&);
void temp_motor_prediction(const allConf&);
int active_db_ecriture(const info_active_sig&);
void active_db_ecriture_thread(const allConf_active&);
void create_prediction_dir(void);
Predictors load_predictors(void);
void predict(const allConf_active&,const Predictors&);
void get_active_db(void);
void predictions_step(void);
std::vector<double> calculate_dist(std::vector<double> , std::vector <double>);
std::vector<std::string> get_csv_files(void);
std::vector<std::vector<double>> read_csv_files(std::vector<std::string>);

