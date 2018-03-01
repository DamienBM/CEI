#pragma once
#include <classes.h>

allConf create_DB(void);
void ecriture_thread(const allConf&);
int ecriture(const info_sig&);
void delete_stuff(void);
void sparse_db(std::ifstream&,allConf&);
allConf first_steps(void);
