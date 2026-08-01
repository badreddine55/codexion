#include "codexion.h"

void ft_clean_evrithing(t_sim *simulation)
{
    int i;

    if (simulation == NULL)
        return;

    /* Destroy per-coder mutexes */
    if (simulation->coders)
    {
        for (i = 0; i < simulation->n_coders; i++)
            pthread_mutex_destroy(&simulation->coders[i].meal_lock);
    }

    /* Destroy per-dongle locks */
    if (simulation->dongles)
    {
        for (i = 0; i < simulation->n_coders; i++)
            pthread_mutex_destroy(&simulation->dongles[i].lock);
    }

    /* Destroy global mutexes / cond exactly once */
    pthread_mutex_destroy(&simulation->scheduler_lock);
    pthread_mutex_destroy(&simulation->finished_lock);
    pthread_mutex_destroy(&simulation->log_lock);
    pthread_mutex_destroy(&simulation->stop_lock);
    pthread_cond_destroy(&simulation->scheduler_cond);

    /* Free memory */
    if (simulation->dongles)
    {
        free(simulation->dongles);
        simulation->dongles = NULL;
    }
    if (simulation->coders)
    {
        free(simulation->coders);
        simulation->coders = NULL;
    }

    free(simulation);
}