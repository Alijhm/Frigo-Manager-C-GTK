#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "types.h"
#include "usersconf.h"
#include "edit_elements.h"
#include "edit_recipes.h"
#include "prints.h"

GtkWidget *window_main;
GtkWidget *window_login;
GtkWidget *entry_username;
GtkWidget *label_frigo;
GtkWidget *label_recipes;
char current_user[50];

char* confHelloMessage() {
    char line[1000];
    FILE *fp = fopen("conf.txt", "r");

    if (fp == NULL) {
        char *defaut = malloc(60);
        strcpy(defaut, "Bonjour ! Saisissez votre nom d'utilisateur :");
        return defaut;
    }

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';
        char *separator = strchr(line, ':');

        if (separator != NULL) {
            *separator = '\0';
            char *key = line;
            char *value = separator + 1;

            if (strcmp(key, "message_bonjour") == 0) {
                char *result = malloc(strlen(value) + 1);
                if (result != NULL) {
                    strcpy(result, value);
                }
                fclose(fp);
                return result;
            }
        }
    }

    fclose(fp);
    char *notfound = malloc(60);
    strcpy(notfound, "Bonjour ! Saisissez votre nom d'utilisateur :");
    return notfound;
}

void refresh_frigo_view() {
    char *content = getFridgeContent(current_user);
    gtk_label_set_text(GTK_LABEL(label_frigo), content);
    g_free(content);
}

void refresh_recipes_view(GtkWidget *widget, gpointer data) {
    int onlyFeasible = GPOINTER_TO_INT(data);
    char *content = getRecipesText(current_user, onlyFeasible);
    gtk_label_set_text(GTK_LABEL(label_recipes), content);
    g_free(content);
}

static void on_login_clicked(GtkWidget *widget, gpointer data) {
    const char *name = gtk_entry_get_text(GTK_ENTRY(entry_username));
    if (strlen(name) > 0) {
        strcpy(current_user, name);
        checkAndCreateUser(current_user);
        g_signal_handlers_disconnect_by_func(window_login, G_CALLBACK(gtk_main_quit), NULL);
        gtk_widget_destroy(window_login);
        gtk_widget_show_all(window_main);
        refresh_frigo_view();
        refresh_recipes_view(NULL, GINT_TO_POINTER(0));
    }
}

static void on_add_product_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget **entries = (GtkWidget **)data;
    const char *ing = gtk_entry_get_text(GTK_ENTRY(entries[0]));
    const char *qty = gtk_entry_get_text(GTK_ENTRY(entries[1]));
    const char *unit = gtk_entry_get_text(GTK_ENTRY(entries[2]));
    const char *type = gtk_entry_get_text(GTK_ENTRY(entries[3]));
    const char *date = gtk_entry_get_text(GTK_ENTRY(entries[4]));

    if (strlen(ing) > 0 && strlen(qty) > 0) {
        addElementGUI(current_user, (char*)ing, atof(qty), (char*)unit, (char*)type, (char*)date);
        for(int i=0; i<5; i++) gtk_entry_set_text(GTK_ENTRY(entries[i]), "");
        refresh_frigo_view();
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_main), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Produit ajouté !");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

static void on_delete_product_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget **entries = (GtkWidget **)data;
    const char *ing = gtk_entry_get_text(GTK_ENTRY(entries[0]));
    const char *qty = gtk_entry_get_text(GTK_ENTRY(entries[1]));

    if (strlen(ing) > 0 && strlen(qty) > 0) {
        deleteElementGUI(current_user, (char*)ing, atof(qty));
        gtk_entry_set_text(GTK_ENTRY(entries[0]), "");
        gtk_entry_set_text(GTK_ENTRY(entries[1]), "");
        refresh_frigo_view();
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_main), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Mise à jour effectuée.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

