#include "interpreter.h"
#include "helpers.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "queries.h"

static int parse_buy_command(char *line, const char *type, char *name_out, char *price_out);
static int parse_sell_command(char *line, const char *type, char *name_out, char *price_out);
static int parse_sell_upgraded_command(char *line, char *name_out, char *price_out);

// 1. GAIN GOLD
// Try to match "Ironclad gains <positive_integer> gold"; return 1 if successful, 0 otherwise
int handle_gains_gold(GameState *state, char *line) {
    
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc != 4) return 0; // Must be exactly 4 tokens
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "gains") != 0) return 0;
    if(strcmp(tokens[3], "gold") != 0) return 0;
    
    if(!is_valid_positive_integer(tokens[2])) return 0;

    state->gold += atoi(tokens[2]);
    printf("Gold obtained\n");
    return 1;

}

//2.GAIN CARD
// Try to match "Ironclad gains card <card>"; return 1 if successful, 0 otherwise

//FIX : ADDED REALLOC IF DECK SIZE EXCEEDS CAPACITY
//FIX : CHECK IF THE CARD IS EXHAUST TAGGED WHEN ADDING TO DECK (exhaust tags are global)
//Also used previously implemented helper functions to make it more readable
int handle_gains_card(GameState *state,char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc < 4) return 0; // Must be at least 4 tokens
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "gains") != 0) return 0;
    if(strcmp(tokens[2], "card") != 0) return 0;

    // Extract the card name (join tokens from index 3 to end)
    char card_name[512];
    join_tokens(card_name, tokens, 3, tc - 1);

    const char *name_in_original = find_token_start(line, 3); // Find the start of the card name in the original line
    if(name_in_original == NULL) return 0; //Somehow
    
    if(has_double_space(name_in_original)) return 0; 

    if(!is_valid_name(card_name)) return 0;



    add_card_to_deck(state, card_name);
    printf("Card added: %s\n", card_name);

    return 1; 
}

//3. GAIN RELIC
// Try to match "Ironclad gains relic <relic>"; return 1 if successful, 0 otherwise
/*FIX: 
REALLOCATION IF RELIC SIZE EXCEEDS CAPACITY
Also used previously implemented helper functions to make it more readable
*/
int handle_gains_relic(GameState *state, char *line) {
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc < 4) return 0; // Must be at least 4 tokens
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "gains") != 0) return 0;
    if(strcmp(tokens[2], "relic") != 0) return 0;

    // Extract the relic name (join tokens from index 3 to end)
    char relic_name[512];
    join_tokens(relic_name, tokens, 3, tc - 1);
    const char *name_in_original = find_token_start(line, 3); // Find the start of the relic name in the original line
    if(name_in_original == NULL) return 0; //Somehow
    if(has_double_space(name_in_original)) return 0;

    if(!is_valid_name(relic_name)) return 0;

    for(int i = 0; i < state -> relics.size; i++){
        if(strcmp(state->relics.names[i], relic_name) == 0){
            printf("Already has relic: %s\n" , relic_name);
            return 1;
        }

    }

    add_relic(state, relic_name);
    printf("Relic obtained: %s\n", relic_name);
    return 1;
    
}

//4. GAIN POTION
// Try to match "Ironclad gains potion <potion>"; return 1 if successful, 0 otherwise
int handle_gains_potion(GameState *state, char *line) {
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc < 4) return 0; //Must be at least 4 tokens
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "gains") != 0) return 0;
    if(strcmp(tokens[2], "potion") != 0) return 0;

    char potion_name[512];
    join_tokens(potion_name, tokens, 3, tc - 1);
    const char *name_in_original = find_token_start(line, 3); // Find the start of the potion name in the original line
    if(name_in_original == NULL) return 0; //Somehow
    if(has_double_space(name_in_original)) return 0;

    if(!is_valid_name(potion_name)) return 0;

    if(state->potions.size >= 3){
        printf("Potion belt is full\n");
        return 1;
    }

    add_potion_to_belt(state, potion_name);
    printf("Potion obtained: %s\n", potion_name);
    return 1;

}
//5. SHOP TRANSACTIONS
/*
Three types of shop transactions: buy card, buy relic, buy potion
I'm going to implement three sub-functions for this purpose.
*/

