/*
 *
 * btdwm
 * src/events.c
 *
 * © 2006-2010 Anselm R Garbe <anselm@garbe.us>
 * © 2006-2007 Sander van Dijk <a dot h dot vandijk at gmail dot com>
 * © 2006-2009 Jukka Salmi <jukka at salmi dot ch>
 * © 2007-2009 Premysl Hruby <dfenze at gmail dot com>
 * © 2007-2009 Szabolcs Nagy <nszabolcs at gmail dot com>
 * © 2007-2009 Christof Musik <christof at sendfax dot de>
 * © 2007-2008 Enno Gottox Boland <gottox at s01 dot de>
 * © 2007-2008 Peter Hartlich <sgkkr at hartlich dot com>
 * © 2008 Martin Hurton <martin dot hurton at gmail dot com>
 * © 2008 Neale Pickett <neale dot woozle dot org>
 * © 2009 Mate Nagy <mnagy@port70.net>
 * © 2012-2015 Julian Goldsmith <julian@juliangoldsmith.com>
 * © 2016-2017 Bastiaan Teeuwen <bastiaan@mkcl.nl>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <btdwm.h>

void event_trigger(int event, const char *str)
{
	unsigned int i, j;
	char *p, *ocmd = NULL, *cmd = NULL, *_str = NULL;

	for (i = 0; i < LENGTH(hooks); i++)
		if (hooks[i].event == event)
			goto fork;

	return;

fork:
	if (!(_str = malloc(strlen(str) * 2)))
		die("out of memory");

	p = _str;

	while (*str) {
		if (*str == '"')
			*p++ = '\\';

		*p++ = *str++;
	}

	/* Not using sprintf here to prevent it from tampering with the rest
	 * of the string. Could escape all other sequences but this is probably
	 * easier in the end.
	 */
	for (j = 1; hooks[i].cmd[j]; j++) {
		if ((p = strchr(hooks[i].cmd[j], '%')) && *++p == 'n') {
			if (!(cmd = malloc(strlen(hooks[i].cmd[j]) +
					strlen(_str) + 1)))
				die("out of memory\n");

			cmd[0] = '\0';
			strncat(cmd, hooks[i].cmd[j], p - hooks[i].cmd[j] - 1);
			strcat(cmd, _str);
			strcat(cmd, ++p);

			ocmd = hooks[i].cmd[j];
			hooks[i].cmd[j] = cmd;

			break;
		}
	}

	switch (fork()) {
	case 0:
		if (conn)
			close(xcb_get_file_descriptor(conn));

		setsid();
		execvp(hooks[i].cmd[0], hooks[i].cmd);

		fprintf(stderr, "failed to execvp %s\n", hooks[i].cmd[0]);
		exit(0);
		break;
	case -1:
		fprintf(stderr, "failed to fork\n");
		break;
	}

	if (ocmd) {
		hooks[i].cmd[j] = ocmd;
		free(cmd);
		free(_str);
	}
}
