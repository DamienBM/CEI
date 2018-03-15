#include <predictor_construction_step.h>

/** PREDICTORS CONSTRUCTION STEPS FUNCTIONS **/

allConf db_filtering(const allConf& db,const std::string& filt){

    /** SPARSE DB WITH ALL FAN SIGNALS **/
    allConf db_filt;

    for (unsigned int i=0; i<db.size(); ++i){
        if(db[i].signalName.find(filt) != std::string::npos){
            unsigned long long cpt=0;
            for(unsigned long long j=0; j<db[i].values.size(); ++j){
                if(db[i].values[j] == 0)
                    cpt++;
            }

            if(cpt != db[i].values.size()) /** IF ALL VALUES ARE EQUAL TO 0 ... **/
               db_filt.push_back(db[i]);

            cpt=0;
        }
    }

    return  db_filt;
}

void save_and_plot_fan_predictors(const vectPred& pred_fan, const allConf& db_fan){
    /** SAVE THE PREDICTOR IN A TXT FILE  FROM VECTOR PRED_FAN**/

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
            fichier.precision(3);
            fichier << std::fixed;
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
            gnu_file.precision(3);
            gnu_file << std::fixed;
            gnu_file << "set title \""<< db_fan[i].signalName <<"\"\n" << "set xlabel \"Temps (msec)\"\n" << "set ylabel \"Speed (tr/min)\"\n" << "plot '-' using 1:2 title \"data\" lc rgb '#ff0000','' using 1:2 title \"thresh\" with lines lc rgb '#0000ff'" << "\n";
            double cpt = 0;
            std::for_each(std::begin(db_fan[i].values), std::end(db_fan[i].values), [&gnu_file,&cpt](const double val) {
                gnu_file << cpt << " " << val << "\n";
                cpt+=0.5;
            });
            cpt=0;
            double thresh = pred_fan[i].mean - 3*pred_fan[i].std_dev; // µ-3*sigma car P( X > µ-3*sigma) = 0.9985 (avec sigma = sigma_data + q)
            gnu_file << "e" << "\n";
            std::for_each(std::begin(db_fan[i].values), std::end(db_fan[i].values), [&gnu_file,&cpt,&thresh](const double val) {
                gnu_file << cpt << " " << thresh << "\n";
                cpt+=0.5;
            });
            gnu_file.close();
        }else
            std::cout << std::endl << "Something went wrong with fan gnuplot data file ..." << std::endl;
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
            fichier.precision(3);
            fichier << std::fixed;
            fichier << "Signal Name" << " , stacks of " << load[0].range.size()
                    << "\n";

    }else
        std::cout << "Something went wrong with prediction load output txt file ..." << std::endl;

    /** PLOT FILES **/
    for(unsigned int i=0; i<load.size(); ++i){

        fichier << load[i].sig_name;
        std::for_each(std::begin(load[i].range),std::end(load[i].range),[&fichier](const int val){fichier << "," << val;});
        fichier << "\n";

        filename = CUR_DIR+"\\pred\\pred_load\\load_pred_"+load[i].sig_name+".plt";
        std::ofstream gnu_file(filename,std::ios::out|std::ios::trunc);

        if(gnu_file){
            gnu_file.precision(3);
            gnu_file << std::fixed;
            gnu_file << "set title \""<< load[i].sig_name <<"\"\n"
                     << "set xlabel \"Load (stack of "<< TAILLE_STACK <<")\"\n" << "set ylabel \"Number of times per range / Total number of times (%)\"\n"
                     << "plot '-' using 1:2 title \"data\" lc rgb '#0000ff' with lines\n";
            unsigned int h=0;
            std::for_each(std::begin(load[i].range),std::end(load[i].range),[&gnu_file,&h](const int val){gnu_file << h++ << " " << val << "\n";
                                                                                                          for(; h%TAILLE_STACK!=0; ++h)
                                                                                                             gnu_file << h << " " << val << "\n";
                                                                                                          });

            gnu_file.close();
        }else
            std::cout << std::endl << "Something went wrong with load gnuplot data file ..." << std::endl;

    }
    fichier.close();

}

