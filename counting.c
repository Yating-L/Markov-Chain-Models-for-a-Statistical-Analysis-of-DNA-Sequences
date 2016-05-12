#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <ctype.h>
#include <math.h>

#define NHASH 263167 //Use a prime number!

#define MULT 31


#define MAX_WORD_LENGTH		15
#define MAX_NUMBER_OF_SEQUENCES 10
#define MAX_FILENAME_LENGTH	255
//#define MAX_FILE_LENGTH 	500000
#define MAX_FILE_LENGTH 	5598450
#define ALPHA_NUM 26

#define max(a,b) (((a) > (b)) ? (a) : (b))

struct node
{
    char *word;
    int total_count;
    int sequence_count;
    int last_matching_sequence_id;
    struct node * next;
} node;

typedef enum {
    ASCENDING_TOTAL,
    ASCENDING_SEQUENCE,
    DESCENDING_TOTAL,
    DESCENDING_SEQUENCE,
    ASCENDING_WORDLENGTH,
    DESCENDING_WORDLENGTH,
    ASCENDING_OE,
    DESCENDING_OE,
    ASCENDING_Z_score,
    DESCENDING_Z_score
} sort_order_t;

sort_order_t sort_order = DESCENDING_TOTAL;
typedef struct node Node;
unsigned long total_number_words = 0;
unsigned long sequence_len = 0;  //total sequence length of all input books
int number_of_sequences = 0;
int letter_prob[ALPHA_NUM] = {0};   //record the p(a), p(b)...
int MODEL = 0;
int MARCOV_ORDER = 1;
Node *count[NHASH];

Node *bin[NHASH];
Node** bin_array;

char input_sequences[MAX_NUMBER_OF_SEQUENCES][MAX_FILE_LENGTH];
unsigned long sequence_lengths[MAX_NUMBER_OF_SEQUENCES];



unsigned int hash(char *p)
{
    unsigned int h = 0;
    for(; *p; p++)
        h = MULT * h + *p;
    return h % NHASH;
}

void incword(char *s, int sequence_id, Node* bin_ptr[NHASH])
{
   // printf("incword\n");
    Node * p;
    int h = hash(s);
    for(p = bin_ptr[h]; p!= NULL; p = p->next)
    {
        if(strcmp(s, p->word) == 0)
        {
            p->total_count++;
            
            if(sequence_id != p->last_matching_sequence_id)
            {
                p->sequence_count++;
                p->last_matching_sequence_id = sequence_id;
            }
            
            return;
        }
    }
    
    p = (Node *)malloc(sizeof(node));
    if(!p)
        return;
    p->total_count = 1;
    p->sequence_count = 1;
    p->last_matching_sequence_id = sequence_id;
    p->word = (char *)malloc(strlen(s)+1);
    total_number_words++;
    strcpy(p->word, s);
    p->next = bin_ptr[h];
    bin_ptr[h] = p;
}

//Count of transition matrix
void transition_matrix() {
   // printf("transition matrix, sequence length = %lu", sequence_lengths[0]);
    int i, j;
    char buff[MAX_WORD_LENGTH+1];
    for (MODEL = 1; MODEL <= 4; MODEL++) {
        
        for( i=0; i < number_of_sequences; i++)
        {
            
            for( j=0; j < max( 0, (signed int)(sequence_lengths[i]-(MODEL+1)+1)); j++)
            {
                
                strncpy(buff, &input_sequences[i][j], MODEL+1);
                buff[MODEL+1] = '\0';
               // printf("%s  ", buff);
                incword(buff, i, count);
            }
        }
//        Node* p;
//        int i;
//        unsigned long nwords = 0;
//        model_word_count[MODEL-1] = total_number_words;
//        trans_mat[MODEL-1] = (Node**)malloc(total_number_words * (sizeof(Node*)));
//
//            for(i = 0; i < NHASH; i++)
//            {
//                for(p = count[i]; p!= NULL; p = p->next)
//                {
//                    (*trans_mat[MODEL-1])[nwords++] = p;
//                }
//                
//            }
//            
        
    }
}