int handle_buy_card(GameState *state, char *line){

    char name[512], price_str[64];

    if(!parse_buy_command(line, "card", name, price_str)) return 0; // Not a valid buy card command
    if(!is_valid_name(name)) return 0;
    if(!is_valid_positive_integer(price_str)) return 0;

    int price = atoi(price_str);
    if(state->gold < price){
        printf("Not enough gold\n");
        return 1;
    }

    state->gold -= price;
    add_card_to_deck(state, name); 
    printf("Card added: %s\n", name);
    return 1;

}

int handle_buy_relic(GameState *state, char *line){
    char name[512], price_str[64];

    if(!parse_buy_command(line, "relic", name, price_str)) return 0; // Not a valid buy relic command
    if(!is_valid_name(name)) return 0;
    if(!is_valid_positive_integer(price_str)) return 0;

    int price = atoi(price_str);
    if(state->gold < price){
        printf("Not enough gold\n");
        return 1;
    }

    // check duplicate BEFORE deducting gold
    for(int i = 0; i < state->relics.size; i++){
        if(strcmp(state->relics.names[i], name) == 0){
            printf("Already has relic: %s\n", name);
            return 1;  // gold is NOT deducted
        }
    }

    state->gold -= price;
    add_relic(state, name); 
    printf("Relic obtained: %s\n", name);
    return 1;
}

int handle_buy_potion(GameState *state, char *line){
    char name[512], price_str[64];

    if(!parse_buy_command(line, "potion", name, price_str)) return 0; // Not a valid buy potion command
    if(!is_valid_name(name)) return 0;
    if(!is_valid_positive_integer(price_str)) return 0;

    int price = atoi(price_str);
    if(state->gold < price){
        printf("Not enough gold\n");
        return 1;
    }

    // check belt BEFORE deducting gold
    if(state->potions.size >= 3){
        printf("Potion belt is full\n");
        return 1;  // gold is NOT deducted
    }

    state->gold -= price;
    add_potion_to_belt(state, name); 
    printf("Potion obtained: %s\n", name);
    return 1;
}

//6. REMOVE CARD
/*
Again, two types of remove card commands: remove from base deck, and remove from upgraded deck.
I'm going to implement two sub-functions for this purpose.
*/

int handle_remove_card(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc < 4) return 0; //Must be at least 4 tokens
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "removes") != 0) return 0;
    if(strcmp(tokens[2], "card") != 0) return 0;

    char card_name[512];
    join_tokens(card_name, tokens, 3, tc - 1);
    const char *name_in_original = find_token_start(line, 3); // Find the start of the card name in the original line
    if(name_in_original == NULL) return 0; //Somehow
    if(has_double_space(name_in_original)) return 0;
    if(!is_valid_name(card_name)) return 0;


    int found = -1;

    for(int i = 0; i < state->deck.size; i++){
        if(strcmp(state->deck.names[i], card_name) == 0){
            found = i;
            break;
        }
    }

    if(found != -1 && state->deck.base[found] > 0){
        state -> deck.base[found]--;
        printf("Card removed: %s\n", card_name);
    }else { 
        printf("Card not found: %s\n", card_name);
    }
    return 1;
}

int handle_remove_upgraded_card(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);
    if(tc < 5) return 0; //Must be at least 5 tokens
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "removes") != 0) return 0;
    if(strcmp(tokens[2], "upgraded") != 0) return 0;
    if(strcmp(tokens[3], "card") != 0) return 0;

    char card_name[512];
    join_tokens(card_name, tokens, 4, tc - 1);
    const char *name_in_original = find_token_start(line, 4); 
    if(name_in_original == NULL) return 0; //Somehow
    if(has_double_space(name_in_original)) return 0;
    if(!is_valid_name(card_name)) return 0;


    int found = -1;

    for(int i = 0; i < state->deck.size; i++){
        if(strcmp(state->deck.names[i], card_name) == 0){
            found = i;
            break;
        }
    }

    if(found != -1 && state->deck.upgraded[found] > 0){
        state -> deck.upgraded[found]--;
        printf("Upgraded card removed: %s\n", card_name);
    }else{
        printf("Upgraded card not found: %s\n", card_name);
    }
    return 1;
}

