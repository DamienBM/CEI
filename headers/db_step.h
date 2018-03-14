#pragma once
#include <classes.h>

allConf create_DB(void);
void ecriture_thread(const allConf&,const allAlarms&);
int ecriture(const info_sig&);
void delete_stuff(void);
void sparse_db(std::ifstream&,allConf&,allAlarms&);
allConf first_steps(void);
