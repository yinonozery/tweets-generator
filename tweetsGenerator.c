/*
 * Author: Yinon Ozery
 * tweets-generator (NLP)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORDS_IN_SENTENCE_GENERATION 20
#define MAX_SENTENCE_LENGTH 1000

typedef struct WordStruct {
    char *word;
    struct WordProbability *prob_list;
    int occurrences;
    int prob_list_size;
} WordStruct;

typedef struct WordProbability {
    struct WordStruct *word_struct_ptr;
    int occ_in_prob;
} WordProbability;

/************ LINKED LIST ************/
typedef struct Node {
    WordStruct *data;
    struct Node *next;
} Node;

typedef struct LinkList {
    Node *first;
    Node *last;
    int size;
} LinkList;

/**
 * Add data to new node at the end of the given link list.
 * @param link_list Link list to add data to
 * @param data pointer to dynamically allocated data
 * @return 0 on success, 1 otherwise
 */
int add(LinkList *link_list, WordStruct *data) {
    Node *new_node = malloc(sizeof(Node));
    if (new_node == NULL) {
        return 1;
    }
    *new_node = (Node) {data, NULL};

    if (link_list->first == NULL) {
        link_list->first = new_node;
        link_list->last = new_node;
    } else {
        link_list->last->next = new_node;
        link_list->last = new_node;
    }

    link_list->size++;
    return 0;
}
/*************************************/

/**
 * Get random number between 0 and max_number [0, max_number).
 * @param max_number
 * @return Random number
 */
int get_random_number(int max_number) {
    return rand() % max_number;
}

/**
 * Choose randomly the next word from the given dictionary, drawn uniformly.
 * The function won't return a word that end's in full stop '.' (Nekuda).
 * @param dictionary Dictionary to choose a word from
 * @return WordStruct of the chosen word
 */
WordStruct *get_first_random_word(LinkList *dictionary) {
    Node *curr = dictionary->first;
    int random = get_random_number(dictionary->size);
    for (int i = 0; i < random; i++)
        curr = curr->next;
    return curr->data;
}

/**
 * Choose randomly the next word. Depend on it's occurrence frequency
 * in word_struct_ptr->WordProbability.
 * @param word_struct_ptr WordStruct to choose from
 * @return WordStruct of the chosen word
 */
WordStruct *get_next_random_word(WordStruct *word_struct_ptr) {
    WordStruct *res = word_struct_ptr->prob_list[0].word_struct_ptr;

    int *prob_cumulative = (int *) malloc(sizeof(int) * word_struct_ptr->prob_list_size);
    if (prob_cumulative == NULL) {
        printf("Allocation failure: prob_cumulative (%lu bytes)", sizeof(int) * word_struct_ptr->prob_list_size);
        exit(EXIT_FAILURE);
    }

    if (word_struct_ptr->prob_list != NULL)
        prob_cumulative[0] = word_struct_ptr->prob_list[0].occ_in_prob;

    // Cumulative sum
    for (int i = 1; i < word_struct_ptr->prob_list_size; i++)
        prob_cumulative[i] = prob_cumulative[i - 1] + word_struct_ptr->prob_list[i].occ_in_prob;

    int random = get_random_number(prob_cumulative[word_struct_ptr->prob_list_size - 1]);
    for (int i = 0; i < word_struct_ptr->prob_list_size; i++)
        if (random < prob_cumulative[i]) {
            res = word_struct_ptr->prob_list[i].word_struct_ptr;
            break;
        }

    free(prob_cumulative);
    return res;
}

/**
 * Receive dictionary, generate and print to stdout random sentence out of it.
 * The sentence most have at least 2 words in it.
 * @param dictionary Dictionary to use
 * @return Amount of words in printed sentence
 */
