#include "codexion.h"

void fr_log(t_coder *coder, char *state)
{
    long timestamp_in_ms;

    pthread_mutex_lock(&coder->sim->log_lock);
    timestamp_in_ms = (fr_get_time_ms() - coder->sim->start_time);
    printf("%ld %d %s\n",timestamp_in_ms , coder->id, state);
    pthread_mutex_unlock(&coder->sim->log_lock);
}