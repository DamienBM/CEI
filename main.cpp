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
       OK    =>      OK     => done : fan_pred ;
                               to do : temp_pred, dist_pred, rotation_pred, ...

A venir ....

V7 : Lire en continu L1Signal_Pool_Active pour avoir une image à l'instant T des signaux afin de pouvoir faire de la prédiction "en continu"

**********************/

int main(int argc, char** argv)
{
    allConf db;
    db = first_steps(); /** Do the first steps : DB => SPARSE DB => OUT DATA **/

    make_predictor_steps(db);

    PAUSE

    return 0;
}

void sparse_db(std::ifstream& fichier, allConf& db){
    /** Sparse DB and fill vector from db **/
    std::string chaine;
    double timespan_msec = 0;

    //std::cout << "Fichier ouvert !" << std::endl;
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

void delete_stuff(std::string cur_dir){
    /** Suppresion des fichiers de 0Ko **/
    std::string del_cmd = "for /r " + cur_dir + "\\out %i in (*.txt) do if %~zi == 0 del %i";
    system(del_cmd.c_str());

    /** Suppression du dossier db (éviter de fuiter l'accès aux données) **/
    std::string tmp_rmdir = "rmdir /s /q " + cur_dir + "\\DB";
    system(tmp_rmdir.c_str());
}

int ecriture(info_sig sig){

    std::string cur_dir = _getcwd(NULL,0);
    std::string path_out = cur_dir+"\\out\\";
    std::string file_out;

    file_out = path_out + sig.signalName + "_TimeCycle_" + std::to_string(sig.readCycle) + ".txt";
    std::ofstream fichier_out(file_out, std::ios::out|std::ios::trunc);

    if(fichier_out){
        for(unsigned int j = 0; j != sig.values.size(); ++j)
            fichier_out << sig.values[j] << std::endl;
        fichier_out.close();
    }else{
        std::cout << "Pb lors de l'ecriture des fichiers out !" << std::endl;
        PAUSE
        exit(EXIT_FAILURE);
    }
    return 1;
}

void ecriture_thread(allConf db){

    std::chrono::milliseconds span (10);
    unsigned int n = std::thread::hardware_concurrency(); // 8 threads au max en même temps

    std::vector<std::future<int>> tab_fut;
    std::future<int> fut;
    unsigned int j = 0;

    for (j = 0; j<n && j<db.size();++j){
        //init threads here
        tab_fut.push_back(std::async(std::launch::async,ecriture,db[j]));
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
    //system("chemin_fichier_batch"); pour lancer les *.bat

    /** get le current directory **/
    std::string cur_dir = _getcwd(NULL,0);


    std::string tmp_mkdir("");
    tmp_mkdir = "mkdir " + cur_dir + "\\DB";

    /** Créer le repertoire pour stocker la BDD **/
    /*if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + cur_dir + "\\DB";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }*/


    tmp_mkdir = "mkdir " + cur_dir + "\\batch";
    /** Créer le repertoire pour les fichiers bat **/
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + cur_dir + "\\batch";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

    //tmp_mkdir = "mkdir " + cur_dir + "\\out";
    /** Créer le repertoire pour les fichiers out **/
    /*if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + cur_dir + "\\out";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }*/

    /** Construction requete MongoDB **/
    std::string path = cur_dir + "\\batch\\Signal.bat";
    std::ofstream fichier(path, std::ios::out|std::ios::trunc);
    if(fichier){
        fichier << "path C:\\FANUC\\MT-LINKi\\MongoDB\\bin" << std::endl << "mongoexport /d MTLINKi /u fanuc /p fanuc /c L1Signal_Pool /o "<< cur_dir << "\\db\\mtlinki_Signal.txt" <<std::endl;
        fichier.close();
    }else
        std::cout << "Oops !" << std::endl;

    /** Lancement de la requête **/
    system(path.c_str());

    /** Suppression du dossier batch (éviter de fuiter l'accès aux données) **/
    std::string tmp_rmdir("");
    tmp_rmdir = "rmdir /s /q " + cur_dir + "\\batch";
    system(tmp_rmdir.c_str());

    /** Récupération des infos dans L0Setting **/
    std::ifstream L0_Setting("C:\\FANUC\\MT-LINKi\\MT-LINKiCollector\\Setting\\L0_Setting.json",std::ios::in);
    std::string chaine;

    if(L0_Setting){
        //std::cout << "Fichier ouvert !" << std::endl;
        struct info_sig dummy;
        std::string sig_name;
        while(std::getline(L0_Setting,chaine)){
            char *ptr = strtok((char*)chaine.c_str(),"{}$,:\"");

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

    allConf db;

    /** CREATE DIR & REQ MONGODB & FILL THE L0NAME OF ALL SIG IN db **/
    db = create_DB();

    t2 = clock();
    temps_read_db = (float)(t2-t1)/CLOCKS_PER_SEC;
    std::cout << "Temps lecture base de donnees : " << temps_read_db << std::endl;
    t1 = clock();

    std::string cur_dir = _getcwd(NULL,0);
    std::string file_in = cur_dir + "\\DB\\mtlinki_Signal.txt";

    /** PAUSE FOR HAVING TIME TO COPY/PASTE THE DB BASE FILE **/
    //PAUSE

    std::ifstream fichier;
    fichier.open(file_in, std::ios::in);

    if (fichier.is_open())
    {
        /** Sparse db and fill vector from db **/
        sparse_db(fichier,db);

        fichier.close(); // On ferme le fichier

        t2 = clock();
        temps_write_db = (float)(t2-t1)/CLOCKS_PER_SEC;
        std::cout << "Temps ecriture base de donnees fichier txt : " << temps_write_db << std::endl;

        t1 = clock();

        /** Write files in out directory **/
        //ecriture_thread(db);

        t2 = clock();
        temps_write_out = (float)(t2-t1)/CLOCKS_PER_SEC;
        std::cout << "Temps ecriture fichiers out : " << temps_write_out << std::endl;
        t1=clock();

    }else
        std::cout << "Impossible d'ouvrir le fichier : mtlinki_Signal.txt" << std::endl;

    /** Delete DB directory and 0 Ko files in OUT directory **/
    //delete_stuff(cur_dir);

    std::cout << std::endl << "First steps completed !" << std::endl;

    t2 = clock();
    temps_all = ((float)(t2-t1)/CLOCKS_PER_SEC)+temps_read_db+temps_write_db+temps_write_out;

    std::cout << "Temps d'execution des premieres etapes : " << temps_all << std::endl;

    return db;
}

allConf filtering_db(const allConf& db){

    /** SPARSE DB WITH ALL FAN SIGNALS **/
    std::string fan("Fan");
    std::vector<std::string> file_to_open;
    allConf db_fan;
    for (unsigned int i=0; i<db.size(); ++i){
        if(db[i].signalName.find(fan) != std::string::npos){
            db_fan.push_back(db[i]);//works
        }
    }

    std::vector<int> idx_at_erase;

    for(unsigned int i=0; i<db_fan.size(); ++i){
        unsigned long int cpt=0;
        for(unsigned int j=0; j<db_fan[i].values.size(); ++j){
            if(db_fan[i].values[j] == 0)
                cpt++;
        }
        if(cpt == db_fan[i].values.size()) /** IF ALL VALUES ARE EQUAL TO 0 ... **/
            idx_at_erase.push_back(i);
    }

    /** MUST ERASE FORM THE END OF THE VECTOR**/
    for (unsigned int i=idx_at_erase.size(); i>0; --i)
        db_fan.erase(db_fan.begin()+idx_at_erase[i-1]);

    /** DB_FAN IS A VECTOR FILLED WITH USEFUL SIGNALS (WITH SOME ZEROS IN THEM ...) FROM FAN SPEED FROM HERE **/

    /** MUST REMOVE ALL 0 INSIDE VALUES ... **/
    for(unsigned int i=0; i<db_fan.size(); ++i){
        for(unsigned int j=db_fan[i].values.size(); j>0; --j){
            if(db_fan[i].values[j-1] == 0)
                db_fan[i].values.erase(db_fan[i].values.begin()+(j-1));
        }
    }

    return  db_fan;

}

void save_and_plot_predictors(const vectPred& pred_fan,const allConf& db_fan){
    /** SAVE THE PREDICTOR IN A TXT FILE  FROM VECTOR PRED_FAN**/
    std::string cur_dir = _getcwd(NULL,0);
    using namespace std::chrono_literals;//enable to write 10ms or 1s

    /** Créer le repertoire pour les fichiers pred*.txt **/
    std::string tmp_mkdir("mkdir " + cur_dir + "\\pred");
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + cur_dir + "\\pred";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

    std::string filename(cur_dir+"\\pred\\pred_fan.txt");
    std::ofstream fichier(filename, std::ios::out|std::ios::trunc);

    if(fichier){
            fichier << "Signal Name" << " , " << "Mean Value" << " , " << "Standard deviation value" << "\n";
        for(auto pred : pred_fan)
            fichier << pred.sig_name << "," << pred.mean << "," << pred.std_dev << "\n";
        fichier.close();
    }else
        std::cout << "Something went wrong with prediction fan output txt file ..." << std::endl;

    /** PLOT WITH GNUPLOT DATA AND PREDICTION THRESHOLD FOR EACH SIGNAL **/
    for(unsigned int i=0; i<db_fan.size(); ++i){
        filename = cur_dir+"\\pred\\pred_fan_"+db_fan[i].signalName+".plt";
        std::ofstream gnu_file(filename,std::ios::out|std::ios::trunc);
        if(gnu_file){
            gnu_file << "plot '-' using 1:2 title \"data\" lc rgb '#00ff00','' using 1:2 title \"thresh\" with lines lc rgb '#00ff00'" << "\n";
            unsigned int cpt = 0;
            double thresh = pred_fan[i].mean - 2*pred_fan[i].std_dev;
            std::for_each(std::begin(db_fan[i].values), std::end(db_fan[i].values), [&gnu_file,&cpt](const double val) {
                gnu_file << cpt++ << " " << val << "\n";
            });
            cpt=0;
            gnu_file << "e" << "\n";
            std::for_each(std::begin(db_fan[i].values), std::end(db_fan[i].values), [&gnu_file,&cpt,&thresh](const double val) {
                gnu_file << cpt++ << " " << thresh << "\n";
            });
            gnu_file.close();
        }else
            std::cout << std::endl << "Something went wrong with gnuplot data file ..." << std::endl;

        /** APPEL SYSTEM PAR THREAD DETACHED MAIS PRG ATTEND QUE LES THREADS SOIENT FERMES POUR QUITTER NORMALEMENT LE MAIN **/
        std::thread([&filename](){std::string cmd("wgnuplot -persist "+filename);system(cmd.c_str());}).detach();
        std::this_thread::sleep_for(1ms); // for temporisation otherwise it crashes !
    }
}

void fan_prediction(const allConf& db){

    /** FIRST STEP : REMOVE ALL USELESS SIGNALS **/
    allConf db_fan = filtering_db(std::cref(db));

    /** NEXT STEP IS TO CALCULATE MEAN AND STANDARD DEVIATION FOR EACH SIGNAL **/
    vectPred pred_fan;

    for(unsigned int i=0; i<db_fan.size(); ++i){
        double sum = std::accumulate(std::begin(db_fan[i].values), std::end(db_fan[i].values), 0.0);
        double mean =  sum / db_fan[i].values.size();
        double accum = 0.0;
        std::for_each (std::begin(db_fan[i].values), std::end(db_fan[i].values), [&](const double d) {
            accum += (d - mean) * (d - mean);
        });
        double stdev = std::sqrt(accum / (db_fan[i].values.size()-1));
        pred_fan.push_back(stat_pred(db_fan[i].signalName,mean,stdev));
    }

    save_and_plot_predictors(std::cref(pred_fan),std::cref(db_fan));
}

void temp_motor_prediction(const allConf& db){

//std::cout << std::endl << "Temp motor prediction only available if Tambiant = 20 degrees Celsius !" << std::endl;

}

void make_predictor_steps(allConf db){ /** Out a predictor object in a file : /pred/predfan,... **/

    std::cout << std::endl << "Pred step" << std::endl;
    clock_t t1,t2;

    t1=clock();

    /** Step 1 (Thread 1) : Fan speed stuff**/
    std::thread th_fan(fan_prediction,std::cref(db)); /** std::ref car besoin d'une référence pour fan_prediction**/

    /** STEP 2 (Thread 2) : Temp Motor **/
    std::thread th_temp(temp_motor_prediction,std::cref(db));

    /** Join of each thread **/
    th_fan.join();
    th_temp.join();
    t2 = clock();

    std::cout << std::endl << "Temps d'execution des etapes de prediction : " << ((float)(t2-t1)/CLOCKS_PER_SEC) << std::endl;
}










