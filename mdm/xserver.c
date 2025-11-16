#define _MILSKO
#define USE_X11
#include "mdm.h"

#include "stb_ds.h"

static pid_t x_server;
static int got_usr1;

Display* xdisplay;
pthread_t xthread;
pthread_mutex_t xmutex;

static void catch_usr1(int sig){
	(void)sig;

	got_usr1 = 1;
}

int launch_x(void){
	got_usr1 = 0;

	signal(SIGUSR1, catch_usr1);

	if((x_server = fork()) == 0){
		signal(SIGUSR1, SIG_IGN);
		execlp("X", "X", ":0", NULL);
		_exit(-1);
	}else{
		while(!got_usr1){
			if(waitpid(x_server, NULL, WNOHANG) != 0){
				fprintf(stderr, "server died. what happened?\n");
				return 1;
			}
			usleep(1000);
		}
	}

	setenv("DISPLAY", ":0", 1);

	return 0;
}

static int wm_detected = 0;
static int wm_detect(Display* disp, XErrorEvent* ev){
	(void)disp;
	(void)ev;

	wm_detected = 1;
}

static void* x11_thread_routine(void* arg){
	x_loop();
	XCloseDisplay(xdisplay);
}

int init_x(void){
	XColor bg;
	void* old;

	xdisplay = XOpenDisplay(NULL);

	bg.red = 0x1c * 256;
	bg.green = 0x45 * 256;
	bg.blue = 0x73 * 256;
	XAllocColor(xdisplay, DefaultColormap(xdisplay, DefaultScreen(xdisplay)), &bg);

	XSetWindowBackground(xdisplay, DefaultRootWindow(xdisplay), bg.pixel);
	XClearWindow(xdisplay, DefaultRootWindow(xdisplay));
	XFlush(xdisplay);

	old = XSetErrorHandler(wm_detect);
	XSelectInput(xdisplay, DefaultRootWindow(xdisplay), SubstructureRedirectMask);
	XSync(xdisplay, False);
	XSetErrorHandler(old);

	if(wm_detected) return 1;

	pthread_mutex_init(&xmutex, NULL);

	pthread_create(&xthread, NULL, x11_thread_routine, NULL);

	return 0;
}

int x_width(void){
	return DisplayWidth(xdisplay, DefaultScreen(xdisplay));
}

int x_height(void){
	return DisplayHeight(xdisplay, DefaultScreen(xdisplay));
}

typedef struct window {
	MwWidget frame;
	Window client;
} window_t;

void x_loop(void){
	XEvent ev;
	window_t* frames = NULL;

	while(1){
		XNextEvent(xdisplay, &ev);
		if(ev.type == MapRequest){
			XWindowAttributes xwa;
			window_t w;
			int i;

			pthread_mutex_lock(&xmutex);
			for(i = 0; i < arrlen(frames); i++){
				if(frames[i].frame->lowlevel->x11.window == ev.xmaprequest.window){
					XReparentWindow(xdisplay, frames[i].client, ev.xmaprequest.window, MwDefaultBorderWidth(root), MwDefaultBorderWidth(root));
					XMapWindow(xdisplay, ev.xmaprequest.window);
					XMapWindow(xdisplay, frames[i].client);
					break;
				}
			}
			if(i != arrlen(frames)){
				pthread_mutex_unlock(&xmutex);
				continue;
			}

			XGetWindowAttributes(xdisplay, ev.xmaprequest.window, &xwa);

			w.client = ev.xmaprequest.window;
			w.frame = MwVaCreateWidget(MwWindowClass, "frame", root, xwa.x - MwDefaultBorderWidth(root), xwa.y - MwDefaultBorderWidth(root), xwa.width + MwDefaultBorderWidth(root) * 2, xwa.height + MwDefaultBorderWidth(root) * 2,
				MwNhasBorder, 1,
				MwNinverted, 0,
			NULL);

			XSelectInput(xdisplay, w.client, StructureNotifyMask);

			arrput(frames, w);
			pthread_mutex_unlock(&xmutex);
		}else if(ev.type == UnmapNotify){
			int i;

			pthread_mutex_lock(&xmutex);
			for(i = 0; i < arrlen(frames); i++){
				if(frames[i].client == ev.xunmap.window){
					MwDestroyWidget(frames[i].frame);
					arrdel(frames, i);
					break;
				}
			}
			pthread_mutex_unlock(&xmutex);

			break;
		}
	}
}
