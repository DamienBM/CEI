#include <db_step.h>

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
        fichier_out << "set title \"" << sig.signalName <<"\"\n" << "set xlabel \"Indice\"\n" << "set ylabel \"Amplitude\"\n" << "plot '-' using 1:2 title \"data\" lc rgb '#ff0000'" ;

        if(sig.signalName.find(std::string("Pos"))!=std::string::npos)
            fichier_out << " with lines";

        fichier_out << "\n";
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

    std::chrono::milliseconds span(1);
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

    std::string file_in = CUR_DIR + "\\DB\\mtlinki_Signal.txt";

    std::ifstream fichier;
    fichier.open(file_in, std::ios::in);

    if (fichier)
    {
        /** Sparse db and fill vector from db **/
        sparse_db(fichier,db);

        fichier.close(); // On ferme le fichier

        t2 = clock();
        temps_write_db = (float)(t2-t1)/CLOCKS_PER_SEC;
        std::cout << std::endl << "Temps pour lire la base depuis fichier txt : " << temps_write_db << std::endl;

        t1 = clock();

        /** Write files in /out directory **/
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
