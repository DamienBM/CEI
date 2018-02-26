#include "header.h"

/**********************
Done :

V1 : Programme réussi à récupérer toutes les valeurs d'un signal
     que l'on demande par l'instanciation de test (std::string).

V2 : On récupère toutes les valeurs du signal nommé partiellement par test
     afin de tracer ensuite sous Excel les données.

V3 : Fonctionne sur pc distant en "copie/coll"-ant le .exe du projet vers le pc distant

V4 : Récupère les données et les met dans une structure de type info_sig
et sort les signaux en fichier txt et supprime les fichiers de 0Ko

V5 : amélioration avec des thread & future lors de l'écriture des fichiers dans /out/...
     Les threads retourne une valeur (int) et on attends que les thread

V6 : séparation du code en plusieurs morceaux selon schéma ci-après
     MongoDB => dossier out => plusieurs thread (un pour la temp, un pour la distance, un pour le sens de rotation etc)
       OK    =>      OK     => done  -> fan_pred with gaussian model : thresh = (µ-3*sigma) ;
                                     -> calculate distance with CSV file from Servo Viewer ;
                                     -> Load histogram ;
                                     -> calculate drilling cycle by auto-correlation with CSV file from Servo Viewer ;
                               TO DO -> temp_pred, rotation_pred, ...

V7 : Lire en continu L1Signal_Pool_Active pour avoir une image à l'instant T des signaux afin de pouvoir faire de la prédiction "en continu"

**********************/

int main(int argc, char** argv)
{
    allConf db = first_steps(); /** Do the first steps : DB => SPARSE DB => OUT DATA **/

    make_predictor_steps(db); /** Do the predictors construction steps **/

    PAUSE

    //predictions_step(); /** Do the predictions step **/

    return 0;
}

/** GET DB STEP FUNCTIONS **/

void sparse_db(std::ifstream& fichier, allConf& db){
    /** Sparse DB and fill vector from db **/
    std::string chaine;
    double timespan_msec = 0;

    while(std::getline(fichier,chaine)){

        char *ptr = strtok((char*)chaine.c_str(),"{}$,:\"");

        while(ptr != NULL){

            while(strcmp(ptr,"timespan")!=0){
                ptr = strtok(NULL,"{}$,:\"");
            }
            timespan_msec = std::atof(strtok(NULL,"{}$,:\""))*MSEC;


            while(strcmp(ptr,"signalname")!=0){
                ptr = strtok(NULL,"{}$,:\"");
            }

            ptr = strtok(NULL,"{}$,:\"");
            //ici ptr vaut le nom du signal dans L1Signal_Pool

            for(auto it = db.begin(); it != db.end();++it){
                if(strcmp(ptr,it->signalName.c_str())==0){
                    ptr = strtok(NULL,"{}$,:\"");
                    ptr = strtok(NULL,"{}$,:\"");
                    //ici ptr vaut la valeur du signal
                    for(int nb_pts = 0; nb_pts < (timespan_msec/(it->readCycle));++nb_pts)
                        it->values.push_back(std::atof(ptr));
                    break;
                }
            }

            while(ptr != NULL)
                ptr = strtok(NULL,"{}$,:\"");
        }
    }
}

void delete_stuff(void){
    /** Suppresion des fichiers de 0Ko **/
    std::string del_cmd = "for /r " + CUR_DIR + "\\out %i in (*.txt) do if %~zi == 0 del %i";
    system(del_cmd.c_str());

    /** Suppression du dossier db (éviter de fuiter l'accès aux données) **/
    std::string tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\DB";
    system(tmp_rmdir.c_str());
}

int ecriture(const info_sig& sig){

    std::string cur_dir = _getcwd(NULL,0);
    std::string path_out = cur_dir+"\\out\\";
    std::string file_out;

    file_out = path_out + sig.signalName + "_TimeCycle_" + std::to_string(sig.readCycle) + ".plt";
    std::ofstream fichier_out(file_out, std::ios::out|std::ios::trunc);

    if(fichier_out){
        fichier_out << "set title \"" << sig.signalName <<"\"\n" << "set xlabel \"Indice\"\n" << "set ylabel \"Amplitude\"\n" << "plot '-' using 1:2 title \"data\" lc rgb '#ff0000'" << "\n";
        for(unsigned int j = 0; j != sig.values.size(); ++j)
            fichier_out << j << " " << sig.values[j] << std::endl;
        fichier_out.close();
    }else{
        std::cout << "Pb lors de l'ecriture des fichiers out !" << std::endl;
        PAUSE
        exit(EXIT_FAILURE);
    }
    return 1;
}

