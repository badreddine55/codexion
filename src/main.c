#include "codexion.h"

int	main(int ac, char **av)
{
	t_sim	*simulation;
	int		i;
	int		created_coders;
	int		monitor_created;
	int		ret;

	simulation = malloc(sizeof(t_sim));
	if (simulation == NULL)
	{
		perror("malloc");
		return (1);
	}
	memset(simulation, 0, sizeof(t_sim));
	pthread_mutex_init(&simulation->log_lock, NULL);
	pthread_mutex_init(&simulation->stop_lock, NULL);
	pthread_mutex_init(&simulation->finished_lock, NULL);
	// pthread_mutex_init(&simulation->scheduler_lock, NULL);
	// pthread_cond_init(&simulation->scheduler_cond, NULL);
	if (pars_args(ac, av, simulation))
	{
		ft_clean_evrithing(simulation);
		return (1);
	}
	if (fr_build_dongle(simulation))
	{
		ft_clean_evrithing(simulation);
		return (1);
	}
	if (fr_build_coder(simulation))
	{
		ft_clean_evrithing(simulation);
		return (1);
	}
	simulation->start_time = fr_get_time_ms();
	i = 0;
	created_coders = 0;
	while (i < simulation->n_coders)
	{
        if (i % 2)
        {
            i++;
            continue;
        }
		ret = pthread_create(&simulation->coders[i].coder_thread,
				NULL, coder_cycle, &simulation->coders[i]);
		if (ret != 0)
		{
			pthread_mutex_lock(&simulation->stop_lock);
			simulation->stop_flag = 1;
			pthread_mutex_unlock(&simulation->stop_lock);
			break ;
		}
		created_coders++;
		i++;
	}
    usleep(2000);
    i = 0;
    while (i < simulation->n_coders)
	{
        if (i % 2 == 0)
        {
            i++;
            continue;
        }
		ret = pthread_create(&simulation->coders[i].coder_thread,
				NULL, coder_cycle, &simulation->coders[i]);
		if (ret != 0)
		{
			pthread_mutex_lock(&simulation->stop_lock);
			simulation->stop_flag = 1;
			pthread_mutex_unlock(&simulation->stop_lock);
			break ;
		}
		created_coders++;
		i++;
	}
	if (created_coders != simulation->n_coders)
	{
		i = 0;
		while (i < created_coders)
		{
			pthread_join(simulation->coders[i].coder_thread, NULL);
			i++;
		}
		ft_clean_evrithing(simulation);
		return (1);
	}
	ret = pthread_create(&simulation->monitor_thread,
			NULL, monitor_thread, simulation);
	if (ret != 0)
	{
		pthread_mutex_lock(&simulation->stop_lock);
		simulation->stop_flag = 1;
		pthread_mutex_unlock(&simulation->stop_lock);
		i = 0;
		while (i < simulation->n_coders)
		{
			pthread_join(simulation->coders[i].coder_thread, NULL);
			i++;
		}
		ft_clean_evrithing(simulation);
		return (1);
	}
	monitor_created = 1;
	i = 0;
	while (i < simulation->n_coders)
	{
		pthread_join(simulation->coders[i].coder_thread, NULL);
		i++;
	}
	if (monitor_created)
		pthread_join(simulation->monitor_thread, NULL);
	ft_clean_evrithing(simulation);
	return (0);
}