void fan_prediction(const allConf& db){ //to evolve in threads

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
        double d_min=100000000000.0,diff=0;
        double old_val=db_filtered[i].values[0];

        std::for_each (std::begin(db_filtered[i].values)+1, std::end(db_filtered[i].values), [&d_min,&old_val,&diff](const double val){
                       if(val!=0){
                          diff = std::fabs(val-old_val);
                          if(diff<=d_min && diff>0)
                             d_min = diff;
                          old_val = val;
                       }
                       });

        if(d_min == 0 || d_min == 100000000000.0) // 2eme condition si signal constant
            db_filtered[i].q = 200; //default value
        else
            db_filtered[i].q = d_min;

        /** Calculate mean and std_dev **/
        double sum = std::accumulate(std::begin(db_filtered[i].values), std::end(db_filtered[i].values), 0.0);
        double mean =  sum / db_filtered[i].values.size();
        double accum = 0.0;
        std::for_each (std::begin(db_filtered[i].values), std::end(db_filtered[i].values), [&mean,&accum](const double x){accum += ((x - mean) * (x - mean));});
        double stdev = std::sqrt(accum / (db_filtered[i].values.size()-1));
        pred_fan.push_back(stat_pred(db_filtered[i].signalName,mean,stdev+db_filtered[i].q,db_filtered[i].q));
    }

    save_and_plot_fan_predictors(std::cref(pred_fan),std::cref(db_filtered));
}

void load_motor_stats(const allConf& db){ //to evolve in threads

    /** FIRST STEP : REMOVE ALL USELESS SIGNALS **/
    const std::string load_str = "Load";
    allConf db_load = db_filtering(std::cref(db),load_str);

    vectLoadStat load;
    load_stat dummy;
    dummy.range.clear();
    dummy.range.resize(LOAD_MAX/TAILLE_STACK);

    for(unsigned int i=0;i<db_load.size();++i){
        dummy.sig_name = db_load[i].signalName;
        double norm = db_load[i].values.size();

        std::for_each(std::begin(db_load[i].values),std::end(db_load[i].values),[&dummy](const double val){dummy.range[(unsigned int)(val/TAILLE_STACK)]++;});

        for(unsigned int j=0; j<dummy.range.size(); ++j) dummy.range[j] =  dummy.range[j]*100/norm;

        load.push_back(dummy);

    }

    /** HERE load IS FULLFILLED WITH USEFUL SIGNALS **/

    save_and_plot_load_pred(load);
}

machining_info speed_motor_stats(const allConf& db){

    /** FIRST STEP : REMOVE ALL USELESS SIGNALS **/
    const std::string speed_str = "ServoSpeed";
    allConf db_speed = db_filtering(std::cref(db),speed_str);

    machining_info machine_info;

    for(auto& sig : db_speed){
        double old_val = sig.values[0];
        unsigned int cpt=0;
        std::for_each(std::begin(sig.values)+1,std::end(sig.values),[&old_val,&cpt](const double val){if( (old_val>0 && val<0) || (old_val<0 && val>0) ){cpt++;} old_val = val;}); // + -> - || - -> +
        machine_info.nb_inv_per_axe.push_back(cpt);
    }

    const std::string over_str = "Override";
    allConf db_over = db_filtering(std::cref(db),over_str);
    histo dum;
    dum.resize(21);

    for(auto& sig : db_over){
        double norm = sig.values.size();
        std::for_each(std::begin(sig.values),std::end(sig.values),[&dum](const double val){dum[(int)(val/TAILLE_STACK_POTAR_OVR)]++;});
        for(unsigned int j=0; j<dum.size(); ++j) dum[j] = dum[j]*100/norm;
        machine_info.override_sig = dum;
    }

    std::string filename = CUR_DIR+"\\pred\\axes\\Override.plt";
    std::ofstream gnu_file_histo(filename,std::ios::out|std::ios::trunc);
    if(gnu_file_histo){
        gnu_file_histo << gnu_file_histo.precision(3);
        gnu_file_histo << std::fixed;
        gnu_file_histo << "set title \"Override signal\"\n"
                 << "set xlabel \"Value (stack of "<< TAILLE_STACK_POTAR_OVR <<")\"\n"
                 << "set ylabel \"Number of times per range / Total number of times (%)\"\n"
                 << "plot '-' using 1:2 title \"data\" lc rgb '#0000ff' with lines\n";

        unsigned int u=0;
        std::for_each(std::begin(machine_info.override_sig),std::end(machine_info.override_sig),[&gnu_file_histo,&u](const int val){gnu_file_histo << u++ << " " << val << "\n";
                                                                                                                                    for(;u%TAILLE_STACK_POTAR_OVR!=0;u++){
                                                                                                                                        gnu_file_histo << u << " " << val << "\n";
                                                                                                                                    }
                                                                                                                                    });
        gnu_file_histo.close();
    }else
        std::cout << std::endl << "Something went wrong with override gnuplot data file ..." << std::endl;

    return machine_info;
}