int generate_sentence(LinkList *dictionary) {
    int wordsCount = 1;
    WordStruct *curr = get_first_random_word(dictionary);

    while (curr->word[strlen(curr->word) - 1] == '.')
        curr = get_first_random_word(dictionary);

    printf(" %s", curr->word);
    while (curr->word[strlen(curr->word) - 1] != '.' && wordsCount < MAX_WORDS_IN_SENTENCE_GENERATION) {
        curr = get_next_random_word(curr);
        printf(" %s", curr->word);
        wordsCount++;
    }
    printf("\n");
    return wordsCount;
}

/**
 * Gets 2 WordStructs. If second_word in first_word's prob_list,
 * update the existing probability value.
 * Otherwise, add the second word to the prob_list of the first word.
 * @param first_word
 * @param second_word
 * @return 0 if already in list, 1 otherwise.
 */
int add_word_to_probability_list(WordStruct *first_word,
                                 WordStruct *second_word) {
    // If first_word is the last word in the sentence
    if (first_word->word[strlen(first_word->word) - 1] == '.')
        return 1;

    // If second_word is the first word in first_word prob_list's
    if (first_word->prob_list_size == 0) {
        first_word->prob_list->word_struct_ptr = second_word;
        first_word->prob_list->occ_in_prob = 1;
        first_word->prob_list_size = 1;
        return 1;
    }

    for (int i = 0; i < first_word->prob_list_size; i++) {
        if (strcmp(first_word->prob_list[i].word_struct_ptr->word, second_word->word) == 0) {
            first_word->prob_list[i].occ_in_prob++;
            if (second_word->occurrences == 1) {
                free(second_word->prob_list);
                free(second_word->word);
                free(second_word);
            }
            return 0;
        }
    }
    first_word->prob_list = realloc(first_word->prob_list,
                                    sizeof(WordProbability) * (first_word->prob_list_size + 1));
    first_word->prob_list[first_word->prob_list_size].word_struct_ptr = second_word;
    first_word->prob_list[first_word->prob_list_size].occ_in_prob = 1;
    first_word->prob_list_size++;
    return 1;
}

/**
 * Create a WordStruct for the given word
 * If the word isn't at the end of the sentence -> initialize it's prob_list
 * @param token - a word
 * @return pointer to the new WordStruct
 */
WordStruct *createWord(WordStruct *ptr, char *token) {
    int wordSize = (int) strlen(token);
    if (token[wordSize - 1] == '\n')
        wordSize--;
    ptr->word = (char *) malloc(wordSize + 1);
    if (ptr->word == NULL) {
        printf("Allocation failure: word (%lu bytes)", strlen(token) + 1);
        exit(EXIT_FAILURE);
    }

    strncpy(ptr->word, token, strlen(token) + 1);
    ptr->occurrences = 1;
    ptr->prob_list_size = 0;

    if (token[strlen(token) - 1] != '.') {
        ptr->prob_list = (WordProbability *) malloc(sizeof(WordProbability));
        if (ptr->prob_list == NULL) {
            printf("Allocation failure: prob_list (%lu bytes)", sizeof(WordProbability));
            exit(EXIT_FAILURE);
        }
    } else
        ptr->prob_list = NULL;
    return ptr;
}

/**
 * Gets a dictionary of words and a word
 * This function search for the given word in the dictionary
 * @param dictionary
 * @param token
 * @return Node* of the word if exist in dictionary, otherwise NULL
 */
Node *previousWord(LinkList *dictionary, char *token) {
    Node *curr = dictionary->first;
    Node *res = NULL;
    while (curr != NULL) {
        if (strcmp(curr->data->word, token) == 0) {
            res = curr;
            break;
        }
        curr = curr->next;
    }
    return res;
}

/**
 * Read word from the given file. Add every unique word to the dictionary.
 * Also, at every iteration, update the prob_list of the previous word with
 * the value of the current word.
 * @param fp File pointer
 * @param words_to_read Number of words to read from file.
 *                      If value is bigger than the file's word count,
 *                      or if words_to_read == -1 than read entire file.
 * @param dictionary Empty dictionary to fill
 */