//7. UPGRADE CARD
int handle_upgrade_card(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);
    if(tc < 4) return 0;
    if(strcmp(tokens[0] , "Ironclad") != 0) return 0;
    if(strcmp(tokens[1] , "upgrades") != 0) return 0;
    if(strcmp(tokens[2] , "card") != 0) return 0;

    char card_name[512];
    join_tokens(card_name, tokens, 3, tc - 1);
    const char *name_in_original = find_token_start(line, 3); // Find the start of the card name in the original line
    if(name_in_original == NULL) return 0; //Somehow
    if(has_double_space(name_in_original)) return 0;
    if(!is_valid_name(card_name)) return 0;

    int found = -1;
    for(int i = 0; i < state->deck.size; i++){
        if(strcmp(state->deck.names[i], card_name) == 0){
            found = i;
            break;
        }
    }

    if(found != -1 && state->deck.base[found] > 0){
        state->deck.base[found]--;
        state->deck.upgraded[found]++;
        printf("Card upgraded: %s\n", card_name);
    } else {
        printf("Card not found: %s\n", card_name);
    }
    return 1;

}

//8. ENTER ROOM
int handle_enter_room(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc != 4) return 0;
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "enters") != 0) return 0;
    if(strcmp(tokens[3], "room") != 0) return 0;

    /* Validating the room type */
    const char *valid_rooms[] = {"Monster", "Elite", "Rest", "Shop" , "Treasure" , "Event" , "Boss", NULL};
    
    int valid = 0;
    for(int i = 0; valid_rooms[i] != NULL; i++){
        if(strcmp(tokens[2], valid_rooms[i]) == 0){
            valid = 1;
            break;
        }
    }

    if(!valid) return 0;
    
    state->floor++;
    strcpy(state->current_room, tokens[2]);
    
    printf("Entered %s room\n", tokens[2]);
    
    
    return 1;
}

//9. RECORD EFFECTIVENESS
int handle_learn_effective(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    /* minimum: "Ironclad learns card X is effective against Y" = 8 tokens */
    if(tc < 8) return 0;
    if(strcmp(tokens[0] , "Ironclad") != 0) return 0;
    if(strcmp(tokens[1] , "learns") != 0) return 0;

    //token[2] must be "card" , "potion" , or "relic"
    char type[16];
    if(strcmp(tokens[2] , "card") == 0) strcpy(type, "card");
    else if(strcmp(tokens[2] , "relic") == 0) strcpy(type,"relic");
    else if(strcmp(tokens[2] , "potion") == 0) strcpy(type,"potion");
    else return 0;

    /* find "is" token which marks the end of the item name */
    int is_idx = -1;
    for(int i = 3; i < tc ; i++){
        if(strcmp(tokens[i] , "is") == 0 ){
            is_idx = i;
            break;
        }
    }

    if(is_idx == -1) return 0;
    if(is_idx <= 3) return 0;

    /*after "is" must be "effective against <enemy>" */
    if(is_idx + 3 >= tc) return 0;
    if(strcmp(tokens[is_idx + 1] , "effective") != 0) return 0;
    if(strcmp(tokens[is_idx + 2] , "against") != 0) return 0;

    /* item name is tokens[3] to tokens[is_idx - 1] */
    char item_name[512];
    join_tokens(item_name, tokens, 3 , is_idx - 1);

    /* enemy name is tokens[is_idx + 3] to tokens[tc - 1] */
    char enemy_name[512];
    join_tokens(enemy_name, tokens, is_idx + 3, tc - 1);

    /* check double spaces in item name portion of the original line*/
    const char *item_start = find_token_start(line , 3);
    if(item_start == NULL) return 0;

    const char *is_in_original = strstr(item_start, " is ");
    if(is_in_original == NULL) return 0;

    int item_len = is_in_original - item_start;
    char item_portion[512];
    strncpy(item_portion, item_start, item_len);
    item_portion[item_len] = '\0';

    if(has_double_space(item_portion)) return 0;

    /*check double spaces in the enemy name portion*/
    const char *against_in_original = strstr(is_in_original, " against ");
    if(against_in_original == NULL) return 0;

    const char *enemy_start = against_in_original + strlen(" against ");
    if(has_double_space(enemy_start)) return 0;

    //validate both names
    if(!is_valid_name(item_name)) return 0;
    if(!is_valid_name(enemy_name)) return 0;

    //find the enemy in codex
    int enemy_idx = find_codex_enemy(&state->codex, enemy_name);

    if(enemy_idx == -1){
        // new enemy
        add_codex_enemy(&state->codex , enemy_name);
        enemy_idx = state -> codex.size - 1;
        add_codex_entry(&state->codex.records[enemy_idx], type, item_name);
        printf("Codex entry created: %s\n" , enemy_name);
    }else { 
        //enemy exists, check for duplicate
        if(codex_has_entry(&state->codex.records[enemy_idx] , type, item_name)){
            printf("Effectiveness already noted\n");
        }
        else{
            add_codex_entry(&state->codex.records[enemy_idx] , type, item_name);
            printf("Codex entry updated: %s\n",enemy_name);
        }
    }

    return 1;

}