void ecriture_thread(const allConf& db){

    std::chrono::milliseconds span (10);
    unsigned int n = std::thread::hardware_concurrency(); // n threads au max en même temps

    std::vector<std::future<int>> tab_fut;
    std::future<int> fut;
    unsigned int j = 0;

    for (j = 0; j<n && j<db.size();++j){
        //init threads here
        tab_fut.push_back(std::async(std::launch::async,ecriture,std::cref(db[j])));
    }

    if(j==db.size()){
        for(unsigned int k = 0; k<n; ++k)
            tab_fut[k].wait();
    }
    else{
        std::cout << "Ecriture fichier dans /out sur " << tab_fut.size() <<  " threads." << std::endl;
        unsigned int i = j; //i = n ...
        while (i < db.size()){
            for (unsigned int j = 0; j<tab_fut.size(); ++j){
                if(tab_fut[j].wait_for(span) == std::future_status::ready){
                    tab_fut[j] = std::async(std::launch::async,ecriture,db[i]);
                    i++;
                }
            }
        }
    }
}

allConf create_DB(void){
    allConf db;

    std::string tmp_mkdir("");
    tmp_mkdir = "mkdir " + CUR_DIR + "\\DB";

    /** Créer le repertoire pour stocker la BDD **/
    /*if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\DB";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }*/


    tmp_mkdir = "mkdir " + CUR_DIR + "\\batch";
    /** Créer le repertoire pour les fichiers bat **/
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\batch";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

    //tmp_mkdir = "mkdir " + CUR_DIR + "\\out";
    /** Créer le repertoire pour les fichiers out **/
    /*if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\out";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }*/

    /** Create directory for pred files **/
    tmp_mkdir = "mkdir " + CUR_DIR + "\\pred";
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\pred";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

    /** Construction requete MongoDB **/
    std::string path = CUR_DIR + "\\batch\\Signal.bat";
    std::ofstream fichier(path, std::ios::out|std::ios::trunc);
    if(fichier){
        fichier << "path C:\\FANUC\\MT-LINKi\\MongoDB\\bin" << std::endl << "mongoexport /d MTLINKi /u fanuc /p fanuc /c L1Signal_Pool /o "<< CUR_DIR << "\\db\\mtlinki_Signal.txt" <<std::endl;
        fichier.close();
    }else
        std::cout << "Oops !" << std::endl;

    /** Lancement de la requête **/
    system(path.c_str());

    /** Suppression du dossier batch (éviter de fuiter l'accès aux données) **/
    std::string tmp_rmdir("");
    tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\batch";
    system(tmp_rmdir.c_str());

    /** Récupération des infos dans L0Setting **/
    std::ifstream L0_Setting("C:\\FANUC\\MT-LINKi\\MT-LINKiCollector\\Setting\\L0_Setting.json",std::ios::in);
    std::string chaine;

    if(L0_Setting){
        //std::cout << "Fichier ouvert !" << std::endl;
        info_sig dummy;
        std::string sig_name;
        while(std::getline(L0_Setting,chaine)){
            char* ptr = strtok((char*)chaine.c_str(),"{}$,:\"");

            while(ptr != NULL){
                if(strcmp(ptr,"L0Name") == 0){
                    ptr = strtok(NULL,"{}$,:\"");
                    //ptr vaut le nom de L0Name
                    dummy.L0Name = ptr;
                }

                else if(strcmp(ptr,"SignalName") == 0){
                    ptr = strtok(NULL,"{}$,:\"");
                    //ptr vaut le nom du signal
                    dummy.signalName = ptr;
                    dummy.signalName+="_"+dummy.L0Name;

                }

                else if(strcmp(ptr,"ReadCycle") == 0) {
                    ptr = strtok(NULL,"{}$,:\"");
                    //ptr vaut le temps d'échantillonage
                    dummy.readCycle = atoi(ptr);
                    db.push_back(dummy);
                }

                ptr = strtok(NULL,"{}$,:\"");
            }
        }
    }else{
        std::cerr << "Probleme lors de l'ouverture de L0_Setting" << std::endl;
        PAUSE
        exit(EXIT_FAILURE);
    }

    return db;
}

