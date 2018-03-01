#pragma once
#include <classes.h>

int active_db_ecriture(const info_active_sig&);
void active_db_ecriture_thread(const allConf_active&);
void create_prediction_dir(void);
Predictors load_predictors(void);
void predict(const allConf_active&,const Predictors&);
void get_active_db(void);
void predictions_step(void);