long double expect_prob(char* word) {
  //  printf("expect prob\n");
    int wordlength = strlen(word);
    char buf1[MAX_WORD_LENGTH+1], buf2[MAX_WORD_LENGTH+1];
    Node* p, *q;
    int found = 0;
    long double prob = 1;
  //  prob = prob * (long double)letter_prob[word[0] - 97]/(long double)sequence_len;

    
    if (MARCOV_ORDER == 1)
        prob = prob * (long double)letter_prob[word[0] - 97]/(long double)sequence_len;
   
    else {
        strncpy(buf2, &word[0], MARCOV_ORDER);
        buf2[MARCOV_ORDER] = '\0';
        int t = hash(buf2);
        for (q = count[t]; q != NULL; q = q->next) {
            if (strcmp(buf2, q->word) == 0) {
                //printf("start:%s ",buf2);
                prob = prob * (long double)q->total_count / (long double)(sequence_len-MARCOV_ORDER+1);
                break;
            }
        }
    }
    
    for (int i = 0; i < wordlength-(MARCOV_ORDER+1)+1; i++) {
        strncpy(buf1, &word[i], MARCOV_ORDER+1);
        buf1[MARCOV_ORDER+1] = '\0';
        int h = hash(buf1);
        for (p = count[h]; p != NULL; p = p->next) {
            if (strcmp(buf1, p->word) == 0) {
                if (MARCOV_ORDER == 1)
                    prob = prob * (long double)p->total_count / (long double)letter_prob[word[i] - 97];
                else {
                    strncpy(buf2, &word[i], MARCOV_ORDER);
                     buf2[MARCOV_ORDER] = '\0';
                    int t = hash(buf2);
                    for (q = count[t]; q != NULL; q = q->next) {
                        if (strcmp(buf2, q->word) == 0) {
                            prob = prob * (long double)p->total_count / (long double)q->total_count;
                            break;
                        }
                    }
                }
                found = 1;
                break;
            }
        }
        if (!found) return 0;
        
        
        //printf("----%Lf\t",prob);
    }
    
    return prob;
    
}

int overlap(char* word, int dist) {
    int wordlength = strlen(word);
    int i = wordlength - dist;
    int j = 0;
    while (j < dist) {
        if (word[i] != word[j]) {
            return 0;
        }
        j++;
        i++;
    }
    return 1;
}

long double overlap_exp(char* word, int dist) {   //u(WdW)
   // printf("caculating overlap_exp \n");
    int wordlength = strlen(word);
    char buf1[MAX_WORD_LENGTH+1], buf2[MAX_WORD_LENGTH+1];
    Node* p, *q;
    int found = 0;
    long double prob = expect_prob(word);
    for (int i = 0; i < wordlength-(MARCOV_ORDER+1)+1; i++) {
        strncpy(buf1, &word[i], MARCOV_ORDER+1);
        buf1[MARCOV_ORDER+1] = '\0';
        int h = hash(buf1);
        for (p = count[h]; p != NULL; p = p->next) {
            if (strcmp(buf1, p->word) == 0) {
                if (MARCOV_ORDER == 1)
                    prob = prob * (long double)p->total_count / (long double)letter_prob[word[i] - 97];
                else {
                    strncpy(buf2, &word[i], MARCOV_ORDER);
                    buf2[MARCOV_ORDER] = '\0';
                    int t = hash(buf2);
                    for (q = count[t]; q != NULL; q = q->next) {
                        if (strcmp(buf2, q->word) == 0) {
                            prob = prob * (long double)p->total_count / (long double)q->total_count;
                            break;
                        }
                    }
                }
                found = 1;
                break;
            }
        }
        if (!found) return 0;
        
        
        //printf("----%Lf\t",prob);
    }
    return prob;
}