//10. FIGHT ENEMY
/*
Handling both enemies with bounty and normal enemies in a single function.
Since they share the same first keywords it is eaiser to just make it into an if statement
*/
int handle_fight(GameState *state, char *line){
    char buf[1024];
    strncpy(buf,line,sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc < 3) return 0;
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1] , "fights") != 0) return 0;

    /* Distinguish regular vs bounty */
    int for_idx = -1;
    int bounty = 0;
    char bounty_str[64] = {0};

    /* search for "for <n> gold" at the end */
    if(tc >= 6){
        for(int i = tc - 1 ; i >= 2 ; i--){
            if(strcmp(tokens[i],"for") == 0){
                if(i + 2 == tc - 1 && strcmp(tokens[tc - 1] , "gold") == 0){
                    for_idx = i;
                    bounty = 1;
                    strcpy(bounty_str, tokens[i+1]);
                }
                break;
            }
        }
    }

    /*extract enemy name*/
    int enemy_end = bounty ? for_idx - 1 : tc - 1;
    if(enemy_end < 2) return 0; //empty name

    char enemy_name[512];
    join_tokens(enemy_name, tokens, 2, enemy_end);

    /* check if there are double spaces in the portion of the enemy name in the original line */
    const char *enemy_start_ptr = find_token_start(line,2);
    if(enemy_start_ptr == NULL) return 0;

    if(bounty){
        //enemy name ends at " for " in original line
        const char *for_in_original = strstr(enemy_start_ptr, " for ");
        if(for_in_original == NULL) return 0;

        int enemy_len = for_in_original - enemy_start_ptr;
        char enemy_portion[512];
        strncpy(enemy_portion, enemy_start_ptr, enemy_len);
        enemy_portion[enemy_len] = '\0';
        if(has_double_space(enemy_portion)) return 0;
    } else {
        //ends at name
        if(has_double_space(enemy_start_ptr)) return 0;
    }

    if(!is_valid_name(enemy_name)) return 0;

    //validate bounty amount if present
    if(bounty && !is_valid_positive_integer(bounty_str)) return 0;

    int enemy_idx = find_codex_enemy(&state->codex,enemy_name);
    int victory = 0;


    if(enemy_idx != -1){
        CodexRecord *rec = &state->codex.records[enemy_idx];
        for(int i = 0; i < rec->entry_count; i++){
            char *type = rec->entries[i].type;
            char *item = rec->entries[i].item;

            if(strcmp(type,"card") == 0){
                int k = find_deck_card(&state->deck, item);
                if(k != -1 && (state->deck.base[k] + state->deck.upgraded[k]) > 0){
                    victory = 1;
                    break;
                }
            }else if(strcmp(type,"relic") == 0){
                if(find_relic(&state->relics, item) != -1){
                    victory = 1;
                    break;
                }
            }else if(strcmp(type,"potion") == 0){
                if(find_potion(&state->potions,item) != -1){
                    victory = 1;
                    break;
                }
            }

        }
    }

    if(victory){
        //consume effective potions
        if(enemy_idx != -1){
            CodexRecord *rec = &state -> codex.records[enemy_idx];
            for(int i = 0; i < rec->entry_count; i++){
                if(strcmp(rec->entries[i].type, "potion") != 0) continue;
                char *item = rec->entries[i].item;

                //find and remove one copy from belt
                for(int j = 0 ; j < state->potions.size; j++){
                    if(strcmp(state->potions.names[j],item) == 0){
                        free(state->potions.names[j]);
                        //shift left
                        for(int k = j; k < state->potions.size - 1; k++){
                            state -> potions.names[k] = state->potions.names[k+1];
                        }

                        state->potions.size--;
                        break;
                    }
                }
            }
            //consume exhaust tagged cards now
            for(int i = 0; i < rec->entry_count; i++){
                if(strcmp(rec->entries[i].type , "card") != 0) continue;
                char *item = rec->entries[i].item;

                int k = find_deck_card(&state->deck,item);
                if(k==-1) continue;
                if(!state->deck.exhaust[k]) continue;

                //prefer base copy, then upgraded
                if(state->deck.base[k] > 0){
                    state->deck.base[k]--;
                }else if (state->deck.upgraded[k] > 0){
                    state->deck.upgraded[k]--;
                }
            }

        }

        //increment defeated count
        int def_idx = -1;
        for(int i = 0; i < state->defeated_log.size; i++){
            if(strcmp(state->defeated_log.entries[i].enemy, enemy_name) == 0){
                def_idx = i;
                break;
            }
        }
        if(def_idx == -1){
            add_defeated_enemy(&state->defeated_log, enemy_name);
            def_idx = state->defeated_log.size - 1;
        }
        state->defeated_log.entries[def_idx].count++;

        //add bounty gold if available
        if(bounty){
            state->gold += atoi(bounty_str);
            printf("Ironclad defeats %s and gains %d gold\n",
                   enemy_name, atoi(bounty_str));
        } else {
            printf("Ironclad defeats %s\n", enemy_name);
        }


    } else{
        //Defeat
        state->current_hp -= 15;
        if(state->current_hp < 0) state->current_hp = 0;
        printf("Ironclad is outmatched and flees with %d hp remaining\n", state->current_hp);
    }

    return 1;
}

