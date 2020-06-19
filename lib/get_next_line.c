/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wpark <wpark@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/15 01:47:57 by wpark             #+#    #+#             */
/*   Updated: 2020/03/24 15:54:28 by wpark            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int
	lst_length(t_cache *cache)
{
	int	total;
	int	i;

	total = 0;
	while (cache)
	{
		i = 0;
		while (cache->content[i] && cache->content[i] != '\n')
			i++;
		total += i;
		if (cache->content[i] == '\n')
			return (total);
		cache = cache->next;
	}
	return (total);
}

static void
	next_cache(t_cache **cache)
{
	t_cache	*nxt;

	nxt = (*cache)->next;
	free((*cache)->content);
	free((*cache));
	*cache = nxt;
}

static int
	rearrange_content(t_cache **cache, int pos)
{
	int i;

	i = 0;
	while ((*cache)->content[pos])
		(*cache)->content[i++] = (*cache)->content[pos++];
	(*cache)->content[i] = 0;
	return (READ);
}

static int
	extract(t_cache **cache, char **line)
{
	int	len;
	int	i;
	int	j;

	len = lst_length(*cache);
	if (!(*line = malloc(len + 1)))
		return (ERROR);
	(*line)[len] = 0;
	i = 0;
	while (*cache)
	{
		j = 0;
		while ((*cache)->content[j] && (*cache)->content[j] != '\n')
			(*line)[i++] = (*cache)->content[j++];
		if ((*cache)->content[j++] == '\n')
			return (rearrange_content(cache, j));
		else
			next_cache(cache);
	}
	return (READ_EOF);
}

int
	get_next_line(int fd, char **line)
{
	static t_cache	*cache = 0;
	char			buff[BUFFER_SIZE + 1];
	ssize_t			r_size;

	if (fd < 0 || !line || BUFFER_SIZE <= 0)
		return (ERROR);
	if (!lst_hasnl(cache))
	{
		while ((r_size = read(fd, buff, BUFFER_SIZE)) > 0)
		{
			buff[r_size] = 0;
			if (!(lst_add(&cache, ft_strdup2(buff, r_size))))
				return (ERROR);
			if (hasnl(buff))
				break;
		}
		if (r_size < 0)
			return (ERROR);
	}
	return (extract(&cache, line));
}