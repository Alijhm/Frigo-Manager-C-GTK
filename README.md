# Frigo Manager

Application de bureau développée en C permettant la gestion intelligente de votre réfrigérateur et de vos repas.

## Fonctionnalités

* **Interface Graphique :** Navigation par onglets développée avec la bibliothèque GTK3.
* **Multi-utilisateurs :** Création automatique d'un profil par utilisateur au démarrage, avec sauvegarde des données dans un dossier `Users/`.
* **Gestion du Stock :** Ajout et retrait d'ingrédients avec gestion des quantités, unités, dates de péremption et catégories (fruit, légume, viande, etc.).
* **Livre de Recettes :** Ajout de nouvelles recettes personnalisées (titre, instructions, ingrédients max 5) sauvegardées dans un fichier `recipes.txt`.
* **Recettes Faisables :** Filtrage et suggestion des recettes que vous pouvez cuisiner instantanément en comparant les prérequis avec le stock actuel de votre profil.
* **Mode "Cuisiner" :** Validation de la préparation d'un plat pour déduire automatiquement les ingrédients consommés de votre stock.

## Prérequis

* Un compilateur C (ex: GCC).
* Les bibliothèques de développement GTK+ 3.

## Compilation et Exécution

Ouvrez un terminal à la racine du projet et utilisez les commandes suivantes :

```bash
# Compilation
gcc main.c -o frigo_app `pkg-config --cflags --libs gtk+-3.0`

# Exécution
./frigo_app
