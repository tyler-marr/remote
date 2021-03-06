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
#define KSTOP		1
#define KKPST		2
#define KVT100		3
#define KANSI		4
#define KMINIX		5
#define KSTART		6
#define KKPAPP		7
#define KCURST  	8
#define KCURAPP		9
#define KSIGIO		10
#define KSETESC		11
#define KSETBS		12
#define KGETKEY		13

#if defined (_COHERENT) && !defined(SIGUSR1)
#  define SIGUSR1 SIGDIVE
#  define SIGUSR2 SIGOVFL
#endif
#define HELLO		SIGUSR1
#define ACK		SIGUSR2

#define KINSTALL 	100
#define KUNINSTALL	101
