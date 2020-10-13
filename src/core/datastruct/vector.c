#include "vector.h"

void add(float *vec1[], float vec2[])
{
    float resVec[3]; 
    for (int i = 0; i < 3; i++)
    {
        *vec1[i] = *vec1[i] + vec2[i];
    }
}

void swap(int i, int j)
{

}
