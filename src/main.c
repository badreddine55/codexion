#include "codexion.h"
void *monitor_thread(void *arg)
{
    t_sim *sim = (t_sim *)arg;
    int   i;
    int   finished_c;
    long  last_compile_start;
    long  interval;

    while (1)
    {
        pthread_mutex_lock(&sim->finished_lock);
        finished_c = sim->finished_coders;
        pthread_mutex_unlock(&sim->finished_lock);

        if (finished_c >= sim->n_coders)
        {
            pthread_mutex_lock(&sim->stop_lock);
            sim->stop_flag = 1;
            pthread_mutex_unlock(&sim->stop_lock);

            i = 0;
            while (i < sim->n_coders)
            {
                pthread_cond_broadcast(&sim->dongles[i].cond);
                i++;
            }
            return (NULL);
        }

        i = 0;
        while (i < sim->n_coders)
        {
            if (sim->coders[i].finished == 1)
            {
                i++;
                continue;
            }
            pthread_mutex_lock(&sim->coders[i].meal_lock);
            last_compile_start = sim->coders[i].last_compile_start;
            pthread_mutex_unlock(&sim->coders[i].meal_lock);

            interval = fr_get_time_ms() - last_compile_start;
            if (interval > sim->time_to_burnout)
            {
                fr_log(&sim->coders[i], "burned out");

                pthread_mutex_lock(&sim->stop_lock);
                sim->stop_flag = 1;
                pthread_mutex_unlock(&sim->stop_lock);

                i = 0;
                while (i < sim->n_coders)
                {
                    pthread_cond_broadcast(&sim->dongles[i].cond);
                    i++;
                }
                return (NULL);
            }
            i++;
        }
        usleep(1000);
    }
    return (NULL);
}
int fr_check_stop(t_sim *sim)
{
    int flag;

    pthread_mutex_lock(&sim->stop_lock);
    flag = sim->stop_flag;
    pthread_mutex_unlock(&sim->stop_lock);
    return (flag);
}

int fr_stoppable_sleep(t_sim *sim, long duration_ms)
{
    long elapsed;
    long remaining;
    long chunk;

    elapsed = 0;
    chunk = 5;
    while (elapsed < duration_ms)
    {
        if (fr_check_stop(sim))
            return (-1);
        remaining = duration_ms - elapsed;
        if (remaining < chunk)
            usleep(remaining * 1000);
        else
            usleep(chunk * 1000);
        elapsed += chunk;
    }
    return (0);
}
void *coder_cycle(void *arg)
{
    t_coder *coder = (t_coder *)arg;
    int j = 0;
    int interrupted;

    while (j < coder->sim->compiles_required)
    {
        if (fr_check_stop(coder->sim))
            break;

        push_id_to_dongles(coder);

    if (coder->id % 2 != 0)   // Odd coder
    {
        if (request_left_dongle(coder) == -1)
            break;

        if (request_right_dongle(coder) == -1)
        {
            release_dongle(coder->left);
            break;
        }
    }
    else                      // Even coder
    {
        if (request_right_dongle(coder) == -1)
            break;

        if (request_left_dongle(coder) == -1)
        {
            release_dongle(coder->right);
            break;
        }
    }

        pthread_mutex_lock(&coder->meal_lock);
        coder->last_compile_start = fr_get_time_ms();
        pthread_mutex_unlock(&coder->meal_lock);

        fr_log(coder, "is compiling");
        interrupted = (fr_stoppable_sleep(coder->sim, coder->sim->time_to_compile) == -1);

        release_dongle(coder->left);
        release_dongle(coder->right);

        if (interrupted)
            break;

        coder->compiles_done++;

        fr_log(coder, "is debugging");
        if (fr_stoppable_sleep(coder->sim, coder->sim->time_to_debug) == -1)
            break;

        fr_log(coder, "is refactoring");
        if (fr_stoppable_sleep(coder->sim, coder->sim->time_to_refactor) == -1)
            break;

        j++;
    }

    pthread_mutex_lock(&coder->meal_lock);
    if (j >= coder->sim->compiles_required)
        coder->finished = 1;
    pthread_mutex_unlock(&coder->meal_lock);

    if (coder->finished)
    {
        pthread_mutex_lock(&coder->sim->finished_lock);
        coder->sim->finished_coders += 1;
        pthread_mutex_unlock(&coder->sim->finished_lock);
    }

    return (NULL);
}

int    main(int ac, char **av)
{
    t_sim    *simulation;
    simulation = malloc(sizeof(t_sim));
    if (simulation == NULL)
    {
        perror("malloc");
        return (1);
    }
    memset(simulation, 0, sizeof(t_sim));
    pthread_mutex_init(&simulation->log_lock, NULL);
    if (pars_args(ac, av, simulation))
        return (1);
    if(fr_build_dongle(simulation))
    {
        ft_clean_evrithing(simulation);
        return (1);
    }
    if(fr_build_coder(simulation))
    {
        ft_clean_evrithing(simulation);
        return (1);
    }
    simulation->start_time = fr_get_time_ms();
    int i = 0;
    while (i < simulation->n_coders )
    {
        pthread_create(&simulation->coders[i].coder_thread, NULL, coder_cycle, &simulation->coders[i]);
        i++;
    }
    pthread_create(&simulation->monitor_thread, NULL, monitor_thread, simulation);
    i = 0;
    while (i < simulation->n_coders )
    {
        pthread_join(simulation->coders[i].coder_thread, NULL);
        i++;
    }
    pthread_join(simulation->monitor_thread, NULL);

    ft_clean_evrithing(simulation);
    return (0);
}