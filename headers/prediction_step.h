#pragma once
#include <classes.h>

/** Function to read and load the predictors from the text file of each predictor **/
Predictors load_predictors(void);

/** Function to do the continuous prediction **/
void predict(const allConf_active&,const Predictors&);

/** Function to store the active data base **/
void get_active_db(void);

/** Function to store the active data base **/
allConf_active read_active_signals_file(void);

/** Function to write the active value of a signal in a text file **/
int active_db_ecriture(const info_active_sig&);

/** Function to manage the writing thread (function above) **/
void active_db_ecriture_thread(const allConf_active&);

/** Function to create the active signal data base file directory **/
void create_prediction_dir(void);

/** Main function of this file **/
void predictions_step(void);
