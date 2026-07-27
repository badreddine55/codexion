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
        simulation->dongles[i].released_at = simulation->start_time - simulation->dongle_cooldown;
        memset(&simulation->dongles[i].queue, 0, sizeof(t_queue));
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

int queue_push(t_queue *q, int id)
{
    if(q->count < SIZE)
    {
        q->arr_q[q->count] = id;
        q->count += 1;
        return 0;
    }
    return -1;
}
int queue_pop_front(t_queue *q)
{
    
    if(q->count == 0)
        return -1;
    int i = 0;
    int id = q->arr_q[0];
    while (i < q->count - 1)
    {
        q->arr_q[i] = q->arr_q[i + 1];
        i++;
    }
    q->count -= 1;
    return id;
    
}
int queue_front(t_queue *q)
{
    if (q->count == 0)
        return -1;
    
    return q->arr_q[0];
}
void push_id_to_dongles(t_coder *coder)
{
    t_dongle *first;
    t_dongle *second;

    if (coder->left < coder->right)
    {
        first = coder->left;
        second = coder->right;
    }
    else
    {
        first = coder->right;
        second = coder->left;
    }

    pthread_mutex_lock(&first->lock);
    pthread_mutex_lock(&second->lock);

    queue_push(&coder->left->queue, coder->id);
    queue_push(&coder->right->queue, coder->id);
    
    pthread_mutex_unlock(&second->lock);
    pthread_mutex_unlock(&first->lock);
}
int request_left_dongle(t_coder *coder)
{
    long target_time;
    long remaining;
    pthread_mutex_lock(&coder->left->lock);

    while (1)
    {
        while (queue_front(&coder->left->queue) != coder->id)
            pthread_cond_wait(&coder->left->cond, &coder->left->lock);

        target_time = coder->left->released_at + coder->sim->dongle_cooldown;
        remaining = target_time - fr_get_time_ms();

        if (remaining <= 0)
            break;

        pthread_mutex_unlock(&coder->left->lock);
        usleep(remaining * 1000);
        pthread_mutex_lock(&coder->left->lock);
    }

    fr_log(coder, "has taken a dongle");
    return(0);
}
int request_right_dongle(t_coder *coder)
{
    long target_time;
    long remaining;
    pthread_mutex_lock(&coder->right->lock);

    while (1)
    {
        while (queue_front(&coder->right->queue) != coder->id)
            pthread_cond_wait(&coder->right->cond, &coder->right->lock);

        target_time = coder->right->released_at + coder->sim->dongle_cooldown;
        remaining = target_time - fr_get_time_ms();

        if (remaining <= 0)
            break;

        pthread_mutex_unlock(&coder->right->lock);
        usleep(remaining * 1000);
        pthread_mutex_lock(&coder->right->lock);
    }

    fr_log(coder, "has taken a dongle");
    return(0);
}
int release_dongle(t_dongle *dongle)
{
    int res;
    res = queue_pop_front(&dongle->queue);
    if (res == -1)
    {
        pthread_mutex_unlock(&dongle->lock);
        return (-1);
    }
    // dongle->available = 1;
    dongle->released_at = fr_get_time_ms();
    pthread_cond_broadcast(&dongle->cond);
    pthread_mutex_unlock(&dongle->lock);
    return (0);
}