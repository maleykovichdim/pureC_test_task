
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>


//TODO: for big text

// max length for text + tags, and it is also a restriction for t-LENGTH-v size choice(1 byte)
#define MAX_LEN 255 

const char* XML_ELM[] = {"<text>","</text>","<numeric>","</numeric>"};
const int   XML_ELM_SIZE[] = { 6,9 };
// length= 1 bite; values: 0 - text with length < 256-tags, 1 - numeric = long
const char  TLV_TAG[] = { 0,1 }; 

int main(int arg_c, char* argv[])
{

  if (arg_c < 2)
  {
    printf("we need two params: input and output file");
  }

  char* inputFile = argv[1];
  char* outputFile = argv[2];

  FILE* file_in = NULL;	// global input file handle
  FILE* file_out = NULL;	// global output file handle

  errno_t err = fopen_s(&file_in, inputFile, "r");
  if (!file_in) {
    printf("wrong input file " );
    exit(-1);
  }
  err = fopen_s(&file_out, outputFile, "w");
  if (!file_out) {
    fclose(file_in);//
    printf("wrong output file ");
    exit(-1);
  }

  int numElements = (sizeof(XML_ELM) / sizeof(XML_ELM[0]))/2;
  char buffer[MAX_LEN+1];
  buffer[MAX_LEN] = 0;
  while (fgets(buffer, MAX_LEN, file_in))
  {
    buffer[strcspn(buffer, "\n")] = 0;
    
    for (int i = 0; i < numElements; i++) {

      char* start = NULL;
      start = strstr(buffer, XML_ELM[i*2]);
      if (!start) continue;
      char* end = NULL;
      end = strstr(buffer, XML_ELM[i * 2 + 1]);
      if (!end) continue;
      start += XML_ELM_SIZE[i];
      end[0] = 0;

      unsigned char byteToWrite = 0;
      if (i == 1) {//numeric
        long ret = strtol(start, NULL, 10);
        start[-2] = TLV_TAG[i];
        if (ret <= 127 && ret >= -128) { //char branch          
          byteToWrite = start[-1] = sizeof(char);
          *start = (char)ret;
        }
        else if (ret <= 32767 && ret >= -32768)
        {
          byteToWrite = start[-1] = sizeof(short);
          *((short*)start) = (short)ret;
        }
        else 
        {
          byteToWrite = start[-1] = sizeof(long);
          *((long*)start) = ret;
        }
      }
      else 
      { //TEXT
        start[-2] = TLV_TAG[i];
        byteToWrite = start[-1] = (unsigned char)strlen(start);
      }
      byteToWrite += 2;

      size_t written = fwrite((void*)(start - 2), 1, byteToWrite, file_out);
      if (written < byteToWrite) {
        fclose(file_out);//
        fclose(file_in);//
        printf("error during writting output file");
        exit(-2);
      }

    }
  }

  fclose(file_out);
  fclose(file_in);
  return 0;
}