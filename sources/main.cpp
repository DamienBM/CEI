#include "header.h"

/**********************
Done :

V1 : Programme r�ussi � r�cup�rer toutes les valeurs d'un signal
     que l'on demande par l'instanciation de test (std::string).

V2 : On r�cup�re toutes les valeurs du signal nomm� partiellement par test
     afin de tracer ensuite sous Excel les donn�es.

V3 : R�cup�re les donn�es et les met dans une structure de type info_sig
     et sort les signaux en fichier txt et supprime les fichiers de 0Ko

V4 : am�lioration avec des thread & future lors de l'�criture des fichiers dans /out/...
     Les threads retourne une valeur (int) et on attends que les thread

V5 : s�paration du code en plusieurs morceaux selon sch�ma ci-apr�s
     MongoDB => dossier out => plusieurs thread (un pour la temp, un pour la distance, un pour le sens de rotation etc)
       OK    =>      OK     => done  -> fan_pred with gaussian model : thresh = (�-3*sigma) ;
                                     -> calculate distance with CSV file from Servo Viewer ;
                                     -> Load histogram ;
                                     -> calculate drilling cycle by auto-correlation with CSV file from Servo Viewer ;
                               TO DO -> temp_pred, rotation_pred, ...

V6 : Lire en continu L1Signal_Pool_Active pour avoir une image � l'instant T des signaux afin de pouvoir faire de la pr�diction "en continu"

TO DO :

V8 : Faire N fichiers CSV par Servo Viewer (1 par axe) avec : POS,SPEED,CURRENT%,CURRENT,LOAD,ERROR,??,?? (8 data max)
     + param�trer le calcul de distance selon si l'on est sur un axe gradu� en mm ou en deg ...

**********************/

int main(int argc, char** argv)
{
    //allConf db = first_steps(); /** Do the first steps : DB => SPARSE DB => OUT DATA **/

    //machining_info mcn_info = make_predictor_steps(db); /** Do the predictors construction steps **/

    launch_python_script(); /** Launch python script for Machine Learning algorithm **/

    //alarm_history_step(std::cref(mcn_info)); /** Get alarm history for stats **/

    PAUSE

    //predictions_step(); /** Do the predictions step **/

    return 0;
}












