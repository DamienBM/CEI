#include <date.h>


bool is_year_bissextile (const unsigned int& year){
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
        return true;
    else
        return false;
}

int date_correcte (const unsigned int& day, const unsigned int& month, const unsigned int& year){

    int jj_aide;
    switch (mm)
    {
        case 1: jj_aide = 31;
            break;
        case 2: if (bissextile (aaaa))
                    jj_aide = 29;
                else
                    jj_aide = 28;
            break;
        case 3: jj_aide = 31;
            break;
        case 4: jj_aide = 30;
            break;
        case 5: jj_aide = 31;
            break;
        case 6: jj_aide = 30;
            break;
        case 7: jj_aide = 31;
            break;
        case 8: jj_aide = 31;
            break;
        case 9: jj_aide = 30;
            break;
        case 10: jj_aide = 31;
            break;
        case 11: jj_aide = 30;
            break;
        case 12 : jj_aide = 31;
            break;
        default : return 0;
    }
    if (jj <= 0 || jj > jj_aide || aaaa  < 0)
        return 0;
    else
        return 1;
}

void saisie_date (int *jj, int *mm, int *aaaa){
    do
    {
        printf ("\t\t\tEntrez la date (jj/mm/aaaa)\n\n\n" );
        printf ("jj : " );
        scanf ("%d", jj);
        printf ("mm : " );
        scanf ("%d", mm);
        printf ("aaaa : " );
        scanf ("%d", aaaa);
    } while (!(date_correcte (*jj, *mm, *aaaa)));
}

long jours_ecoules (int jj, int mm, int aaaa){
    long compteur_jours, i;
    compteur_jours = 0;
    /* SOMME DES ANNEES */
    for (i = 0; i < aaaa; i++)
        if (bissextile (i))
            compteur_jours+= 366;
        else
            compteur_jours+= 365;
    /* SOMME DES JOURS DU MOIS COURANT */
    compteur_jours+= jj;
    /* SOMME DES JOURS DES MOIS RESTANT */
    for (i = mm - 1; i > 0; i--)
    {
        switch (i)
        {
            case 1: compteur_jours+= 31;
                break;
            case 2: if (bissextile (aaaa))
                    compteur_jours+= 29;
                else
                    compteur_jours+= 28;
                break;
            case 3: compteur_jours+= 31;
                break;
            case 4: compteur_jours+= 30;
                break;
            case 5: compteur_jours+= 31;
                break;
            case 6: compteur_jours+= 30;
                break;
            case 7: compteur_jours+= 31;
                break;
            case 8: compteur_jours+= 31;
                break;
            case 9: compteur_jours+= 30;
                break;
            case 10: compteur_jours+= 31;
                break;
            default: compteur_jours+= 30;
                break;
        }
    }
    return compteur_jours;
}

void jour_semaine (long nbre_jours){

    char *jours[] = {"Vendredi", "Samedi", "Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi"};
    nbre_jours = (nbre_jours) % 7;
    printf ("Le Jour de votre naissance est : %s", jours[nbre_jours]);
}

int main (void)
{
    int jj, mm, aaaa;
    saisie_date (&jj, &mm, &aaaa);
    jour_semaine (jours_ecoules (jj, mm, aaaa));
    getch ();
    return 0;
}

