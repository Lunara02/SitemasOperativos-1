#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include <limits.h>

#define MAX_COMMAND_LENGTH 1024

char mensaje[32];
static char *prompt = "sys:";
char dir[PATH_MAX];
pid_t pid =-1;

void parse_command(char *command, char **args, char *cut) {
    int i = 0;
    args[i] = strtok(command, cut);
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, cut);
    }
}

void ejecutar_comando(char **args, int *E) {
    pid = fork();
    if (pid == 0) {  
        execvp(args[0], args);
        perror("exec failed");
        exit(1);
    } else if (pid > 0) {  
        int status;
        waitpid(pid, &status, 0);  
        *E = WEXITSTATUS(status);
        pid = -1;    
    } else {
        perror("fork failed");
        exit(1);
    }
}

void ejecutar_con_pipe(char **commands, int num_pipes, int *E) {
    int pipefd[2];
    int input_fd = 0; 
    char *args[64];
    int status;

    for (int i = 0; i < num_pipes; i++) {
        if (i < num_pipes - 1) { 
            if (pipe(pipefd) == -1) {
                perror("pipe failed");
                exit(1);
            }
        }

        pid = fork();
        if (pid == 0) {  
            if (i < num_pipes - 1) {
                dup2(pipefd[1], STDOUT_FILENO); 
            }
            dup2(input_fd, STDIN_FILENO); 

            if (i < num_pipes - 1) {
                close(pipefd[0]);
                close(pipefd[1]);
            }

           
            parse_command(commands[i], args, " ");
            execvp(args[0], args);
            perror("exec failed");
            exit(1);
        } else if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

       
        if (i < num_pipes - 1) {
            close(pipefd[1]); 
            input_fd = pipefd[0]; 
        }

        waitpid(pid, &status, 0); 
        *E = WEXITSTATUS(status);
        pid = -1; 
    }
}

void sig_handler(int sig){
    
    if(pid==-1){
        printf("\n");
        printf("%s~%s$ ", prompt, dir);
        fflush(stdout);
    }
    
    else{
        printf("\n");
        kill(pid, SIGINT);
    }
}
int esMensaje(char *str){
    if (*str == '\0') {
        return 0;
    }
    if(*str=='"'){
        str += strlen(str)-2;
        if(*str=='"'){
            return 1;
        }
        else{
            return 0;
        }
    }
    else{
        return 0;
    }
}

int esNumero(char *str) {
    if(str == NULL){
        return 0;
    }
    if (*str == '\0') {
        return 0;
    }

    while (*str) {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

void alarm_handler(int sig){
    printf("\n%s\n", mensaje);
    printf("%s~%s$ ", prompt, dir);
    fflush(stdout);
}

int set_recordatorio(char **args){
    if(args[0]!=NULL && args[1]!=NULL){
        if(strcmp(args[0], "set")==0 || strcmp(args[1], "recordatorio")==0){
            if(esNumero(args[2])){
                memset(mensaje, '\0', sizeof(mensaje));
                for(int i = 3;; i++){
                    if(args[i] != NULL){
                        strcat(mensaje, args[i]);
                        strcat(mensaje, " ");
                    }
                    else{
                        break;
                    }
                }
                if(esMensaje(mensaje)){
                    signal(SIGALRM, alarm_handler);
                    alarm(atoi(args[2]));
                    return 1;
                }        
            }
        }
    }
    return 0;
}

int detectar_cd(char **args){
    if (args[1] == NULL || strcmp(args[1], "~") == 0) {
        args[1] = getenv("HOME");
    }
    if (chdir(args[1]) != 0) {
        perror("cd falló");
        return 0;
    }
    else{
        return 1;
    }
}

int comando_existe(FILE *f, const char *comando) {
    char line[MAX_COMMAND_LENGTH];
    rewind(f);
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, comando) == 0) {
            return 1;
        }
    }
    return 0;
}