//11. HEAL
int handle_heal(GameState *state, char *line){
    char buf[1024];
    strncpy(buf,line,sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf,tokens,128);

    if(tc != 4) return 0; //Must be exactly 4 tokens
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "heals") != 0) return 0;
    if(strcmp(tokens[3], "hp") != 0) return 0;

    if(!is_valid_positive_integer(tokens[2])) return 0;

    int amount = atoi(tokens[2]);
    state->current_hp += amount;

    if(state->current_hp > state -> max_hp){
        state->current_hp = state->max_hp;
    }
    printf("Ironclad heals to %d\n", state->current_hp);
    return 1;
}
//12. TAKE DAMAGE
int handle_take_damage(GameState *state, char *line){
    char buf[1024];
    strncpy(buf,line,sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc != 4) return 0;
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "takes") != 0) return 0;
    if(strcmp(tokens[3], "damage") != 0) return 0;

    if(!is_valid_positive_integer(tokens[2])) return 0;

    int amount = atoi(tokens[2]);
    state -> current_hp -= amount;
    if(state->current_hp < 0){
        state->current_hp = 0;
    }
    
    printf("Ironclad health drops to %d\n", state->current_hp);
    return 1;

}
//13. DISCARD POTION
int handle_discard_potion(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc < 4) return 0;
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "discards") != 0) return 0;
    if(strcmp(tokens[2], "potion") != 0) return 0;

    char potion_name[512];
    join_tokens(potion_name, tokens, 3, tc - 1);

    const char *name_in_original = find_token_start(line, 3); // Find the start of the potion name in the original line
    if(name_in_original == NULL) return 0; //Somehow
    if(has_double_space(name_in_original)) return 0;
    if(!is_valid_name(potion_name)) return 0;

    for(int i = 0; i < state->potions.size; i++){
        if(strcmp(state->potions.names[i], potion_name)==0){
            free(state->potions.names[i]);
            //shift remaining potions left
            for(int j = i; j < state->potions.size - 1; j++){
                state->potions.names[j] = state->potions.names[j+1];
            }
            state->potions.size--;
            printf("Potion discarded: %s\n", potion_name);
            return 1;
        }
    }

    printf("Potion not found: %s\n", potion_name);
    return 1; // Return 1 even if potion not found, since the command format is correct and we don't want to print INVALID in this case
}
//14. SELL ITEM
/*
Three types of sell commands: sell card, sell upgraded card, sell relic
I'm going to implement three sub-functions for this purpose.
*/

