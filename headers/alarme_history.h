#pragma once

#include <classes.h>

alarm_history_db get_alarm_history_db(void);
void alarm_stats(const alarm_history_db&,const machining_info&);
void save_stats(const alarm_history_db&,const double&);
void alarm_history_step(const machining_info&);
