#ifndef UTILS_H
#define UTILS_H

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 */
#define ERR_EXIT(MSG)              \
  do {                             \
    fprintf(stderr, "%s\n", MSG);  \
    exit(EXIT_FAILURE);            \
  } while (0)



/**
    Helper for string comparison.
 */
int  vz_streq(char* inLeft, char* inRight);


typedef struct vz_token_result
{
  char tokens[100][0x1000];
  int tokenSizes[100];
  int tokenCount;
  int lastUnfinished;
} vz_token_result_t;

/**
 * @brief vz_strtok_imp
 * @param inBuffer
 * @param inBufferSize
 * @param rfResult
 *
 * Custom strtok to split string
 * into substring, with the sequence { '\n', '\n' }
 * as token separator.
 */
void  vz_strtok_imp(char* inBuffer,
                    int inBufferSize,
                    vz_token_result_t* rfResult);


#endif
