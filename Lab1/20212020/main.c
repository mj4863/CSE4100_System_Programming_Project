#include "bitmap.h"
#include "debug.h"
#include "hash.h"
#include "hex_dump.h"
#include "limits.h"
#include "list.h"
#include "round.h"

struct list create_lists[10];
struct hash create_hash[10];
struct bitmap *create_bitmap[10];

bool list_less (const struct list_elem *a, const struct list_elem *b, void *aux) {
    struct list_item *A = list_entry(a, struct list_item, elem);
    struct list_item *B = list_entry(b, struct list_item, elem);

    return A->data < B->data;
} 

unsigned hash_hash (const struct hash_elem *e, void *aux) {
    return hash_int(e->data);
}

bool hash_less (const struct hash_elem *a, const struct hash_elem *b, void *aux) {
    return a->data < b->data;
}

void hash_square(struct hash_elem *e, void *aux) {
    e->data = e->data * e->data;
}

void hash_triple(struct hash_elem *e, void *aux) {
    e->data = e->data * e->data * e->data;
}

void hash_print(struct hash_elem *e, void *aux) {
    printf("%d ", e->data);
}

void hash_destructor(struct hash_elem *e, void *aux) {
    list_remove(&e->list_elem);
}

void dumpdata(struct bitmap *b) {
    if (b != NULL) {
        for (size_t i = 0; i < bitmap_size(b); ++i)
            printf("%d", bitmap_test(b, i) ? 1 : 0);
        printf("\n");
    }
}

