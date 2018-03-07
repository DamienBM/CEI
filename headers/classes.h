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

typedef std::vector<int> histo;
struct load_stat{
    std::string sig_name;
    histo range;
    load_stat():sig_name(""),range(){}
};
typedef std::vector<load_stat> vectLoadStat;

struct Predictors{
    vectPred fan_pred;

    Predictors():fan_pred(){}
};

typedef std::vector<std::string> csv_filename;
typedef std::vector<std::vector<double>> all_dist;
typedef std::vector<double> dist_vect;
typedef std::vector<double> nb_inv_vect;
typedef std::vector<double> mean_torque_vect;

struct machining_info{
    double To;
    dist_vect total_vect_dist;
    dist_vect cycle_dist_vect;
    nb_inv_vect nb_inv_per_axe;
    histo override_sig;
    mean_torque_vect mean_torque;

    machining_info():To(0.0),total_vect_dist(),cycle_dist_vect(),nb_inv_per_axe(),override_sig(),mean_torque(){override_sig.reserve(NB_VAL_POTAR_OVR);}
    machining_info(double To, dist_vect total_vect_dist,dist_vect cycle_dist_vect,nb_inv_vect nb_inv_per_axe,histo override_sig,mean_torque_vect mean_torque):
        To(To),total_vect_dist(total_vect_dist),cycle_dist_vect(cycle_dist_vect),nb_inv_per_axe(nb_inv_per_axe),override_sig(override_sig),mean_torque(mean_torque){}

};