long double self_count_sum(char* word) {
   // printf("caculating self_count_sum \n");
    int wordlength = strlen(word);
    long double sum = 0, sum_1 = 0, sum_2 = 0;
    Node *self_count[NHASH] = {};
    Node *self_count_2[NHASH] = {};
    char s[MAX_WORD_LENGTH+1];
    char s2[MAX_WORD_LENGTH+1];
    int found = 0;
    Node * p;
    /* cacluate self count for len = m */
    for (int i = 0; i < wordlength - MARCOV_ORDER + 1; i++) {
        strncpy(s, &word[i], MARCOV_ORDER);
        s[MARCOV_ORDER] = '\0';
        int h = hash(s);
        for(p = self_count[h]; p!= NULL; p = p->next)
        {
            if(strcmp(s, p->word) == 0)
            {
                p->total_count++;
                found = 1;
                break;
            }
        }
        if (found == 0) {
            p = (Node *)malloc(sizeof(node));
            if(!p)
                return -1;
            p->total_count = 1;
            p->word = (char *)malloc(strlen(s)+1);
            strcpy(p->word, s);
            p->next = self_count[h];
            self_count[h] = p;
        }
       // printf("insert %s\n", s);
        found = 0;
    }
    /* cacluate the sum with len m */
    int wm = 0;
    strncpy(s2, &word[0], MARCOV_ORDER);
    s2[MARCOV_ORDER] = '\0';
    int h = hash(s2);
    for(p = self_count[h]; p!= NULL; p = p->next)
    {
        if(strcmp(s2, p->word) == 0)
        {
            wm = p->total_count;
            break;
        }
    }
   // printf("wm = %d\n", wm);
    
    for(int i = 0; i < NHASH; i++)
    {
        for(p = self_count[i]; p!= NULL; p = p->next)
        {
          //  printf("word = %s  i = %d ", p->word, i);
            sum_1 += (p->total_count) * (p->total_count) / expect_prob(p->word);
        }
        
        
    }
    /* cacluate self count for len = m+1 */
    for (int i = 0; i < wordlength - MARCOV_ORDER; i++) {
        strncpy(s, &word[i], MARCOV_ORDER+1);
        s[MARCOV_ORDER+1] = '\0';
        int h = hash(s);
        for(p = self_count_2[h]; p!= NULL; p = p->next)
        {
            if(strcmp(s, p->word) == 0)
            {
                p->total_count++;
                found = 1;
                break;
            }
        }
        if (found == 0) {
            p = (Node *)malloc(sizeof(node));
            if(!p)
                return -1;
            p->total_count = 1;
            p->word = (char *)malloc(strlen(s)+1);
            strcpy(p->word, s);
            p->next = self_count_2[h];
            self_count_2[h] = p;
        }
        found = 0;
    }
    
    /* cacluate the sum with len m+1 */
   // printf("m+1\n");
    for(int i = 0; i < NHASH; i++)
    {
        for(p = self_count_2[i]; p!= NULL; p = p->next)
        {
           // printf("word = %s  count = %d \n", p->word, p->total_count);
            sum_2 += (p->total_count) * (p->total_count) / expect_prob(p->word);
           // printf("exp = %Lf\n", expect_prob(p->word));
        }
        
    }
    
    sum = sum_1 - sum_2 + (1 - 2 * wm)/ expect_prob(s2);
   // printf("%s selfsum = %Lf\n", word, sum);
    //printf("sum1 = %Lf, sum2 = %Lf, wm = %d, expect = %Lf\n", sum_1, sum_2, wm, expect_prob(s2));
    
    return sum;
}

long double variation(char* word) {
   // printf("caculating variation \n");
   // printf("%s\n", word);
    int wordlength = strlen(word);
    long double var = 0;
    long double overlap_value = 0;
    for (int d = 1; d < wordlength-MARCOV_ORDER-1; d++) {
        if (overlap(word, d))
            overlap_value += overlap_exp(word, d);
    }
    long double word_exp = expect_prob(word);
    long double cor_var = 2 * overlap_value + word_exp * word_exp * self_count_sum(word);
    var = (word_exp + cor_var) * (sequence_len - wordlength + 1);
    //printf("sequence_len = %lu\n", sequence_len);
    //printf("wordlenghth = %d\n", wordlength);
    //printf("%s corvar = %Lf", word, cor_var);
    return var;
    
}


