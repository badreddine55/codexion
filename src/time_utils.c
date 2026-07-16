#include "codexion.h"

long fr_get_time_ms(void)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    long milliseconds = (now.tv_sec * 1000) + (now.tv_usec / 1000);
    return milliseconds;
}