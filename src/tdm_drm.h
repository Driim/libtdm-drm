#ifndef _TDM_DRM_H_
#define _TDM_DRM_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <tbm_surface.h>
#include <tbm_surface_internal.h>
#include <tdm_backend.h>
#include <tdm_log.h>
#include <tdm_list.h>

#if HAVE_UDEV
#include <libudev.h>
#endif

/* drm backend functions (display) */
tdm_error	drm_display_get_capability(tdm_backend_data *bdata, tdm_caps_display *caps);
tdm_error	drm_display_get_pp_capability(tdm_backend_data *bdata, tdm_caps_pp *caps);
tdm_output** drm_display_get_outputs(tdm_backend_data *bdata, int *count, tdm_error *error);
tdm_error	drm_display_get_fd(tdm_backend_data *bdata, int *fd);
tdm_error	drm_display_handle_events(tdm_backend_data *bdata);
tdm_pp*	  drm_display_create_pp(tdm_backend_data *bdata, tdm_error *error);
tdm_error	drm_output_get_capability(tdm_output *output, tdm_caps_output *caps);
tdm_layer**  drm_output_get_layers(tdm_output *output, int *count, tdm_error *error);
tdm_error	drm_output_set_property(tdm_output *output, unsigned int id, tdm_value value);
tdm_error	drm_output_get_property(tdm_output *output, unsigned int id, tdm_value *value);
tdm_error	drm_output_wait_vblank(tdm_output *output, int interval, int sync, void *user_data);
tdm_error	drm_output_set_vblank_handler(tdm_output *output, tdm_output_vblank_handler func);
tdm_error	drm_output_commit(tdm_output *output, int sync, void *user_data);
tdm_error	drm_output_set_commit_handler(tdm_output *output, tdm_output_commit_handler func);
tdm_error	drm_output_set_dpms(tdm_output *output, tdm_output_dpms dpms_value);
tdm_error	drm_output_get_dpms(tdm_output *output, tdm_output_dpms *dpms_value);
tdm_error	drm_output_set_mode(tdm_output *output, const tdm_output_mode *mode);
tdm_error	drm_output_get_mode(tdm_output *output, const tdm_output_mode **mode);
tdm_error	drm_output_set_status_handler(tdm_output *output, tdm_output_status_handler func, void *user_data);
tdm_error	drm_layer_get_capability(tdm_layer *layer, tdm_caps_layer *caps);
tdm_error	drm_layer_set_property(tdm_layer *layer, unsigned int id, tdm_value value);
tdm_error	drm_layer_get_property(tdm_layer *layer, unsigned int id, tdm_value *value);
tdm_error	drm_layer_set_info(tdm_layer *layer, tdm_info_layer *info);
tdm_error	drm_layer_get_info(tdm_layer *layer, tdm_info_layer *info);
tdm_error	drm_layer_set_buffer(tdm_layer *layer, tbm_surface_h buffer);
tdm_error	drm_layer_unset_buffer(tdm_layer *layer);
void		 drm_pp_destroy(tdm_pp *pp);
tdm_error	drm_pp_set_info(tdm_pp *pp, tdm_info_pp *info);
tdm_error	drm_pp_attach(tdm_pp *pp, tbm_surface_h src, tbm_surface_h dst);
tdm_error	drm_pp_commit(tdm_pp *pp);
tdm_error	drm_pp_set_done_handler(tdm_pp *pp, tdm_pp_done_handler func, void *user_data);

/* drm module internal macros, structures, functions */
#define NEVER_GET_HERE() TDM_ERR("** NEVER GET HERE **")

#define C(b, m)			  (((b) >> (m)) & 0xFF)
#define B(c, s)			  ((((unsigned int)(c)) & 0xff) << (s))
#define FOURCC(a, b, c, d)	 (B(d, 24) | B(c, 16) | B(b, 8) | B(a, 0))
#define FOURCC_STR(id)	  C(id, 0), C(id, 8), C(id, 16), C(id, 24)

#define IS_RGB(format)	  (format == TBM_FORMAT_XRGB8888 || format == TBM_FORMAT_ARGB8888 || \
							 format == TBM_FORMAT_XBGR8888 || format == TBM_FORMAT_ABGR8888)

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define SWAP(a, b)  ({int t; t = a; a = b; b = t; })
#define ROUNDUP(x)  (ceil(floor((float)(height) / 4)))

#define ALIGN_TO_16B(x)	((((x) + (1 <<  4) - 1) >>  4) <<  4)
#define ALIGN_TO_32B(x)	((((x) + (1 <<  5) - 1) >>  5) <<  5)
#define ALIGN_TO_128B(x)   ((((x) + (1 <<  7) - 1) >>  7) <<  7)
#define ALIGN_TO_2KB(x)	((((x) + (1 << 11) - 1) >> 11) << 11)
#define ALIGN_TO_8KB(x)	((((x) + (1 << 13) - 1) >> 13) << 13)
#define ALIGN_TO_64KB(x)   ((((x) + (1 << 16) - 1) >> 16) << 16)

#define RETURN_VAL_IF_FAIL(cond, val) {\
	if (!(cond)) {\
		TDM_ERR("'%s' failed", #cond);\
		return val;\
	} \
}

#define GOTO_IF_FAIL(cond, val) {\
	if (!(cond)) {\
		TDM_ERR("'%s' failed", #cond);\
		goto val;\
	} \
}

typedef struct _tdm_drm_data {
	tdm_display *dpy;

	int drm_fd;

#if LIBDRM_MAJOR_VERSION >= 2 && LIBDRM_MINOR_VERSION >= 4  && LIBDRM_MICRO_VERSION >= 47
	int has_universal_plane;
#endif

#if HAVE_UDEV
	struct udev_monitor *uevent_monitor;
	tdm_event_loop_source *uevent_source;
#endif

	drmModeResPtr mode_res;
	drmModePlaneResPtr plane_res;

	struct list_head output_list;
	struct list_head buffer_list;
} tdm_drm_data;

uint32_t	tdm_drm_format_to_drm_format(tbm_format format);
tbm_format  tdm_drm_format_to_tbm_format(uint32_t format);

void		tdm_drm_display_update_output_status(tdm_drm_data *drm_data);
tdm_error	tdm_drm_display_create_output_list(tdm_drm_data *drm_data);
void		tdm_drm_display_destroy_output_list(tdm_drm_data *drm_data);
tdm_error	tdm_drm_display_create_layer_list(tdm_drm_data *drm_data);

#endif /* _TDM_DRM_H_ */
