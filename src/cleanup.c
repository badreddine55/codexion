#include "codexion.h"
void ft_clean_evrithing(t_sim *simulation)
{
    int i;

    i = 0;
    pthread_mutex_destroy(&simulation->log_lock);
    if (simulation->dongles)
    {
        while (i < simulation->n_coders)
        {
            pthread_mutex_destroy(&simulation->scheduler_lock);
            pthread_mutex_destroy(&simulation->finished_lock);
            pthread_mutex_destroy(&simulation->log_lock);
            pthread_mutex_destroy(&simulation->stop_lock);
            pthread_cond_destroy(&simulation->scheduler_cond);
            i++;
        }
    }
    if (simulation->dongles)
        free(simulation->dongles);
    if (simulation->coders)
        free(simulation->coders);
    if (simulation)
        free(simulation);
}