#include "string_hashing.h"

int main()
{

    char* key1 = "hello";
    char* key2 = "goodbye";
    char* key3 = "talkative";


    String_Hash_Table* ht = string_hashtable_init();
    string_hashtable_add(&ht, key1, strdup("HELLO"));
    string_hashtable_add(&ht, key2, strdup("GOODBYE"));
    string_hashtable_add(&ht, key3, strdup("TALKATIVE"));

    fputs((char*) string_hashtable_get(ht, key1), stdout);
    printf("\n");
    fputs((char*) string_hashtable_get(ht, key2), stdout);
    printf("\n");
    fputs((char*) string_hashtable_get(ht, key3), stdout);
    printf("\n");

    char* key4 = "random";

    char* val = string_hashtable_get(ht, key4);
    if (!val) printf("Passed\n");

    string_hashtable_print(ht);
    string_hashtable_delete(ht, key2);
    printf("\n");
    string_hashtable_print(ht);



    return 0;
}