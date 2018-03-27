#pragma once
#include <classes.h>

/** Function to sparse Signal History file and fill vectors **/
void sparse_db(std::ifstream&,allConf&,allAlarms&);

/** Function to delete all unnecessary files **/
void delete_stuff(void);

/** Function to write each signal in a text file **/
int ecriture(const info_sig&);

/** Function to manage the writing thread (function above) **/
void ecriture_thread(const allConf&,const allAlarms&);

/** Function to create initialize the data base **/
allConf create_DB(void);

/** Main function of this file **/
allConf first_steps(void);
