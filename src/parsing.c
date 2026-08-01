#include "codexion.h"
#include <limits.h>

static int	is_digits(char *s)
{
	int	i;

	if (s == NULL || s[0] == '\0')
		return (0);
	i = 0;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

int	is_valid_number(char *str)
{
	int	len;

	if (!is_digits(str))
		return (-1);
	len = strlen(str);
	if (len > 10)
		return (-1);
	if (len == 10 && strcmp(str, "2147483647") > 0)
		return (-1);
	return (atoi(str));
}
int pars_args(int ac, char **av, t_sim	*simulation)
{
	int		i;
	int		nbr;
	if (ac != 9)
	{
		printf("Usage:\n");
		printf("./codexion number_of_coders time_to_burnout ");
		printf("time_to_compile time_to_debug ");
		printf("time_to_refactor number_of_compiles_required ");
		printf("dongle_cooldown scheduler\n");
		ft_clean_evrithing(simulation);
		return (1);
	}
	i = 1;
	while (i < 8)
	{
		nbr = is_valid_number(av[i]);
		if (i == 1 && nbr <= 0)
		{
			printf("Error: '%s' number of coders must be greater than 1.\n", av[i]);
			ft_clean_evrithing(simulation);
			return (1);
		}
		if (i != 1 && nbr < 0)
		{
			printf("Error: '%s' must be a non-negative integer.\n", av[i]);
			ft_clean_evrithing(simulation);
			return (1);
		}
		if (i == 1)
			simulation->n_coders = nbr;
		else if (i == 2)
			simulation->time_to_burnout = nbr;
		else if (i == 3)
			simulation->time_to_compile = nbr;
		else if (i == 4)
			simulation->time_to_debug = nbr;
		else if (i == 5)
			simulation->time_to_refactor = nbr;
		else if (i == 6)
			simulation->compiles_required = nbr;
		else if (i == 7)
			simulation->dongle_cooldown = nbr;
		i++;
	}
	if (strcmp(av[8], "fifo") != 0 && strcmp(av[8], "edf") != 0)
	{
		printf("Error: scheduler must be 'fifo' or 'edf'.\n");
		ft_clean_evrithing(simulation);
		return (1);
	}
	else
		simulation->scheduler = av[8];
	return 0;
}