#pragma once
#include <classes.h>

/** Function to filter the signals and get only useful ones **/
allConf db_filtering(const allConf&,const std::string&);

/** Function to save fan predictors in text file **/
void save_and_plot_fan_predictors(const vectPred&,const allConf&);

/** Function to save the load predictors in a text file **/
void save_and_plot_load_pred (const vectLoadStat&);

/** Function to calculate the mean and standard deviation for each fan signal **/
void fan_prediction(const allConf&);

/** Function to calculate histogram of each load signal **/
void load_motor_stats(const allConf&);

/** Function to get all information from motor speed **/
machining_info speed_motor_stats(const allConf&);

/** Function to get all information from motor position **/
machining_info dist_stats(const allConf&);

/** Function to get all information from motor torque or output power **/
machining_info torque_motor_stats (const allConf&);

/** Function to calculate the total distance for all axes **/
dist_vect calculate_dist(const all_dist&);

/** Function to calculate machining cycle with autocovariance **/
double get_periode(const all_dist&, const double&);

/** Function to save in a text file the information calculate previously **/
void save_axes_stats(const machining_info&);

/** Function to gather all information in one step **/
machining_info get_all_axes_info(const allConf&);

/** Function to read CSV files **/
std::vector<std::vector<double>> read_csv_files(std::vector<std::string>,double&);

/** Function to get the CSV files **/
std::vector<std::string> get_csv_files(void);

/** Main function of this file **/
machining_info make_predictor_steps(const allConf&);