int handle_sell_card(GameState *state, char *line){
    char name[512], price_str[64];

    if(!parse_sell_command(line, "card", name, price_str)) return 0; //Not a valid sell command
    if(!is_valid_name(name)) return 0;
    if(!is_valid_positive_integer(price_str)) return 0;

    /* Find base copy */
    for(int i = 0; i < state->deck.size; i++){
        if(strcmp(state->deck.names[i],name) == 0 && state->deck.base[i] > 0){
            state -> deck.base[i]--;
            state->gold += atoi(price_str);
            printf("Card sold: %s\n", name);
            return 1;
        }
    }

    printf("Card not found: %s\n", name);
    return 1; 
}

int handle_sell_upgraded_card(GameState *state, char *line){
    char name[512], price_str[64];

    if(!parse_sell_upgraded_command(line, name, price_str)) return 0; 
    if(!is_valid_name(name)) return 0;
    if(!is_valid_positive_integer(price_str)) return 0;

    /* Find upgraded copy */
    for(int i = 0; i < state->deck.size; i++){
        if(strcmp(state->deck.names[i], name) == 0 && state -> deck.upgraded[i] > 0){
            state->deck.upgraded[i]--;
            state->gold += atoi(price_str);
            printf("Upgraded card sold: %s\n", name);
            return 1;
        }
    }

    printf("Upgraded card not found: %s\n", name);
    return 1; 
}

int handle_sell_potion(GameState *state, char *line){
    char name[512], price_str[64];

    if(!parse_sell_command(line, "potion", name, price_str)) return 0;
    if(!is_valid_name(name)) return 0;
    if(!is_valid_positive_integer(price_str)) return 0;

    /*Find and remove copy from the belt*/

    for(int i = 0; i < state->potions.size; i++){
        if(strcmp(state->potions.names[i], name) == 0){
            free(state->potions.names[i]);
            //shift remaining potions left
            for(int j = i; j < state->potions.size - 1; j++){
                state->potions.names[j] = state->potions.names[j+1];
            }
            state->potions.size--;
            state->gold += atoi(price_str);
            printf("Potion sold: %s\n", name);
            return 1;
        }
    }

    printf("Potion not found: %s\n", name);
    return 1;
}

//15. MAKE CARD AS EXHAUST
int handle_mark_exhaust(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);
    if(tc < 6) return 0;
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "marks") != 0) return 0;
    if(strcmp(tokens[2], "card") != 0) return 0;

    /* Last tokens must be "as" , "exhaust" */
    if(strcmp(tokens[tc - 2], "as") != 0) return 0;
    if(strcmp(tokens[tc - 1], "exhaust") != 0) return 0;

    /* name is tokens[3] to tokens[tc - 3] */
    if(tc - 3 < 3) return 0; // Empty name
    char card_name[512];
    join_tokens(card_name, tokens, 3, tc-3);

    /* Check double spaces in name portion of the original line */
    const char *name_start = find_token_start(line, 3);
    if(name_start == NULL) return 0; //Somehow

    const char *as_in_original = strstr(name_start, " as ");
    if(as_in_original == NULL) return 0; //Somehow

    int name_len = as_in_original - name_start;
    char name_portion[512];
    strncpy(name_portion, name_start, name_len);
    name_portion[name_len] = '\0';
    
    //Routine checks for the name portion
    if(has_double_space(name_portion)) return 0;
    if(!is_valid_name(card_name)) return 0;

    /* Check if already tagged as exhaust */
    if(is_exhaust_tagged(&state->exhaust_list, card_name)){
        printf("Card already exhausts: %s\n", card_name);
        return 1;
    }

    /* Add card to exhaust list */
    if(state->exhaust_list.size == state->exhaust_list.capacity){
        state->exhaust_list.capacity *= 2;
        char **tmp = realloc(state->exhaust_list.names,
                             state->exhaust_list.capacity * sizeof(char *));
        if(tmp == NULL){ fprintf(stderr, "Memory allocation failed\n"); exit(1); }
        state->exhaust_list.names = tmp;
    }

    char *dup = strdup(card_name);
    if(dup == NULL){ fprintf(stderr, "Memory allocation failed\n"); exit(1); }
    state->exhaust_list.names[state->exhaust_list.size++] = dup;

    /* also update exhaust flag in deck if card exists there */
    for(int i = 0; i < state->deck.size; i++){
        if(strcmp(state->deck.names[i], card_name) == 0){
            state->deck.exhaust[i] = 1;
            break;
        }
    }

    printf("Card marked as exhaust: %s\n", card_name);
    return 1;

}

