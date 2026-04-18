#include "queries.h"
#include "helpers.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int compare_strings(const void *a, const void *b){
    return strcmp(*(const char **)a, *(const char **)b);
}

typedef struct {
    char *name;
    int   count;
} PotionEntry;

static int compare_potion_entries(const void *a, const void *b){
    return strcmp(((PotionEntry *)a)->name, ((PotionEntry *)b)->name);
}


int handle_gold_query(GameState *state, char *line){
    char buf[1024];
    strncpy(buf,line,sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc != 3) return 0;
    if(strcmp(tokens[0], "Total") != 0) return 0;
    if(strcmp(tokens[1], "gold")  != 0) return 0;
    if(strcmp(tokens[2], "?")     != 0) return 0;

    printf("%d\n", state->gold);
    return 1;
}

int handle_floor_query(GameState *state, char *line){
    char buf[1024];
    strncpy(buf,line,sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf,tokens,128);

    if(tc != 2) return 0;
    if(strcmp(tokens[0], "Floor") != 0) return 0;
    if(strcmp(tokens[1], "?")     != 0) return 0;

    printf("%d\n", state->floor);
    return 1;
}

int handle_where_query(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc != 2) return 0;
    if(strcmp(tokens[0], "Where") != 0) return 0;
    if(strcmp(tokens[1], "?")     != 0) return 0;

    printf("%s\n", state->current_room);
    return 1;
}

int handle_health_query(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc != 2) return 0;
    if(strcmp(tokens[0], "Health") != 0) return 0;
    if(strcmp(tokens[1], "?")      != 0) return 0;

    printf("%d/%d\n", state->current_hp, state->max_hp);
    return 1;
}

int handle_deck_size_query(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc != 3) return 0;
    if(strcmp(tokens[0], "Deck") != 0) return 0;
    if(strcmp(tokens[1], "size") != 0) return 0;
    if(strcmp(tokens[2], "?")    != 0) return 0;

    int total = 0;
    for(int i = 0; i < state->deck.size; i++){
        total += state->deck.base[i] + state->deck.upgraded[i];
    }
    printf("%d\n", total);
    return 1;
}

int handle_total_card_query(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    /* minimum: "Total card X ?" = 4 tokens */
    if(tc < 4) return 0;
    if(strcmp(tokens[0], "Total") != 0) return 0;
    if(strcmp(tokens[1], "card")  != 0) return 0;
    if(strcmp(tokens[tc - 1], "?") != 0) return 0;

    /* card name is tokens[2] to tokens[tc-2] */
    char card_name[512];
    join_tokens(card_name, tokens, 2, tc - 2);

    /* check double spaces in name portion of original line */
    const char *name_start = find_token_start(line, 2);
    if(name_start == NULL) return 0;

    /* name ends before " ?" */
    const char *q_in_original = strstr(name_start, " ?");
    if(q_in_original == NULL) return 0;

    int name_len = q_in_original - name_start;
    char name_portion[512];
    strncpy(name_portion, name_start, name_len);
    name_portion[name_len] = '\0';
    if(has_double_space(name_portion)) return 0;

    if(!is_valid_name(card_name)) return 0;

    int found = find_deck_card(&state->deck, card_name);
    if(found == -1){
        printf("0\n");
    } else {
        printf("%d\n", state->deck.base[found] + state->deck.upgraded[found]);
    }
    return 1;
}

int handle_total_upgraded_card_query(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    /* minimum: "Total upgraded card X ?" = 5 tokens */
    if(tc < 5) return 0;
    if(strcmp(tokens[0], "Total")    != 0) return 0;
    if(strcmp(tokens[1], "upgraded") != 0) return 0;
    if(strcmp(tokens[2], "card")     != 0) return 0;
    if(strcmp(tokens[tc - 1], "?")   != 0) return 0;

    /* card name is tokens[3] to tokens[tc-2] */
    char card_name[512];
    join_tokens(card_name, tokens, 3, tc - 2);

    /* check double spaces */
    const char *name_start = find_token_start(line, 3);
    if(name_start == NULL) return 0;

    const char *q_in_original = strstr(name_start, " ?");
    if(q_in_original == NULL) return 0;

    int name_len = q_in_original - name_start;
    char name_portion[512];
    strncpy(name_portion, name_start, name_len);
    name_portion[name_len] = '\0';
    if(has_double_space(name_portion)) return 0;

    if(!is_valid_name(card_name)) return 0;

    int found = find_deck_card(&state->deck, card_name);
    if(found == -1){
        printf("0\n");
    } else {
        printf("%d\n", state->deck.upgraded[found]);
    }
    return 1;
}

