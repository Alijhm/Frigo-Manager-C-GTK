#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int checkAndCreateUser(char *username) {
    char filename[100];
    sprintf(filename, "Users/%s.txt", username);

    FILE *fp = fopen(filename, "r");
    if (fp) {
        fclose(fp);
        return 1;
    } else {
        #ifdef _WIN32
            mkdir("Users");
        #else
            mkdir("Users", 0777);
        #endif
        
        fp = fopen(filename, "w");
        if (fp) {
            fclose(fp);
            return 0;
        }
    }
    return -1;
}

void usernameToFilename(char *username, char *filename) {
    sprintf(filename, "Users/%s.txt", username);
}