allConf first_steps(void){

    float temps_read_db,temps_write_out,temps_write_db,temps_all;
    clock_t t1, t2;
    t1 = clock();

    /** CREATE DIR & REQ MONGODB & FILL THE L0NAME OF ALL SIG IN db **/
    allConf db = create_DB();

    t2 = clock();
    temps_read_db = (float)(t2-t1)/CLOCKS_PER_SEC;
    std::cout << std::endl << "Temps lecture base de donnees : " << temps_read_db << std::endl;
    t1 = clock();

    std::string cur_dir = _getcwd(NULL,0);
    std::string file_in = cur_dir + "\\DB\\mtlinki_Signal.txt";

    std::ifstream fichier;
    fichier.open(file_in, std::ios::in);

    if (fichier.is_open())
    {
        /** Sparse db and fill vector from db **/
        sparse_db(fichier,db);

        fichier.close(); // On ferme le fichier

        t2 = clock();
        temps_write_db = (float)(t2-t1)/CLOCKS_PER_SEC;
        std::cout << std::endl << "Temps ecriture base de donnees fichier txt : " << temps_write_db << std::endl;

        t1 = clock();

        /** Write files in out directory **/
        ecriture_thread(db);

        t2 = clock();
        temps_write_out = (float)(t2-t1)/CLOCKS_PER_SEC;
        std::cout << std::endl << "Temps ecriture fichiers out : " << temps_write_out << std::endl;
        t1=clock();

    }else
        std::cout << "Impossible d'ouvrir le fichier : mtlinki_Signal.txt" << std::endl;

    /** Delete DB directory and 0 Ko files in OUT directory **/
    //delete_stuff();

    std::cout << std::endl << "First steps completed !" << std::endl;

    t2 = clock();
    temps_all = ((float)(t2-t1)/CLOCKS_PER_SEC)+temps_read_db+temps_write_db+temps_write_out;

    std::cout << std::endl << "Temps d'execution des premieres etapes : " << temps_all << std::endl;

    return db;
}

/** END GET DB STEP FUNCTIONS **/


/** PREDICTORS CONSTRUCTION STEPS FUNCTIONS **/

allConf db_filtering(const allConf& db,const std::string& filt){

    /** SPARSE DB WITH ALL FAN SIGNALS **/
    std::vector<std::string> file_to_open;
    allConf db_filt;
    for (unsigned int i=0; i<db.size(); ++i){
        if(db[i].signalName.find(filt) != std::string::npos){
            db_filt.push_back(db[i]);//works
        }
    }

    std::vector<int> idx_at_erase;

    for(unsigned int i=0; i<db_filt.size(); ++i){
        unsigned long int cpt=0;
        for(unsigned int j=0; j<db_filt[i].values.size(); ++j){
            if(db_filt[i].values[j] == 0)
                cpt++;
        }
        if(cpt == db_filt[i].values.size()) /** IF ALL VALUES ARE EQUAL TO 0 ... **/
            idx_at_erase.push_back(i);
    }

    /** MUST ERASE FROM THE END OF THE VECTOR**/
    for (unsigned int i=idx_at_erase.size(); i>0; --i)
        db_filt.erase(db_filt.begin()+idx_at_erase[i-1]);

    return  db_filt;

}

void save_and_plot_fan_predictors(const vectPred& pred_fan,const allConf& db_fan){
    /** SAVE THE PREDICTOR IN A TXT FILE  FROM VECTOR PRED_FAN**/
    //using namespace std::chrono_literals;//enable to write 10ms or 1s

    /** Create directory for pred*.txt files **/
    std::string tmp_mkdir("mkdir " + CUR_DIR + "\\pred\\pred_fan");
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\pred\\pred_fan";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

    std::string filename(CUR_DIR+"\\pred\\pred_fan\\pred_fan.txt");
    std::ofstream fichier(filename, std::ios::out|std::ios::trunc);

    if(fichier){
            fichier << "Signal Name" << " , " << "Mean Value" << " , " << "Standard deviation value" << "," << "Quantization step" << "\n";
        for(auto pred : pred_fan)
            fichier << pred.sig_name << "," << pred.mean << "," << pred.std_dev << "," << pred.q << "\n";
        fichier.close();
    }else
        std::cout << "Something went wrong with prediction fan output txt file ..." << std::endl;

    /** PLOT WITH GNUPLOT DATA AND PREDICTION THRESHOLD FOR EACH SIGNAL **/
    for(unsigned int i=0; i<db_fan.size(); ++i){
        filename = CUR_DIR+"\\pred\\pred_fan\\pred_fan_"+db_fan[i].signalName+".plt";
        std::ofstream gnu_file(filename,std::ios::out|std::ios::trunc);
        if(gnu_file){

            gnu_file << "set title \""<< db_fan[i].signalName <<"\"\n" << "set xlabel \"Indice\"\n" << "set ylabel \"Speed (tr/min)\"\n" << "plot '-' using 1:2 title \"data\" lc rgb '#ff0000','' using 1:2 title \"thresh\" with lines lc rgb '#0000ff'" << "\n";
            unsigned int cpt = 0;
            std::for_each(std::begin(db_fan[i].values), std::end(db_fan[i].values), [&gnu_file,&cpt](const double val) {
                gnu_file << cpt++ << " " << val << "\n";
            });
            cpt=0;
            double thresh = pred_fan[i].mean - 3*pred_fan[i].std_dev; // µ-3*sigma car P( X > µ-3*sigma) = 0.9985 (avec sigma = sigma_data + q)
            gnu_file << "e" << "\n";
            std::for_each(std::begin(db_fan[i].values), std::end(db_fan[i].values), [&gnu_file,&cpt,&thresh](const double val) {
                gnu_file << cpt++ << " " << thresh << "\n";
            });
            gnu_file.close();
        }else
            std::cout << std::endl << "Something went wrong with fan gnuplot data file ..." << std::endl;

        /** APPEL SYSTEM PAR THREAD DETACHED MAIS PRG ATTEND QUE LES THREADS SOIENT FERMES POUR QUITTER NORMALEMENT LE MAIN **/
        /*std::thread([&filename](){std::string cmd("wgnuplot -persist "+filename);system(cmd.c_str());}).detach();
        std::this_thread::sleep_for(1ms);*/ // for temporisation otherwise it crashes !
    }
}

