#include <db_step.h>

/** GET DB STEP FUNCTIONS **/

void sparse_db(std::ifstream& fichier, allConf& db,allAlarms& alarms){
    /** Sparse DB and fill vector from db **/
    std::string chaine;
    double timespan_msec = 0;

    while(std::getline(fichier,chaine)){

        char *ptr = strtok((char*)chaine.c_str(),"{}$,:\"");

        while(ptr != NULL){

            if(chaine.find("\"signalName\":\"ALARM\",\"value\":true") != std::string::npos){
                alarm_signal alarm;
                while(strcmp(ptr,"updatedate")!=0)ptr = strtok(NULL,"{}$,:\"");
                ptr = strtok(NULL,"{}$,:\"");
                alarm.updatedate  = strtok(NULL,"{}$,:\"");
                alarm.updatedate += ":";
                alarm.updatedate += strtok(NULL,"{}$,:\"");
                alarm.updatedate += ":";
                alarm.updatedate += strtok(NULL,"{}$,:\"");
                alarm.updatedate.resize(alarm.updatedate.size()-1);

                while(strcmp(ptr,"enddate")!=0)ptr = strtok(NULL,"{}$,:\"");
                ptr = strtok(NULL,"{}$,:\"");
                alarm.enddate  = strtok(NULL,"{}$,:\"");
                alarm.enddate += ":";
                alarm.enddate += strtok(NULL,"{}$,:\"");
                alarm.enddate += ":";
                alarm.enddate += strtok(NULL,"{}$,:\"");
                alarm.enddate.resize(alarm.enddate.size()-1);

                while(strcmp(ptr,"timespan")!=0)ptr = strtok(NULL,"{}$,:\"");
                alarm.duration = std::atof(strtok(NULL,"{}$,:\""));

                alarms.push_back(alarm);
            }

            else{

                while(strcmp(ptr,"timespan")!=0)ptr = strtok(NULL,"{}$,:\"");
                timespan_msec = std::atof(strtok(NULL,"{}$,:\""))*MSEC;

                while(strcmp(ptr,"signalname")!=0)ptr = strtok(NULL,"{}$,:\"");

                ptr = strtok(NULL,"{}$,:\"");
                //ici ptr vaut le nom du signal dans L1Signal_Pool

                for(auto it = db.begin(); it != db.end();++it){
                    if(strcmp(ptr,it->signalName.c_str())==0){
                        ptr = strtok(NULL,"{}$,:\"");
                        ptr = strtok(NULL,"{}$,:\"");
                        //ici ptr vaut la valeur du signal
                        for(unsigned long nb_pts = 0; nb_pts < (timespan_msec/(it->readCycle));++nb_pts)
                            it->values.push_back(std::atof(ptr));
                        break;
                    }
                }


            }

            ptr = NULL;
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

    std::string path_out = CUR_DIR+"\\out\\";
    std::string file_out;

    file_out = path_out + sig.signalName + "_TimeCycle_" + std::to_string(sig.readCycle) + ".plt";
    std::ofstream fichier_out(file_out, std::ios::out|std::ios::trunc);

    if(fichier_out){
        fichier_out.precision(3);
        fichier_out << std::fixed;
        fichier_out << "set title \"" << sig.signalName <<"\"\n" << "set xlabel \"Temps (sec)\"\n" << "set ylabel \"Amplitude\"\n" << "plot '-' using 1:2 title \"data\" lc rgb '#ff0000'" ;

        if(sig.signalName.find(std::string("Pos"))!=std::string::npos
           || sig.signalName.find(std::string("Servo"))!=std::string::npos
           || sig.signalName.find(std::string("Spindle"))!=std::string::npos)
            fichier_out << " with lines";

        fichier_out << "\n";
        for(double j = 0; j != sig.values.size(); j += 0.5)
            fichier_out << j*sig.readCycle << " " << sig.values[j] << std::endl;
        fichier_out.close();
    }else{
        std::cout << "Pb lors de l'ecriture des fichiers out !" << std::endl;
        PAUSE
        exit(EXIT_FAILURE);
    }
    return 1;
}

void ecriture_thread(const allConf& db,const allAlarms& alarms){

    std::chrono::milliseconds span(1);
    unsigned int nb_thread_max = std::thread::hardware_concurrency(); // nb threads max en même temps

    std::vector<std::future<int>> tab_fut;
    std::future<int> fut;
    unsigned int j = 0;

    for (j = 0; j<nb_thread_max && j<db.size();++j){
        //init threads here
        tab_fut.push_back(std::async(std::launch::async,ecriture,std::cref(db[j])));
    }

    if(j==db.size()){
        for(unsigned int k = 0; k<tab_fut.size(); ++k)
            tab_fut[k].wait();
    }
    else{
        std::cout << "Ecriture fichier dans /out sur " << tab_fut.size() <<  " threads." << std::endl;
        unsigned int i = j; //i = n ...
        while (i < db.size()){
            for (unsigned int j = 0; j<tab_fut.size(); ++j){
                if(tab_fut[j].wait_for(span) == std::future_status::ready){
                    tab_fut[j] = std::async(std::launch::async,ecriture,std::cref(db[i]));
                    i++;
                }
            }
        }
    }

    std::string filename = CUR_DIR+"\\out\\Alarms.txt";
    std::ofstream file(filename,std::ios::out|std::ios::trunc);

    for(auto& alarm: alarms) {
        if(file) file << "Alarm From " << alarm.updatedate << " to " << alarm.enddate << ".Wether "<< alarm.duration <<" seconds.\n";
        else     std::cout << std::endl << "Something went wrong with Alarm file ..." << std::endl;
    }

    if(file) file << "Total number of Alarm : " << alarms.size() << "\n";
    else     std::cout << std::endl << "Something went wrong with Alarm file ..." << std::endl;
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

    tmp_mkdir = "mkdir " + CUR_DIR + "\\out";
    /** Créer le repertoire pour les fichiers out **/
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\out";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

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

    auto tDebut_all = std::chrono::high_resolution_clock::now();
    auto tDebut = std::chrono::high_resolution_clock::now();

    /** CREATE DIR & REQ MONGODB & FILL THE L0NAME OF ALL SIG IN db **/
    allConf db = create_DB();

    auto tFin = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duree = tFin - tDebut;
    std::cout << std::endl << "Temps lecture base de donnees : " << duree.count() << std::endl;
    tDebut = std::chrono::high_resolution_clock::now();

    std::string file_in = CUR_DIR + "\\DB\\mtlinki_Signal.txt";

    std::ifstream fichier;
    fichier.open(file_in, std::ios::in);

    if (fichier)
    {
        /** Sparse db and fill vector from db **/
        allAlarms alarms;
        sparse_db(fichier,db,alarms);

        fichier.close(); // On ferme le fichier

        tFin = std::chrono::high_resolution_clock::now();
        duree = tFin - tDebut;
        std::cout << std::endl << "Temps pour lire la base depuis fichier txt : " << duree.count() << std::endl;

        tDebut = std::chrono::high_resolution_clock::now();

        /** Write files in /out directory **/
        ecriture_thread(std::cref(db),std::cref(alarms));

        tFin = std::chrono::high_resolution_clock::now();
        duree = tFin - tDebut;
        std::cout << std::endl << "Temps ecriture fichiers out : " << duree.count() << std::endl;
        tDebut = std::chrono::high_resolution_clock::now();

    }else
        std::cout << "Impossible d'ouvrir le fichier : mtlinki_Signal.txt" << std::endl;

    /** Delete DB directory and 0 Ko files in OUT directory **/
    //delete_stuff();

    std::cout << std::endl << "First steps completed !" << std::endl;

    auto tFin_all = std::chrono::high_resolution_clock::now();
    duree = tFin_all - tDebut_all;

    std::cout << std::endl << "Temps d'execution des premieres etapes : " << duree.count() << std::endl;

    return db;
}

/** END GET DB STEP FUNCTIONS **/