int compare( const void* ptr1, const void* ptr2)
{
    Node** np1, *(*np2);
    np1 = (Node**) ptr1;
    np2 = (Node**) ptr2;
    long double expect1,expect2, var1, var2,z_score1,z_score2;
    int word_length1, word_length2;
    
    
    switch(sort_order)
    {
        case ASCENDING_TOTAL:
            if( (*np1)->total_count < (*np2)->total_count )
                return -1;
            if( (*np1)->total_count == (*np2)->total_count )
                sort_order = ASCENDING_WORDLENGTH;
            if( (*np1)->total_count > (*np2)->total_count )
                return 1;
            break;
            
        case ASCENDING_SEQUENCE:
            if( (*np1)->sequence_count < (*np2)->sequence_count )
                return -1;
            if( (*np1)->sequence_count == (*np2)->sequence_count )
                return 0;
            if( (*np1)->sequence_count > (*np2)->sequence_count )
                return 1;
            break;
            
        case DESCENDING_TOTAL:
            if( (*np1)->total_count > (*np2)->total_count )
                return -1;
            if( (*np1)->total_count == (*np2)->total_count )
                return 0;
            if( (*np1)->total_count < (*np2)->total_count )
                return 1;
            break;
            
        case DESCENDING_SEQUENCE:
            if( (*np1)->sequence_count > (*np2)->sequence_count )
                return -1;
            if( (*np1)->sequence_count == (*np2)->sequence_count )
                return 0;
            if( (*np1)->sequence_count < (*np2)->sequence_count )
                return 1;
            break;
            
        case ASCENDING_WORDLENGTH:
            if( strlen( (*np1)->word ) < strlen( (*np2)->word ))
                return -1;
            if( strlen( (*np1)->word ) == strlen( (*np2)->word ))
                return 0;
            if( strlen( (*np1)->word ) > strlen( (*np2)->word ))
                return 1;
            break;
            
        case DESCENDING_WORDLENGTH:
            if( strlen( (*np1)->word ) > strlen( (*np2)->word ))
                return -1;
            if( strlen( (*np1)->word ) == strlen( (*np2)->word ))
                return 0;
            if( strlen( (*np1)->word ) < strlen( (*np2)->word ))
                return 1;
            break;
        case ASCENDING_OE:
            word_length1 = strlen((*np1)->word);
            word_length2 = strlen((*np2)->word);
            expect1 = (*np1)->total_count / (expect_prob((*np1)->word) *(long double)(sequence_len - (word_length1 -1)*number_of_sequences));
            expect2 = (*np2)->total_count / (expect_prob((*np2)->word) *(long double)(sequence_len - (word_length2 -1)*number_of_sequences));
            if (expect1 < expect2)
                return -1;
            if (expect1 > expect2)
                return 1;
            if (expect1 == expect2)
                return 0;
            break;
        case DESCENDING_OE:
            word_length1 = strlen((*np1)->word);
            word_length2 = strlen((*np2)->word);
            expect1 = (*np1)->total_count / (expect_prob((*np1)->word)*(long double)(sequence_len - (word_length1 -1)*number_of_sequences));
            expect2 = (*np2)->total_count / (expect_prob((*np2)->word)*(long double)(sequence_len - (word_length2 -1)*number_of_sequences));
            if (expect1 < expect2)
                return 1;
            if (expect1 > expect2)
                return -1;
            if (expect1 == expect2)
                return 0;
            break;
        case ASCENDING_Z_score:
            var1 = variation((*np1)->word);
            var2 = variation((*np2)->word);
            z_score1 = ((*np1)->total_count - (expect_prob((*np1)->word)*(long double)(sequence_len - (word_length1 -1)*number_of_sequences))) / sqrt(var1);
            z_score2 = ((*np2)->total_count - (expect_prob((*np2)->word)*(long double)(sequence_len - (word_length2 -1)*number_of_sequences))) / sqrt(var2);
            if (z_score1 < z_score2)
                return -1;
            if (z_score1 > z_score2)
                return 1;
            if (z_score1 == z_score2)
                return 0;
            break;
        case DESCENDING_Z_score:
            var1 = variation((*np1)->word);
            var2 = variation((*np2)->word);
            z_score1 = ((*np1)->total_count - (expect_prob((*np1)->word)*(long double)(sequence_len - (word_length1 -1)*number_of_sequences))) / sqrt(var1);
            z_score2 = ((*np2)->total_count - (expect_prob((*np2)->word)*(long double)(sequence_len - (word_length2 -1)*number_of_sequences))) / sqrt(var2);
            if (z_score1 < z_score2)
                return 1;
            if (z_score1 > z_score2)
                return -1;
            if (z_score1 == z_score2)
                return 0;
            break;

        default:
            return 0;
            break;
    }
}