int handle_defeated_query(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    /* minimum: "Defeated X ?" = 3 tokens */
    if(tc < 3) return 0;
    if(strcmp(tokens[0], "Defeated")  != 0) return 0;
    if(strcmp(tokens[tc - 1], "?")    != 0) return 0;

    /* enemy name is tokens[1] to tokens[tc-2] */
    char enemy_name[512];
    join_tokens(enemy_name, tokens, 1, tc - 2);

    /* check double spaces */
    const char *name_start = find_token_start(line, 1);
    if(name_start == NULL) return 0;

    const char *q_in_original = strstr(name_start, " ?");
    if(q_in_original == NULL) return 0;

    int name_len = q_in_original - name_start;
    char name_portion[512];
    strncpy(name_portion, name_start, name_len);
    name_portion[name_len] = '\0';
    if(has_double_space(name_portion)) return 0;

    if(!is_valid_name(enemy_name)) return 0;

    /* look up defeated log */
    for(int i = 0; i < state->defeated_log.size; i++){
        if(strcmp(state->defeated_log.entries[i].enemy, enemy_name) == 0){
            printf("%d\n", state->defeated_log.entries[i].count);
            return 1;
        }
    }

    printf("0\n");
    return 1;
}

int handle_exhausts_query(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc != 2) return 0;
    if(strcmp(tokens[0], "Exhausts") != 0) return 0;
    if(strcmp(tokens[1], "?")        != 0) return 0;

    if(state->exhaust_list.size == 0){
        printf("None\n");
        return 1;
    }

    char **sorted = malloc(state->exhaust_list.size * sizeof(char *));
    if(sorted == NULL){ fprintf(stderr, "Memory allocation failed\n"); exit(1); }

    for(int i = 0; i < state->exhaust_list.size; i++){
        sorted[i] = state->exhaust_list.names[i];
    }

    qsort(sorted, state->exhaust_list.size, sizeof(char *), compare_strings);

    for(int i = 0; i < state->exhaust_list.size; i++){
        if(i < state->exhaust_list.size - 1){
            printf("%s, ", sorted[i]);
        } else {
            printf("%s\n", sorted[i]);
        }
    }

    free(sorted);
    return 1;

}


int handle_relics_query(GameState *state, char *line){

    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc != 2) return 0;
    if(strcmp(tokens[0], "Relics") != 0) return 0;
    if(strcmp(tokens[1], "?")      != 0) return 0;

    if(state->relics.size == 0){
        printf("None\n");
        return 1;
    }

    char **sorted = malloc(state->relics.size * sizeof(char *));
    if(sorted == NULL){ fprintf(stderr, "Memory allocation failed\n"); exit(1); }

    for(int i = 0; i < state->relics.size; i++){
        sorted[i] = state->relics.names[i];
    }

    qsort(sorted, state->relics.size, sizeof(char *), compare_strings);

    for(int i = 0; i < state->relics.size; i++){
        if(i < state->relics.size - 1){
            printf("%s, ", sorted[i]);
        } else {
            printf("%s\n", sorted[i]);
        }
    }

    free(sorted);
    return 1;

}

int handle_potions_query(GameState *state, char *line){

    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc != 2) return 0;
    if(strcmp(tokens[0], "Potions") != 0) return 0;
    if(strcmp(tokens[1], "?")       != 0) return 0;

    if(state->potions.size == 0){
        printf("None\n");
        return 1;
    }
    
    /* build unique name + count pairs */
    PotionEntry entries[3];
    int unique = 0;

    for(int i = 0; i < state->potions.size; i++){
        int found = -1;
        for(int j = 0; j < unique; j++){
            if(strcmp(entries[j].name, state->potions.names[i]) == 0){
                found = j;
                break;
            }
        }
        if(found != -1){
            entries[found].count++;
        } else {
            entries[unique].name  = state->potions.names[i];
            entries[unique].count = 1;
            unique++;
        }
    }

    qsort(entries, unique, sizeof(PotionEntry), compare_potion_entries);

    for(int i = 0; i < unique; i++){
        if(i < unique - 1){
            printf("%d %s, ", entries[i].count, entries[i].name);
        } else {
            printf("%d %s\n", entries[i].count, entries[i].name);
        }
    }

    return 1;
}