void save_and_plot_load_pred (const vectLoadStat& load){

    /** SAVE THE PREDICTOR IN A TXT FILE  FROM VECTOR PRED_FAN**/

    /** Create directory for pred*.txt files **/
    std::string tmp_mkdir("mkdir " + CUR_DIR + "\\pred\\pred_load");
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\pred\\pred_load";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

    std::string filename(CUR_DIR+"\\pred\\pred_load\\pred_load.txt");
    std::ofstream fichier(filename, std::ios::out|std::ios::trunc);

    /** TXT FILE **/
    if(fichier){
            fichier << "Signal Name" << " , " << "0_50_range" << " , " << "51_100_range" << " , " << "101_150_range" << " , "
                    << "151_200_range" << " , " << "201_250_range" << " , " << "251_300_range" << " , " << "301_350_range" << " , "
                    << "351_400_range" << " , " << "401_450_range" << " , " << "451_500_range"
                    << "\n";
        for(auto sig : load)
            fichier << sig.sig_name << "," << sig._0_50_range << "," << sig._51_100_range << "," << sig._101_150_range << ","
                    << sig._151_200_range << "," << sig._201_250_range << "," << sig._251_300_range << "," << sig._301_350_range << ","
                    << sig._351_400_range << "," << sig._401_450_range << "," << sig._451_500_range
                    << "\n";
        fichier.close();
    }else
        std::cout << "Something went wrong with prediction load output txt file ..." << std::endl;

    /** PLOT FILES **/
    for(unsigned int i=0; i<load.size(); ++i){

        filename = CUR_DIR+"\\pred\\pred_load\\load_pred_"+load[i].sig_name+".plt";
        std::ofstream gnu_file(filename,std::ios::out|std::ios::trunc);

        if(gnu_file){

            gnu_file << "set title \""<< load[i].sig_name <<"\"\n"
                     << "set xlabel \"Load (stack of 50)\"\n" << "set ylabel \"Number of times per range / Total number of times (%)\"\n"
                     << "plot '-' using 1:2 title \"data\" lc rgb '#0000ff' with lines\n";
            unsigned int val=0;
            for(unsigned int cpt=0; cpt<10; ++cpt){// don't go further than 500 !!!
                int tmp = cpt*50;


                if(cpt==0)       val = load[i]._0_50_range;
                else if(cpt==1) {val = load[i]._51_100_range;  gnu_file << tmp << " " << val << "\n";}
                else if(cpt==2) {val = load[i]._101_150_range; gnu_file << tmp << " " << val << "\n";}
                else if(cpt==3) {val = load[i]._151_200_range; gnu_file << tmp << " " << val << "\n";}
                else if(cpt==4) {val = load[i]._201_250_range; gnu_file << tmp << " " << val << "\n";}
                else if(cpt==5) {val = load[i]._251_300_range; gnu_file << tmp << " " << val << "\n";}
                else if(cpt==6) {val = load[i]._301_350_range; gnu_file << tmp << " " << val << "\n";}
                else if(cpt==7) {val = load[i]._351_400_range; gnu_file << tmp << " " << val << "\n";}
                else if(cpt==8) {val = load[i]._401_450_range; gnu_file << tmp << " " << val << "\n";}
                else            {val = load[i]._451_500_range; gnu_file << tmp << " " << val << "\n";}

                for(unsigned int j=1+tmp; j<50+tmp+1; ++j)
                    gnu_file << j << " " << val << "\n";
            }

            gnu_file.close();
        }else
            std::cout << std::endl << "Something went wrong with load gnuplot data file ..." << std::endl;

    }

}

