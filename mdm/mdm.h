#ifndef __MDM_H__
#define __MDM_H__

#include "config.h"

#include <MDE/Foundation.h>
#include <Mw/Milsko.h>

#include <X11/Xlib.h>

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* xserver.c */
extern pthread_t xthread;
extern pthread_mutex_t xmutex;
extern Display* xdisplay;

int launch_x(void);
int x_width(void);
int x_height(void);
int init_x(void);
void x_loop(void);

/* login.c */
extern MwWidget root;

void login_window(void);

/* config.c */
extern char* config_picture;

void parse_config(void);

#endif
