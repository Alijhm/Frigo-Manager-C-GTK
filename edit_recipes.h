#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[200];
    double qty;
    char unit[50];
} SimpleIng;

void sanitizeString(char *str) {
    while (*str) {
        if (*str == ';') *str = ','; 
        if (*str == '\n') *str = ' ';
        str++;
    }
}

char* extractField(char **cursor) {
    char *start = *cursor;
    char *end = strchr(start, ';');
    if (end) {
        *end = '\0';
        *cursor = end + 1;
        return start;
    }
    return start;
}

int findRecipeForCooking(char *targetTitle, char *infoBuf, SimpleIng *ingList, int *ingCount) {
    FILE *fp = fopen("recipes.txt", "r");
    if (!fp) return 0;

    char lineBuffer[4096];
    int found = 0;
    *ingCount = 0;

    while (fgets(lineBuffer, sizeof(lineBuffer), fp)) {
        char *cursor = lineBuffer;
        
        char *rTitle = extractField(&cursor);
        char *rInst = extractField(&cursor);
        int nbIng = atoi(extractField(&cursor));

        if (strcmp(rTitle, targetTitle) == 0) {
            found = 1;
            sprintf(infoBuf, "Recette : %s\n\nInstructions :\n%s\n\nIngrédients à retirer :\n", rTitle, rInst);
            
            for (int i = 0; i < nbIng; i++) {
                char *iName = extractField(&cursor);
                double iQty = atof(extractField(&cursor));
                char *iUnit = extractField(&cursor);

                strcpy(ingList[*ingCount].name, iName);
                ingList[*ingCount].qty = iQty;
                strcpy(ingList[*ingCount].unit, iUnit);
                (*ingCount)++;

                char tempLine[300];
                sprintf(tempLine, "- %s : %.2f %s\n", iName, iQty, iUnit);
                strcat(infoBuf, tempLine);
            }
            break;
        }
    }
    fclose(fp);
    return found;
}

int isRecipeFeasible(char *username, char *recipeTitle) {
    char userFile[100];
    usernameToFilename(username, userFile);
    FILE *fpR = fopen("recipes.txt", "r");
    if (!fpR) return 0;

    char lineBuffer[4096];
    int found = 0;
    int feasible = 1;
    SimpleIng needs[50];
    int countNeeds = 0;

    while (fgets(lineBuffer, sizeof(lineBuffer), fpR)) {
        char *cursor = lineBuffer;
        char *rTitle = extractField(&cursor);
        extractField(&cursor);
        int nbIng = atoi(extractField(&cursor));

        if (strcmp(rTitle, recipeTitle) == 0) {
            for (int i = 0; i < nbIng; i++) {
                strcpy(needs[countNeeds].name, extractField(&cursor));
                needs[countNeeds].qty = atof(extractField(&cursor));
                strcpy(needs[countNeeds].unit, extractField(&cursor));
                countNeeds++;
            }
            found = 1;
            break;
        }
    }
    fclose(fpR);
    if (!found) return 0;

    for (int i = 0; i < countNeeds; i++) {
        FILE *fpU = fopen(userFile, "r");
        if (!fpU) { feasible = 0; break; }
        Element stock;
        double totalStock = 0;
        while (fscanf(fpU, "%199[^;];%lf;%49[^;];%49[^;];%14[^\n]\n", stock.ingredient, &stock.quantity, stock.unit, stock.type, stock.date) == 5) {
            if (strcmp(stock.ingredient, needs[i].name) == 0 && strcmp(stock.unit, needs[i].unit) == 0) {
                totalStock += stock.quantity;
            }
        }
        fclose(fpU);
        if (totalStock < needs[i].qty) {
            feasible = 0; break;
        }
    }
    return feasible;
}

char* getRecipesText(char *username, int onlyFeasible) {
    FILE *fp = fopen("recipes.txt", "r");
    if (!fp) return g_strdup("Aucune recette pour le moment.\nAjoutez-en une via l'onglet 'Nouvelle Recette' !");

    GString *str = g_string_new("");
    char lineBuffer[4096];
    int recipeCount = 0;

    while (fgets(lineBuffer, sizeof(lineBuffer), fp)) {
        char *cursor = lineBuffer;
        char *currentTitle = extractField(&cursor);
        char *currentInst = extractField(&cursor);
        int nbIng = atoi(extractField(&cursor));
        
        int isOk = 1;
        if (onlyFeasible) isOk = isRecipeFeasible(username, currentTitle);

        if (isOk) {
            g_string_append_printf(str, "\n================================\nPLAT : %s\n", currentTitle);
            if(onlyFeasible) g_string_append(str, "--> (FAISABLE MAINTENANT !)\n");
            g_string_append_printf(str, "\nINSTRUCTIONS :\n%s\n\nINGREDIENTS :\n", currentInst);
        }

        for (int i = 0; i < nbIng; i++) {
            char *ingName = extractField(&cursor);
            double ingQty = atof(extractField(&cursor));
            char *ingUnit = extractField(&cursor);
            if (isOk) g_string_append_printf(str, " - %s : %.2f %s\n", ingName, ingQty, ingUnit);
        }
        
        if (isOk) {
            g_string_append_printf(str, "\n");
            recipeCount++;
        }
    }
    fclose(fp);
    if (recipeCount == 0) {
        if (onlyFeasible) g_string_append(str, "Aucune recette faisable avec votre frigo actuel.");
        else g_string_append(str, "Aucune recette trouvée.");
    }
    return g_string_free(str, FALSE);
}

int addRecipeGUI(char *title, char *inst, int nbIng, char ingredients[10][200], double quantities[10], char units[10][50]) {
    FILE *fp = fopen("recipes.txt", "r");
    if (!fp) return 1;

    char checkBuffer[4096];
    while (fgets(checkBuffer, sizeof(checkBuffer), fp)) {
        char *cursor = checkBuffer;
        char *rTitle = extractField(&cursor);
        if (strcmp(rTitle, title) == 0) {
            fclose(fp);
            return 2;
        }
    }
    fclose(fp);

    fp = fopen("recipes.txt", "a");
    if (!fp) return 1;

    sanitizeString(title);
    sanitizeString(inst);
    if (strlen(title) == 0) strcpy(title, "Recette sans titre");
    if (strlen(inst) == 0) strcpy(inst, "Pas d'instructions");

    fprintf(fp, "%s;%s;%d;", title, inst, nbIng);
    for (int i = 0; i < nbIng; i++) {
        sanitizeString(ingredients[i]);
        sanitizeString(units[i]);
        fprintf(fp, "%s;%.2f;%s;", ingredients[i], quantities[i], units[i]);
    }
    fprintf(fp, "\n");
    fclose(fp);
    return 0;
}