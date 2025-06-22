#include "hashmap.h"
int main(){

 
//programme test à mettre dans un .c
    printf("hello ssaz!\n");
    printf("let's create a hashmap hihi\n \n"); 


    hashTable * table = create_table(6);
    char val[255];
    int check;

    add_item(table, "link","summer");
/*     add_item(table, "zelda","winter");
    add_item(table, "ganon","summer");
    add_item(table, "noya","winter");
    add_item(table, "impa","winter");
    add_item(table, "rauru","summer"); */
/*     check = add_item(table, "sonia","summer");
    printf("check : %d\n",check); */
    printf("nb elem : %d\n",table->nb_elem);

    //ne pas oubier dépassement ! 
    // pour vote ajouter gestion verifiation du vote pour choix existants?

    print_table(table);

    printf("\n");
    printf("let's try to add an item already there\n"); 
    printf(" *add_item(table, link, 2)* it shoudln't change anything\n\n");

    check = add_item(table,"link", "summer");
    printf("check : %d\n",check); 

    print_table(table);

    printf("\n");
    printf("now let's check the search function\n"); 
    printf("return the value of rauru:");

    if(search_item(table,"rauru")){
        strcpy(val, search_item(table,"rauru"));
        printf(" %s\n",val);
    }

    

 
    return 0;
}

