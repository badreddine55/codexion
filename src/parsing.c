/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: badiyaf <badiyaf@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 18:50:11 by badiyaf           #+#    #+#             */
/*   Updated: 2026/08/18 15:57:56 by badiyaf          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_valid_number(char *str)
{
	int	len;

	len = strlen(str);
	if (len > 10)
		return (-1);
	if (len == 10 && strcmp(str, "2147483647") > 0)
		return (-1);
	return (atoi(str));
}

int	is_valid_chars(const char *str)
{
	int			i;
	int			j;
	const char	*allowed;

	allowed = "-0123456789+";
	if (!str || !*str)
		return (0);
	i = 0;
	while (str[i])
	{
		j = 0;
		while (allowed[j])
		{
			if (str[i] == allowed[j])
				break ;
			j++;
		}
		if (!allowed[j])
			return (0);
		i++;
	}
	return (1);
}

static void	set_simulation_arg(t_sim *sim, int i, int nbr)
{
	if (i == 1)
		sim->n_coders = nbr;
	else if (i == 2)
		sim->time_to_burnout = nbr;
	else if (i == 3)
		sim->time_to_compile = nbr;
	else if (i == 4)
		sim->time_to_debug = nbr;
	else if (i == 5)
		sim->time_to_refactor = nbr;
	else if (i == 6)
		sim->compiles_required = nbr;
	else if (i == 7)
		sim->dongle_cooldown = nbr;
}

static int	validate_and_set_args(char **av, t_sim *sim)
{
	int	i;
	int	nbr;

	i = 1;
	while (i < 8)
	{
		nbr = is_valid_number(av[i]);
		if (!is_valid_chars(av[i]))
			return (printf("Error: '%s' must be a integer.\n", av[i]), 1);
		if (i == 1 && nbr < 1)
		{
			printf("Error: number_of_coders must be greater than 1.\n");
			return (1);
		}
		if (i != 1 && nbr < 0)
		{
			printf("Error: '%s' must be a non-negative integer.\n", av[i]);
			return (1);
		}
		set_simulation_arg(sim, i, nbr);
		i++;
	}
	return (0);
}

int	pars_args(int ac, char **av, t_sim *simulation)
{
	if (ac != 9)
	{
		printf("Usage:\n./codexion number_of_coders time_to_burnout "
			"time_to_compile time_to_debug time_to_refactor "
			"number_of_compiles_required dongle_cooldown scheduler\n");
		return (1);
	}
	if (validate_and_set_args(av, simulation))
		return (1);
	if (strcmp(av[8], "fifo") != 0 && strcmp(av[8], "edf") != 0)
	{
		printf("Error: scheduler must be 'fifo' or 'edf'.\n");
		return (1);
	}
	simulation->scheduler = av[8];
	return (0);
}
