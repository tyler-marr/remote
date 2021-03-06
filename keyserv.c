/*
########################################################################
# This file is part of the minicom communications package for WRAMP.
#
# Copyright 1991-1995 Miquel van Smoorenburg.
# Copyright (C) 2019 The University of Waikato, Hamilton, New Zealand.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
########################################################################
*/
#include "port.h"
#include "remote.h"

/* Emulation modes */
#define EVT100	1
#define EANSI	2

/* Pipe file descriptors */
#define from_remote 3
#define to_remote   4

/* Set modes to normal */
int keypadmode = NORMAL;
int cursormode = NORMAL;

char *_tptr = (char *)NULL;	/* Pointer to termcap information */
int mode = EVT100;		/* Emulation mode selector */
int parent;			/* Process ID of minicom */
jmp_buf mainloop;		/* To jump to after a 'HELLO' signal */
char **escseq;			/* Translation table to use */
static int argument;		/* Argument to 'HELLO' command */
static int esc_char = 1;	/* Escape character (initially ^A) */
static int bs_code = 8;		/* Code that backspace key sends */
extern int pendingkeys;		/* From wkeys.c */

char *st_vtesc[] = {
  "", "\033OP", "\033OQ", "\033OR", "\033OS", "", "", "", "", "", "",
  "\033[H", "", "\033[A", "\033[D", "\033[C", "\033[B", "\033[K", "",
  "", "\177" };

char *app_vtesc[] = {
  "", "\033OP", "\033OQ", "\033OR", "\033OS", "", "", "", "", "", "",
  "\033[H", "", "\033OA", "\033OD", "\033OC", "\033OB", "\033[K", "",
  "", "\177" };

char *ansiesc[] = {
  "", "\033OP", "\033OQ", "\033OR", "\033OS", "\033OT", "\033OU", "\033OV",
  "\033OW", "\033OX", "\033OY", "\033[H", "\033[V", "\033[A", "\033[D",
  "\033[C", "\033[B", "\033[Y", "\033[U", "0", "\177" };

/*
 * We got a signal. This means that there is some information for us.
 * Read it, and jump to the main loop.
 */
/*ARGSUSED*/
void handler(dummy)
int dummy;
{
  unsigned char buf[8];
  int n;

  signal(HELLO, handler);
  n = read(from_remote, buf, 8);
  if (n <= 0) {
  	n = 2;
  	buf[0] = 0;
  }
  if (n % 2 == 1) {
	n++;
	read(from_remote, buf + n, 1);
  }
  argument = buf[n-1];
  longjmp(mainloop, (int)buf[n - 2]);
}

/*
 * Send a string to the modem
 */
void sendstr(s)
char *s;
{
  write(1, s, strlen(s));
}

/*
 * Main program of keyserv.
 */
int main(argc, argv)
int argc;
char **argv;
{
  int c;
  char ch;
  int f, fun;
  int stopped = 0;

  parent = atoi(argv[1]);
  
  /* Initialize signal handlers */
  /* signal(SIGHUP, SIG_IGN); */
  signal(SIGQUIT, SIG_IGN);
  signal(SIGINT, SIG_IGN);
#ifdef SIGTSTP
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
#endif
  signal(HELLO, handler);

  /* Set up escape sequence table */
  escseq = st_vtesc;

  /* Cbreak, no echo (remote itself sets to raw if needed) */
  (void) setcbreak(1);

  if ((fun = setjmp(mainloop)) != 0) {

  	switch(fun) {

  	/* We come here after remote has told us something */

  	case KVT100: /* VT100 keyboard */
		mode = EVT100;
		escseq = st_vtesc;
		break;
	case KANSI:  /* ANSI keyboard */
		mode = EANSI;
		escseq = ansiesc;
		break;
	case KKPST: /* Keypad in standard mode, not used */
		keypadmode = NORMAL; 
		break;
	case KKPAPP: /* Keypad in applications mode, not used */
		keypadmode = APPL;
		break;
	case KCURST: /* Standard cursor keys */
		cursormode = NORMAL;
		if (mode == EVT100)
			escseq = st_vtesc;
		break;
	case KCURAPP: /* cursor keys in applications mode */
		cursormode = APPL;
		if (mode == EVT100)
			escseq = app_vtesc;
		break;
	case KSTOP:  /* Sleep until further notice */
		stopped = 1;
		break;
	case KSIGIO: /* Wait for keypress and tell parent */
		kill(parent, ACK);
		f = read(0, &ch, 1);
		if (f == 1) {
			write(to_remote, &ch, 1);
			(void) kill(parent, HELLO);
		}
		break;
	case KSTART: /* Restart when stopped */
		stopped  = 0;
		break;
	case KSETBS: /* Set code that BS key sends */
		bs_code = argument;
		break;
	case KSETESC: /* Set escape character */
		esc_char = argument;
		break;
	default:
		break;
    }
    if (fun != KSIGIO) {
	kill(parent, ACK);
    }
  }
  /* Wait if stopped */
  if (stopped) pause();

  /* Main loop: read keyboard, send to modem */
  while(1) {
  	c = wxgetch();
  	if (c > 256 && c < 256 + NUM_KEYS) {
  		sendstr(escseq[c - 256]);
  	}
  	if (c < 256) {
  		if (c == K_ERA) c = bs_code;
  		ch = c;
  		/* Test for escape characters */
  		if (c == esc_char || (esc_char == 128 && c > 128)) { 
			/* If we typed too fast, and the escape sequence
			 * was not that of a function key, the next key
			 * is already in the buffer.
			 */
			if (c == esc_char && pendingkeys > 0) {
				ch = wxgetch();
			}
  			write(to_remote, &ch, 1);
  			(void) kill(parent, HELLO);
  		} else {
  			write(1, &ch, 1);
  		}
  	}
  }
  /*NOTREACHED*/
  return(0);
}