void sort_hash()
{
    Node* p;
    int i;
    unsigned long nwords = 0;
    
    bin_array = (Node**)malloc(total_number_words * (sizeof(Node*)));
   // printf("total num words = %lu\n", total_number_words);
    for(i = 0; i < NHASH; i++)
    {
        for(p = bin[i]; p!= NULL; p = p->next)
        {
           // printf("bin: %s  ", bin[i]->word);
            bin_array[nwords++] = p;
        }
        
    }
    
    qsort(&bin_array[0], total_number_words, sizeof(Node*), compare);
    
}



void print_cmdline_error() {
    fprintf(stderr, "Error: improper command line arguments\n");
    fprintf(stderr, "Usage: word_count -min X -max Y [-descseq|-ascseq|-desctot|-asctot|-desclength|-asclength|-ascoe|-desoe|-asczscore|-deszscore] filename [other filenames]\n");
    fprintf(stderr, "-min minimum word length\n");
    fprintf(stderr, "-max maximum word length\n");
    fprintf(stderr, "-descseq causes the output to be sorted in descending order of sequence counts\n");
    fprintf(stderr, "-ascseq  causes the output to be sorted in ascending order of sequence counts\n");
    fprintf(stderr, "-desctot causes the output to be sorted in descending order of total counts\n");
    fprintf(stderr, "-asctot  causes the output to be sorted in ascending order of total counts\n");
    fprintf(stderr, "-desclength causes the output to be sorted in descending order of word length\n");
    fprintf(stderr, "-asclength  causes the output to be sorted in ascending order of word length\n");
    fprintf(stderr, "-ascoe  causes the output to be sorted in ascending order of O/E\n");
    fprintf(stderr, "-desoe  causes the output to be sorted in ascending order of O/E\n");
    fprintf(stderr, "-ascvar  causes the output to be sorted in ascending order of Z-score\n");
    fprintf(stderr, "-desvar  causes the output to be sorted in ascending order of Z-score\n");
    fprintf(stderr, "-order  pick the marcov order 1, 2 or 3\n");
    fprintf(stderr, "Example: word_count -min 3 -max 10 -asctot -order 2 filename1 filename2 filname3\n");
    exit(1);
}


unsigned long open_copy(char* filename, char* cbuf)
{
    FILE *fp;
    fp = fopen(filename, "r");
    char next_char;
    unsigned long sequence_length = 0;
    char* p;
    
    if( fp == NULL)
    {
        fprintf(stderr, "Error: unable to open file %s\n", filename);
        exit(1);
    }
    
    while( fgets(cbuf, MAX_FILE_LENGTH, fp) == NULL );
    
    // remove non alphabetic char
    int i;
    p = cbuf;
    
    for (i = 0; cbuf[i] != '\0'; i++) {
        if (isalpha(cbuf[i]) && (cbuf[i] == 'A' || cbuf[i] == 'C' || cbuf[i] == 'G' || cbuf[i] == 'T')) {
            *p = tolower(cbuf[i]);
            letter_prob[*p - 97]++;
            p++;
        }
    }
    *p = '\0';
    
    fclose(fp) ;
    return strlen(cbuf);
    
}


