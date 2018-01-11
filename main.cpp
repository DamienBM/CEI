#include "header.h"

int main()
{
    std::ifstream fichier;
    std::string chaine;
    std::vector<double> tab_val;
    std::vector<std::string> name;
    std::string test("McnPos_1_path1_");
    int dejavu = 0;

    fichier.open("C:\\Users\\94000187\\Desktop\\projet_en_cours\\CEI\\fichier_txt_db\\mtlinki_Signal_History.txt", std::ios::in);

    //TODO : -récupérer le L1Name uniquement pour les signaux voulus
    //       -après ça, continuer de parser jusqu'à la valeur du signal

    if (fichier.is_open())
    {
        std::cout << "Fichier ouvert !" << std::endl;
        while(std::getline(fichier,chaine)){
            //std::cout << chaine << std::endl;

            char *ptr = strtok((char*)chaine.c_str(),"{}$,:\""); //sale mais fonctionne ...

            while(ptr != NULL){
                //std::cout << ptr << std::endl;

                if(dejavu==0){
                    std::cout << "Avant le while :" << ptr << std::endl;
                    while(strcmp(ptr,"L1Name")!=0){
                        ptr = strtok(NULL,"{}$,:\"");
                        std::cout << "Recherche L1Name" << std::endl;
                    }
                    dejavu=1;
                    ptr = strtok(NULL,"{}$,:\"");
                    //std::cout << ptr << std::endl;
                    test += ptr;
                    name.push_back(test);
                    std::cout << "Nom complet signal : " << name.back() << std::endl;
                }

                std::cout << ptr << std::endl;

                while(strcmp(ptr,"signalname")!=0){
                    ptr = strtok(NULL,"{}$,:\"");
                    std::cout << "Recherche" << ptr << std::endl;
                }

                while(strcmp(ptr,"value")!=0){
                    ptr = strtok(NULL,"{}$,:\"");
                    std::cout << ptr << std::endl;
                }

                std::cout << ptr << std::endl;
                ptr = strtok(NULL,"{}$,:\"");

                //std::cout << ptr << std::endl;
                double val_atof = std::atof(ptr);
                tab_val.push_back(val_atof);
                //std::cout << "Valeur : " << tab_val[0] << std::endl;


                ptr = strtok(NULL,"{}$,:\"");
            }

        }
        fichier.close(); // On ferme le fichier qui a été ouvert
    }

    else
    {
        // On affiche un message d'erreur si on veut
        std::cout << "Impossible d'ouvrir le fichier : part_of_mtlinki_Signal_History.txt" << std::endl;
    }

    remove("C:\\Users\\94000187\\Desktop\\projet_en_cours\\CEI\\fichier_txt_db\\test.txt");

    std::ofstream fichier_out("C:\\Users\\94000187\\Desktop\\projet_en_cours\\CEI\\fichier_txt_db\\test.txt", std::ios::out);

    if(fichier_out){
        for(auto& ptr_val : tab_val)
            fichier_out /*<< "Valeur : "*/ << ptr_val << std::endl;
        //fichier_out << "Nb :" << tab_val.size() << std::endl;
        fichier_out.close();
    }

    return 0;
}
