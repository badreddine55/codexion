#include "codexion.h"
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

int queue_remove_id(t_queue *q, int id)
{
    int i;
    int j;

    i = 0;
    while (i < q->count)
    {
        if (q->arr_q[i] == id)
        {
            j = i;
            while (j < q->count - 1)
            {
                q->arr_q[j] = q->arr_q[j + 1];
                j++;
            }
            q->count -= 1;
            return (0);
        }
        i++;
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
    int  best_index;
    int  tmp;
    long best_deadline;
    long current;

    if (q == NULL || q->count == 0)
        return (0);

    best_index = 0;
    best_deadline = fr_deadline(sim, q->arr_q[0]);

    i = 1;
    while (i < q->count)
    {
        current = fr_deadline(sim, q->arr_q[i]);

        if (current < best_deadline)
        {
            best_deadline = current;
            best_index = i;
        }
        i++;
    }

    if (best_index != 0)
    {
        tmp = q->arr_q[0];
        q->arr_q[0] = q->arr_q[best_index];
        q->arr_q[best_index] = tmp;
    }

    return (q->arr_q[0] == me);
}