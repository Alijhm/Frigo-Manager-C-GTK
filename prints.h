#ifndef PRINTS_H
#define PRINTS_H

#include <gtk/gtk.h>

char* getFridgeContent(char *username) {
    char filename[60];
    char *categories[] = {"fruit", "legume", "viande blanche", "viande rouge", "poisson", "produits frais", "produits laitiers", "autre"};
    Element current;
    usernameToFilename(username, filename);
    
    GString *str = g_string_new("--- MON FRIGO ---\n");
    FILE *fp = fopen(filename, "r");
    if(!fp) return g_string_free(str, FALSE);

    for(int i = 0; i < 8; i++){
        g_string_append_printf(str, "\n[%s]\n", categories[i]);
        fseek(fp, 0, SEEK_SET);
        int hasItem = 0;
        while(fscanf(fp, "%199[^;];%lf;%49[^;];%49[^;];%14[^\n]\n", current.ingredient, &current.quantity, current.unit, current.type, current.date) == 5){
            if(strcmp(current.type, categories[i]) == 0){
                g_string_append_printf(str, " - %s : %.2f %s", current.ingredient, current.quantity, current.unit);
                if(strcmp(current.date, "/") != 0) g_string_append_printf(str, " (Fin: %s)", current.date);
                g_string_append(str, "\n");
                hasItem = 1;
            }
        }
        if(!hasItem) g_string_append(str, " (Vide)\n");
    }
    fclose(fp);
    return g_string_free(str, FALSE);
}

#endif