void fan_prediction(const allConf& db){

    /** FIRST STEP : REMOVE ALL USELESS SIGNALS **/
    const std::string fan = "Fan";
    allConf db_fan = db_filtering(std::cref(db),fan);

    /** DB_FAN IS A VECTOR FILLED WITH USEFUL SIGNALS (WITH SOME ZEROS IN THEM ...) FROM FAN SPEED FROM HERE **/
    /** MUST REMOVE ALL 0 INSIDE VALUES ... **/
    allConf db_filtered;
    for(unsigned int i=0; i<db_fan.size(); ++i){

        info_sig dum(db_fan[i].L0Name,db_fan[i].signalName,db_fan[i].readCycle,db_fan[i].q);

        for(unsigned int j=0; j<db_fan[i].values.size(); ++j){ // fill values vector of dum with only non-zero values
            if(db_fan[i].values[j] != 0)
                dum.values.push_back(db_fan[i].values[j]);
        }
        db_filtered.push_back(dum);
    }

    /** NEXT STEP IS TO CALCULATE MEAN AND STANDARD DEVIATION FOR EACH SIGNAL **/
    vectPred pred_fan;

    for(unsigned int i=0; i<db_filtered.size(); ++i){
        /** Get the quantification step **/
        double d_min=100000000000;
        double old_val=db_filtered[i].values[0];
        std::for_each (std::begin(db_filtered[i].values)+1, std::end(db_filtered[i].values), [&d_min,&old_val](const double val){if(val!=0){if(std::fabs(val-old_val))d_min = std::fabs(val-old_val);old_val = val;}});
        if(d_min == 0)
            db_filtered[i].q = 200; //default value
        else
            db_filtered[i].q = d_min;

        /** Calculate mean and std_dev **/
        double sum = std::accumulate(std::begin(db_filtered[i].values), std::end(db_filtered[i].values), 0.0);
        double mean =  sum / db_filtered[i].values.size();
        double accum = 0.0;
        std::for_each (std::begin(db_filtered[i].values), std::end(db_filtered[i].values), [&mean,&accum](const double x){accum += (x - mean) * (x - mean);});
        double stdev = std::sqrt(accum / (db_filtered[i].values.size()-1));
        pred_fan.push_back(stat_pred(db_filtered[i].signalName,mean,stdev+db_filtered[i].q,db_filtered[i].q));
    }

    save_and_plot_fan_predictors(std::cref(pred_fan),std::cref(db_filtered));
}

void load_motor_prediction(const allConf& db){

    /** FIRST STEP : REMOVE ALL USELESS SIGNALS **/
    const std::string load_str = "Load";
    allConf db_load = db_filtering(std::cref(db),load_str);

    vectLoadStat load;
    load_stat dummy;

    for(unsigned int i=0;i<db_load.size();++i){
        dummy.sig_name = db_load[i].signalName;
        double sum = db_load[i].values.size();
        std::for_each(std::begin(db_load[i].values),std::end(db_load[i].values),[&dummy,&sum](const double val){
                        if(val<=50)       dummy._0_50_range++;
                        else if(val<=100) dummy._51_100_range++;
                        else if(val<=150) dummy._101_150_range++;
                        else if(val<=200) dummy._151_200_range++;
                        else if(val<=250) dummy._201_250_range++;
                        else if(val<=300) dummy._251_300_range++;
                        else if(val<=350) dummy._301_350_range++;
                        else if(val<=400) dummy._351_400_range++;
                        else if(val<=450) dummy._401_450_range++;
                        else              dummy._451_500_range++;
                      });

        dummy._0_50_range*=(100/sum);
        dummy._51_100_range*=(100/sum);
        dummy._101_150_range*=(100/sum);
        dummy._151_200_range*=(100/sum);
        dummy._201_250_range*=(100/sum);
        dummy._251_300_range*=(100/sum);
        dummy._301_350_range*=(100/sum);
        dummy._351_400_range*=(100/sum);
        dummy._401_450_range*=(100/sum);
        dummy._451_500_range*=(100/sum);

        load.push_back(dummy);

    }

    /** HERE load IS FULLFILLED WITH USEFUL SIGNALS **/

    save_and_plot_load_pred(load);
}

std::vector<double> calculate_dist(const all_dist& axes){

    std::vector<double> all_dist;


    for(auto& axe : axes){
        double dist=0;
        double old_val=axe[0];

        std::for_each(axe.begin()+1,axe.end(),[&dist,&old_val](const double val){ dist += std::fabs(val-old_val); old_val = val;});

        all_dist.push_back(dist);
    }
    return all_dist;
}

