#include <script_python.h>


void launch_python_script(void){

    /** Create directory for pred*.txt files **/
    std::string tmp_mkdir("mkdir " + CUR_DIR + "\\pred\\cluster");
    if( system(tmp_mkdir.c_str()) ){
        std::string tmp_rmdir("");
        tmp_rmdir = "rmdir /s /q " + CUR_DIR + "\\pred\\cluster";
        system(tmp_rmdir.c_str());
        system(tmp_mkdir.c_str());
    }
    std::string cmd = "python "+CUR_DIR+"\\python-code\\src\\main.py";
    std::cout << std::endl << "Launchin Python script !" << std::endl;
    system(cmd.c_str());

}

