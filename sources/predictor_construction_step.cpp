#include <predictor_construction_step.h>

/** PREDICTORS CONSTRUCTION STEPS FUNCTIONS **/

allConf db_filtering(const allConf& db,const std::string& filt){

    /** SPARSE DB WITH ALL FAN SIGNALS **/
    std::vector<std::string> file_to_open;
    allConf db_filt,db_tmp;

    for (unsigned int i=0; i<db.size(); ++i){
        if(db[i].signalName.find(filt) != std::string::npos){
            db_tmp.push_back(db[i]);
        }
    }

    for(unsigned int i=0; i<db_tmp.size(); ++i){
        unsigned long int cpt=0;
        for(unsigned int j=0; j<db_tmp[i].values.size(); ++j){
            if(db_tmp[i].values[j] == 0)
                cpt++;
        }

        if(cpt != db_tmp[i].values.size()) /** IF ALL VALUES ARE EQUAL TO 0 ... **/
           db_filt.push_back(db_tmp[i]);
    }

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
            fichier.precision(3);
            fichier << std::fixed;
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
            gnu_file.precision(3);
            gnu_file << std::fixed;
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

dist_vect calculate_dist(const all_dist& axes){

    dist_vect all_distance;

    for(auto& axe : axes){
        double dist=0;
        double old_val=axe[0];

        std::for_each(axe.begin()+1,axe.end(),[&dist,&old_val](const double val){ dist += std::fabs(val-old_val); old_val = val;});

        all_distance.push_back(dist/1000.0);
    }

    return all_distance;
}

double get_periode(const all_dist& axes, const double& Te_servo){

    /** CREATE FILE FOR PLOTTING **/
    std::string tmp_mkdir("mkdir " + CUR_DIR + "\\pred\\dist");
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\pred\\dist";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }

    std::vector<double> res_tmp;
    std::vector<double> T_cycle_usinage;
    unsigned num_axe=0;
    bool catched=false;
    std::string filename;


    /** AUTCO-CORRELATION **/
    for(auto& axe : axes){
        filename = CUR_DIR+"\\pred\\dist\\auto_corr_axe_"+std::to_string(num_axe)+".plt";
        std::ofstream gnu_file(filename,std::ios::out|std::ios::trunc);
        res_tmp.resize(axe.size());
        double val=0;
        double val_max=0;

        double mean = 0;
        std::for_each(axe.begin(),axe.end(),[&mean](const double val){mean+=val;});
        mean /= axe.size();
        double sum;
        std::for_each(axe.begin(),axe.end(),[&mean,&sum](const double val){sum+=((val-mean)*(val-mean));});
        double var = std::sqrt(sum/(axe.size()-1));
        catched = false;
		for (unsigned int k = 0; k<(axe.size()/2); ++k){
            for(unsigned int i = 0; i<axe.size();++i) val += (axe[i]-mean) * (axe[(i+k)%axe.size()]-mean);//auto-corrélation statistique pour un processus stationnaire
            res_tmp[k] = val/var;

            if(k==0){
                val_max = val;
                if(gnu_file){
                    gnu_file << "set title \"Auto-correlation of axes number "<< num_axe <<"\"\n"
                     << "set xlabel \"Tau\"\n" << "set ylabel \"Amplitude\"\n"
                     << "plot '-' using 1:2 title \"auto-corr\" lc rgb '#0000ff' with lines\n";
                } else
                    std::cout << std::endl << "Something went wrong with data file ..." << std::endl;
            }

            else if(k>=2){
                if(val>(val_max*2/3)&&!catched){//SEUIL A DEPASSER
                    if( ( (res_tmp[k-1] - res_tmp[k-2]) > 0 ) && ( (res_tmp[k] - res_tmp[k-1]) <= 0 ) ){
                        T_cycle_usinage.push_back((k-1)*Te_servo/1000.0);
                        catched=true;
                        break;
                    }
                }
            }

            if(gnu_file) gnu_file << k << " " << res_tmp[k] << "\n";
            else         std::cout << std::endl << "Something went wrong with data file ..." << std::endl;

            val = 0;
		}
		num_axe++;
		gnu_file.close();
    }


    double To;
    for_each(std::begin(T_cycle_usinage),std::end(T_cycle_usinage),[&To](const double val){if(val>To)To=val;});

    return To;
}

void save_axes_stats(const machining_info& mcn_info){

    std::string filename(CUR_DIR+"\\pred\\dist\\axes_en_metre.txt");
    std::ofstream file(filename,std::ios::out|std::ios::trunc);

    if(file){
        file.precision(3);
        file << std::fixed;
        file << "T0 usinage " << mcn_info.To << "\n";

        unsigned int num_axe=0;
        for(unsigned int i=0; i < mcn_info.cycle_dist_vect.size(); ++i)
            file << "axe " << i << ", total dist : " << mcn_info.total_vect_dist[i] << ", dist per machining cycle : " << mcn_info.cycle_dist_vect[i] << "\n";

        file.close();
    }else
        std::cout << "Something went wrong during the saving of axes stats ..." << std::endl;
}

machining_info get_all_axes_info(const allConf& db){ /** Without Servo Viewer, only MTLINKi **/

    /** FIRST STEP : REMOVE ALL USELESS SIGNALS **/
    const std::string mcn = "Mcn";
    allConf db_pos = db_filtering(std::cref(db),mcn);

    /** FOR ALL AXES, CALCULATE THE TOTAL DIST AND THE CYCLE TIME**/

    machining_info machine_info;

    all_dist all_axes_value;
    std::vector<double> dum;

    for(auto& sig : db_pos){all_axes_value.push_back(sig.values);} // construct all_axes_value in order to use it in get_periode

    /** FIRST, CALCULATE THE CYCLE TIME **/
    machine_info.To = get_periode(std::cref(all_axes_value),db_pos[0].readCycle);

    /** THEN, CALCULATE THE TOTAL DISTANCE **/
    machine_info.total_vect_dist = calculate_dist(std::cref(all_axes_value));

    /** AND CACLULATE DISTANCE PER MACHINING CYCLE **/
    double nb_points_per_cycle = (machine_info.To*MSEC)/db_pos[0].readCycle;
    std::vector<double> dist_per_cycle;
    double dist;

    for(auto& axe_values : all_axes_value){
        for(unsigned int i=1; i<=nb_points_per_cycle; ++i) dist += std::fabs(axe_values[i]-axe_values[i-1]);
        machine_info.cycle_dist_vect.push_back(dist);
    }

    /** AFTER ALL, SAVE AXES INFORMATION **/
    save_axes_stats(std::cref(machine_info));

    return machine_info;
}

all_dist read_csv_files(std::vector<std::string> files,double& Te_servo){ /** Without MTLINKi, only Servo Viewer **/

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

void make_predictor_steps(const allConf& db){ /** Out a predictor object in a file : /pred/predfan,... **/

    std::cout << std::endl << "Pred step" << std::endl;
    clock_t t1,t2;

    t1=clock();

    /** Step 1 (Thread 1) : Fan speed stuff**/
    std::thread th_fan(fan_prediction,std::cref(db)); /** std::ref car besoin d'une référence pour fan_prediction**/

    /** STEP 2 (Thread 2) : Temp Motor **/
    std::thread th_load(load_motor_prediction,std::cref(db));

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

    t2 = clock();

    std::cout << std::endl << "Temps d'execution des etapes de prediction : " << ((float)(t2-t1)/CLOCKS_PER_SEC) << std::endl;
    t1 = clock();

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

    t2 = clock();

    std::cout << std::endl << "Temps d'execution des etapes de CSV : " << ((float)(t2-t1)/CLOCKS_PER_SEC) << std::endl;
}

/** END PREDICTORS CONSTRUCTION STEPS FUNCTIONS **/
