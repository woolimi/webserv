/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wpark <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/11 11:12:43 by wpark             #+#    #+#             */
/*   Updated: 2019/10/11 11:18:31 by wpark            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

/*
** order start from 0
*/

static int		pos_word(char *s, char c, unsigned int order)
{
	int				is_word;
	unsigned int	cnt;
	unsigned int	pos;

	is_word = 0;
	cnt = 0;
	pos = 0;
	while (s[pos] != '\0')
	{
		if (is_word == 0 && s[pos] != c)
		{
			cnt++;
			if (cnt == order + 1)
				break ;
			is_word = 1;
		}
		else if (is_word == 1 && s[pos] == c)
			is_word = 0;
		pos++;
	}
	return (pos);
}

static int		count_words(char *s, char c)
{
	int				is_word;
	unsigned int	cnt;

	cnt = 0;
	is_word = 0;
	while (*s != '\0')
	{
		if (is_word == 0 && *s != c)
		{
			cnt++;
			is_word = 1;
		}
		else if (is_word == 1 && *s == c)
			is_word = 0;
		s++;
	}
	return (cnt);
}

static char		*assign_word(char *s, char c)
{
	unsigned int	i;
	unsigned int	len;
	char			*ret;

	len = 0;
	while (s[len] != '\0' && s[len] != c)
		len++;
	ret = (char*)malloc(sizeof(char) * (len + 1));
	i = 0;
	while (i < len)
		ret[i++] = *s++;
	ret[i] = '\0';
	return (ret);
}

char			**ft_split(char const *s, char c)
{
	unsigned int	nb;
	unsigned int	i;
	char			**ret;
	char			*ptr;

	if (!s)
		return (0);
	ptr = (char*)s;
	nb = count_words(ptr, c);
	if (!(ret = (char**)malloc(sizeof(char*) * (nb + 1))))
		return (0);
	i = 0;
	while (i < nb)
	{
		ret[i] = assign_word(ptr + pos_word(ptr, c, i), c);
		i++;
	}
	ret[i] = 0;
	return (ret);
}
