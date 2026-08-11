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
        simulation->dongles[i].owner = NULL;
        simulation->dongles[i].released_at = simulation->start_time - simulation->dongle_cooldown;
        memset(&simulation->dongles[i].queue, 0, sizeof(t_queue));
        pthread_mutex_init(&simulation->dongles[i].lock, NULL);
        i++;
    }

    return (0);
}



void push_id_to_dongles(t_coder *coder)
{
    pthread_mutex_lock(&coder->sim->scheduler_lock);

    queue_push(&coder->left->queue, coder->id);
    queue_push(&coder->right->queue, coder->id);

    pthread_mutex_unlock(&coder->sim->scheduler_lock);
}

int fr_both_free(t_coder *coder)
{
    long now;

    now = fr_get_time_ms();

    if (coder->left == coder->right)
        return (0);
    if (coder->left->owner != NULL || coder->right->owner != NULL)
        return (0);
    if (now < coder->left->released_at + coder->sim->dongle_cooldown)
        return (0);
    if (now < coder->right->released_at + coder->sim->dongle_cooldown)
        return (0);
    return (1);
}



int fr_has_priority(t_coder *coder)
{

    if (strcmp(coder->sim->scheduler, "fifo") == 0)
    {
        if (queue_front(&coder->left->queue) != coder->id)
            return (0);
        if (queue_front(&coder->right->queue) != coder->id)
            return (0);
        return (1);
    }

    if (!fr_earliest_in_queue(coder->sim, &coder->left->queue, coder->id))
        return (0);
    if (!fr_earliest_in_queue(coder->sim, &coder->right->queue, coder->id))
        return (0);
    return (1);
}

int request_dongle(t_coder *coder)
{
    push_id_to_dongles(coder);

    while (1)
    {
        // pthread_mutex_lock(&coder->sim->scheduler_lock);

        if (fr_check_stop(coder->sim))
        {
            // pthread_mutex_unlock(&coder->sim->scheduler_lock);
            return (-1);
        }

        if (fr_both_free(coder) && fr_has_priority(coder))
        {
            coder->left->owner = coder;
            coder->right->owner = coder;

            queue_remove_id(&coder->left->queue, coder->id);
            queue_remove_id(&coder->right->queue, coder->id);

            fr_log(coder, "has taken a dongle");
            fr_log(coder, "has taken a dongle");

            pthread_mutex_unlock(&coder->sim->scheduler_lock);
            return (0);
        }

        // pthread_mutex_unlock(&coder->sim->scheduler_lock);
        usleep(2000);
    }
}

void fr_release_dongles(t_coder *coder)
{
    pthread_mutex_lock(&coder->sim->scheduler_lock);

    coder->left->owner = NULL;
    coder->left->released_at = fr_get_time_ms();

    coder->right->owner = NULL;
    coder->right->released_at = fr_get_time_ms();

    pthread_cond_broadcast(&coder->sim->scheduler_cond);
    pthread_mutex_unlock(&coder->sim->scheduler_lock);
}