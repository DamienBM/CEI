#include "header.h"

/** Main function of this software **/
int main(int argc, char** argv)
{
    allConf db = first_steps(); /** Do the first steps : DB => SPARSE DB => OUT DATA **/

    machining_info mcn_info = make_predictor_steps(db); /** Do the predictors construction steps **/

    launch_python_script(); /** Launch python script for Machine Learning algorithm **/

    alarm_history_step(std::cref(mcn_info)); /** Get alarm history for stats **/

    PAUSE

    //predictions_step(); /** Do the continuous prediction steps **/

    return 0;
}