machining_info dist_stats(const allConf& db){

    machining_info machine_info;
    all_dist all_axes_value;
    std::vector<double> dum;

    /** FIRST STEP : REMOVE ALL USELESS SIGNALS **/
    const std::string mcn = "McnPos";
    allConf db_pos = db_filtering(std::cref(db),mcn);
    for(auto& sig : db_pos) all_axes_value.push_back(std::pair(sig.signalName,sig.values));

    /** FOR ALL AXES, CALCULATE THE TOTAL DIST AND THE CYCLE TIME**/

    /** AND CALCULATE THE CYCLE TIME **/
    machine_info.To = get_periode(std::cref(all_axes_value),db_pos[0].readCycle);

    /** FIRST, CALCULATE THE TOTAL DISTANCE **/
    machine_info.total_vect_dist = calculate_dist(std::cref(all_axes_value));

    /** THEN CACLULATE DISTANCE PER MACHINING CYCLE **/
    double nb_points_per_cycle = (machine_info.To*MSEC)/db_pos[0].readCycle;
    std::vector<double> dist_per_cycle;
    double dist;

    for(auto& axe : all_axes_value){
        std::vector<double> axe_values = axe.second;
        for(unsigned int i=1; i<=nb_points_per_cycle; ++i) dist += std::fabs(axe_values[i]-axe_values[i-1]);
        machine_info.cycle_dist_vect.push_back(dist/1000.0);
        dist = 0;
    }

    return machine_info;
}

machining_info torque_motor_stats (const allConf& db){

    /** FIRST STEP : REMOVE ALL USELESS SIGNALS **/
    const std::string load_str = "Load";
    allConf db_torque = db_filtering(std::cref(db),load_str);

    std::vector<double> dum_mean_torque,dum_ouput_power;

    for(auto& sig : db_torque){

        if(sig.signalName.find("Servo") != std::string::npos){ //servo : calcul à faire en rated torque
            double tmp=0;
            double cpt=0;
            std::string filename(CUR_DIR+"\\pred\\axes\\Torque_"+sig.signalName+".plt");
            std::ofstream file(filename, std::ios::out|std::ios::trunc);
            if(file){
                file.precision(3);
                file << std::fixed;
                file << "set title \"Torque of : "<< sig.signalName <<"\"\n"
                     << "set xlabel \"Time (msec)\"\n" << "set ylabel \"Torque (Nm)\"\n"
                     << "plot '-' using 1:2 title \"data\" lc rgb '#0000ff' with lines\n";
            } else std::cout<< std::endl << "Something went wrong with Torque file ..." << std::endl;
            for(auto& val : sig.values){
                tmp += (RATED_TORQUE_SERVO*val/100.0); //tmp en Nm
                if(file){
                    file << cpt << " " << val << "\n";
                    cpt += 0.5;
                } else
                    std::cout<< std::endl << "Something went wrong with Torque file ..." << std::endl;
            }
            tmp /= (sig.values.size());
            dum_mean_torque.push_back(tmp);
        }

        else{ //spindle : calcul par rated output power
            double tmp=0;
            double cpt=0;
            std::string filename(CUR_DIR+"\\pred\\axes\\Output_Power_"+sig.signalName+".plt");
            std::ofstream file(filename, std::ios::out|std::ios::trunc);
            if(file){
                file.precision(3);
                file << std::fixed;
                file << "set title \"Output Power of : "<< sig.signalName <<"\"\n"
                     << "set xlabel \"Time (msec)\"\n" << "set ylabel \"Output Power (kW)\"\n"
                     << "plot '-' using 1:2 title \"data\" lc rgb '#0000ff' with lines\n";
            } else std::cout<< std::endl << "Something went wrong with Output Power file ..." << std::endl;
            for(auto& val : sig.values){
                tmp += (RATED_POWER_OUTPUT_SPDL*val/100.0); //tmp en kW
                if(file){
                    file << cpt << " " << val << "\n";
                    cpt += 0.5;
                } else
                    std::cout<< std::endl << "Something went wrong with Output Power file ..." << std::endl;
            }
            tmp /= (sig.values.size());
            dum_ouput_power.push_back(tmp);
        }
    }

    machining_info mcn_info_tmp;
    mcn_info_tmp.mean_torque = dum_mean_torque;
    mcn_info_tmp.mean_output_power = dum_ouput_power;

    return mcn_info_tmp;
}