std::vector<double> get_periode(const all_dist& axes, const double& Te_servo){

    std::vector<std::vector<double>> all_auto_corr;
    std::vector<double> res_tmp;

    //Normalize the signal maybe ???

    /** AUTCO-CORRELATION **/
    for(auto& axe : axes){
        res_tmp.resize(axe.size());
        double val=0;
		for (unsigned int k = 0; k<axe.size();k++){
            for(unsigned int i = k; i<axe.size();i++){
                val += axe[i] * axe[i-k];
            }
            res_tmp[k] = val/axe.size();
            val = 0;
		}
		all_auto_corr.push_back(res_tmp);
    }

    /** CREATE FILE FOR PLOTTING **/
    std::string tmp_mkdir("mkdir " + CUR_DIR + "\\pred\\dist");
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\pred\\dist";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }
    unsigned num_axe=0;

    std::vector<double> thresh;
    /** PLOT WITH GNUPLOT DATA AND PREDICTION THRESHOLD FOR EACH SIGNAL **/
    for(auto& auto_cor : all_auto_corr){
        std::string filename = CUR_DIR+"\\pred\\dist\\auto_corr_axe_"+std::to_string(num_axe)+".plt";
        std::ofstream gnu_file(filename,std::ios::out|std::ios::trunc);

        if(gnu_file){

            gnu_file << "set title \"Auto-correlation of axes number "<< num_axe <<"\"\n"
                     << "set xlabel \"Tau\"\n" << "set ylabel \"Amplitude\"\n"
                     << "plot '-' using 1:2 title \"auto-corr\" lc rgb '#0000ff' with lines\n";

            unsigned int cpt=0;
            double _max=0;
            std::for_each(std::begin(auto_cor),std::end(auto_cor),[&_max,&gnu_file,&cpt](const double val){if(val>_max) _max=val;
                                                                                                          gnu_file << cpt++ << " " << val << "\n";
                                                                                                         });
            thresh.push_back(_max*2/3);

            gnu_file.close();
        }else
            std::cout << std::endl << "Something went wrong with data file ..." << std::endl;
        num_axe++;
    }



    /** THRESHOLDING THE AUTO-CORRELATION **/
    std::vector<double> T_cycle_usinage;
    for(unsigned int i=0; i<all_auto_corr.size();++i){ // auto_cor is a vector of double
        for(unsigned int u=0; u<all_auto_corr[i].size(); ++u){if(all_auto_corr[i][u]<thresh[i]) all_auto_corr[i][u]=0;} //value under thresh are equal to zero from here
        for(unsigned int id = 1; id<all_auto_corr[i].size()-1; ++id){
            if( ( (all_auto_corr[i][id] - all_auto_corr[i][id-1]) > 0 ) && ( (all_auto_corr[i][id+1] - all_auto_corr[i][id]) < 0 ) ){
                T_cycle_usinage.push_back(id);
                break;
            }
        }
        // got all peak id from here

        for(auto& To : T_cycle_usinage) To = To*Te_servo/1000.0;

    }

    return T_cycle_usinage;
}

std::vector<std::vector<double>> read_csv_files(std::vector<std::string> files,double& Te_servo){

    std::vector<std::vector<double>> X_Y;
    std::vector<double> X,Y;

    for(auto& file : files){

        std::ifstream fichier(file);
        if(fichier){
            /** TO DO  GET X AND Y POS**/
            std::string line;
            std::getline(fichier,line);
            std::getline(fichier,line);
            std::getline(fichier,line);
            std::getline(fichier,line);
            int Te_line = 4; // sixth line
            while(std::getline(fichier,line)){
                if(Te_line<6) Te_line++;
                std::cout.precision(10);
                char* ptr = strtok((char*)line.c_str(),",");
                ptr = strtok(NULL,",");
                if(Te_line == 6) {Te_servo = std::atof(ptr);Te_line++;}
                X.push_back(std::atof(strtok(NULL,",")));
                Y.push_back(std::atof(strtok(NULL,",")));

            }
        }else
            std::cout << "Something went wrong when opening CSV file " << file << std::endl;
        X_Y.push_back(X);
        X_Y.push_back(Y);
    }
    return X_Y;
}

std::vector<std::string> get_csv_files(void){

    boost::filesystem::path target(CUR_DIR+"\\CSV");
    std::vector<std::string> test;
    for( auto& p : boost::filesystem::directory_iterator( target ) )
        test.push_back(p.path().string()); //test is the std::string filename

    return test;
}