int handle_effective_query(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    /* minimum: "What is effective against X ?" = 6 tokens */
    if(tc < 6) return 0;
    if(strcmp(tokens[0], "What")      != 0) return 0;
    if(strcmp(tokens[1], "is")        != 0) return 0;
    if(strcmp(tokens[2], "effective") != 0) return 0;
    if(strcmp(tokens[3], "against")   != 0) return 0;
    if(strcmp(tokens[tc-1], "?")      != 0) return 0;

    /* enemy name is tokens[4] to tokens[tc-2] */
    char enemy_name[512];
    join_tokens(enemy_name, tokens, 4, tc - 2);

    /* check double spaces */
    const char *name_start = find_token_start(line, 4);
    if(name_start == NULL) return 0;

    const char *q_in_original = strstr(name_start, " ?");
    if(q_in_original == NULL) return 0;

    int name_len = q_in_original - name_start;
    char name_portion[512];
    strncpy(name_portion, name_start, name_len);
    name_portion[name_len] = '\0';
    if(has_double_space(name_portion)) return 0;

    if(!is_valid_name(enemy_name)) return 0;

    /* look up codex */
    int enemy_idx = find_codex_enemy(&state->codex, enemy_name);
    if(enemy_idx == -1){
        printf("No codex data for %s\n", enemy_name);
        return 1;
    }

     CodexRecord *rec = &state->codex.records[enemy_idx];

    /* build sorted list of "type name" strings */
    char **entries = malloc(rec->entry_count * sizeof(char *));
    if(entries == NULL){ fprintf(stderr, "Memory allocation failed\n"); exit(1); }

    for(int i = 0; i < rec->entry_count; i++){
        /* format: "card Bash" or "relic Anchor" or "potion Fire Potion" */
        int len = strlen(rec->entries[i].type) + 1 + strlen(rec->entries[i].item) + 1;
        entries[i] = malloc(len * sizeof(char));
        if(entries[i] == NULL){ fprintf(stderr, "Memory allocation failed\n"); exit(1); }
        sprintf(entries[i], "%s %s", rec->entries[i].type, rec->entries[i].item);
    }

    qsort(entries, rec->entry_count, sizeof(char *), compare_strings);

    for(int i = 0; i < rec->entry_count; i++){
        if(i < rec->entry_count - 1){
            printf("%s, ", entries[i]);
        } else {
            printf("%s\n", entries[i]);
        }
        free(entries[i]);
    }

    free(entries);
    return 1;
}

typedef struct {
    char display_name[256];  /* "Bash", "Bash+", "Bash*", "Bash+*" */
    char base_name[256];     /* "Bash" for sorting */
    int  is_upgraded;        /* for secondary sort */
    int  count;
} DeckEntry;

static int compare_deck_entries(const void *a, const void *b){
    const DeckEntry *da = (const DeckEntry *)a;
    const DeckEntry *db = (const DeckEntry *)b;

    /* primary sort: base name lexicographic */
    int cmp = strcmp(da->base_name, db->base_name);
    if(cmp != 0) return cmp;

    /* secondary sort: base before upgraded */
    return da->is_upgraded - db->is_upgraded;
}

int handle_deck_query(GameState *state, char *line){

    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc != 2) return 0;
    if(strcmp(tokens[0], "Deck") != 0) return 0;
    if(strcmp(tokens[1], "?")    != 0) return 0;


     /* count total nonzero entries */
    int total = 0;
    for(int i = 0; i < state->deck.size; i++){
        if(state->deck.base[i]     > 0) total++;
        if(state->deck.upgraded[i] > 0) total++;
    }

    if(total == 0){
        printf("None\n");
        return 1;
    }

    /* build deck entries */
    DeckEntry *entries = malloc(total * sizeof(DeckEntry));
    if(entries == NULL){ fprintf(stderr, "Memory allocation failed\n"); exit(1); }

    int idx = 0;
    for(int i = 0; i < state->deck.size; i++){
        int is_exhaust = state->deck.exhaust[i];

        if(state->deck.base[i] > 0){
            strcpy(entries[idx].base_name, state->deck.names[i]);
            entries[idx].is_upgraded = 0;
            entries[idx].count       = state->deck.base[i];

            /* build display name */
            if(is_exhaust){
                sprintf(entries[idx].display_name, "%s*", state->deck.names[i]);
            } else {
                strcpy(entries[idx].display_name, state->deck.names[i]);
            }
            idx++;
        }
        if(state->deck.upgraded[i] > 0){
            strcpy(entries[idx].base_name, state->deck.names[i]);
            entries[idx].is_upgraded = 1;
            entries[idx].count       = state->deck.upgraded[i];

            /* build display name */
            if(is_exhaust){
                sprintf(entries[idx].display_name, "%s+*", state->deck.names[i]);
            } else {
                sprintf(entries[idx].display_name, "%s+", state->deck.names[i]);
            }
            idx++;
        }
    }
    qsort(entries, total, sizeof(DeckEntry), compare_deck_entries);

    for(int i = 0; i < total; i++){
        if(i < total - 1){
            printf("%d %s, ", entries[i].count, entries[i].display_name);
        } else {
            printf("%d %s\n", entries[i].count, entries[i].display_name);
        }
    }

    free(entries);
    return 1;

}