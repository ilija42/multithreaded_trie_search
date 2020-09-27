#include <stdio.h> 
#include <pthread.h> 
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include <dirent.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
   pthread_t threads_scanning[256];
	 trie_init();
   scanner_init();
	 char addCommand[] = "_add_";
	 char stopCommand[] = "_stop_";
	 char *dir  = malloc(DIRSIZE*sizeof(char));
	 char *token = malloc(BUFFERSIZE*sizeof(char));
	 char *buffer = malloc(BUFFERSIZE*sizeof(char));

    while(fgets(buffer,BUFFERSIZE,stdin)){
        if(!strcmp(buffer,"\n"))continue;
    	strtok(buffer,"\n");
    	strcpy(token,buffer);
    	strtok(token," ");
        if(token == NULL) continue;
    	
    	 if(!strcmp(stopCommand,token)){
    		break;
    	}
    	else if(!strcmp(addCommand,token)){
    		dir = strtok(NULL," ");
    		if(dir == NULL)continue;
            printf("Pravim thread");
    		pthread_create(&threads_scanning[dir_cnt], NULL, scanner_work, (void*)dir); 

    	}
    	else{
            if(token ==NULL || !strcmp(token,"") || !strcmp(token," ") || !strcmp(token,"\0"))continue;
            int level; 
            int length = strlen(token); 
            int index; 
            search_result *sr =  (struct search_result*)malloc(sizeof(struct search_result));
            sr->result_count = 0;        
            struct trie_node *pCrawl = root; 
            char word[length+5];

            int lock_at = CTI(token[0]);
            if (lock_at < 0 || lock_at > 25) continue;
            pthread_mutex_lock(&trie_mtx[lock_at]); 
          
             for (level = 0; level < length; level++) 
             { 
                index = CTI(token[level]);     
                if (index > 25 || index < 0) {
                     pthread_mutex_unlock(&trie_mtx[lock_at]);
                     goto badinput;
                }
                if (!pCrawl->children[index]){
                    pthread_mutex_unlock(&trie_mtx[lock_at]);    
                    goto nopref;                                            
                }  
                pCrawl = pCrawl->children[index]; 
             }   
             sr->words = malloc(sizeof(char*)*(pCrawl->subwords+1));
              
    		 trie_get_words(&sr,pCrawl,token);
             pthread_mutex_unlock(&trie_mtx[lock_at]);  
             if (sr->result_count >=1) {
                for (int i = 0; i < sr->result_count; i++) {
                    printf("Word num %d is: %s \n",i,sr->words[i]);
                }
                trie_free_result(sr);
             }  
             else
             {
                 nopref: printf("No results for this prefix\n");    
                 continue;                   
             }     
              

            trie_set_current_prefix(token,print_new_discovery);

            while(fgets(buffer,BUFFERSIZE,stdin)){if(!strcmp(buffer,"\n"))break;}
            free(curr_prefix);
            curr_prefix = NULL;
            printf("Search over \n");
                //break;  
    	}
    	badinput : continue;
    
    }
    programending = 1;
    printf("Program ending %d\n",programending);
    for (int i = 0; i < dir_cnt; i++) {  
        printf("Waiting on dir %s \n",dirs[i]);
        pthread_join (threads_scanning[i], NULL);
    }
   
    pthread_mutex_destroy(&file_mtx);

    for (int i = 0; i< LETTERS; i++) pthread_mutex_destroy(&trie_mtx[i]);
    

    return 0;	
}


