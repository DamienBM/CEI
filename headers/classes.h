#pragma once

#include <librairie.h>

/** Structure for summarize the information of a signal **/
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

/** Structure for summarize the information of an alarm **/
struct alarm_signal{
    std::string updatedate;
    std::string enddate;
    double duration;

    alarm_signal():updatedate(""),enddate(""),duration(0){}
};
typedef std::vector<alarm_signal> allAlarms;

/** Structure for summarize the information of an active signal **/
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

/** Structure for fan prediction **/
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

/** Structure for summarize the load signals **/
struct load_stat{
    std::string sig_name;
    histo range;
    load_stat():sig_name(""),range(){}
};
typedef std::vector<load_stat> vectLoadStat;

/** Structure for summarize all of the predictors **/
struct Predictors{
    vectPred fan_pred;

    Predictors():fan_pred(){}
};

typedef std::vector<std::string> csv_filename;
typedef std::vector<std::pair<std::string,std::vector<double>>> all_dist;
typedef std::vector<double> dist_vect;
typedef std::vector<double> nb_inv_vect;
typedef std::vector<double> mean_torque_vect;
typedef std::vector<double> mean_output_power_vect;

/** Structure for summarize the machining cycle information**/
struct machining_info{
    double To;
    dist_vect total_vect_dist;
    dist_vect cycle_dist_vect;
    nb_inv_vect nb_inv_per_axe;
    histo override_sig;
    mean_torque_vect mean_torque;//servo
    mean_output_power_vect mean_output_power;//spindle

    machining_info():To(0.0),total_vect_dist(),cycle_dist_vect(),nb_inv_per_axe(),override_sig(),mean_torque(),mean_output_power(0.0){}
    machining_info(double To, dist_vect total_vect_dist,dist_vect cycle_dist_vect,nb_inv_vect nb_inv_per_axe,histo override_sig,mean_torque_vect mean_torque,mean_output_power_vect mean_output_power):
        To(To),total_vect_dist(total_vect_dist),cycle_dist_vect(cycle_dist_vect),nb_inv_per_axe(nb_inv_per_axe),override_sig(override_sig),mean_torque(mean_torque),mean_output_power(mean_output_power){}
};

/** Structure for summarize all lines from the Alarm History file **/
struct alarm_history{

    std::string L1Name;
    std::string L0Name;
    unsigned int number;
    std::string updatedate;
    std::string enddate;
    std::string message;
    unsigned int level;
    std::string type;
    double timespan;

    alarm_history():L1Name(""),L0Name(""),number(0),updatedate(""),enddate(""),message(""),level(0),type(""),timespan(0.0){}
    alarm_history(const alarm_history& alarm):L1Name(alarm.L1Name),L0Name(alarm.L0Name),number(alarm.number),
                                       updatedate(alarm.updatedate),enddate(alarm.enddate),message(alarm.message),level(alarm.level),
                                       type(alarm.type),timespan(alarm.timespan){}

};
typedef std::vector<alarm_history> alarm_history_db;
