/* Copyright 2009, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to 
 * QNX Software Systems before you may reproduce, modify or distribute this 
 * software, or any work that includes all or part of this software.  Free 
 * development licenses are available for evaluation and non-commercial purposes.  
 * For more information visit http://licensing.qnx.com or email licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/
 * for other information.
 */

#include <stdio.h>
#include <stdlib.h>
#ifndef _MSC_VER
#include <unistd.h>
#include <strings.h>
#endif
#include <errno.h>
#ifdef __QNXNTO__
#include <sys/neutrino.h>
#include <sys/syspage.h>
#endif
#include <GLES/gl.h>
#include <GLES/egl.h>

#include <gf/gf3d.h>

extern void app_printhelp();
extern void app_options(int argc, char** argv);
extern void app_init(int width, int height);
extern void app_draw();
extern void app_fini();

static EGLint attribute_list[]=
{
    EGL_NATIVE_VISUAL_ID, 0,
    EGL_RED_SIZE, 4,
    EGL_GREEN_SIZE, 4,
    EGL_BLUE_SIZE, 4,
    EGL_ALPHA_SIZE, EGL_DONT_CARE,
    EGL_DEPTH_SIZE, 16,
    EGL_STENCIL_SIZE, EGL_DONT_CARE,
    EGL_NONE
};

#ifdef __QNXNTO__
static int time_elapsed()
{
    static uint64_t init_clock_cycles;
    static int      timer_installed;
    static uint64_t cycles_per_sec;
    uint64_t        timestamp;

    /* Return number of milliseconds since first call */
    if (!timer_installed)
    {
        init_clock_cycles=ClockCycles();
        timer_installed=1;
        return 0;
    }

    timestamp=ClockCycles();

    if (timestamp<init_clock_cycles)
    {
        /* Counter wrapped */
        timestamp+=(UINT64_MAX-init_clock_cycles)+1;
    }
    else
    {
        timestamp-=init_clock_cycles;
    }

    if (cycles_per_sec==0)
    {
        cycles_per_sec=SYSPAGE_ENTRY(qtime)->cycles_per_sec;
    }

    if (timestamp>1000*1000*1000)
    {
        return timestamp/cycles_per_sec*1000;
    }
    else
    {
        return timestamp*1000/cycles_per_sec;
    }

    return 0;
}
#endif

