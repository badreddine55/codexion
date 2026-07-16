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
            pthread_mutex_destroy(&simulation->dongles[i].lock);
            pthread_cond_destroy(&simulation->dongles[i].cond);
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