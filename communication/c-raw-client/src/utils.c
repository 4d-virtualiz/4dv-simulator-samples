#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 */
int vz_streq(char* inLeft, char* inRight)
{
  assert(inLeft != NULL);
  assert(inRight != NULL);
  return (strcmp(inLeft, inRight) == 0);
}

/**
 * /////////////////////////////////////
 * /////////////////////////////////////
 */
void  vz_strtok_imp(char* inBuffer,
                    int inBufferSize,
                    vz_token_result_t* rfResult)
{
  int sequenceCounter = -1;
  int nextResultStartIndex = 0;
  int nextResultIndex = 0;

  assert(inBuffer != NULL);
  assert(rfResult != NULL);
  assert(inBufferSize > -1);

  memset(rfResult, 0, sizeof(vz_token_result_t));

  if (inBufferSize == 0)
  {

  }
  else
  {
    nextResultStartIndex = 0;
    for (sequenceCounter = 0 ; sequenceCounter < inBufferSize
         ; sequenceCounter++)
    {
      if (sequenceCounter >= 1)
      {
        if (inBuffer[sequenceCounter - 1] == '\n'
            && inBuffer[sequenceCounter] == '\n')
        {
          memcpy(rfResult->tokens[nextResultIndex],
                 &inBuffer[nextResultStartIndex],
                 ((sequenceCounter - 1) - nextResultStartIndex) * sizeof(char));
          rfResult->tokenSizes[nextResultIndex] =
              ((sequenceCounter - 1) - nextResultStartIndex);
          nextResultIndex++;
          rfResult->tokenCount = nextResultIndex;

          if ((sequenceCounter + 1) < inBufferSize)
          {
            nextResultStartIndex = sequenceCounter + 1;
          }
          else
          {
            nextResultStartIndex = -1;
          }
        }
      }
    }

    if (nextResultStartIndex != -1)
    {
      memcpy(rfResult->tokens[nextResultIndex],
             &inBuffer[nextResultStartIndex],
             (inBufferSize - nextResultStartIndex) * sizeof(char));
      rfResult->tokenSizes[nextResultIndex] =
          (inBufferSize - nextResultStartIndex);
      nextResultIndex++;
      nextResultIndex++;
      rfResult->tokenCount = nextResultIndex;
      rfResult->lastUnfinished = 1;
    }
  }
}

