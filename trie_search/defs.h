
#define BUFFERSIZE 256
#define DIRSIZE 50
#define LETTERS 26
#define CTI(c) ((int)c - (int)'a')

extern int dir_cnt;
extern struct trie_node *root;
extern pthread_mutex_t trie_mtx[LETTERS];
extern char* curr_prefix;
extern pthread_mutex_t file_mtx;
extern int programending;
extern char dirs[100][50];
typedef struct trie_node 
{
    char c; 
    int term; 
    int subwords; 
    struct trie_node *parent; 
    struct trie_node *children[LETTERS]; 
} trie_node;

typedef struct search_result 
{
    int result_count; 
    char **words; 
} search_result;

typedef struct scanned_file 
{
    char file_name[256]; 
    time_t mod_time;
 	struct scanned_file *next;
} scanned_file;

extern scanned_file *headsf;
extern void scanner_init(); 
extern void *scanner_work(void *_args); 

extern void trie_init(); 
extern void trie_add_word(char *word); 
extern void trie_get_words(search_result **rs,trie_node *node,char *prefix);
extern void trie_free_result(search_result *result); 
extern void trie_set_current_prefix(char *prefix, void (*callback)(char *word)); //adjusting current search prefix and callback function 
extern void print_new_discovery(char *new); // for cback