void fill_dictionary(FILE *fp, int words_to_read, LinkList *dictionary) {
    char sent[MAX_SENTENCE_LENGTH];
    Node *prev;
    int count = 0;

    while (fgets(sent, MAX_SENTENCE_LENGTH, fp) != NULL) {
        sent[strlen(sent) - 1] = '\0'; // Remove '\n' from sentence
        char *token = strtok(sent, " "); // Get first word in sentence

        // Insert first word in dictionary
        if (dictionary->size == 0) {
            WordStruct *ptr = (WordStruct *) malloc(sizeof(WordStruct));
            if (ptr == NULL) {
                printf("Allocation failure: WordStruct (%lu bytes)", sizeof(WordStruct));
                exit(EXIT_FAILURE);
            }
            ptr = createWord(ptr, token);
            add(dictionary, ptr);
            prev = dictionary->first;
            token = strtok(NULL, " ");
        }

        // While loop separate words in sentence
        while (token != NULL) {
            // Check words to read limit
            if (words_to_read != -1 && count >= words_to_read - 1)
                return;
            count++;

            // Search the word in the dictionary
            Node *curr = dictionary->first;
            while (curr != NULL) {
                if (strcmp(token, curr->data->word) == 0) {
                    // Word found in dictionary
                    curr->data->occurrences++;
                    add_word_to_probability_list(prev->data, curr->data);
                    goto next;
                }
                curr = curr->next;
            }

            // Create a word struct and insert it to last word prob_list's
            WordStruct *ptr = (WordStruct *) malloc(sizeof(WordStruct));
            if (ptr == NULL) {
                printf("Allocation failure: WordStruct (%lu bytes)", sizeof(WordStruct));
                exit(EXIT_FAILURE);
            }
            ptr = createWord(ptr, token);
            if (add_word_to_probability_list(prev->data, ptr))
                add(dictionary, ptr);
            next:
            prev = previousWord(dictionary, token);
            token = strtok(NULL, " ");
        }
    }
}

/**
 * Free the given dictionary and all of it's content from memory.
 * @param dictionary Dictionary to free
 */
void free_dictionary(LinkList *dictionary) {
    Node *curr = dictionary->first;
    while (curr != NULL) {
        free(curr->data->prob_list);
        free(curr->data->word);
        free(curr->data);
        Node *prev = curr;
        curr = curr->next;
        free(prev);
    }
    free(dictionary);
}

/**
 * @param argc
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 *             3) Path to file
 *             4) Optional - Number of words to read
 */
int main(int argc, char *argv[]) {
    // Check if all necessary arguments are exists
    if (argc < 4 || argc > 5) {
        printf("Usage: tweetsGenerator <seed> <number of sentences> <file path> <number of dic>(Optional)\n");
        exit(EXIT_FAILURE);
    }
    int words_to_read = -1;
    int sentences_to_generate = (int) strtol(argv[2], NULL, 10);

    // Generate random number, argv[1] = Seed, 1 as default
    unsigned int seed = strtol(argv[1], NULL, 10);
    if (strcmp(argv[1], "0") == 0)
        seed = 1;

    if (argc == 5)
        words_to_read = (int) strtol(argv[4], NULL, 10);

    // Open tweets text file
    FILE *fp = fopen(argv[3], "r");
    if (fp == NULL) {
        printf("Error: Open file failure, permission denied or wrong file path.");
        exit(EXIT_FAILURE);
    }

    // Create a dictionary
    LinkList *dictionary = (LinkList *) malloc(sizeof(LinkList));
    if (dictionary == NULL) {
        printf("Allocation failure: LinkList (%lu bytes)", sizeof(LinkList));
        exit(EXIT_FAILURE);
    }

    fill_dictionary(fp, words_to_read, dictionary);

    for (int i = 0; i < sentences_to_generate; ++i) {
        srand(seed - i);
        printf("Tweet %d:", i + 1);
        generate_sentence(dictionary);
    }

    free_dictionary(dictionary);
    fclose(fp);
    return 0;
}
