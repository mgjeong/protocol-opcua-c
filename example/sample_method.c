#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CHAR_SIZE   128


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                  METHOD CALLBACK FUNCTIONS                                                 //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *  No Argument method
 **/
void test_method_shutdown(int inpSize, void **input, int outSize, void **output)
{
    printf("shutdown method called\n");
}

void test_method_print(int inpSize, void **input, int outSize, void **output)
{
    int *inp = (int*) input[0];
    printf("print method invoked :: %d\n", *inp);
}

void test_method_version(int inpSize, void **input, int outSize, void **output)
{
    printf("version  method invoked\n");
    char *version = (char*) malloc(sizeof(char) * CHAR_SIZE);
    strcpy(version, "09131759");
    output[0] = version;
}

void test_method_sqrt(int inpSize, void **input, int outSize, void **output)
{
    printf("sqrt method invoked\n");
    double *inp = (double *) input[0];
    double *sq_root = (double *) malloc(sizeof(double));
    *sq_root = sqrt(*inp);
    output[0] = (void *) sq_root;
}

void test_method_increment_int32Array(int inpSize, void **input, int outSize, void **output)
{
    printf("increment_int32Array method invoked\n");
    int32_t *inputArray = (int32_t *) input[0];
    int *delta = (int *) input[1];

    int32_t *outputArray = (int32_t *) malloc(sizeof(int32_t) * 5);
    for (int i = 0; i < 5; i++)
    {
        outputArray[i] = inputArray[i] + *delta;
    }
    output[0] = (void *) outputArray;
}
