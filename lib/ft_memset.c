/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memset.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wpark <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/07 11:18:25 by wpark             #+#    #+#             */
/*   Updated: 2019/10/07 14:32:20 by wpark            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memset(void *b, int c, size_t len)
{
	unsigned char	*ptr;
	unsigned char	tmp_c;

	if (b == 0)
		return (0);
	ptr = (unsigned char*)b;
	tmp_c = (unsigned char)c;
	while (len--)
		*ptr++ = tmp_c;
	return (b);
}
