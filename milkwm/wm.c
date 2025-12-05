#define _MILSKO
#include "milkwm.h"

static MwWidget root;

typedef struct wmframe {
	MwWidget titlebar;
	MwWidget title;

	int left_space;
	int right_space;
} wmframe_t;

void loop_wm(void) {
	void* out;

	pthread_mutex_lock(&xmutex);

	MwLibraryInit();

	root = MwCreateWidget(NULL, "root", NULL, 0, 0, 0, 0);

	pthread_mutex_unlock(&xmutex);

	while(1) {
		int s = 0;

		pthread_mutex_lock(&xmutex);
		while(MwPending(root)) {
			if((s = MwStep(root)) != 0) break;
		}
		pthread_mutex_unlock(&xmutex);
		if(s != 0) break;

		MwTimeSleep(30);
	}

	pthread_join(xthread, &out);
}

static void resize(MwWidget handle, void* user, void* client) {
	wmframe_t* f  = handle->opaque;
	int	   ww = MwGetInteger(handle, MwNwidth);
	int	   wh = MwGetInteger(handle, MwNheight);

	MwVaApply(f->titlebar,
		  MwNx, MwDefaultBorderWidth(handle),
		  MwNy, MwDefaultBorderWidth(handle),
		  MwNwidth, ww - MwDefaultBorderWidth(handle) * 2,
		  MwNheight, TitleBarHeight + MwDefaultBorderWidth(handle) * 2,
		  NULL);

	MwVaApply(f->title,
		  MwNx, MwDefaultBorderWidth(f->titlebar) + f->left_space + 8,
		  MwNy, MwDefaultBorderWidth(f->titlebar),
		  MwNwidth, MwGetInteger(f->titlebar, MwNwidth) - MwDefaultBorderWidth(f->titlebar) * 2 - f->left_space - f->right_space - 16,
		  MwNheight, MwGetInteger(f->titlebar, MwNheight) - MwDefaultBorderWidth(f->titlebar) * 2,
		  NULL);
}

static int alignment(const char* str) {
	if(strcmp(str, "Left") == 0) return MwALIGNMENT_BEGINNING;
	if(strcmp(str, "Right") == 0) return MwALIGNMENT_END;
	if(strcmp(str, "Center") == 0) return MwALIGNMENT_CENTER;

	return MwALIGNMENT_CENTER;
}

static void apply_config(MwWidget wnd) {
	wmframe_t*  f = wnd->opaque;
	const char* str;

	if(config_lookup_string(&wm_config, "Window.TitleBar.Align", &str)) {
		MwVaApply(f->title,
			  MwNalignment, alignment(str),
			  NULL);
	} else {
		MwVaApply(f->title,
			  MwNalignment, MwALIGNMENT_CENTER,
			  NULL);
	}
}

MwWidget wm_frame(int w, int h) {
	MwWidget   wnd, titlebar;
	int	   i;
	wmframe_t* f = malloc(sizeof(*f));

	pthread_mutex_lock(&xmutex);

	wnd	    = MwVaCreateWidget(MwWindowClass, "frame", root, MwDEFAULT, MwDEFAULT, w + MwDefaultBorderWidth(root) * 2, h + MwDefaultBorderWidth(root) * 4 + TitleBarHeight,
				       MwNhasBorder, 1,
				       MwNinverted, 0,
				       NULL);
	wnd->opaque = f;

	f->left_space  = TitleBarHeight;
	f->right_space = 0;

	f->titlebar = MwVaCreateWidget(MwFrameClass, "titlebar", wnd, 0, 0, 0, 0,
				       MwNhasBorder, 1,
				       MwNinverted, 1,
				       NULL);

	f->title = MwCreateWidget(MwLabelClass, "title", f->titlebar, 0, 0, 0, 0);

	MwCreateWidget(MwButtonClass, "button", f->titlebar, MwDefaultBorderWidth(wnd), MwDefaultBorderWidth(wnd), TitleBarHeight, TitleBarHeight);

	apply_config(wnd);

	resize(wnd, NULL, NULL);

	pthread_mutex_unlock(&xmutex);

	return wnd;
}

void wm_destroy(MwWidget widget) {
	pthread_mutex_lock(&xmutex);
	free(widget->opaque);
	MwDestroyWidget(widget);
	pthread_mutex_unlock(&xmutex);
}

void wm_set_name(MwWidget widget, const char* name) {
	wmframe_t* f = widget->opaque;

	pthread_mutex_lock(&xmutex);
	MwVaApply(f->title,
		  MwNtext, name,
		  NULL);
	pthread_mutex_unlock(&xmutex);
}
