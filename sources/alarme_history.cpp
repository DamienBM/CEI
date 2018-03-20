#include <alarme_history.h>

/** START OF FILE **/

alarm_history_db get_alarm_history_db(void){

    alarm_history_db alarm_db;
    alarm_history tmp;

    std::string filename = CUR_DIR+"\\DB\\Alarm_History.txt";
    std::ifstream file(filename,std::ios::in);

    if(file){
        std::string chaine;

        while(std::getline(file,chaine)){
            char *ptr = strtok((char*)chaine.c_str(),"{}$,:\"");
            while(strcmp(ptr,"L1Name")!=0) ptr = strtok(NULL,"{}$,:\"");
            ptr = strtok(NULL,"{}$,:\"");
            tmp.L1Name = ptr;

            while(strcmp(ptr,"L0Name")!=0) ptr = strtok(NULL,"{}$,:\"");
            ptr = strtok(NULL,"{}$,:\"");
            tmp.L0Name = ptr;

            while(strcmp(ptr,"number")!=0) ptr = strtok(NULL,"{}$,:\"");
            ptr = strtok(NULL,"{}$,:\"");
            tmp.number = std::atoi(ptr);

            while(strcmp(ptr,"updatedate")!=0) ptr = strtok(NULL,"{}$,:\"");
            ptr = strtok(NULL,"{}$,:\"");
            ptr = strtok(NULL,"\"");
            ptr = strtok(NULL,"\"Z");
            tmp.updatedate = ptr;

            while(strcmp(ptr,"message")!=0) ptr = strtok(NULL,"{}$,:\"");
            ptr = strtok(NULL,"\"");
            ptr = strtok(NULL,"\"");
            tmp.message = ptr;

            while(strcmp(ptr,"enddate")!=0) ptr = strtok(NULL,"{}$,:\"");
            ptr = strtok(NULL,"{}$,:\"");
            if(std::string(ptr)!="null"){
                ptr = strtok(NULL,"\"");
                ptr = strtok(NULL,"\"Z");
                tmp.enddate = ptr;
            }else
                tmp.enddate = "null";
            while(strcmp(ptr,"level")!=0) ptr = strtok(NULL,"{}$,:\"");
            ptr = strtok(NULL,"{}$,:\"");
            tmp.level = std::atoi(ptr);

            while(strcmp(ptr,"type")!=0) ptr = strtok(NULL,"{}$,:\"");
            ptr = strtok(NULL,"{}$,:\"");
            tmp.type = ptr;

            while(strcmp(ptr,"timespan")!=0) ptr = strtok(NULL,"{}$,:\"");
            ptr = strtok(NULL,"{}$,:\"");
            tmp.timespan = std::atof(ptr);

            alarm_db.push_back(tmp);
        }

    } else std::cout << std::endl << "Something went wrong with Alarm_History file in DB directory !" << std::endl;

    return alarm_db;
}

void alarm_stats(const alarm_history_db& alarm_db,const machining_info& mcn_info){

    alarm_history_db alarmStats;

    for (auto& alarm : alarm_db){

        if(alarmStats.size() == 0) alarmStats.push_back(alarm);

        else{
            unsigned int cpt=0;
            for(auto& alrm_stat : alarmStats){

                if(alrm_stat.number == alarm.number){
                    alrm_stat.timespan += alarm.timespan;
                    break;
                }

                else cpt++;

            }
            if(cpt == alarmStats.size()) alarmStats.push_back(alarm);
        }
    }

    if(alarmStats.size()!=0) save_stats(alarmStats,mcn_info.To);

}

void save_stats(const alarm_history_db& alarmStats,const double& To){

    /** CREATE FILE FOR SAVING STATS **/
    std::string tmp_mkdir("mkdir " + CUR_DIR + "\\pred\\alarm");
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\pred\\alarm";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

    std::string filename = CUR_DIR+"\\pred\\alarm\\Alarm_Stats.txt";
    std::ofstream file(filename, std::ios::out|std::ios::trunc);

    if(file){

        file.precision(3);
        file << std::fixed;

        for(auto& alrm : alarmStats){
            file << "Machine " << alrm.L0Name << " of Equipment " << alrm.L1Name
                 << " has ";
            if(alrm.level == 1) file << "an Alarm";
            else                file << "an Operator Message";

            file << " number " << alrm.number << " of type " << alrm.type << " which indicates that : \""
                 << alrm.message << "\" during at least " << std::floor(alrm.timespan*100/To)/100 << " machining cycle (but not longer than "
                 << (std::floor(alrm.timespan*100/To)/100)+1 << "cycles) or " << alrm.timespan << " seconds (during the capture period)."
                 << "\n";

        }
    file.close();
    }else   std::cout << std::endl << "Something went wrong when saving the alarm stats ..." << std::endl;


}

void alarm_history_step(const machining_info& mcn_inf){ //Get Alarm History and do some stats

    auto tDebut = std::chrono::high_resolution_clock::now();

    alarm_history_db alarm_db = get_alarm_history_db();

    alarm_stats(alarm_db,mcn_inf);

    auto tFin = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duree = tFin - tDebut;

    std::cout << std::endl << "Alarm stats done in " << duree.count() << " seconds !" << std::endl;

}

/** END OF FILE **/
