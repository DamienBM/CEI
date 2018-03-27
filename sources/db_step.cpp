#include <db_step.h>

/** START OF FILE **/

/** Function to sparse Signal History file and fill vectors **/
void sparse_db(std::ifstream& fichier, allConf& db,allAlarms& alarms){
    std::string chaine;
    double timespan_msec = 0;

    while(std::getline(fichier,chaine)){ /** Read line by line the file and fill vector with information from the file **/

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

                for(auto it = db.begin(); it != db.end();++it){
                    if(strcmp(ptr,it->signalName.c_str())==0){
                        ptr = strtok(NULL,"{}$,:\"");
                        ptr = strtok(NULL,"{}$,:\"");
                        for(unsigned long nb_pts = 0; nb_pts < (timespan_msec/(it->readCycle));++nb_pts)
                            it->values.push_back(std::atof(ptr));
                        break;
                    }
                }


            }

            ptr = NULL; /** In order to be faster by not going at the end of the file **/
        }
    }
}

/** Function to delete all unnecessary files **/
void delete_stuff(void){
    /** Deletion of 0kO files **/
    std::string del_cmd = "for /r " + CUR_DIR + "\\out %i in (*.txt) do if %~zi == 0 del %i";
    system(del_cmd.c_str());

    /** Deletion of the DB file **/
    std::string tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\DB";
    system(tmp_rmdir.c_str());
}

/** Function to write each signal in a text file **/
int ecriture(const info_sig& sig){

    std::string path_out = CUR_DIR+"\\out\\";
    std::string file_out;

    file_out = path_out + sig.signalName + "_TimeCycle_" + std::to_string(sig.readCycle) + ".plt";
    std::ofstream fichier_out(file_out, std::ios::out|std::ios::trunc);

    if(fichier_out){/** If the file is correctly opened **/
        fichier_out.precision(3);/** For clarity **/
        fichier_out << std::fixed;/** For non-scientific expression **/
        fichier_out << "set title \"" << sig.signalName <<"\"\n" << "set xlabel \"Temps (sec)\"\n" << "set ylabel \"Amplitude\"\n" << "plot '-' using 1:2 title \"data\" lc rgb '#ff0000'" ;

        if(sig.signalName.find(std::string("Pos"))!=std::string::npos
           || sig.signalName.find(std::string("Servo"))!=std::string::npos
           || sig.signalName.find(std::string("Spindle"))!=std::string::npos)
            fichier_out << " with lines";

        fichier_out << "\n";
        for(double j = 0; j != sig.values.size(); j += 0.5) /** Fill the text file with an increment step of 0.5 because all signals are sampled at 500MSEC **/
            fichier_out << j*sig.readCycle << " " << sig.values[j] << std::endl;
        fichier_out.close();
    }else{
        std::cout << "Pb lors de l'ecriture des fichiers out !" << std::endl;
        PAUSE
        exit(EXIT_FAILURE);
    }
    return 1;
}

