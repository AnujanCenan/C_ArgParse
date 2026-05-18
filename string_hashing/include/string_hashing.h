// Based on the article at https://cp-algorithms.com/string/string-hashing.html

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct String_HT_Item String_HT_Item;
typedef struct String_Hash_Table String_Hash_Table;
typedef void (*Item_Free_Fn) (String_HT_Item* item);


typedef uint64_t hash_t;

struct String_HT_Item
{
    char* key;
    void* value;        // assumes the value is on the heap
    hash_t hash;
    bool tombstone;
};

struct String_Hash_Table
{
    String_HT_Item** ht;
    Item_Free_Fn item_free;
    int num_active;
    int num_elements;       // num active + num tombstones
    int capacity;

};

String_Hash_Table* string_hashtable_init(Item_Free_Fn item_free);
void string_hashtable_add(String_Hash_Table** h, char* key, void* value);
void* string_hashtable_get(String_Hash_Table* h, char* key);
void string_hashtable_delete(String_Hash_Table* h, char* key);
void string_hashtable_print(String_Hash_Table* h);
void string_hashtable_free(String_Hash_Table* h);