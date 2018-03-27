#pragma once
#include <classes.h>

/** Sparse the Alarm History file **/
alarm_history_db get_alarm_history_db(void);

/** Summarize alarms **/
void alarm_stats(const alarm_history_db&,const machining_info&);

/** Save the summarized vector in a text file **/
void save_stats(const alarm_history_db&,const double&);

/** Main function of this file **/
void alarm_history_step(const machining_info&);
