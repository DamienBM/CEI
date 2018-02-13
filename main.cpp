#include "header.h"


/**********************

V1 : Programme réussi à récupérer toutes les valeurs d'un signal
que l'on demande par l'instanciation de test (std::string).

V2 : On récupère toutes les valeurs du signal nommé partiellement par test
afin de tracer ensuite sous Excel les données.

V3 : Fonctionne sur pc distant en "copie/coll"-ant le .exe du projet vers le pc distant

V4 : Récupère les données et les met dans une structure de type info_sig
et sort les signaux en fichier txt et supprime les fichiers de 0Ko

V5 : amélioration avec des thread & future lors de l'écriture des fichiers dans /out/...
     Les threads retourne une valeur (int) et on attends que les thread

**********************/

int main(int argc, char** argv)
{
    float temps_read_db,temps_write_out,temps_write_db,temps_all;
    clock_t t1, t2;
    t1 = clock();
    std::string cur_dir = _getcwd(NULL,0);
    std::string file_in = cur_dir + "\\DB\\mtlinki_Signal.txt";
    std::ifstream fichier;
    std::string chaine;

    allConf db;
    double timespan_msec = 0;

    db = create_DB();
    t2 = clock();
    temps_read_db = (float)(t2-t1)/CLOCKS_PER_SEC;
    std::cout << "Temps lecture base de donnees : " << temps_read_db << std::endl;
    t1 = clock();

    PAUSE

    fichier.open(file_in, std::ios::in);

    if (fichier.is_open())
    {
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
                        //std::cout << "Match !" << std::endl;
                        ptr = strtok(NULL,"{}$,:\"");
                        ptr = strtok(NULL,"{}$,:\"");
                        //ici ptr vaut la valeur du signal
                        for(int nb_pts = 0; nb_pts < (timespan_msec/(it->readCycle));++nb_pts){
                            it->values.push_back(std::atof(ptr));
                            //std::cout << "Valeur : " << std::atof(ptr) << ", signal : " << it->signalName << std::endl;
                            //PAUSE
                        }
                        break;
                    }
                }

                while(ptr != NULL)
                    ptr = strtok(NULL,"{}$,:\"");
            }
        }
        fichier.close(); // On ferme le fichier

        t2 = clock();
        temps_write_db = (float)(t2-t1)/CLOCKS_PER_SEC;
        std::cout << "Temps ecriture base de donnees fichier txt : " << temps_write_db << std::endl;

        /*std::string path_out = cur_dir+"\\out\\";
        std::string file_out;*/

        t1 = clock();

        /*for(unsigned int i = 0; i != db.size();++i){

            file_out = path_out + db[i].signalName +".txt";
            std::ofstream fichier_out(file_out, std::ios::out|std::ios::trunc);

            if(fichier_out){
                for(unsigned int j = 0; j != db[i].values.size(); ++j)
                    fichier_out << db[i].values[j] << std::endl;
                fichier_out.close();
            }else{
                std::cout << "Pb lors de l'ecriture des fichiers out !" << std::endl;
                PAUSE
                exit(EXIT_FAILURE);
            }
        }*/

        ecriture_thread(db);

        t2 = clock();
        temps_write_out = (float)(t2-t1)/CLOCKS_PER_SEC;
        std::cout << "Temps ecriture fichiers out : " << temps_write_out << std::endl;
        t1=clock();
    }else
        std::cout << "Impossible d'ouvrir le fichier : mtlinki_Signal.txt" << std::endl;

    /** Suppresion des fichiers de 0Ko **/
    std::string del_cmd = "for /r " + cur_dir + "\\out %i in (*.txt) do if %~zi == 0 del %i";
    system(del_cmd.c_str());

    /** Suppression du dossier db (éviter de fuiter l'accès aux données) **/
    std::string tmp_rmdir = "rmdir /s /q " + cur_dir + "\\db";
    system(tmp_rmdir.c_str());

    //std::cout << "Done !" << std::endl;

    t2 = clock();
    temps_all = ((float)(t2-t1)/CLOCKS_PER_SEC)+temps_read_db+temps_write_db+temps_write_out;
    std::cout << "Temps d'execution : " << temps_all << std::endl;
    PAUSE
    return 0;
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

    std::string cur_dir = _getcwd(NULL,0);

    /** get le current directory **/
    std::string tmp_mkdir("");
    tmp_mkdir = "mkdir " + cur_dir + "\\DB";

    /** Créer le repertoire pour stocker la BDD **/
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + cur_dir + "\\db";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }


    tmp_mkdir = "mkdir " + cur_dir + "\\batch";
    /** Créer le repertoire pour les fichiers bat **/
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + cur_dir + "\\batch";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

    tmp_mkdir = "mkdir " + cur_dir + "\\out";
    /** Créer le repertoire pour les fichiers out **/
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + cur_dir + "\\out";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

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

    /**test for loop : okay !**/
    /*int i=0;
    for(auto it = db.begin(); it != db.end(); ++it,i++)
        std::cout << "Name " << i << " : "<<it->signalName << " avec Te = " << it->readCycle << std::endl;*/

    return db;
}