dist_vect calculate_dist(const all_dist& axes){

    dist_vect all_distance;

    for(auto& tmp : axes){
        std::vector<double> axe = tmp.second;
        double dist=0;
        double old_val=axe[0];

        std::for_each(axe.begin()+1,axe.end(),[&dist,&old_val](const double val){ dist += std::fabs(val-old_val); old_val = val;});

        all_distance.push_back(dist/1000.0);
    }
    return all_distance;
}

double get_periode(const all_dist& axes, const double& Te_servo){ //get periode with mcnpos, servospeed and servoload

    /** CREATE FILE FOR PLOTTING **/
    std::string tmp_mkdir("mkdir " + CUR_DIR + "\\pred\\axes");
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\pred\\axes";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

    std::vector<double> res_tmp;
    std::vector<double> T_cycle_usinage;
    unsigned num_axe=0;
    std::string filename;

    /** AUTCO-CORRELATION **/
    for(auto& tmp : axes){
        std::vector<double> axe = tmp.second;
        filename = CUR_DIR+"\\pred\\axes\\auto_corr_"+ tmp.first +".plt";
        std::ofstream gnu_file(filename,std::ios::out|std::ios::trunc);
        res_tmp.resize(axe.size());
        double val=0;
        double val_max=0;

        double mean = 0;
        std::for_each(axe.begin(),axe.end(),[&mean](const double val){mean+=val;});
        mean /= axe.size();

        if(gnu_file){
            gnu_file << "set title \"Auto-correlation of "<< tmp.first <<"\"\n"
             << "set xlabel \"Tau\"\n" << "set ylabel \"Amplitude\"\n"
             << "plot '-' using 1:2 title \"auto-corr\" lc rgb '#0000ff' with lines\n";
        } else
            std::cout << std::endl << "Something went wrong with data file ..." << std::endl;
        for (unsigned long k = 0; k<(axe.size()/2); ++k){
            for(unsigned long i = k; i<axe.size();++i) val += ( (axe[i]-mean) * (axe[((i-k)/*%axe.size()*/)]-mean) );//auto-corrélation statistique pour un processus stationnaire

            if(k==0) val_max = val;

            if(val>=0){
                res_tmp[k] = val/val_max;
            } else
                res_tmp[k] = 0;

            if(k>=4){
                if(res_tmp[k-2]>THRESH_AUTOCORRELATION){ //SEUIL A DEPASSER
                    if( ( (res_tmp[k-2] - res_tmp[k-4]) > 0 ) && ( (res_tmp[k-2] - res_tmp[k-3]) > 0 ) && ( (res_tmp[k-1] - res_tmp[k-2]) <= 0 ) && ( (res_tmp[k] - res_tmp[k-2]) <= 0 ) ){
                        T_cycle_usinage.push_back((k-2)*Te_servo/1000.0);
                        if(gnu_file) gnu_file << k << " " << res_tmp[k] << "\n";
                        else std::cout << std::endl << "Something went wrong with data file ..." << std::endl;
                        break;
                    }
                }
            }

            if(gnu_file) gnu_file << k << " " << res_tmp[k] << "\n";
            else         std::cout << std::endl << "Something went wrong with data file ..." << std::endl;
            val=0;
		}
		num_axe++;
		gnu_file.close();
    }

    double To;
    std::for_each(std::begin(T_cycle_usinage),std::end(T_cycle_usinage),[&To](const double val){To+=val;});
    To = To/(T_cycle_usinage.size());

    return To;
}

void save_axes_stats(const machining_info& mcn_info){

    std::string filename(CUR_DIR+"\\pred\\axes\\stats.txt");
    std::ofstream file(filename,std::ios::out|std::ios::trunc);

    if(file){
        file.precision(3);
        file << std::fixed;
        file << "T0 usinage " << mcn_info.To << "sec\n";

        unsigned int num_axe=0;
        for(unsigned int i=0; i < mcn_info.cycle_dist_vect.size(); ++i)
            file << "Servo " << i << ", total dist : " << mcn_info.total_vect_dist[i]
                 << "metre(s), dist per machining cycle : " << mcn_info.cycle_dist_vect[i]
                 << "metre(s), number of reversal of the rotation direction : "<< mcn_info.nb_inv_per_axe[i]
                 << ", mean torque : " << mcn_info.mean_torque[i] << " Nm"
                 <<"\n";
        for(unsigned int i=0; i < mcn_info.mean_output_power.size(); ++i)
            file << "Spindle " << i << ", mean rated output :" << mcn_info.mean_output_power[i] << " kW."
                 << "Or " << mcn_info.mean_output_power[i]*100/RATED_POWER_OUTPUT_SPDL << " percent of Rated output power"
                 <<"\n";

        file.close();
    }else
        std::cout << "Something went wrong during the saving of axes stats ..." << std::endl;

}

