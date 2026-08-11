#include "codexion.h"

void	ft_clean_evrithing(t_sim *simulation)
{
	int	i;

	if (simulation == NULL)
		return ;
	/* Must be called only after all threads have been joined. */
	if (simulation->coders)
	{
		i = 0;
		while (i < simulation->n_coders)
		{
			pthread_mutex_destroy(&simulation->coders[i].meal_lock);
			i++;
		}
	}
	if (simulation->dongles)
	{
		i = 0;
		while (i < simulation->n_coders)
		{
			free(simulation->dongles[i].queue.arr_q);
			simulation->dongles[i].queue.arr_q = NULL;
			pthread_mutex_destroy(&simulation->dongles[i].lock);
			i++;
		}
	}
	pthread_mutex_destroy(&simulation->scheduler_lock);
	pthread_mutex_destroy(&simulation->finished_lock);
	pthread_mutex_destroy(&simulation->log_lock);
	pthread_mutex_destroy(&simulation->stop_lock);
	pthread_cond_destroy(&simulation->scheduler_cond);
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