/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wpark <wpark@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/11 11:12:43 by wpark             #+#    #+#             */
/*   Updated: 2020/05/28 00:16:37 by wpark            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdio.h>

static int		isdelimit(char c, const char *delimit)
{
	int i;

	i = 0;
	while (delimit[i] != '\0')
	{
		if (delimit[i] == c)
			return (1);
		i++;
	}
	return (0);
}

static int		pos_word(char *s, const char *delimit, unsigned int order)
{
	int				is_word;
	unsigned int	cnt;
	unsigned int	pos;

	is_word = 0;
	cnt = 0;
	pos = 0;
	while (s[pos] != '\0')
	{
		if (is_word == 0 && !isdelimit(s[pos], delimit))
		{
			cnt++;
			if (cnt == order + 1)
				break ;
			is_word = 1;
		}
		else if (is_word == 1 && isdelimit(s[pos], delimit))
			is_word = 0;
		pos++;
	}
	return (pos);
}

static int		count_words(char *s, const char *delimit)
{
	int				is_word;
	unsigned int	cnt;

	cnt = 0;
	is_word = 0;
	while (*s != '\0')
	{
		if (is_word == 0 && !isdelimit(*s, delimit))
		{
			cnt++;
			is_word = 1;
		}
		else if (is_word == 1 && isdelimit(*s, delimit))
			is_word = 0;
		s++;
	}
	return (cnt);
}

static char		*assign_word(char *s, const char *delimit)
{
	unsigned int	i;
	unsigned int	len;
	char			*ret;

	len = 0;
	while (s[len] != '\0' && !isdelimit(s[len], delimit))
		len++;
	ret = (char*)malloc(sizeof(char) * (len + 1));
	i = 0;
	while (i < len)
		ret[i++] = *s++;
	ret[i] = '\0';
	return (ret);
}

char			**ft_split(char const *s, char const *delimit)
{
	unsigned int	nb;
	unsigned int	i;
	char			**ret;
	char			*ptr;

	if (!s)
		return (0);
	ptr = (char*)s;
	nb = count_words(ptr, delimit);
	if (!(ret = (char **)malloc(sizeof(char *) * (nb + 1))))
		return (0);
	i = 0;
	while (i < nb)
	{
		ret[i] = assign_word(ptr + pos_word(ptr, delimit, i), delimit);
		i++;
	}
	ret[i] = 0;
	return (ret);
}