//17. GAIN MAX HP
int handle_gain_max_hp(GameState *state, char *line){
    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc != 5) return 0; //Must be exactly 5 tokens
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "gains") != 0) return 0;
    if(strcmp(tokens[3], "max") != 0) return 0;
    if(strcmp(tokens[4], "hp") != 0) return 0;

    if(!is_valid_positive_integer(tokens[2])) return 0;

    int amount = atoi(tokens[2]);

    state->max_hp += amount;

    printf("Max health increased to %d\n", state->max_hp);
    return 1;
}


//A small helper for buy commands, to avoid repetition
//Returns 1 if the parsing was successful, 0 otherwise. If successful, the name and price are outputted to name_out and price_out respectively
static int parse_buy_command(char *line, const char *type, char *name_out, char *price_out){

    char buf[1024];
    strncpy(buf,line,sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    //int token_count = 0;
    int tc = tokenize(buf, tokens, 128);

    if(tc < 7) return 0; // Must be at least 7 tokens
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "buys") != 0) return 0;
    if(strcmp(tokens[2], type) != 0) return 0;

    int for_idx = -1;
    for(int i = 3; i < tc; i++){
        if(strcmp(tokens[i], "for") == 0){
            for_idx = i;
            break;
        }
    }

    if(for_idx == -1) return 0; // "for" not found
    if(for_idx + 2 != tc - 1) return 0; // Must be exactly two tokens after "for"
    if(strcmp(tokens[tc - 1], "gold") != 0) return 0; // Last token must be "gold"
    if(for_idx <= 3) return 0; // There must be at least one token for the name

    /*Check double spaces in the portion of the name*/
    const char *name_start = find_token_start(line , 3);
    if(name_start == NULL) return 0; //Somehow

    /*Find "for" in original line AFTER the name starts*/
    const char *for_in_original = strstr(name_start, " for ");
    if(for_in_original == NULL) return 0; //Somehow

    /*Now, extract the name portion and do the necessary checks*/
    int name_len = for_in_original - name_start;
    char name_portion[512];
    strncpy(name_portion, name_start, name_len);
    name_portion[name_len] = '\0';

    if(has_double_space(name_portion)) return 0;
    //if(!is_valid_name(name_portion)) return 0; redundant, the parent function handle_buy_<> already checks this, so i commented it out

    join_tokens(name_out, tokens, 3, for_idx - 1);
    strcpy(price_out, tokens[for_idx + 1]);
    return 1;

}

static int parse_sell_command(char *line, const char *type, char *name_out, char *price_out){
    /*
    Pretty similar to parse_buy_command helper but with only two types (card and potion)
    For upgraded card selling, I define a new helper parse_sell_upgraded_command since the keywords are different
    */

    char buf[1024];
    strncpy(buf,line,sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc < 7) return 0;
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "sells") != 0) return 0;
    if(strcmp(tokens[2], type) != 0) return 0;

    /* Find the "for" token */

    int for_idx = -1;
    for(int i = 3; i < tc; i++){
        if(strcmp(tokens[i], "for") == 0){
            for_idx = i;
            break;
        }
    }

    if(for_idx == -1) return 0; // "for" not found
    if(for_idx + 2 != tc - 1) return 0; // Must be exactly two tokens after "for"
    if(strcmp(tokens[tc - 1], "gold") != 0) return 0; // Last token must be "gold"
    if(for_idx <= 3) return 0; // There must be at least one token for the name

    /* Check double spaces in the name portion of the original line */
    const char *name_start = find_token_start(line, 3);
    if(name_start == NULL) return 0;    

    const char *for_in_original = strstr(name_start, " for ");
    if(for_in_original == NULL) return 0;   

    int name_len = for_in_original - name_start;
    char name_portion[512];
    strncpy(name_portion, name_start, name_len);
    name_portion[name_len] = '\0';
    if(has_double_space(name_portion)) return 0;

    join_tokens(name_out, tokens, 3, for_idx - 1);
    strcpy(price_out, tokens[for_idx + 1]);
    return 1;


}

