#include <stdio.h> 
#include <pthread.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "defs.h"

scanned_file *headsf = NULL;
pthread_mutex_t file_mtx;
int dir_cnt;
int programending;
char dirs[100][50];

void scanner_init(){
    int programending = 0;
    dir_cnt = 0;
    pthread_mutex_init(&file_mtx, NULL);
}

struct scanned_file* addScanned(char *name, time_t time)
{
    scanned_file *newsf;
    if(headsf == NULL){
        headsf = malloc(sizeof(scanned_file));
        
        headsf->mod_time = time;
        strcpy(headsf->file_name,name);
        headsf->next = NULL;
        return headsf;
    }
    scanned_file *currentsf = headsf;
    while(currentsf != NULL){
        if(currentsf->next == NULL){
        newsf = malloc(sizeof(scanned_file));
        newsf->mod_time = time;
        strcpy(newsf->file_name,name);
        newsf->next = NULL;

        currentsf->next = newsf;

        break;
        }
        currentsf = currentsf->next;
    }
   return newsf;
}

struct scanned_file* isAlreadyScanned(char* name){
    if(headsf == NULL) return NULL;
    scanned_file  *currentsf = headsf;

    while(currentsf != NULL){
        if (strncmp(currentsf->file_name, name, strlen(name)) == 0) {
            pthread_mutex_unlock(&file_mtx);
            return currentsf;
        }
        currentsf = currentsf->next;
    }
    return NULL;
}

void openEntry(char *path){
  
    char *foo = malloc(200);
    char rv = 0;
    FILE *fp;
    fp = fopen(path, "r");
    char *word; 
      printf(" OPEN %s \n",path);
    do {
      
        rv = fscanf(fp, "%s", foo);

        if(rv == EOF){
            printf("File %s closed\n",path);
            break;
        }

        word = strtok(foo," ");
        while(word!=NULL){
            int i = 0;
            while(i < strlen(word)){
                if (! (word[i] >= 'a' && word[i] <= 'z')){
                    i = -1; 
                    break;
                }
                i++;
            }
            if(i == -1){
                 word = strtok(NULL," ");
                continue;
            } 
            trie_add_word(word);
            word = strtok(NULL," ");
        }
      
    }while(1);
    free(foo);
    fclose(fp);
}

void* scanner_work(void* _args){
    dir_cnt++;
    DIR *dir;
    char name[DIRSIZE];
    strcpy(name, (char*)_args);
    char *fpath = malloc(DIRSIZE*sizeof(char));
    dir = opendir(name);    
    if (! dir) {
        printf("Missing directory\n");
        dir_cnt--;
        pthread_exit(NULL);
    }
    strcpy(dirs[dir_cnt-1],name);
    for(;;) {
        if(! dir) {
            printf("Directory deleted\n");
            break;
        }
        if(programending){
             break;
        } 
        rewinddir(dir);                  
        struct dirent *direntry;  
        for(;;){

            if(programending){         
             break;
             } 

            if((direntry = readdir(dir)) == NULL){
                printf("Directory scanned, now scanning again for changes\n");
                break;
            } 

            if (direntry->d_type == DT_REG)  
            {
                strcpy(fpath, name);
                strcat(fpath, "/");
                strcat(fpath, direntry->d_name);   
                struct stat filestats;        
                stat(fpath, &filestats);      
                time_t time = filestats.st_mtime;
                pthread_mutex_lock(&file_mtx);                   
                struct scanned_file *sf;   
                if (!(sf = isAlreadyScanned(direntry->d_name))){
                    sf = addScanned(direntry->d_name, filestats.st_mtime);  
                    printf("New file found at %s\n",fpath); 
                }  
                else if (sf->mod_time != filestats.st_mtime){                             
                     sf->mod_time = time;
                }
                else {                                                         
                    pthread_mutex_unlock(&file_mtx);
                    continue;           
                }
                pthread_mutex_unlock(&file_mtx);
                openEntry(fpath);
               // memset(fpath,0,80);
            }
        }
        sleep(5);
    }
    if (dir) closedir(dir);
    pthread_exit(NULL);
}