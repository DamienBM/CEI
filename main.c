#include "header.h"

int main()
{
    FILE* fichier = NULL;
    char chaine[TAILLE_MAX_LIGNE] = "";
    char* ptr_comma;
    int compteur=0;
    double* tab_val = NULL;

    fichier = fopen("C:\\Users\\94000187\\Desktop\\projet_en_cours\\CEI\\fichier_txt_db\\part_of_mtlinki_Signal_History.txt", "r");

    if (fichier != NULL)
    {
        printf("Fichier ouvert !\n");
        fgets(chaine,TAILLE_MAX_LIGNE,fichier);
        //printf("%s",chaine);

        ptr_comma = strtok(chaine,",:\"");

        while(ptr_comma != NULL){
            printf("%s\n",ptr_comma);

            if(!strcmp(ptr_comma,"signalname")){
                printf("Coucou !\n");
                free(tab_val);
                tab_val = malloc((++compteur)*sizeof(double));
                printf("%d\n",compteur);
            }


            ptr_comma = strtok(NULL,",:\"");
        }

        fclose(fichier); // On ferme le fichier qui a été ouvert
    }

    else
    {
        // On affiche un message d'erreur si on veut
        printf("Impossible d'ouvrir le fichier : part_of_mtlinki_Signal_History.txt");
    }

    return 0;
}