int main(int argc, char *argv[])
{
    
    int min_wordlength = 0;
    int max_wordlength = 0;
    
    int i,k;
    signed int j;
    
    if (argc == 1) {
        print_cmdline_error();
    }
    
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-min") == 0) {  // input file
            ++i;
            min_wordlength = atoi(argv[i]);
            
        } else if (strcmp(argv[i], "-asctot") == 0) {
            sort_order = ASCENDING_TOTAL;
        } else if (strcmp(argv[i], "-ascseq") == 0) {
            sort_order = ASCENDING_SEQUENCE;
        } else if (strcmp(argv[i], "-desctot") == 0) {
            sort_order = DESCENDING_TOTAL;
        } else if (strcmp(argv[i], "-descseq") == 0) {
            sort_order = DESCENDING_SEQUENCE;
        } else if (strcmp(argv[i], "-asclength") == 0) {
            sort_order = ASCENDING_WORDLENGTH;
        } else if (strcmp(argv[i], "-desclength") == 0) {
            sort_order = DESCENDING_WORDLENGTH;
        } else if (strcmp(argv[i], "-ascoe") == 0) {
            sort_order = ASCENDING_OE;
        } else if (strcmp(argv[i], "-desoe") == 0) {
            sort_order = DESCENDING_OE;
        } else if (strcmp(argv[i], "-asczscore") == 0) {
            sort_order = ASCENDING_Z_score;
        } else if (strcmp(argv[i], "-deszscore") == 0) {
            sort_order = DESCENDING_Z_score;
        } else if (strcmp(argv[i], "-max") == 0) {
            ++i;
            max_wordlength = atoi(argv[i]);
        } else if (strcmp(argv[i], "-order") == 0) {
            ++i;
            MARCOV_ORDER = atoi(argv[i]);
        } else {
            sequence_lengths[number_of_sequences] = open_copy(argv[i], &input_sequences[number_of_sequences][0]);
            printf("seq len = %lu\n", sequence_lengths[0]);
            ++number_of_sequences;
        }
    }
    
    
    if (MARCOV_ORDER != 1 && MARCOV_ORDER != 2 && MARCOV_ORDER != 3 && MARCOV_ORDER != 4) {
        printf("%d\n", MARCOV_ORDER);
        fprintf(stderr, "Error: need to clarify which marcov model, input 1, 2, 3 or 4\n");
        exit(1);
    }
    
    if( min_wordlength == 0 || max_wordlength == 0 || min_wordlength > max_wordlength )
    {
        fprintf(stderr, "Error: incorrect or missing minimum or maximum word length\n");
        exit(1);
    }
    
    if( number_of_sequences == 0)
    {
        fprintf(stderr, "Error: no input file names specified\n");
        exit(1);
    }
    
    fprintf(stderr,"min wordlength = %d\n", min_wordlength);
    fprintf(stderr,"max wordlength = %d\n", max_wordlength);
    fprintf(stderr,"number of input files = %d\n", number_of_sequences);
    fprintf(stderr,"Marcov Order = %d\n", MARCOV_ORDER);

    
    
    
    for (i = 0; i < number_of_sequences; i++)
        sequence_len += sequence_lengths[i];
   
//calcuate word count with length 2, 3, 4 and 5
    transition_matrix();
    total_number_words = 0;
    // Enumeration
char buf[MAX_WORD_LENGTH+1];
    int wl;
    
    for( i=0; i < number_of_sequences; i++)
    {
        for( wl = min_wordlength; wl < max_wordlength+1; wl++)
        {
            for( j=0; j < max( 0, (signed int)(sequence_lengths[i]-wl+1)); j++)
            {
                strncpy(buf, &input_sequences[i][j], wl);
                buf[wl] = '\0';
                //printf("%s", buf);
                incword(buf, i, bin);
            }
        }
    }
    
    // Sort the output
    sort_hash();
    
    printf("total number of words = %ld\n", total_number_words);
    int wordlen = 0;
    //for(i = 0; i < total_number_words; i++)
    for(i = 0; i < 50; i++)
    {
        if( bin_array[i] == NULL )
        {
            exit(1);
        }
        
        wordlen = strlen(bin_array[i]->word);
        //printf("%d %d",wordlen,sequence_len);
        long double expect = expect_prob(bin_array[i]->word);
        expect = expect*(long double)(sequence_len - number_of_sequences * (wordlen - 1));
        long double var = variation(bin_array[i]->word);
        long double stad_var = sqrt(var);
        long double z_score = (bin_array[i]->total_count - expect)/stad_var;
       // long double expect = 1;
        printf("%-*s \t\t %.10Lf\n", max_wordlength, bin_array[i]->word, z_score);
        
    
    }
   
    /*
    int sum = 0;
    for (j = 0; j < ALPHA_NUM; j++) {
        printf("%Lf ", (long double)letter_prob[j]/sequence_len);
        sum = sum + letter_prob[j];
    }
    printf("\n%d\n%lu\n",sum,sequence_len);
    printf("%Lf\n",expect_prob("bcd"));
    */
    
    
}











