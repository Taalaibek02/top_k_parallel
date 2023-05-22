#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define MAX_ENTRY 9322

typedef struct threadArg
{
  short int start;
  short int end;
} ThreadArg;

typedef struct info
{
  int numLogs;
  int hour;
} Info;

long startTime;
Info count[MAX_ENTRY];

// heapify a tree
int heapTraverse(Info *a, int i, int start, int end)
{
  int root = i;
  int leftIdx = (i - start) * 2 + 1 + start;
  int rightIdx = (i - start) * 2 + 2 + start;
  if (leftIdx < end &&
      leftIdx >= start &&
      a[leftIdx].numLogs > a[root].numLogs)
  {
    root = leftIdx;
  }
  else if (leftIdx < end &&
           leftIdx >= start &&
           a[leftIdx].numLogs == a[root].numLogs &&
           a[leftIdx].hour > a[root].hour)
  {
    root = leftIdx;
  }
  if (rightIdx < end &&
      rightIdx >= start &&
      a[rightIdx].numLogs > a[root].numLogs)
  {
    root = rightIdx;
  }
  else if (rightIdx < end &&
           rightIdx >= start &&
           a[rightIdx].numLogs == a[root].numLogs &&
           a[rightIdx].hour > a[root].hour)
  {
    root = rightIdx;
  }
  if (root != i)
  {
    Info temp = a[i];
    a[i] = a[root];
    a[root] = temp;
    heapTraverse(a, root, start, end);
  }
}

// build a heap from a part of an array
void *buildHeap(void *args)
{
  ThreadArg *arg = (ThreadArg *)args;
  for (int i = (arg->end - arg->start) / 2 + arg->start; i >= arg->start; i--)
  {
    heapTraverse(count, i, arg->start, arg->end);
  }
}

int main(int argc, char **argv)
{
  startTime = atoi(argv[2]);
  struct dirent *file;
  DIR *dir = opendir(argv[1]);
  FILE *input;

  // loop through files
  while ((file = readdir(dir)) != NULL)
  {
    if (!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
      continue;
    char *fileName = malloc(strlen(argv[1]) + strlen(file->d_name) + 1);
    fileName[0] = '\0';
    strcat(fileName, argv[1]);
    strcat(fileName, file->d_name);

    input = fopen(fileName, "r");

    int bufferSize = 40;
    char buffer[bufferSize + 1];
    // read data
    while (fgets(buffer, bufferSize, input) != NULL)
    {
      char *temp;
      long timestamp = strtol(buffer, &temp, 10);
      count[(timestamp - startTime) / 3600].numLogs += 1;
      count[(timestamp - startTime) / 3600].hour = timestamp / 3600;
    }
    fclose(input);
  }

  int k = atoi(argv[3]);

  // init threads
  int threadNum = 4;
  int threadSize = MAX_ENTRY / threadNum;
  ThreadArg threadArgs[threadNum];
  pthread_t threads[threadNum];
  int threadIndex = 0, start = 0, i = 0;

  Info max;
  max.numLogs = 0;
  max.hour = 0;

  // divide the data between threads
  for (i = 0; i < threadNum - 1; i++)
  {
    threadArgs[i].start = start;
    start += threadSize;
    threadArgs[i].end = start;
  }
  threadArgs[i].start = start;
  threadArgs[i].end = MAX_ENTRY;

  printf("Top K frequently accessed hour:\n");

  // start threads
  for (i = 0; i < threadNum; i++)
  {
    pthread_create(&threads[i], NULL, buildHeap, &threadArgs[i]);
  }
  for (i = 0; i < threadNum; i++)
  {
    pthread_join(threads[i], NULL);
  }

  // get top k elements
  for (i = 0; i < k; i++)
  {
    for (int j = 0; j < threadNum; j++)
    {
      if (max.numLogs < count[threadArgs[j].start].numLogs)
      {
        max = count[threadArgs[j].start];
        threadIndex = j;
      }
      else if (max.numLogs == count[threadArgs[j].start].numLogs)
      {
        if (max.hour < count[threadArgs[j].start].hour)
        {
          max = count[threadArgs[j].start];
          threadIndex = j;
        }
      }
    }
    time_t date = max.hour * 3600;
    printf("%.24s\t%d\n", ctime(&date), max.numLogs);
    max.numLogs = 0;
    max.hour = 0;

    // remove max from the corresponding tree, then heapify
    count[threadArgs[threadIndex].start].numLogs = -1;
    heapTraverse(count, threadArgs[threadIndex].start, threadArgs[threadIndex].start, threadArgs[threadIndex].end);
  }
  return 0;
}
