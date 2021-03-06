/* FastDup (http://dev.dereferenced.net/fastdup/)
 * Copyright 2008 - John Brooks <special@dereferenced.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * A copy of the GNU General Public License is available in the
 * LICENSE file distributed with this program.
 */

#include "main.h"
#include "util.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdexcept>

/* There is probably a more efficient way to do this; research that. */
bool stricompare(const std::string &s1, const std::string &s2)
{
	if (s1.length() != s2.length())
		return false;
	
	for (std::string::const_iterator i1 = s1.begin(), i2 = s2.begin(); i1 != s1.end(); ++i1, ++i2)
	{
		if (tolower(*i1) != tolower(*i2))
			return false;
	}
	
	return true;
}

const char *PathMerge(char *buf, size_t size, const char *p1, const char *p2)
{
	if (!buf || !size)
		throw std::logic_error("Invalid buffer passed to PathMerge");
	
	if (!p1 || !*p1)
	{
		if (!p2 || !*p2)
		{
			buf[0] = '\0';
			return buf;
		}
		
		if (size >= strlcpy(buf, p2, size))
			throw std::runtime_error("Path merging failed due to undersized buffer");
		return buf;
	}
	
	size_t p1len = strlcpy(buf, p1, size);
	if (p1len >= size)
		throw std::runtime_error("Path merging failed due to undersized buffer");
	
	if (buf[p1len - 1] != '/' && (!p2 || *p2 != '/'))
	{
		buf[p1len++] = '/';
		buf[p1len] = '\0';
	}
	
	if (!p2 || !*p2)
		return buf;
	
	size_t p2len = strlcpy(buf + p1len, p2, size - p1len);
	if (p2len >= (size - p1len))
		throw std::runtime_error("Path merging failed due to undersized buffer");
	
	return buf;
}

std::string PathMerge(const std::string &p1, const std::string &p2)
{
	if (p1.empty())
	{
		if (p2.empty())
			return std::string();
		
		return p2;
	}
	
	std::string re;
	re.reserve(p1.length() + p2.length() + 1);
	re.assign(p1);
	
	if (re[re.length() - 1] != '/' && (p2.empty() || p2[0] != '/'))
		re.push_back('/');
	
	re.append(p2);
	
	return re;
}

bool FileExists(const char *path)
{
	struct stat st;
	if (stat(path, &st) < 0 || !S_ISREG(st.st_mode))
		return false;
	
	return true;
}

bool DirectoryExists(const char *path)
{
	struct stat st;
	if (stat(path, &st) < 0 || !S_ISDIR(st.st_mode))
		return false;
	
	return true;
}

std::string ByteSizes(off_t size)
{
	static const char *szl[] = { "K", "M", "G" };
	int szlp = 0;
	
	double sz = size / (double)1024;
	
	while (sz >= 1024)
	{
		sz = sz / 1024;
		if (++szlp >= 3)
			break;
	}
	
	char b[12];
	snprintf(b, sizeof(b), "%.2f %s", sz, szl[szlp]);
	return b;
}

off_t ParseHumanSize(const char *in)
{
	off_t re = 0, v = 0;
	
	for (; *in; ++in)
	{
		if (*in == 'b' || *in == 'B')
		{
			re += v;
			v = 0;
		}
		else if (*in == 'k' || *in == 'K')
		{
			re += v*1024;
			v = 0;
		}
		else if (*in == 'm' || *in == 'M')
		{
			re += v*1048576;
			v = 0;
		}
		else if (*in == 'g' || *in == 'G')
		{
			re += v*1073741824;
			v = 0;
		}
		else if (*in >= '0' && *in <= '9')
		{
			v = (v*10)+((*in)-'0');
		}
		else
			return 0;
	}
	
	re += v;
	return re;
}

bool PromptChoice(const char *prompt, bool fallback)
{
	char rbuf[64];
	for (;;)
	{
		fputs(prompt, stdout);
		if (!fgets(rbuf, sizeof(rbuf), stdin))
			return fallback;
		
		if (rbuf[0] == 'y')
			return true;
		else if (rbuf[0] == 'n')
			return false;
	}
}

/* strlcpy and strlcat:
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	
	/* Copy as many bytes as will fit */
	if (n != 0)
	{
		while (--n != 0)
		{
			if ((*d++ = *s++) == '\0')
				break;
		}
	}
	
	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0)
	{
		if (siz != 0)
			*d = '\0'; /* NUL-terminate dst */
		while (*s++)
			;
	}
	
	return (s - src - 1); /* count does not include NUL */
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t strlcat(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;
	
	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;
	
	if (n == 0)
		return (dlen + strlen(s));
	
	while (*s != '\0')
	{
		if (n != 1)
		{
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';
	
	return (dlen + (s - src));        /* count does not include NUL */
}

/* Resolve the actual location of a path by parsing out '..'
 * and '.' segments. Note that if the path given is relative,
 * the resulting path may still contain '..' segments, but
 * will only do so at the beginning. Return value is 0 for
 * invalid input or insufficient buffer space, and the length
 * of the new path on success. Buffer will be null terminated.
 * It will always be enough for bufsz to be strlen(path) + 1.
 */
size_t PathResolve(char *buf, size_t bufsz, const char *path)
{
	if (!path || !*path || !buf || !bufsz)
		return 0;
	
	size_t bp = 0;
	const char *segb = path;
	for (int i = 0; ; ++i)
	{
		if (!path[i] || path[i] == '/')
		{
			if (i && segb == path + i)
			{
				if (!path[i])
					break;
				segb = path + i + 1;
				continue;
			}
			
			int segl = (path + i - segb) + 1;
			if (bp + segl >= bufsz)
				return 0;
			
			/* segl will be one higher because it copies the / or null */
			if (segl == 2 && *segb == '.')
			{
				// ./ refers to the current directory, so it has no effect.
				if (!path[i])
					break;
				segb = path + i + 1;
				continue;
			}
			else if (segl == 3 && *segb == '.' && segb[1] == '.')
			{
				// ../ refers to the directory above
				if (bp == 1 && buf[0] == '/')
				{
					// Going below / is impossible, ignore.
					if (!path[i])
						break;
					segb = path + i + 1;
					continue;
				}
				
				// If the path is relative, it is possible to go below what we know, so treat the ../ as something normal
				if (bp)
				{
					for (int j = bp - 2; j >= 0; --j)
					{
						if (buf[j] == '/' || j == 0)
						{
							if (buf[j] == '/')
								j++;
							
							if ((unsigned int) j + 2 < bp && buf[j] == '.' && buf[j + 1] == '.' && buf[j + 2] == '/')
							{
								// Pile it on.
								memcpy(buf + bp, segb, segl);
								bp += segl;
							}
							else
							{
								// Overwrite this segment
								bp = j;
								buf[bp] = '\0';
							}
							
							break;
						}
					}
					
					if (!path[i])
						break;
					segb = path + i + 1;
					continue;
				}
			}
			
			memcpy(buf + bp, segb, segl);
			bp += segl;
			
			if (!path[i])
				break;
			
			segb = path + i + 1;
		}
	}
	
	if (bp && !buf[bp - 1])
		bp--;
	else
		buf[bp] = 0;
	
	return bp;
}
