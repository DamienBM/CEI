#pragma once

#include <classes.h>

void make_predictor_steps(const allConf&);
allConf db_filtering(const allConf&,const std::string&);
void save_and_plot_fan_predictors(const vectPred&,const allConf&);
void save_and_plot_load_pred (const vectLoadStat&);
void fan_prediction(const allConf&);
void load_motor_prediction(const allConf&);
dist_vect calculate_dist(const all_dist&);
double get_periode(const all_dist&, const double&);
void save_axes_stats(const machining_info&);
std::vector<std::string> get_csv_files(void);
all_dist read_csv_files(std::vector<std::string>,double&);
machining_info get_all_axes_info(const allConf&);
