#pragma once

#include <librairie.h>

struct info_sig {
    std::string L0Name;
    std::string signalName;
    int readCycle;
    std::vector<double> values;
    double q;
    info_sig():L0Name(""),signalName(""),readCycle(0),values(),q(0.0){}
    info_sig(std::string L0Name,std::string signalName,int readCycle,double q):L0Name(L0Name),signalName(signalName),readCycle(readCycle),values(),q(q){}
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

struct load_stat{
    std::string sig_name;
    double _0_50_range;
    double _51_100_range;
    double _101_150_range;
    double _151_200_range;
    double _201_250_range;
    double _251_300_range;
    double _301_350_range;
    double _351_400_range;
    double _401_450_range;
    double _451_500_range;
    load_stat():sig_name(""),_0_50_range(0),_51_100_range(0),_101_150_range(0),_151_200_range(0),_201_250_range(0),_251_300_range(0),_301_350_range(0),_351_400_range(0),_401_450_range(0),_451_500_range(0){}
};
typedef std::vector<load_stat> vectLoadStat;

struct Predictors{
    vectPred fan_pred;

    Predictors():fan_pred(){}
};

typedef std::vector<std::string> csv_filename;
typedef std::vector<std::vector<double>> all_dist;
typedef std::vector<double> dist_vect;

struct machining_info{

    double To;
    dist_vect total_vect_dist;
    dist_vect cycle_dist_vect;
    machining_info():To(0.0),total_vect_dist(),cycle_dist_vect(){}

};