int main() {
    char input[100];
    while (1) {
        fgets(input, sizeof(input), stdin);

        if (strcmp(input, "quit\n") == 0) break;


        char* words[6];
        int word_count = 0;

        char *token = strtok(input, " ");
        while (token != NULL && word_count < 6) {
            words[word_count++] = token;
            token = strtok(NULL, " ");
        }
        
        if (strcmp(words[0], "create") == 0) {

            if (strcmp(words[1], "list") == 0)
                list_init(&create_lists[words[2][4] - '0']);
            
            else if (strcmp(words[1], "hashtable") == 0)
                hash_init(&create_hash[words[2][4] - '0'], *hash_hash, *hash_less, NULL);

            else if (strcmp(words[1], "bitmap") == 0)
                create_bitmap[words[2][2] - '0'] = bitmap_create(atoi(words[3]));
        }

        else if (strcmp(words[0], "delete") == 0) {
            if (words[1][0] == 'l') {
                struct list_elem *e;
                while (!list_empty(&create_lists[words[1][4] - '0'])) {
                    e = list_pop_front(&create_lists[words[1][4] - '0']);
                    free(list_entry(e, struct list_item, elem));
                }
            }

            else if (words[1][0] == 'h')
                hash_destroy(&create_hash[words[1][4] - '0'], *hash_destructor);

            else if (words[1][0] == 'b') {
                bitmap_destroy(create_bitmap[words[1][2] - '0']);
            }
        }

        else if (strcmp(words[0], "dumpdata") == 0) {
            if (words[1][0] == 'l') {
                if (!list_empty(&create_lists[words[1][4] - '0'])) {
                    struct list_elem *e;
                    for (e = list_begin(&create_lists[words[1][4] - '0']); e != list_end(&create_lists[words[1][4] - '0']); e = list_next(e)) {
                        struct list_item *item = list_entry(e, struct list_item, elem);
                        printf("%d ", item->data);
                    }
                    printf("\n");
                }
            }
            else if (words[1][0] == 'h') {
                if (!hash_empty(&create_hash[words[1][4] - '0'])) {
                    hash_apply(&create_hash[words[1][4] - '0'], *hash_print);
                    printf("\n");
                }
            }
            else if (words[1][0] == 'b')
                dumpdata(create_bitmap[words[1][2] - '0']);
        } 

        // ----------------------------------------List-------------------------------------------
        else if (strcmp(words[0], "list_push_front") == 0 || strcmp(words[0], "list_push_back") == 0) {
            int data = atoi(words[2]);
            struct list_item *item = malloc(sizeof(struct list_item));
            item->data = data;
            if (strcmp(words[0], "list_push_front") == 0)
                list_push_front(&create_lists[words[1][4] - '0'], &item->elem);
            else
                list_push_back(&create_lists[words[1][4] - '0'], &item->elem);
        }

        else if (strcmp(words[0], "list_shuffle") == 0)
            list_shuffle(&create_lists[words[1][4] - '0']);

        else if (strcmp(words[0], "list_front") == 0) {
            struct list_elem *front_elem = list_front(&create_lists[words[1][4] - '0']);
            struct list_item *front_item = list_entry(front_elem, struct list_item, elem);
            printf("%d\n", front_item->data);
        }

        else if (strcmp(words[0], "list_back") == 0) {
            struct list_elem *back_elem = list_back(&create_lists[words[1][4] - '0']);
            struct list_item *back_item = list_entry(back_elem, struct list_item, elem);
            printf("%d\n", back_item->data);
        }

        else if (strcmp(words[0], "list_pop_back") == 0)
            list_pop_back(&create_lists[words[1][4] - '0']);

        else if (strcmp(words[0], "list_pop_front") == 0)
            list_pop_front(&create_lists[words[1][4] - '0']);

        else if (strcmp(words[0], "list_insert") == 0) {
            int pos = atoi(words[2]);
            
            struct list_item *item = malloc(sizeof(struct list_item));
            item->data = atoi(words[3]);

            if (pos == 0)
                list_push_front(&create_lists[words[1][4] - '0'], &item->elem);
            else {
                struct list_elem *before = list_begin(&create_lists[words[1][4] - '0']);
                for (int i = 0; i < pos && before != list_end(&create_lists[words[1][4] - '0']); ++i)
                    before = list_next(before);
                list_insert(before, &item->elem);
            }
        }

        else if (strcmp(words[0], "list_insert_ordered") == 0) {
            struct list_item *item = malloc(sizeof(struct list_item));
            item->data = atoi(words[2]);
            list_insert_ordered(&create_lists[words[1][4] - '0'], &item->elem, list_less, NULL);
        }

        else if (strcmp(words[0], "list_empty") == 0)
            printf("%s\n", list_empty(&create_lists[words[1][4] - '0']) ? "true" : "false");

        else if (strcmp(words[0], "list_size") == 0)
            printf("%zu\n", list_size(&create_lists[words[1][4] - '0']));

        else if (strcmp(words[0], "list_max") == 0) {
            if (!list_empty(&create_lists[words[1][4] - '0'])) {
                struct list_elem *max_elem = list_max(&create_lists[words[1][4] - '0'], list_less, NULL);
                struct list_item *max_item = list_entry(max_elem, struct list_item, elem);
                printf("%d\n", max_item->data);
            }
        }

        else if (strcmp(words[0], "list_min") == 0) {
            if (!list_empty(&create_lists[words[1][4] - '0'])) {
                struct list_elem *min_elem = list_min(&create_lists[words[1][4] - '0'], list_less, NULL);
                struct list_item *min_item = list_entry(min_elem, struct list_item, elem);
                printf("%d\n", min_item->data);
            }
        }

        else if (strcmp(words[0], "list_remove") == 0) {
            int pos = atoi(words[2]);

            struct list_elem *e = list_begin(&create_lists[words[1][4] - '0']);
            for (int i = 0; i < pos && e != list_end(&create_lists[words[1][4] - '0']); ++i)
                e = list_next(e);
            if (e != list_end(&create_lists[words[1][4] - '0']))
                list_remove(e);
        }

        else if (strcmp(words[0], "list_reverse") == 0)
            list_reverse(&create_lists[words[1][4] - '0']);
        
        else if (strcmp(words[0], "list_sort") == 0)
            list_sort(&create_lists[words[1][4] - '0'], list_less, NULL);

        else if (strcmp(words[0], "list_splice") == 0) {
            int pos = atoi(words[2]);
            int start = atoi(words[4]);
            int end = atoi(words[5]);

            struct list_elem *before = list_begin(&create_lists[words[1][4] - '0']);
            struct list_elem *first = list_begin(&create_lists[words[3][4] - '0']);
            struct list_elem *last = list_begin(&create_lists[words[3][4] - '0']);
            for (int i = 0; i < pos && before != list_end(&create_lists[words[1][4] - '0']); ++i)
                before = list_next(before);
            for (int i = 0; i < start && first != list_end(&create_lists[words[3][4] - '0']); ++i)
                first = list_next(first);
            for (int i = 0; i < end && last != list_end(&create_lists[words[3][4] - '0']); ++i)
                last = list_next(last);

            list_splice(before, first, last);
        }

        else if (strcmp(words[0], "list_unique") == 0) {
            if (word_count == 2)
                list_unique(&create_lists[words[1][4] - '0'], NULL, list_less, NULL);
            else
                list_unique(&create_lists[words[1][4] - '0'], &create_lists[words[2][4] - '0'], list_less, NULL);
        }

        else if (strcmp(words[0], "list_swap") == 0) {
            int pos1 = atoi(words[2]);
            int pos2 = atoi(words[3]);

            struct list_elem *a = list_begin(&create_lists[words[1][4] - '0']);
            struct list_elem *b = list_begin(&create_lists[words[1][4] - '0']);
            for (int i = 0; i < pos1 && a != list_end(&create_lists[words[1][4] - '0']); ++i)
                a = list_next(a);
            for (int i = 0; i < pos2 && b != list_end(&create_lists[words[1][4] - '0']); ++i)
                b = list_next(b);

        
            list_swap(a, b);
        }

        // ----------------------------------------Hash Table-------------------------------------------
        else if (strcmp(words[0], "hash_insert") == 0) {
            struct hash_elem *e = malloc(sizeof(struct hash_elem));
            e->data = atoi(words[2]);
            hash_insert(&create_hash[words[1][4] - '0'], e);
        }
    
        else if (strcmp(words[0], "hash_apply") == 0) {
            if (strcmp(words[2], "square\n") == 0)
                hash_apply(&create_hash[words[1][4] - '0'], *hash_square);
            else if (strcmp(words[2], "triple\n") == 0)
                hash_apply(&create_hash[words[1][4] - '0'], *hash_triple);
        }

        else if (strcmp(words[0], "hash_delete") == 0) {
            struct hash_elem *e = malloc(sizeof(struct hash_elem));
            e->data = atoi(words[2]);
            hash_delete(&create_hash[words[1][4] - '0'], e);
        }

        else if (strcmp(words[0], "hash_empty") == 0)
            printf("%s\n", hash_empty(&create_hash[words[1][4] - '0']) ? "true" : "false");

        else if (strcmp(words[0], "hash_size") == 0)
            printf("%zu\n", hash_size(&create_hash[words[1][4] - '0']));
 
        else if (strcmp(words[0], "hash_clear") == 0)
            hash_clear(&create_hash[words[1][4] - '0'], *hash_destructor);

        else if (strcmp(words[0], "hash_find") == 0) {
            int value = atoi(words[2]);
            struct hash_elem *e = malloc(sizeof(struct hash_elem));
            e->data = value;
            struct hash_elem *find_e = hash_find(&create_hash[words[1][4] - '0'], e);
            if (find_e != NULL)
                printf("%d\n", value);
        }

        else if (strcmp(words[0], "hash_replace") == 0) {
            struct hash_elem *e = malloc(sizeof(struct hash_elem));
            e->data = atoi(words[2]);
            hash_replace(&create_hash[words[1][4] - '0'], e);
        }

        // ----------------------------------------Bitmap-------------------------------------------
        else if (strcmp(words[0], "bitmap_mark") == 0)
            bitmap_mark(create_bitmap[words[1][2] - '0'], atoi(words[2]));

        else if (strcmp(words[0], "bitmap_all") == 0) {
            bool result = bitmap_all(create_bitmap[words[1][2] - '0'], atoi(words[2]), atoi(words[3]));
            printf("%s\n", result ? "true" : "false");
        }

        else if (strcmp(words[0], "bitmap_any") == 0) {
            bool result = bitmap_any(create_bitmap[words[1][2] - '0'], atoi(words[2]), atoi(words[3]));
            printf("%s\n", result ? "true" : "false");
        }

        else if (strcmp(words[0], "bitmap_contains") == 0) {
            bool value = (strcmp(words[4], "true\n") == 0) ? true : false;
            bool result = bitmap_contains(create_bitmap[words[1][2] - '0'], atoi(words[2]), atoi(words[3]), value);
            printf("%s\n", result ? "true" : "false");
        }

        else if (strcmp(words[0], "bitmap_count") == 0) {
            bool value = (strcmp(words[4], "true\n") == 0) ? true : false;
            int count = bitmap_count(create_bitmap[words[1][2] - '0'], atoi(words[2]), atoi(words[3]), value);
            printf("%d\n", count);
        }

        else if (strcmp(words[0], "bitmap_dump") == 0)
            bitmap_dump(create_bitmap[words[1][2] - '0']);

        else if (strcmp(words[0], "bitmap_expand") == 0)
            bitmap_expand(create_bitmap[words[1][2] - '0'], atoi(words[2]));

        else if (strcmp(words[0], "bitmap_flip") == 0)
            bitmap_flip(create_bitmap[words[1][2] - '0'], atoi(words[2]));
        
        else if (strcmp(words[0], "bitmap_none") == 0) {
            bool result = bitmap_none(create_bitmap[words[1][2] - '0'], atoi(words[2]), atoi(words[3]));
            printf("%s\n", result ? "true" : "false");
        } 
        
        else if (strcmp(words[0], "bitmap_reset") == 0)
            bitmap_reset(create_bitmap[words[1][2] - '0'], atoi(words[2]));
        
        else if (strcmp(words[0], "bitmap_scan_and_flip") == 0) {
            bool value = (strcmp(words[4], "true\n") == 0) ? true : false;
            printf("%zu\n", bitmap_scan_and_flip(create_bitmap[words[1][2] - '0'], atoi(words[2]), atoi(words[3]), value));
        }
         
        else if (strcmp(words[0], "bitmap_scan") == 0) {
            bool value = (strcmp(words[4], "true\n") == 0) ? true : false;
            printf("%zu\n", bitmap_scan(create_bitmap[words[1][2] - '0'], atoi(words[2]), atoi(words[3]), value));
        } 
        
        else if (strcmp(words[0], "bitmap_set") == 0) {
            bool value = (strcmp(words[3], "true\n") == 0) ? true : false;
            bitmap_set(create_bitmap[words[1][2] - '0'], atoi(words[2]), value);
        } 
        
        else if (strcmp(words[0], "bitmap_set_all") == 0) {
            bool value = (strcmp(words[2], "true\n") == 0) ? true : false;
            bitmap_set_all(create_bitmap[words[1][2] - '0'], value);
        } 
        
        else if (strcmp(words[0], "bitmap_set_multiple") == 0) {
            bool value = (strcmp(words[4], "true\n") == 0) ? true : false;
            bitmap_set_multiple(create_bitmap[words[1][2] - '0'], atoi(words[2]), atoi(words[3]), value);
        } 
        
        else if (strcmp(words[0], "bitmap_size") == 0)
            printf("%zu\n", bitmap_size(create_bitmap[words[1][2] - '0']));
        
        else if (strcmp(words[0], "bitmap_test") == 0) {
            bool result = bitmap_test(create_bitmap[words[1][2] - '0'], atoi(words[2]));
            printf("%s\n", result ? "true" : "false");
        }

    }

    return 0;
}