void make_predictor_steps(const allConf& db){ /** Out a predictor object in a file : /pred/predfan,... **/

    std::cout << std::endl << "Pred step" << std::endl;
    clock_t t1,t2;

    t1=clock();

    /** Step 1 (Thread 1) : Fan speed stuff**/
    std::thread th_fan(fan_prediction,std::cref(db)); /** std::ref car besoin d'une référence pour fan_prediction**/

    /** STEP 2 (Thread 2) : Temp Motor **/
    std::thread th_load(load_motor_prediction,std::cref(db));

    /** Join of each thread **/
    th_fan.join();
    th_load.join();

    std::cout << std::endl << "Temps d'execution des etapes de prediction : " << ((float)(t2-t1)/CLOCKS_PER_SEC) << std::endl;
    t1 = clock();

    /** CSV FILES PART **/
    double Te_servo=0;
    csv_filename csv_files = get_csv_files();
    all_dist axes = read_csv_files(csv_files,Te_servo);
    total_dist_vect dist_axes = calculate_dist(std::cref(axes));
    std::vector<double> periode_usinage = get_periode(std::cref(axes),Te_servo);

    std::cout.precision(10);
    std::cout << "distance sur X : " << dist_axes[0]/10000 << " m et sur Y : " << dist_axes[1]/10000 << " m" << std::endl;
    std::cout << "en 2.01 heures (7239.4 sec)" << std::endl;

    double T_cycle_max=0;
    for_each(std::begin(periode_usinage),std::end(periode_usinage),[&T_cycle_max](const double val){if(val>T_cycle_max)T_cycle_max=val;});

    std::cout << std::endl << "T_cycle_usinage = " << T_cycle_max << " sec." << std::endl;

    t2 = clock();

    std::cout << std::endl << "Temps d'execution des etapes de CSV : " << ((float)(t2-t1)/CLOCKS_PER_SEC) << std::endl;
}

/** END PREDICTORS CONSTRUCTION STEPS FUNCTIONS **/


/** PREDICTION STEP **/

Predictors load_predictors(void){ /** READ TXT FILEs FROM ALL SAVED PREDICTORS **/

    Predictors predictors;
    vectPred fan_pred;
    stat_pred fan_tmp;

    std::string path(CUR_DIR+"\\pred\\");

    /** LOAD FAN PRED **/

    std::string file_pred(path + "\\pred_fan.txt");
    std::ifstream pred_fan_file(file_pred,std::ios::in);
    if(pred_fan_file){
        std::string chaine;
        std::getline(pred_fan_file,chaine);/** SKIP THE FIRST LINE **/
        while(std::getline(pred_fan_file,chaine)){
            fan_tmp.sig_name = strtok((char*)chaine.c_str(),",");
            fan_tmp.mean = std::atof(strtok(NULL,","));
            fan_tmp.std_dev = std::atof(strtok(NULL,","));
        }
        predictors.fan_pred.push_back(fan_tmp);

    }else
        std::cout << std::endl << "Something went wrong during loading of fan predictor ! " << std::endl;

    /** END LOAD FAN PRED **/

    return predictors;
}

void predict(const allConf_active& active_db,const Predictors& predictors){

    /** FAN PREDICTION **/
    for(auto& pred_fan : predictors.fan_pred){
        //pred_fan is a fan predictor from object predictors
        for(auto& active_signal : active_db){
            if(active_signal.signalName == pred_fan.sig_name)
            {
                if(active_signal.value < (pred_fan.mean-2*pred_fan.std_dev))
                    std::cout << std::endl << " Warning , signal : " << active_signal.signalName << " has a current value under the threshold !" << std::endl
                              << " At time : " << active_signal.time1 << ":" << active_signal.time2 << ":" << active_signal.time3 << std::endl;
                break;
            }
        }
    }
}

void get_active_db(void){


    /** Construction requête Mongo DB **/
    std::string active_db("\\ACTIVE_DB");
    std::string path = CUR_DIR + active_db + "\\Active_Signals.bat";
    std::ofstream fichier(path, std::ios::out|std::ios::trunc);
    if(fichier){
        fichier << "path C:\\FANUC\\MT-LINKi\\MongoDB\\bin" << std::endl << "mongoexport /d MTLINKi /u fanuc /p fanuc /c L1Signal_Pool_Active /o "<< CUR_DIR+active_db <<"\\" << ACTIVE_SIGNALS_FILE <<std::endl;
        fichier.close();
    }else
        std::cout << "Oops !" << std::endl;

    /** Lancement de la requête **/
    system(path.c_str());
}