static void on_add_recipe_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget **entries = (GtkWidget **)data;
    char title[150], inst[500];
    char ingTab[10][200];
    double qtyTab[10];
    char unitTab[10][50];
    int count = 0;

    strcpy(title, gtk_entry_get_text(GTK_ENTRY(entries[0])));
    strcpy(inst, gtk_entry_get_text(GTK_ENTRY(entries[1])));

    for(int i=0; i<5; i++) {
        int baseIdx = 2 + (i*3);
        const char *iName = gtk_entry_get_text(GTK_ENTRY(entries[baseIdx]));
        const char *iQty = gtk_entry_get_text(GTK_ENTRY(entries[baseIdx+1]));
        const char *iUnit = gtk_entry_get_text(GTK_ENTRY(entries[baseIdx+2]));

        if(strlen(iName) > 0) {
            strcpy(ingTab[count], iName);
            qtyTab[count] = atof(iQty);
            strcpy(unitTab[count], iUnit);
            count++;
        }
    }

    if(strlen(title) > 0 && count > 0) {
        addRecipeGUI(title, inst, count, ingTab, qtyTab, unitTab);
        refresh_recipes_view(NULL, GINT_TO_POINTER(0));
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_main), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Recette ajoutée !");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

static void on_cook_action_clicked(GtkWidget *widget, gpointer data) {
    GtkEntry *entry = GTK_ENTRY(data);
    const char *title = gtk_entry_get_text(entry);
    
    if (strlen(title) == 0) return;

    SimpleIng ingredients[50];
    int count = 0;
    char infoBuf[2000];

    if (findRecipeForCooking((char*)title, infoBuf, ingredients, &count)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_main), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s\n\nVoulez-vous cuisiner cette recette ?\n(Cela retirera les ingrédients du frigo)", infoBuf);
        
        int response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        if (response == GTK_RESPONSE_YES) {
            for (int i = 0; i < count; i++) {
                deleteElementGUI(current_user, ingredients[i].name, ingredients[i].qty);
            }
            refresh_frigo_view();
            
            GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(window_main), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Bon appétit !\nIngrédients retirés (selon stock disponible).");
            gtk_dialog_run(GTK_DIALOG(msg));
            gtk_widget_destroy(msg);
        }

    } else {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_main), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Recette introuvable : '%s'\n\nVeuillez la créer dans l'onglet 'Nouvelle Recette' si vous souhaitez l'ajouter.", title);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

void create_login_window() {
    window_login = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_login), "Connexion Frigo");
    gtk_window_set_position(GTK_WINDOW(window_login), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window_login), 50);
    g_signal_connect(window_login, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window_login), box);

    char *msg = confHelloMessage();
    gtk_box_pack_start(GTK_BOX(box), gtk_label_new(msg), FALSE, FALSE, 0);
    free(msg);

    entry_username = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry_username, FALSE, FALSE, 0);
    GtkWidget *btn = gtk_button_new_with_label("Entrer");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_login_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 0);
    gtk_widget_show_all(window_login);
}

