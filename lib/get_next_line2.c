/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line2.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wpark <wpark@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/15 01:47:57 by wpark             #+#    #+#             */
/*   Updated: 2020/03/24 15:55:51 by wpark            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int
	hasnl(char *s)
{
	while (*s)
	{
		if (*s == '\n')
			return (1);
		s++;
	}
	return (0);
}

int
	lst_hasnl(t_cache *cache)
{
	while (cache)
	{
		if (hasnl(cache->content))
			return (1);
		cache = cache->next;
	}
	return (0);
}

static t_cache
	*lst_new(char *content)
{
	t_cache	*new;

	new = malloc(sizeof(t_cache));
	if (!new)
		return (0);
	new->content = content;
	new->next = 0;
	return (new);
}

t_cache
	*lst_add(t_cache **cache, char *content)
{
	t_cache	*tmp;
	
	tmp = *cache;
	if (!content)
		return (0);
	if (!tmp)
		return (*cache = lst_new(content));
	else
	{
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = lst_new(content);
		return (tmp->next);
	}
}

char
	*ft_strdup2(char *str, ssize_t len)
{
	char	*res;
	ssize_t	i;

	if (!(res = malloc(len + 1)))
		return (0);
	i = 0;
	while (*str)
		res[i++] = *str++;
	res[i] = 0;
	return (res);
}