static int parse_sell_upgraded_command(char *line, char *name_out, char *price_out){
    //Only made to handle the sell upgraded card command

    char buf[1024];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tokens[128];
    int tc = tokenize(buf, tokens, 128);

    if(tc < 8) return 0;
    if(strcmp(tokens[0], "Ironclad") != 0) return 0;
    if(strcmp(tokens[1], "sells") != 0) return 0;
    if(strcmp(tokens[2], "upgraded") != 0) return 0;
    if(strcmp(tokens[3], "card") != 0) return 0;

    /* Find the "for" token again */
    int for_idx = -1;
    for(int i = 4; i < tc; i++){
        if(strcmp(tokens[i], "for") == 0){
            for_idx = i;
            break;
        }
    }

    if(for_idx == -1) return 0;
    if(for_idx + 2 != tc - 1) return 0;
    if(strcmp(tokens[tc - 1], "gold") != 0) return 0;
    if(for_idx <= 4) return 0;

    /* check double spaces */ 
    const char *name_start = find_token_start(line , 4);
    if(name_start == NULL) return 0;

    const char *for_in_original = strstr(name_start, " for ");
    if(for_in_original == NULL) return 0;

    int name_len = for_in_original - name_start;
    char name_portion[512];
    strncpy(name_portion, name_start, name_len);
    name_portion[name_len] = '\0';

    if(has_double_space(name_portion)) return 0;
    
    join_tokens(name_out, tokens, 4, for_idx - 1);
    strcpy(price_out, tokens[for_idx + 1]);
    return 1;
}



// 3. THE SWITCHBOARD
int execute_line(GameState *state, char *line) {
    // We try each function one by one.
    // If one returns 1, we stop and tell main.c we succeeded.
    if (handle_gain_max_hp(state, line)) return 1; //Put this one first because of the few first same keywords with the following few commands
    if (handle_gains_gold(state, line)) return 1;
    if (handle_gains_card(state, line)) return 1;
    if (handle_gains_relic(state, line)) return 1;
    if (handle_gains_potion(state, line)) return 1;
    if (handle_buy_card(state, line)) return 1;
    if (handle_buy_relic(state, line)) return 1;
    if (handle_buy_potion(state, line)) return 1;
    if (handle_remove_upgraded_card(state, line)) return 1;
    if (handle_remove_card(state, line)) return 1;
    if (handle_upgrade_card(state, line)) return 1;
    if (handle_heal(state, line)) return 1;
    if (handle_take_damage(state, line)) return 1;
    if (handle_enter_room(state, line)) return 1;
    if (handle_discard_potion(state, line)) return 1;
    if (handle_mark_exhaust(state, line)) return 1;
    if (handle_sell_upgraded_card(state, line)) return 1;  // before sell_card
    if (handle_sell_card(state, line)) return 1;
    if (handle_sell_potion(state, line)) return 1;
    if (handle_learn_effective(state,line)) return 1;
    if (handle_fight(state,line)) return 1;
    if (handle_gold_query(state, line))     return 1;
    if (handle_floor_query(state, line))    return 1;
    if (handle_where_query(state, line))    return 1;
    if (handle_health_query(state, line))   return 1;
    if (handle_deck_size_query(state, line)) return 1;
    if (handle_total_card_query(state,line)) return 1;
    if (handle_total_upgraded_card_query(state,line)) return 1;
    if (handle_defeated_query(state, line)) return 1;
    if (handle_exhausts_query(state,line)) return 1;
    if (handle_relics_query(state,line)) return 1;
    if (handle_potions_query(state,line)) return 1;
    if (handle_effective_query(state,line)) return 1;
    if (handle_deck_query(state,line)) return 1;


    return 0; // If no function matched, return 0 (main.c will print INVALID)
}