machining_info get_all_axes_info(const allConf& db){ /** Without Servo Viewer, only MTLINKi **/

    machining_info dum_dist  = dist_stats(std::cref(db));

    machining_info dum_speed = speed_motor_stats(std::cref(db));

    machining_info dum_torque = torque_motor_stats(std::cref(db));

    machining_info mcn_inf(dum_dist.To,
                           dum_dist.total_vect_dist,
                           dum_dist.cycle_dist_vect,
                           dum_speed.nb_inv_per_axe,
                           dum_speed.override_sig,
                           dum_torque.mean_torque,
                           dum_torque.mean_output_power);

    /** AFTER ALL, SAVE AXES INFORMATION **/
    save_axes_stats(std::cref(mcn_inf));

    return mcn_inf;
}

std::vector<std::vector<double>> read_csv_files(std::vector<std::string> files, double& Te_servo){ /** Without MTLINKi, only Servo Viewer **/

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

std::vector<std::string> get_csv_files(void){ /** Without MTLINKi, only Servo Viewer **/

    boost::filesystem::path target(CUR_DIR+"\\CSV");
    std::vector<std::string> test;
    for( auto& p : boost::filesystem::directory_iterator( target ) )
        test.push_back(p.path().string()); //test is the std::string filename

    return test;
}

machining_info make_predictor_steps(const allConf& db){ /** Out a predictor object in a file : /pred/predfan,... **/

    std::cout << std::endl << "Pred step" << std::endl;

    auto tDebut = std::chrono::high_resolution_clock::now();

    /** Step 1 (Thread 1) : Fan speed stuff**/
    std::thread th_fan(fan_prediction,std::cref(db)); /** std::ref car besoin d'une référence pour fan_prediction**/

    /** STEP 2 (Thread 2) : Temp Motor **/
    std::thread th_load(load_motor_stats,std::cref(db));

    /** STEP 3 (Thread 3) : Axes total dist and Machining cycle**/
    std::packaged_task<machining_info()> th_dist_axes(std::bind(get_all_axes_info,std::cref(db)));
    std::future<machining_info> res = th_dist_axes.get_future();
    std::thread(std::move(th_dist_axes)).detach(); //detach the thread so it's no longer useful to join() it. Only get() is useful here

    /** join() or get() of each thread **/
    th_fan.join();
    std::cout << std::endl << "Fan done !" << std::endl;
    th_load.join();
    std::cout << std::endl << "Load done !" << std::endl;
    machining_info mcn_info = res.get(); //wait until the result is available
    std::cout << std::endl << "Mcn_info done !" << std::endl;

    auto tFin = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duree = tFin - tDebut;

    std::cout << std::endl << "Temps d'execution des etapes de prediction : " << duree.count() << std::endl;
    tDebut = std::chrono::high_resolution_clock::now();

    std::cout.precision(3);
    std::cout << std::fixed;
    std::cout << "distance sur X : " << mcn_info.total_vect_dist[0] << " m et sur Y : " << mcn_info.total_vect_dist[1] << " m" << std::endl;
    std::cout << std::endl << "T_cycle_usinage = " << mcn_info.To << " sec." << std::endl;

    /** CSV FILES PART **/
    /*double Te_servo=0;
    csv_filename csv_files = get_csv_files();
    all_dist axes = read_csv_files(csv_files,Te_servo);
    total_dist_vect dist_axes = calculate_dist(std::cref(axes));
    double periode_usinage = get_periode(std::cref(axes),Te_servo);

    save_axes_stats(std::cref(dist_axes),periode_usinage);

    std::cout.precision(3);
    std::cout << std::fixed;
    std::cout << "distance sur X : " << dist_axes[0] << " m et sur Y : " << dist_axes[1] << " m" << std::endl;

    std::cout << std::endl << "T_cycle_usinage = " << periode_usinage << " sec." << std::endl;*/

    tFin = std::chrono::high_resolution_clock::now();
    duree = tFin - tDebut;

    std::cout << std::endl << "Temps d'execution des etapes de CSV : " << duree.count() << std::endl;

    return mcn_info;
}

/** END PREDICTORS CONSTRUCTION STEPS FUNCTIONS **/
