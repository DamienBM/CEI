#include <prediction_step.h>

/** START OF FILE **/

/** Function to read and load the predictors from the text file of each predictor **/
Predictors load_predictors(void){

    Predictors predictors;
    vectPred fan_pred;
    stat_pred fan_tmp;

    std::string path(CUR_DIR+"\\pred\\");

    /** Load the fan predictors **/
    std::string file_pred(path + "\\pred_fan\\pred_fan.txt");
    std::ifstream pred_fan_file(file_pred,std::ios::in);
    if(pred_fan_file){
        std::string chaine;
        std::getline(pred_fan_file,chaine);/** Skip the first line because useless **/
        while(std::getline(pred_fan_file,chaine)){
            fan_tmp.sig_name = strtok((char*)chaine.c_str(),",");
            fan_tmp.mean = std::atof(strtok(NULL,","));
            fan_tmp.std_dev = std::atof(strtok(NULL,","));
        }
        predictors.fan_pred.push_back(fan_tmp);

    }else
        std::cout << std::endl << "Something went wrong during loading of fan predictor ! " << std::endl;

    return predictors;
}

/** Function to do the continuous prediction **/
void predict(const allConf_active& active_db,const Predictors& predictors){
    for(auto& active_signal : active_db){
        /** FAN PREDICTION **/
        for(auto& pred_fan : predictors.fan_pred){
            if(active_signal.signalName == pred_fan.sig_name)
            {
                if(active_signal.value < (pred_fan.mean-3*pred_fan.std_dev))
                    std::cout << std::endl << " Warning ! Signal : " << active_signal.signalName << " has a current value under the threshold !" << std::endl
                              << " At time : " << active_signal.time1 << ":" << active_signal.time2 << ":" << active_signal.time3 << std::endl;
                break;
            }
        }
    }
}

/** Function to store the active data base **/
void get_active_db(void){

    /** Construct the MongoDB request **/
    std::string active_db("\\ACTIVE_DB");
    std::string path = CUR_DIR + active_db + "\\Active_Signals.bat";
    std::ofstream fichier(path, std::ios::out|std::ios::trunc);
    if(fichier){
        fichier << "path C:\\FANUC\\MT-LINKi\\MongoDB\\bin" << std::endl << "mongoexport /d MTLINKi /u fanuc /p fanuc /c L1Signal_Pool_Active /o "<< CUR_DIR+active_db <<"\\" << ACTIVE_SIGNALS_FILE <<std::endl;
        fichier.close();
    }else
        std::cout << "Oops !" << std::endl;

    /** Start the request **/
    system(path.c_str());
}

/** Function to store the active data base **/
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

            ptr = strtok(NULL,"{}$,:\"");
            while(strcmp(ptr,"value")!=0){
                ptr = strtok(NULL,"{}$,:\"");
            }

            ptr = strtok(NULL,"{}$,:\"");

            if(strcmp(ptr,"null")!=0){
                dummy.value = std::atof(strtok(NULL,"{}$,:\""));
                db_tmp.push_back(dummy);
            }
        }

    }else
        std::cout << std::endl << "Something went wrong while opening Active Signals file !" << std::endl;

    return db_tmp;
}

/** Function to write the active value of a signal in a text file **/
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

/** Function to manage the writing thread (function above) **/
void active_db_ecriture_thread(const allConf_active& active_db){

    std::chrono::milliseconds span (1); /** Check time : 1 millisecond **/
    unsigned int n = std::thread::hardware_concurrency(); /** Get maximum number of threads **/

    std::vector<std::future<int>> tab_fut;
    std::future<int> fut;
    unsigned int j = 0;

    /** For loop to initialize the threads **/
    for (j = 0; j<n && j<active_db.size(); ++j) tab_fut.push_back(std::async(std::launch::async,active_db_ecriture,active_db[j]));

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

/** Function to create the active signal data base file directory **/
void create_prediction_dir(void){
    std::string tmp_mkdir("mkdir ");
    std::string active_db_dir("\\ACTIVE_DB");
    tmp_mkdir += ( CUR_DIR + active_db_dir );
    system(tmp_mkdir.c_str());
    tmp_mkdir = "mkdir ";
    std::string path_out = "mkdir " + CUR_DIR+"\\out_active\\";
    system(path_out.c_str());
}

/** Main function of this file **/
void predictions_step(void){

    /** Function to create active data base signal **/
    create_prediction_dir();
    auto tDebut = std::chrono::high_resolution_clock::now();

    /** Function to store the active data base **/
    get_active_db();
    allConf_active active_db = read_active_signals_file();

    auto tFin = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duree = tFin - tDebut;
    std::cout << std::endl << "Temps pour avoir active_db : " << duree.count() << " msec !" << std::endl;
    active_db_ecriture_thread(active_db);

    /** Function to read and load the predictors from the text file of each predictor **/
    Predictors predictors = load_predictors();
    using namespace std::chrono_literals;
    while(true){
        std::cout << std::endl << "Loop done !" << std::endl;
        /** Function to do the continuous prediction **/
        predict(active_db,predictors);
        std::this_thread::sleep_for(500ms);
        /** Function to store the active data base **/
        get_active_db();
        /** Function to read the active data base signals **/
        active_db = read_active_signals_file();
    }
}

/** END OF FILE **/
