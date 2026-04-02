#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mettreEnMinuscule(char *texte) {
    int i = 0;
    while(texte[i] != '\0') {
        if (texte[i] >= 'A' && texte[i] <= 'Z') {
            texte[i] = texte[i] + 32;
        }
        i++;
    }
}

int addElementGUI(char *username, char *ing, double qty, char *unit, char *type, char *date) {
    char filename[60], tempFilename[60];
    Element current;
    int found = 0;

    mettreEnMinuscule(username);
    mettreEnMinuscule(ing);
    mettreEnMinuscule(unit);
    mettreEnMinuscule(type);

    usernameToFilename(username, filename);
    sprintf(tempFilename, "Users/%s.tmp", username);

    FILE *fp = fopen(filename, "r");
    FILE *temp = fopen(tempFilename, "w");

    if (!temp) return 1;
    if (fp) {
        while (fscanf(fp, "%199[^;];%lf;%49[^;];%49[^;];%14[^\n]\n", current.ingredient, &current.quantity, current.unit, current.type, current.date) == 5) {
            if (strcmp(current.ingredient, ing) == 0 && strcmp(current.unit, unit) == 0 && strcmp(current.date, date) == 0) {
                current.quantity += qty;
                found = 1;
            }
            fprintf(temp, "%s;%.2f;%s;%s;%s\n", current.ingredient, current.quantity, current.unit, current.type, current.date);
        }
        fclose(fp);
    }

    if (!found) {
        fprintf(temp, "%s;%.2f;%s;%s;%s\n", ing, qty, unit, type, date);
    }

    fclose(temp);
    remove(filename);
    rename(tempFilename, filename);
    return 0;
}

int deleteElementGUI(char *username, char *ing, double qty) {
    char filename[60], tempFilename[60];
    Element current;
    double remaining = qty;

    mettreEnMinuscule(username);
    mettreEnMinuscule(ing);

    usernameToFilename(username, filename);
    sprintf(tempFilename, "Users/%s.tmp", username);

    FILE *fp = fopen(filename, "r");
    FILE *temp = fopen(tempFilename, "w");
    if (!fp || !temp) return 1;

    while(fscanf(fp, "%199[^;];%lf;%49[^;];%49[^;];%14[^\n]\n", current.ingredient, &current.quantity, current.unit, current.type, current.date) == 5){
        if (strcmp(current.ingredient, ing) == 0 && remaining > 0){
            if (current.quantity > remaining){
                current.quantity -= remaining;
                remaining = 0;
                fprintf(temp, "%s;%.2f;%s;%s;%s\n", current.ingredient, current.quantity, current.unit, current.type, current.date);
            } else {
                remaining -= current.quantity;
            }
        } else {
            fprintf(temp, "%s;%.2f;%s;%s;%s\n", current.ingredient, current.quantity, current.unit, current.type, current.date);
        }
    }
    fclose(fp); fclose(temp);
    remove(filename); rename(tempFilename, filename);
    return 0;
}
