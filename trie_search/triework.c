#include <stdio.h> 
#include <pthread.h> 
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "defs.h"

pthread_mutex_t trie_mtx[LETTERS];
struct trie_node *root;
char *curr_prefix = NULL;
void (*callafter)(char *word) = print_new_discovery;

struct trie_node* make_node()
{
	struct trie_node *node = (struct trie_node*)malloc(sizeof(struct trie_node));
	node->c = 0;
	node->term = 0;
	node->subwords = 0;
	node->parent = NULL;
	for (int i=0; i<LETTERS; i++){
		node->children[i] = NULL;
	}
	return node;
}

void trie_init(){
	root = make_node();
	for(int i = 0; i < LETTERS; i++)
	{
		pthread_mutex_init(&trie_mtx[i], NULL);
	}
}

void trie_add_word(char *word){
	int level; 
    int length = strlen(word); 
    int index; 
  
    struct trie_node *pCrawl = root; 
  	
  	int lock = CTI(word[0]);
	if(lock < 0 || lock > 25) return;
	pthread_mutex_lock(&trie_mtx[lock]);

    for (level = 0; level < length; level++) 
    {         		
        index = CTI(word[level]); 
        if (index > 25 || index < 0) {
			pthread_mutex_unlock(&trie_mtx[lock]);
			return;	
		}	
        if (!pCrawl->children[index]){
            pCrawl->children[index] = make_node(); 
            pCrawl->children[index]->parent = pCrawl;
			pCrawl->children[index]->c = word[level];
        } 
        pCrawl = pCrawl->children[index]; 
    } 
  	
  	if (pCrawl->term == 0){							
		pCrawl->term = 1;								
		for (level = length-1; level >= 0; level--){
			pCrawl->parent->subwords++;
			pCrawl = pCrawl->parent;
		}
		if (curr_prefix != NULL &&strncmp(curr_prefix, word, strlen(curr_prefix)) == 0 ) {
			callafter(word);
		}
	}
 	pthread_mutex_unlock(&trie_mtx[lock]);
}

void trie_get_words(search_result **sr,trie_node *node,char *prefix){
	int i;
	char k[100]= {0};
	char a[2] = {0};
	if(node == NULL) return;
	if(node->term == 1 ){
		(*sr)->words[(*sr)->result_count] = malloc(100 * sizeof(char));
		strcpy((*sr)->words[(*sr)->result_count], prefix);
		(*sr)->result_count++;
	}
		for(int i=0;i<26;i++){
			strcpy(k,prefix);
			a[0] = i+ 'a';
			a[1] = '\0';
			strcat(k,a);
			trie_get_words(sr,node->children[i],k);
		}
}

void trie_free_result(search_result *result){

	while(result->result_count >0){
		result->result_count--;
		free(result->words[result->result_count]);
		result->words[result->result_count] = NULL;
	}

	free(result->words);
	result->words = NULL;
	free(result);

}


void print_new_discovery(char *new) {
	printf("Newly discovered word is %s\n", new);
}

void trie_set_current_prefix(char *prefix, void (*callback)(char *word)){
	curr_prefix = malloc(sizeof(char)*100);
	strcpy(curr_prefix,prefix);
	callafter = callback;
} 