int main() {
    char command[MAX_COMMAND_LENGTH], commandPaste[MAX_COMMAND_LENGTH], commandget[MAX_COMMAND_LENGTH];
    char *commands[64]; 
    char *args[64];
    char *del[256];
    FILE *file = NULL;
    FILE *fav = fopen("favs.txt", "a+");
    FILE *delet = NULL;
    int num_pipes = 0, statusT, x;

    signal(SIGINT, sig_handler);

    file = fopen("misfavoritos.txt", "r");
    if (file != NULL) { 
           fclose(file);      
           file = fopen("misfavoritos.txt", "a+");
    }
    
    while (1) {
        // SHELL
        if (getcwd(dir, sizeof(dir)) != NULL) {
            printf("%s~%s$ ", prompt, dir);
        } else {
            perror("getcwd() error");
        }

        if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) {
            perror("Error al leer el comando");
            exit(1);
        }
        
        command[strcspn(command, "\n")] = '\0'; 
        if (strlen(command) == 0) {
            continue;
        }

        strcpy(commandPaste, command);

        if (strcmp(command, "exit") == 0) {
            break;
        }


        if (strchr(command, '|') == NULL) {
            parse_command(command, args, " ");
            if(set_recordatorio(args)){
                continue;
            }
            if(strcmp(args[0], "cd") == 0){
                if(detectar_cd(args)){
                    continue;
                }
            }
            if(strcmp(args[0], "favs") == 0 && args[1] != NULL){
                if(strcmp(args[1], "crear") == 0){
                    file = fopen("misfavoritos.txt", "a+");
                }
                else if(strcmp(args[1], "mostrar") == 0){
                    x = 1;
                    rewind(fav);
                    while(fgets(commandget, sizeof(commandget), fav)){
                        printf("%d) %s", x, commandget);
                        x++;
                    }
                    x = 1;
                }
                else if(file != NULL){
                    if(strcmp(args[1], "guardar") == 0){
                        rewind(fav);
                        while(fgets(commandget, sizeof(commandget), fav)){
                               commandget[strcspn(commandget, "\n")] = '\0';
                            if (!comando_existe(file, commandget)) {
                                fprintf(file, "%s\n", commandget);
                            }
                        }
                    }
                    else if(strcmp(args[1], "cargar") == 0){
                        x = 1;
                        rewind(file);
                        while(fgets(commandget, sizeof(commandget), file)){
                            printf("%d) %s", x, commandget);
                            x++;
                        }
                        x = 1;
                    }
                    else if(strcmp(args[1], "buscar") == 0){
                        x = 1;
                        rewind(file);
                        while(fgets(commandget, sizeof(commandget), file)){
                            if(strstr(commandget, args[2])){
                                printf("%d) %s", x, commandget);
                            }
                            x++;
                        }
                        x = 1;
                    }
                    else if(strcmp(args[1], "borrar") == 0){
                        fclose(file);
                        file = fopen("misfavoritos.txt", "w");
                        fclose(file);
                        file = fopen("misfavoritos.txt", "a+");
                    }
                    else if(strcmp(args[1], "eliminar") == 0){
                        if (args[2] != NULL) {
                            int indices[256], num_indices = 0, num_favoritos = 0;
                            char favoritos[256][MAX_COMMAND_LENGTH]; 
                            parse_command(args[2], del, ","); 
                            for(int i = 0; del[i] != NULL; i++){
                                if(esNumero(del[i])){
                                    indices[num_indices++] = atoi(del[i]);
                                }
                            }
                            rewind(file);
                            while(fgets(commandget, sizeof(commandget), file)) {
                                commandget[strcspn(commandget, "\n")] = '\0'; 
                                strcpy(favoritos[num_favoritos++], commandget); 
                            }
                            file = fopen("misfavoritos.txt", "w");
                            for(int i = 0; i < num_favoritos; i++) {
                                int eliminar = 0;
                                for(int j = 0; j < num_indices; j++) {
                                    if(indices[j] == i + 1) { 
                                        eliminar = 1;
                                        break;
                                    }
                                }
                                if(!eliminar) {
                                    fprintf(file, "%s\n", favoritos[i]); 
                                }
                            }
                            fclose(file);
                            file = fopen("misfavoritos.txt", "a+");
                        }
                    }
                    else if(args[2] != NULL && strcmp(args[2], "ejecutar") == 0){
                        int Num = atoi(args[1]);
                        x = 1;
                        rewind(file);
                        while(fgets(commandget, sizeof(commandget), file)){
                            commandget[strcspn(commandget, "\n")] = '\0'; 
                            if(Num == x){
                                if(strchr(commandget, '|') == NULL){
                                    parse_command(commandget, args, " ");
                                    ejecutar_comando(args, &statusT);
                                   
                                } else {
                                    
                                    char *subcommand = strtok(commandget, "|");
                                    num_pipes = 0;
                                    while (subcommand != NULL && num_pipes < 64) {
                                        commands[num_pipes++] = subcommand;
                                        subcommand = strtok(NULL, "|");
                                    }
                                    commands[num_pipes] = NULL;
                                    ejecutar_con_pipe(commands, num_pipes, &statusT);
                                }
                            }
                            x++;
                        }
                    } else {
                        perror("exec failed");
                    }
                }
                else {
                    perror("exec failed");
                    printf("misfavoritos.txt no existe\n");
                }
            } else {
                ejecutar_comando(args, &statusT);
                if(statusT == 0){
                    if (!comando_existe(fav, commandPaste)) {
                        fprintf(fav, "%s\n", commandPaste);
                    }
                }
            }        

        } else {  
            
            char *subcommand = strtok(command, "|");
            num_pipes = 0;
            while (subcommand != NULL) {
                commands[num_pipes++] = subcommand;
                subcommand = strtok(NULL, "|");
            }
            commands[num_pipes] = NULL;
            ejecutar_con_pipe(commands, num_pipes, &statusT);
            if(statusT == 0){
                if (!comando_existe(fav, commandPaste)) {
                    fprintf(fav, "%s\n", commandPaste);
                }
            }
        }      
    }
    if(file != NULL){
        fclose(file);
    }
    fclose(fav);
    remove("favs.txt");
    return 0;
}