allConf_active read_active_signals_file(void){

    allConf_active db_tmp;

    std::string path = CUR_DIR + "\\ACTIVE_DB\\" + ACTIVE_SIGNALS_FILE;
    std::ifstream fichier(path, std::ios::in);

    if(fichier){

        info_active_sig dummy;
        std::string chaine;
        char *ptr;
        while(std::getline(fichier,chaine)){
             ptr = strtok((char*)chaine.c_str(),"{}$,:\"");

             while(strcmp(ptr,"L1Name")!=0){
                ptr = strtok(NULL,"{}$,:\"");
            }
            ptr = strtok(NULL,"{}$,:\"");
            dummy.L1Name = ptr;

            while(strcmp(ptr,"updatedate")!=0){
                ptr = strtok(NULL,"{}$,:\"");
            }
            ptr = strtok(NULL,"{}$,:\"");
            dummy.time1 = strtok(NULL,"{}$,:\"");
            dummy.time2 = strtok(NULL,"{}$,:\"");
            ptr = strtok(NULL,"{}$,:\"");
            dummy.time3 = ptr;

             while(strcmp(ptr,"signalname")!=0){
                ptr = strtok(NULL,"{}$,:\"");
            }
            dummy.signalName = strtok(NULL,"{}$,:\"");
            //ici ptr vaut le nom du signal dans L1Signal_Pool

            ptr = strtok(NULL,"{}$,:\"");
            while(strcmp(ptr,"value")!=0){
                ptr = strtok(NULL,"{}$,:\"");
            }

            ptr = strtok(NULL,"{}$,:\"");

            if(strcmp(ptr,"null")!=0){
                dummy.value = std::atof(strtok(NULL,"{}$,:\""));

                while(ptr != NULL)
                    ptr = strtok(NULL,"{}$,:\"");

                db_tmp.push_back(dummy);
            }
        }

    }else
        std::cout << std::endl << "Something went wrong while opening Active Signals file !" << std::endl;

    return db_tmp;
}

int active_db_ecriture(const info_active_sig& sig){

    std::string path_out = CUR_DIR+"\\out_active\\";
    std::string file_out;

    file_out = path_out + sig.signalName + "_At_Time_" + sig.time1 + "_" + sig.time2 + "_" + sig.time3 + ".txt";

    std::ofstream fichier_out(file_out, std::ios::out|std::ios::trunc);

    if(fichier_out){
        fichier_out << sig.value << std::endl;
        fichier_out.close();
    }else{
        std::cout << "Pb lors de l'ecriture des fichiers out ! " << std::endl << "filename : " << file_out << std::endl;
        exit(EXIT_FAILURE);
    }
    return 1;
}

void active_db_ecriture_thread(const allConf_active& active_db){

    std::chrono::milliseconds span (5);
    unsigned int n = std::thread::hardware_concurrency(); // n threads au max en même temps

    std::vector<std::future<int>> tab_fut;
    std::future<int> fut;
    unsigned int j = 0;

    for (j = 0; j<n && j<active_db.size();++j){
        //init threads here
        tab_fut.push_back(std::async(std::launch::async,active_db_ecriture,active_db[j]));
    }

    if(j==active_db.size()){
        for(unsigned int k = 0; k<n; ++k)
            tab_fut[k].wait();
    }

    else{
        std::cout << "Ecriture fichier dans /out sur " << tab_fut.size() <<  " threads." << std::endl;
        unsigned int i = j; //i = n ...
        while (i < active_db.size()){
            for (unsigned int j = 0; j<tab_fut.size(); ++j){
                if(tab_fut[j].wait_for(span) == std::future_status::ready){
                    tab_fut[j] = std::async(std::launch::async,active_db_ecriture,active_db[i]);
                    i++;
                }
            }
        }
    }
}

void create_prediction_dir(void){
    std::string tmp_mkdir("mkdir ");
    std::string active_db_dir("\\ACTIVE_DB");
    tmp_mkdir += ( CUR_DIR + active_db_dir );
    system(tmp_mkdir.c_str());
    tmp_mkdir = "mkdir ";
    std::string path_out = "mkdir " + CUR_DIR+"\\out_active\\";
    system(path_out.c_str());
}

void predictions_step(void){

    create_prediction_dir(); /** CREATE ACTIVE DB DIR **/
    get_active_db(); /** STORE ACTIVE DB **/
    allConf_active active_db = read_active_signals_file();
    active_db_ecriture_thread(active_db);

    Predictors predictors = load_predictors();

    using namespace std::chrono_literals;//enable to write 10ms or 1s
    while(true){
        std::cout << std::endl << "Loop done !" << std::endl;
        predict(active_db,predictors);
        std::this_thread::sleep_for(500ms);
    }
}

/** END PREDICTION STEP **/