/** Function to manage the writing thread (function above) **/
void ecriture_thread(const allConf& db,const allAlarms& alarms){

    std::chrono::milliseconds span(1); /** Check time : 1 millisecond **/
    unsigned int nb_thread_max = std::thread::hardware_concurrency(); /** Get maximum number of threads **/

    std::vector<std::future<int>> tab_fut;
    std::future<int> fut;
    unsigned int j = 0;

    /** For loop to initialize the threads **/
    for (j = 0; j<nb_thread_max && j<db.size();++j) tab_fut.push_back(std::async(std::launch::async,ecriture,std::cref(db[j])));

    if(j==db.size()){
        for(unsigned int k = 0; k<tab_fut.size(); ++k) tab_fut[k].wait();
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


    /** Special part for the alarm file **/
    std::string filename = CUR_DIR+"\\out\\Alarms.txt";
    std::ofstream file(filename,std::ios::out|std::ios::trunc);

    for(auto& alarm: alarms) {
        if(file) file << "Alarm From " << alarm.updatedate << " to " << alarm.enddate << ".Wether "<< alarm.duration <<" seconds.\n";
        else     std::cout << std::endl << "Something went wrong with Alarm file ..." << std::endl;
    }

    if(file) file << "Total number of Alarm : " << alarms.size() << "\n";
    else     std::cout << std::endl << "Something went wrong with Alarm file ..." << std::endl;
}

/** Function to create initialize the data base **/
allConf create_DB(void){
    allConf db;

    std::string tmp_mkdir("");
    tmp_mkdir = "mkdir " + CUR_DIR + "\\DB";

    /** Create the directory to store the data base **/
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\DB";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

    tmp_mkdir = "mkdir " + CUR_DIR + "\\batch";
    /** Create the directory to store the bat files **/
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\batch";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

    tmp_mkdir = "mkdir " + CUR_DIR + "\\out";
    /** Create the directory to store the out files **/
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\out";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

    /** Create the directory to store the pred files **/
    tmp_mkdir = "mkdir " + CUR_DIR + "\\pred";
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\pred";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

    /** Create the MongoDB request **/
    std::string path = CUR_DIR + "\\batch\\Signal.bat";
    std::ofstream fichier(path, std::ios::out|std::ios::trunc);
    if(fichier){
        fichier << "path C:\\FANUC\\MT-LINKi\\MongoDB\\bin" << std::endl << "mongoexport /d MTLINKi /u fanuc /p fanuc /c L1Signal_Pool /o "<< CUR_DIR << "\\db\\mtlinki_Signal.txt" <<std::endl;
        fichier.close();
    }else
        std::cout << "Oops !" << std::endl;

    /** Start the request **/
    system(path.c_str());

    /** Deletion of the batch directory (for sake of clarity) **/
    std::string tmp_rmdir("");
    tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\batch";
    system(tmp_rmdir.c_str());

    /** Get information from L0Setting file **/
    std::ifstream L0_Setting("C:\\FANUC\\MT-LINKi\\MT-LINKiCollector\\Setting\\L0_Setting.json",std::ios::in);
    std::string chaine;

    if(L0_Setting){
        info_sig dummy;
        std::string sig_name;
        while(std::getline(L0_Setting,chaine)){
            char* ptr = strtok((char*)chaine.c_str(),"{}$,:\"");

            while(ptr != NULL){
                if(strcmp(ptr,"L0Name") == 0){
                    ptr = strtok(NULL,"{}$,:\"");
                    dummy.L0Name = ptr;
                }

                else if(strcmp(ptr,"SignalName") == 0){
                    ptr = strtok(NULL,"{}$,:\"");
                    dummy.signalName = ptr;
                    dummy.signalName+="_"+dummy.L0Name;

                }

                else if(strcmp(ptr,"ReadCycle") == 0) {
                    ptr = strtok(NULL,"{}$,:\"");
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

/** Main function of this file **/
allConf first_steps(void){

    auto tDebut_all = std::chrono::high_resolution_clock::now();
    auto tDebut = std::chrono::high_resolution_clock::now();

    /** Function to create initialize the data base **/
    allConf db = create_DB();

    auto tFin = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duree = tFin - tDebut;
    std::cout << std::endl << "Time to read the data base : " << duree.count() << " msec" << std::endl;
    tDebut = std::chrono::high_resolution_clock::now();

    std::string file_in = CUR_DIR + "\\DB\\mtlinki_Signal.txt";

    std::ifstream fichier;
    fichier.open(file_in, std::ios::in);

    if (fichier)
    {
        /** Function to sparse Signal History file and fill vectors **/
        allAlarms alarms;
        sparse_db(fichier,db,alarms);

        fichier.close();

        tFin = std::chrono::high_resolution_clock::now();
        duree = tFin - tDebut;
        std::cout << std::endl << "Time to read the data base from the text file : " << duree.count() << " msec" << std::endl;

        tDebut = std::chrono::high_resolution_clock::now();

        /** Function to manage the writing thread **/
        ecriture_thread(std::cref(db),std::cref(alarms));

        tFin = std::chrono::high_resolution_clock::now();
        duree = tFin - tDebut;
        std::cout << std::endl << "Time to write the out files : " << duree.count() << " msec" << std::endl;
        tDebut = std::chrono::high_resolution_clock::now();

    }else
        std::cout << "Can't open the file : mtlinki_Signal.txt" << std::endl;

    /** Function to delete all unnecessary files **/
    delete_stuff();

    std::cout << std::endl << "First steps completed !" << std::endl;

    auto tFin_all = std::chrono::high_resolution_clock::now();
    duree = tFin_all - tDebut_all;

    std::cout << std::endl << "Time of first steps : " << duree.count() << " msec" << std::endl;

    return db;
}

/** END OF FILE **/
