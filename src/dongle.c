#include "codexion.h"

int fr_build_dongle(t_sim *simulation)
{
    int i;

    simulation->dongles = malloc(sizeof(t_dongle) * simulation->n_coders);
    if (simulation->dongles == NULL)
    {
        perror("malloc");
        return (1);
    }

    i = 0;
    while (i < simulation->n_coders)
    {
        simulation->dongles[i].available = 1;
        // I need to get the gettimeofday ones when the program start 
        simulation->dongles[i].released_at = fr_get_time_ms();

        if (pthread_mutex_init(&simulation->dongles[i].lock, NULL) != 0)
        {
            perror("pthread_mutex_init");
            while (--i >= 0)
            {
                pthread_mutex_destroy(&simulation->dongles[i].lock);
                pthread_cond_destroy(&simulation->dongles[i].cond);
            }
            free(simulation->dongles);
            simulation->dongles = NULL;
            return (1);
        }

        if (pthread_cond_init(&simulation->dongles[i].cond, NULL) != 0)
        {
            perror("pthread_cond_init");

            pthread_mutex_destroy(&simulation->dongles[i].lock);

            while (--i >= 0)
            {
                pthread_mutex_destroy(&simulation->dongles[i].lock);
                pthread_cond_destroy(&simulation->dongles[i].cond);
            }

            free(simulation->dongles);
            simulation->dongles = NULL;
            return (1);
        }

        i++;
    }

    return (0);
}