void create_main_window() {
    window_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_main), "Frigo Intelligent - Menu Principal");
    gtk_window_set_default_size(GTK_WINDOW(window_main), 900, 700);
    g_signal_connect(window_main, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window_main), notebook);

    GtkWidget *vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    label_frigo = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(label_frigo), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label_frigo), 0.0);
    GtkWidget *scroll1 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll1), label_frigo);
    gtk_box_pack_start(GTK_BOX(vbox1), scroll1, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox1, gtk_label_new("Mon Frigo"));

    GtkWidget *grid2 = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid2), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid2), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid2), 20);
    static GtkWidget *entries_add[5];
    char *lbls_add[] = {"Aliment:", "Quantité:", "Unité:", "Type:", "Date (ou /):"};
    for(int i=0; i<5; i++) {
        gtk_grid_attach(GTK_GRID(grid2), gtk_label_new(lbls_add[i]), 0, i, 1, 1);
        entries_add[i] = gtk_entry_new();
        gtk_grid_attach(GTK_GRID(grid2), entries_add[i], 1, i, 1, 1);
    }
    GtkWidget *btn_add = gtk_button_new_with_label("Ajouter");
    g_signal_connect(btn_add, "clicked", G_CALLBACK(on_add_product_clicked), entries_add);
    gtk_grid_attach(GTK_GRID(grid2), btn_add, 1, 5, 1, 1);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid2, gtk_label_new("Ajouter Produit"));

    GtkWidget *grid3 = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid3), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid3), 20);
    static GtkWidget *entries_del[2];
    gtk_grid_attach(GTK_GRID(grid3), gtk_label_new("Nom de l'aliment à retirer :"), 0, 0, 1, 1);
    entries_del[0] = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid3), entries_del[0], 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid3), gtk_label_new("Quantité à retirer :"), 0, 1, 1, 1);
    entries_del[1] = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid3), entries_del[1], 1, 1, 1, 1);
    GtkWidget *btn_del = gtk_button_new_with_label("Supprimer / Retirer");
    g_signal_connect(btn_del, "clicked", G_CALLBACK(on_delete_product_clicked), entries_del);
    gtk_grid_attach(GTK_GRID(grid3), btn_del, 1, 2, 1, 1);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid3, gtk_label_new("Supprimer Produit"));

    GtkWidget *vbox4 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    label_recipes = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(label_recipes), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label_recipes), 0.0);
    GtkWidget *scroll4 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll4), label_recipes);
    gtk_box_pack_start(GTK_BOX(vbox4), scroll4, TRUE, TRUE, 0);
    GtkWidget *hbox_btns = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *btn_all_rec = gtk_button_new_with_label("Voir toutes");
    GtkWidget *btn_feas_rec = gtk_button_new_with_label("Voir faisables uniquement");
    g_signal_connect(btn_all_rec, "clicked", G_CALLBACK(refresh_recipes_view), GINT_TO_POINTER(0));
    g_signal_connect(btn_feas_rec, "clicked", G_CALLBACK(refresh_recipes_view), GINT_TO_POINTER(1));
    gtk_box_pack_start(GTK_BOX(hbox_btns), btn_all_rec, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_btns), btn_feas_rec, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox4), hbox_btns, FALSE, FALSE, 5);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox4, gtk_label_new("Recettes"));

    GtkWidget *grid5 = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid5), 5);
    GtkWidget *scroll5 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll5), grid5);
    static GtkWidget *entries_rec[20];
    gtk_grid_attach(GTK_GRID(grid5), gtk_label_new("Titre de la recette :"), 0, 0, 1, 1);
    entries_rec[0] = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid5), entries_rec[0], 1, 0, 3, 1);
    gtk_grid_attach(GTK_GRID(grid5), gtk_label_new("Instructions :"), 0, 1, 1, 1);
    entries_rec[1] = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid5), entries_rec[1], 1, 1, 3, 1);
    gtk_grid_attach(GTK_GRID(grid5), gtk_label_new("Ingrédients (Max 5)"), 0, 2, 4, 1);
    gtk_grid_attach(GTK_GRID(grid5), gtk_label_new("Nom"), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid5), gtk_label_new("Qté"), 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid5), gtk_label_new("Unité"), 2, 3, 1, 1);
    for(int i=0; i<5; i++) {
        int r = 4+i;
        int idx = 2 + (i*3);
        entries_rec[idx] = gtk_entry_new();
        entries_rec[idx+1] = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(entries_rec[idx+1]), 5);
        entries_rec[idx+2] = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(entries_rec[idx+2]), 5);
        gtk_grid_attach(GTK_GRID(grid5), entries_rec[idx], 0, r, 1, 1);
        gtk_grid_attach(GTK_GRID(grid5), entries_rec[idx+1], 1, r, 1, 1);
        gtk_grid_attach(GTK_GRID(grid5), entries_rec[idx+2], 2, r, 1, 1);
    }
    GtkWidget *btn_add_rec = gtk_button_new_with_label("Enregistrer la Recette");
    g_signal_connect(btn_add_rec, "clicked", G_CALLBACK(on_add_recipe_clicked), entries_rec);
    gtk_grid_attach(GTK_GRID(grid5), btn_add_rec, 0, 10, 3, 1);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroll5, gtk_label_new("Nouvelle Recette"));

    GtkWidget *box6 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_set_border_width(GTK_CONTAINER(box6), 40);
    
    gtk_box_pack_start(GTK_BOX(box6), gtk_label_new("Entrez le nom exact de la recette que vous voulez cuisiner :"), FALSE, FALSE, 0);
    
    GtkWidget *entry_cook = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box6), entry_cook, FALSE, FALSE, 0);

    GtkWidget *btn_cook = gtk_button_new_with_label("Cuisiner cette recette");
    g_signal_connect(btn_cook, "clicked", G_CALLBACK(on_cook_action_clicked), entry_cook);
    gtk_box_pack_start(GTK_BOX(box6), btn_cook, FALSE, FALSE, 0);

    GtkWidget *label_info_cook = gtk_label_new("Conseil : Assurez-vous d'avoir les ingrédients !");
    gtk_box_pack_start(GTK_BOX(box6), label_info_cook, FALSE, FALSE, 20);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), box6, gtk_label_new("Cuisiner"));
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    create_login_window();
    create_main_window();
    gtk_main();
    return 0;
}