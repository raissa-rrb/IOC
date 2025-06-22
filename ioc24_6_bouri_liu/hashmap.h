#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct hash_item {

    char* key; //voter's name
    char*   value; //0 :  winter / 1 : summer / -1 : didn't vote
    struct hash_item* next; //pointer to the next item
    
} hash_item;

typedef struct hashTable {
    
    size_t size; //capacité max
    size_t nb_elem; //current number of elements
    hash_item ** item; // pointer to hash_item
} hashTable;

/* create a hash_item, returns the created item 
*   ini the key : name 
*   ini the value 
*/

hash_item* create_item(char * key, char* value)
{
    /* memory allocation for the item */
    hash_item* item = (hash_item*) malloc(sizeof(hash_item));

    /* allocates strlen + 1 for '\n */
    item->key = (char*) malloc(strlen(key)+1);
    item->value = (char*) malloc(strlen(value)+1);

    item->next = (hash_item*) malloc(sizeof(hash_item));

    /* copy of given key into field key of the item */
    strcpy(item->key, key);
    strcpy(item->value, value);


    item->next = NULL;

    return item;

}

/* create a hash_table
*   allocates memory
*   ini size and nb_elem
*/

 hashTable* create_table(int size)
 {
    /* memory allocation for the table */
    hashTable* hTable =  (hashTable*) malloc(sizeof(hashTable));
    /* ini fields size and nb_elem */
    hTable->size = size;
    hTable->nb_elem = 0;
    /* allocates memory for the items, we have size items */
    hTable->item = (struct hash_item**) malloc(sizeof(struct hash_item*)*size);

    /*set items to null for clean space*/
    for(int i =0; i < size; i++){
        hTable->item[i] = NULL;
    }

    return hTable;
 }
/* free item 
* desallocates the allocated memory
*/

void free_item(hash_item* item)
{
    free(item->key);
    free(item->value);
    if(item->next)
        free(item->next);
    free(item);
}

/* free a hash_table
*   desallocates memory
*/

void free_table(hashTable* table)
{
    /* free all the items it contains */
    for(int i =0; i < table->size; i++){
        if(table->item[i] != NULL)
            free(table->item[i]);
    }

    /* then we can free the fields */
    free(table->item);
    free(table);

}

int hash_function(hashTable* mp, char* key){
    int bucketIndex;
    int sum = 0, factor = 31;
    for (int i = 0; i < strlen(key); i++) {
 
        // sum = sum + (ascii value of
        // char * (primeNumber ^ x))...
        // where x = 1, 2, 3....n
        sum = ((sum % mp->size)
               + (((int)key[i]) * factor) % mp->size)
              % mp->size;
 
        // factor = factor * prime
        // number....(prime
        // number) ^ x
        factor = ((factor % __INT16_MAX__)
                  * (31 % __INT16_MAX__))
                 % __INT16_MAX__;
    }
 
    bucketIndex = sum;
    return bucketIndex;
}

int add_item(hashTable* table, char* key, char* value)
{
    /* create the item */
    hash_item* item = create_item(key,value);
    
    /* item's index */
    int index = hash_function(table,key);

    hash_item* check_item = table->item[index];

    /* if empty at this index then we create new item */
    if(check_item ==  NULL){
        table->item[index] = item;
        table->nb_elem++;
        return 0;
    }

    else {
        /* if not null check if it's the same */
        if( strcmp(check_item->key , key) == 0 ){
            printf("you already voted \n");
            return 1; // item already in the table
        }
        else{
            /* it's not the head let's go through it */
            check_item = check_item->next;
            while(check_item){
                if( strcmp(check_item->key, key))
                    return 1;
                check_item = check_item->next;
            }
            /* if we get here that means it's not in the table so we add it at the head */
            item->next = table->item[index]; //link the new and the previous head
            table->item[index] = item; //add new one at the head
            table->nb_elem++;
            return 0;

        }
    }
}

char* search_item( hashTable* table, char* key )
{
    int index = hash_function(table, key);

    hash_item* cur = table->item[index]; 

    while(cur){
        if( strcmp(cur->key, key) == 0)
            return cur->value;

        cur = cur->next;
    }
    return NULL;
}

void print_table(  hashTable * table){

    printf("****** start printing table ******\n");

    /* to print we have to print the content of a whole line then get to the i+1 line */
    for(int i = 0; i < table->size ; i++ ){

        hash_item* cur = table->item[i];

        while(cur){
            
            printf(" key : %s \t value : %s \n", cur->key, cur->value);
            cur = cur->next;
        }
    }
    printf("****** end of table ******\n");
}