int main(int argc, char** argv)
{
    gf_3d_target_t      target;
    gf_display_t        gf_disp;
    gf_dev_t            gf_dev;
    gf_layer_t          layer;
    int                 i;
    EGLConfig           config;
    EGLContext          econtext;
    EGLDisplay          display;
    EGLSurface          surface;
    EGLint              num_config;
    gf_dev_info_t       info;
    gf_layer_info_t     linfo;
    gf_display_info_t   disp_info;
    int                 width=-1, height=-1;
    int                 xpos=0, ypos=0;
    int                 layer_idx=-1;
    int                 display_idx=0;
    unsigned int        t_last=0, frames=0;
    int                 show_info=0;
    int                 swap=1;

    for (i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-info")==0)
        {
            /* Show info later after context initialization */
            show_info=1;
        } else if (strncmp(argv[i], "-layer=", 7)==0)
        {
            layer_idx=strtol(&argv[i][7], 0, 0);
        } else if (strncmp(argv[i], "-display=", 9)==0)
        {
            display_idx=strtol(&argv[i][9], 0, 0);
        } else if (strncmp(argv[i], "-width=", 7)==0)
        {
            width=strtol(&argv[i][7], 0, 0);
        } else if (strncmp(argv[i], "-height=", 8)==0)
        {
            height=strtol(&argv[i][8], 0, 0);
        } else if (strncmp(argv[i], "-xpos=", 6)==0)
        {
            xpos=strtol(&argv[i][6], 0, 0);
        } else if (strncmp(argv[i], "-ypos=", 6)==0)
        {
            ypos=strtol(&argv[i][6], 0, 0);
        } else if (strncmp(argv[i], "-swap=", 6)==0)
        {
            swap=strtol(&argv[i][6], 0, 0);
        } else if ((strncmp(argv[i], "-help", 5)==0) || (strncmp(argv[i], "-h", 2)==0))
        {
            fprintf(stderr, "Options: [-info] [-layer=n] [-display=n] [-width=n] [-height=n]\n");
            fprintf(stderr, "         [-ypos=n] [-xpos=] [-swap=n]\n");
            app_printhelp();
            return -1;
        }
    }

    /* initialize the graphics device */
    if (gf_dev_attach(&gf_dev, NULL, &info)!=GF_ERR_OK)
    {
        perror("gf_dev_attach()");
        return -1;
    }

    if (gf_display_attach(&gf_disp, gf_dev, display_idx, &disp_info)!=GF_ERR_OK)
    {
        fprintf(stderr, "gf_display_attach() failed\n");
        exit(EXIT_FAILURE);
    }

    /* If application is running under Photon, use another layer automatically */
    if (layer_idx==-1)
    {
        if (getenv("PHOTON2_PATH")!=NULL)
        {
            if (disp_info.main_layer_index==0)
            {
                layer_idx=1;
            }
            else
            {
                layer_idx=0;
            }
        }
        else
        {
            layer_idx=disp_info.main_layer_index;
        }
    }

    /* get an EGL display connection */
    display=eglGetDisplay(gf_dev);

    if (display==EGL_NO_DISPLAY)
    {
        fprintf(stderr, "eglGetDisplay() failed\n");
        return -1;
    }

    if (width==-1)
    {
        width=disp_info.xres;
    }
    if (height==-1)
    {
        height=disp_info.yres;
    }

    if (gf_layer_attach(&layer, gf_disp, layer_idx, 0)!=GF_ERR_OK)
    {
        fprintf(stderr, "gf_layer_attach() failed\n");
        exit(EXIT_FAILURE);
    }

    /* initialize the EGL display connection */
    if (eglInitialize(display, NULL, NULL)!=EGL_TRUE)
    {
        fprintf(stderr, "eglInitialize: error 0x%x\n", eglGetError());
        exit(EXIT_FAILURE);
    }

    for (i=0; ; i++)
    {
        /* Walk through all possible pixel formats for this layer */
        if (gf_layer_query(layer, i, &linfo)!=GF_ERR_OK)
        {
            fprintf(stderr, "Couldn't find a compatible frame "
                "buffer configuration on layer %d\n", layer_idx);
            exit(EXIT_FAILURE);
        }

        /*
         * We want the color buffer format to match the layer format,
         * so request the layer format through EGL_NATIVE_VISUAL_ID.
         */
        attribute_list[1]=linfo.format;

        /* Look for a compatible EGL frame buffer configuration */
        if (eglChooseConfig(display, attribute_list, &config, 1, &num_config)==EGL_TRUE)
        {
            if (num_config>0)
            {
                break;
            }
        }
    }

    /* create a 3D rendering target */
    if (gf_3d_target_create(&target, layer, NULL, 0, width, height, linfo.format)!=GF_ERR_OK)
    {
        fprintf(stderr, "Unable to create rendering target\n");
        return -1;
    }

    gf_layer_set_src_viewport(layer, 0, 0, width-1, height-1);
    gf_layer_set_dst_viewport(layer, xpos, ypos, xpos+width-1, ypos+height-1);
    gf_layer_set_filter(layer, GF_LAYER_FILTER_NONE);
    gf_layer_enable(layer);

    /*
     * The layer settings haven't taken effect yet since we haven't
     * called gf_layer_update() yet.  This is exactly what we want,
     * since we haven't supplied a valid surface to display yet.
     * Later, the OpenGL ES library calls will call gf_layer_update()
     * internally, when  displaying the rendered 3D content.
     */

    /* create an EGL rendering context */
    econtext=eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);

    /* create an EGL window surface */
    surface=eglCreateWindowSurface(display, config, target, NULL);

    if (surface==EGL_NO_SURFACE)
    {
        fprintf(stderr, "Create surface failed: 0x%x\n", eglGetError());
        exit(EXIT_FAILURE);
    }

    /* connect the context to the surface */
    if (eglMakeCurrent(display, surface, surface, econtext)==EGL_FALSE)
    {
        fprintf(stderr, "Make current failed: 0x%x\n", eglGetError());
        exit(EXIT_FAILURE);
    }

    eglSwapInterval(display, swap);

    /* Show info only when context has been initialized */
    if (show_info)
    {
        printf("GL_RENDERER   = %s\n", (char*)glGetString(GL_RENDERER));
        printf("GL_VERSION    = %s\n", (char*)glGetString(GL_VERSION));
        printf("GL_VENDOR     = %s\n", (char*)glGetString(GL_VENDOR));
        printf("GL_EXTENSIONS = %s\n", (char*)glGetString(GL_EXTENSIONS));
    }

    app_init(width, height);
    app_options(argc, argv);

    while (1)
    {
        app_draw();

        if (eglSwapBuffers(display, surface)==EGL_FALSE && eglGetError()==EGL_BAD_SURFACE)
        {
            /*
             * Surfaces were destroyed e.g. due to
             * display modeswitch event.
             */

            /* re-create 3D rendering target */
            gf_3d_target_free(target);
            if (gf_3d_target_create(&target, layer, NULL, 0, width, height, linfo.format)!=GF_ERR_OK)
            {
                fprintf(stderr, "Unable to create rendering target\n");
                return NULL;
            }

            /* Unbind and destroy surface */
            eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            eglDestroySurface(display, surface);

            /* re-create EGL window surface */
            surface=eglCreateWindowSurface(display, config, target, NULL);

            if (surface==EGL_NO_SURFACE)
            {
                fprintf(stderr, "Create surface failed: 0x%x\n", eglGetError());
                exit(EXIT_FAILURE);
            }

            /* re-connect the context to the surface */
            if (eglMakeCurrent(display, surface, surface, econtext)==EGL_FALSE)
            {
                fprintf(stderr, "Make current failed: 0x%x\n", eglGetError());
                exit(EXIT_FAILURE);
            }

            /* Re-set layer configuration */
            gf_layer_set_src_viewport(layer, 0, 0, width-1, height-1);
            gf_layer_set_dst_viewport(layer, xpos, ypos, xpos+width-1, ypos+height-1);
            gf_layer_set_filter(layer, GF_LAYER_FILTER_NONE);
            gf_layer_enable(layer);
        }

        frames++;

#ifdef __QNXNTO__
        {
            unsigned t=time_elapsed();

            if (t-t_last>=5000)
            {
                GLfloat seconds=(t-t_last)/1000.0f;
                GLfloat fps=frames/seconds;
                printf("%d frames in %6.3f seconds = %6.3f FPS\n", frames, seconds, fps);
                t_last=t;
                frames=0;
            }
        }
#endif
    }

    app_fini();

    return 0;
}
