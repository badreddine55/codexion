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
        simulation->dongles[i].available = 0;
        simulation->dongles[i].released_at = simulation->start_time - simulation->dongle_cooldown;
        memset(&simulation->dongles[i].queue, 0, sizeof(t_queue));
        pthread_mutex_init(&simulation->dongles[i].lock, NULL);
        i++;
    }

    return (0);
}

int queue_push(t_queue *q, int id)
{
    if (q->count < SIZE)
    {
        q->arr_q[q->count] = id;
        q->count += 1;
        return (0);
    }
    return (-1);
}

int queue_pop_front(t_queue *q)
{
    int i;
    int id;

    if (q->count == 0)
        return (-1);
    id = q->arr_q[0];
    i = 0;
    while (i < q->count - 1)
    {
        q->arr_q[i] = q->arr_q[i + 1];
        i++;
    }
    q->count -= 1;
    return (id);
}

int queue_front(t_queue *q)
{
    if (q->count == 0)
        return (-1);
    return (q->arr_q[0]);
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

    if (coder->left->owner != NULL)
        return (0);
    if (coder->right->owner != NULL)
        return (0);

    if (now < coder->left->released_at + coder->sim->dongle_cooldown)
        return (0);
    if (now < coder->right->released_at + coder->sim->dongle_cooldown)
        return (0);

    return (1);
}

long fr_deadline(t_sim *sim, int id)
{
    long start;

    pthread_mutex_lock(&sim->coders[id - 1].meal_lock);
    start = sim->coders[id - 1].last_compile_start;
    pthread_mutex_unlock(&sim->coders[id - 1].meal_lock);
    return (start + sim->time_to_burnout);
}

int fr_earliest_in_queue(t_sim *sim, t_queue *q, int me)
{
    int  i;
    int  best_id;
    long best_deadline;
    long current;

    best_id = q->arr_q[0];
    best_deadline = fr_deadline(sim, best_id);

    i = 1;
    while (i < q->count)
    {
        current = fr_deadline(sim, q->arr_q[i]);
        if (current < best_deadline ||
            (current == best_deadline && q->arr_q[i] < best_id))
        {
            best_deadline = current;
            best_id = q->arr_q[i];
        }
        i++;
    }
    return (best_id == me);
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

    pthread_mutex_lock(&coder->sim->scheduler_lock);

    while (1)
    {
        if (fr_check_stop(coder->sim))
        {
            pthread_mutex_unlock(&coder->sim->scheduler_lock);
            return (-1);
        }

        if (fr_both_free(coder) && fr_has_priority(coder))
        {
            coder->left->owner = coder;
            coder->right->owner = coder;

            queue_pop_front(&coder->left->queue);
            queue_pop_front(&coder->right->queue);

            pthread_mutex_unlock(&coder->sim->scheduler_lock);

            fr_log(coder, "has taken a dongle");
            fr_log(coder, "has taken a dongle");
            return (0);
        }

        pthread_cond_wait(&coder->sim->scheduler_cond, &coder->sim->scheduler_lock);
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