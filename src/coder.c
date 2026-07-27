#include "codexion.h"

int fr_build_coder(t_sim *simulation)
{
    int i;
    
    simulation->coders = malloc(sizeof(t_coder) * simulation->n_coders);
    if (simulation->coders == NULL)
    {
        perror("malloc");
        return (1);
    }
    
    i = 0;
    while (i < simulation->n_coders)
    {
        pthread_mutex_init(&simulation->coders[i].meal_lock, NULL);
        simulation->coders[i].id = i + 1;
        simulation->coders[i].compiles_done = 0;
        simulation->coders[i].last_compile_start = fr_get_time_ms();

        simulation->coders[i].right = &simulation->dongles[i];
        simulation->coders[i].left = &simulation->dongles[(i + 1) % simulation->n_coders];

        simulation->coders[i].sim = simulation;

        i++;
    }

    return (0);
}