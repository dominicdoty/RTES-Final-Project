// Data sharing header
// ECEN 5623
// 
//

#ifndef DATA_H
#define DATA_H

#include <pthread.h>


extern pthread_mutex_t mutex0;
extern pthread_mutex_t mutex1;


extern int *buffer0;
extern int *buffer1;

#endif
