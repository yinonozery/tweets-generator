# tweets-generator (NLP)
The program analyzes tweets from a given text file and generates new tweets by sampling these words using a Markov Chain model. The next choice of word is determined on a probabilistic basis: the more often a word appears after another, the higher the probability that it will be selected in the tweets generator.

## Program Files
tweetsGenerator.c

## Usage
Compile:

```
gcc -Wall -Wextra -Wvla -std=c99 tweetsGenerator.c -o tweetsGenerator
```

Run:
```
./tweetsGenerator <random number> <num of sentences to generate> <file path> <words to read>

<random number> - seed for srand() function, every running of the program should be with different numbers for different results.
<num of sentences to generate> - how many sentences to generate.
<file path> - path to the text file with tweets. each tweet must end with a full stop (period).
<words to read> (optional) - how many words to read from the text file, the last word must end with a full stop.
```

## Output
```
Tweet {No.}: generated sentence
```

## Functions
```
int get_random_number (int max_number)
WordStruct *get_first_random_word (LinkList *dictionary)
WordStruct *get_next_random_word (WordStruct *word_struct_ptr)
int generate_sentence (LinkList *dictionary)
int add_word_to_probability_list (WordStruct *first_word, WordStruct *second_word)
WordStruct *createWord(char *token)
Node *previousWord(LinkList *dictionary, char *token)
void fill_dictionary (FILE *fp, int words_to_read, LinkList *dictionary)
void free_dictionary (LinkList *dictionary)
```
