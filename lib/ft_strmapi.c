/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strmapi.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wpark <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/11 10:48:50 by wpark             #+#    #+#             */
/*   Updated: 2019/10/11 10:49:03 by wpark            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strmapi(char *s, char (*f)(unsigned int, char))
{
	size_t	len;
	size_t	i;
	char	*ret;

	if (!s || !f)
		return (0);
	len = ft_strlen(s);
	if (!(ret = (char*)malloc(sizeof(char) * (len + 1))))
		return (0);
	ret[len] = '\0';
	i = 0;
	while (i < len)
	{
		ret[i] = f(i, *s++);
		i++;
	}
	ret[i] = '\0';
	return (ret);
}
