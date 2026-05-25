#include "string_hashing.h"
#include <math.h>

#define STRING_HT_MIN_SIZE 8
#define MAX_FULLNESS_CAP 0.75
#define TOMBSTONE_NOT_FOUND -1

#define PRIME 67        // not because of the meme i swear
                    // there are 62 == 26 + 26 + 10 characters in the language I want to hash (lowercase + uppercase + digits)
                    // the next highest prime number is 67
#define MOD 1000000009


void hash_set_resize(String_Hash_Table** h);
int string_hash(char* key);


/**
 * One of those algorithms for exponentiation when you have a modulus involved
 */
hash_t power_mod(hash_t base, hash_t exp, hash_t mod) 
{
    int res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (res * base) % mod;
        base = (base * base) % mod;
        exp /= 2;
    }
    return res;
}

String_Hash_Table* string_hashtable_init(Item_Free_Fn item_free)
{
    String_Hash_Table* ht = malloc(sizeof(String_Hash_Table));
    ht->ht = calloc(STRING_HT_MIN_SIZE,  sizeof(String_HT_Item*));
    ht->capacity = STRING_HT_MIN_SIZE;
    ht->num_active = 0;
    ht->num_elements = 0;
    ht->item_free = item_free;

    return ht;
}

void hash_set_resize(String_Hash_Table** h)
{
    if (1.0 * (*h)->num_elements / (*h)->capacity < MAX_FULLNESS_CAP) return ;

    String_Hash_Table* hs_new = malloc(sizeof(String_Hash_Table));
    hs_new->ht = calloc(((*h)->capacity << 1), sizeof(String_HT_Item *));
    hs_new->capacity = (*h)->capacity << 1;
    hs_new->num_elements = (*h)->num_active;
    hs_new->num_active = (*h)->num_active;
    hs_new->item_free = (*h)->item_free;


    for (int i = 0; i < (*h)->capacity; ++i)
    {
        if ((*h)->ht[i] == NULL) continue;
        else if ((*h)->ht[i]->tombstone)
        {
            free((*h)->ht[i]->value);
            free((*h)->ht[i]);
        } else
        {
            int index = ((*h)->ht[i]->hash) & (hs_new->capacity - 1);

            String_HT_Item* item = (*h)->ht[i];
            for (int count = 0; count < hs_new->capacity; ++count)
            {
                if (index >= hs_new->capacity) index = 0;
                if (hs_new->ht[index] == NULL)
                {
                    hs_new->ht[index] = item;
                    break;
                }
                ++index;
            }
        }
    }

    free((*h)->ht);
    free((*h));

    *h = hs_new;

    return;
}

int string_hash(char* key)
{
    hash_t hash = 0;
    for (int i = 0; key[i] != '\0'; ++i)
    {

        hash += key[i] * power_mod(PRIME, i, MOD);
    }

    return hash;
}


/**
 * Doubles up as an update if the key already exists. Note, if doing an update, 
 * if the old value's address does not match the new value's address, then the
 * old value is FREED
 */
void string_hashtable_add(String_Hash_Table** h, char* key, void* value)
{
    // Check for existence of this key
    if (1.0 * (*h)->num_elements / (*h)->capacity >= MAX_FULLNESS_CAP)
    {
        hash_set_resize(h);
    }

    hash_t hash = string_hash(key);
    int start = hash & ((*h)->capacity - 1);

    int index = start;
    int first_tombstone = TOMBSTONE_NOT_FOUND;       // -1 means not found here
    bool already_exists = false;
    for (int count = 0; count < (*h)->capacity; ++count)
    {
        if (index == (*h)->capacity) index = 0;

        if ((*h)->ht[index] == NULL)
        {
            break;
        } else if (first_tombstone == TOMBSTONE_NOT_FOUND && (*h)->ht[index]->tombstone)
        {
            first_tombstone = index;
        } else if (strcmp((*h)->ht[index]->key, key) == 0) 
        {
            already_exists = true;
            break;
        }
        ++index;
    }

    if (already_exists)
    {
        if ((*h)->ht[index]->value == value) return;
        free((*h)->ht[index]->value);       // free the old 
        (*h)->ht[index]->value = value;     // update with the new
    }

    String_HT_Item* item = malloc(sizeof(String_HT_Item));
    item->tombstone = false;
    item->hash = hash;
    item->value = value;
    item->key = key;

    if (first_tombstone == TOMBSTONE_NOT_FOUND)
    {
        (*h)->ht[index] = item;
    } else
    {
        free((*h)->ht[first_tombstone]);
        (*h)->ht[first_tombstone] = item;
    }

    (*h)->num_active++;
    (*h)->num_elements++;

    return;
}

void* string_hashtable_get(String_Hash_Table* h, char* key)
{
    hash_t hash = string_hash(key);
    int start = hash & (h->capacity - 1);

    int index = start;
    for (int count = 0; count < h->capacity; ++count)
    {
        if (index >= h->capacity) index = 0;

        if (h->ht[index] == NULL) return NULL;
        
        if (!h->ht[index]->tombstone && strcmp(h->ht[index]->key, key) == 0) return h->ht[index]->value;

        ++index;
    }

    return NULL;
}

void string_hashtable_delete(String_Hash_Table* h, char* key)
{
    hash_t hash = string_hash(key);
    int start = hash & (h->capacity - 1);

    int index = start;
    for (int count = 0; count < h->capacity; ++count)
    {
        if (index >= h->capacity) index = 0;

        if (h->ht[index] == NULL) return;
        
        if (!h->ht[index]->tombstone && strcmp(h->ht[index]->key, key) == 0)
        {
            h->ht[index]->tombstone = true;
            return;
        }

        ++index;
    }

    return;
}

// For debugging only
void string_hashtable_print(String_Hash_Table* h)
{
    for (int i = 0; i < h->capacity; ++i)
    {
        if (h->ht[i] == NULL || h->ht[i]->tombstone) continue;
        printf("\t[%d]  ", i);
        fputs(h->ht[i]->key, stdout);
        printf("\n");
    }
}


void string_hashtable_free(String_Hash_Table* h)
{
    for (int i = 0; i < h->capacity; ++i)
    {
        if (!h->ht[i]) continue;

        if (h->item_free)
        {
            h->item_free(h->ht[i]);
        } else
        {
            free(h->ht[i]);
        }
    }
    free(h->ht);
    free(h);
}
