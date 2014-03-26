/*****************************************************************************/
/* Line benchmark test for Photon                                            */
/*****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#include <Ph.h>
#include <Pt.h>
#include <photon/PkKeyDef.h>

PtWidget_t* phwindow;

int frames=0;
int fps=0;
int alpha=0;

uint64_t seed=0x235488438FEA7675ULL;

uint64_t simple_random(uint64_t max)
{
    seed=seed*6364136223846793005ULL+1442695040888963407ULL;

    return seed%max;
}

int draw()
{
    int it;
    int jt;
    PgSpan_t spans[256];
    PhPoint_t p={0, 0};

    if (alpha)
    {
        PgSetAlpha(Pg_BLEND_SRC_1 | Pg_BLEND_DST_0, NULL, NULL, 0, 0);
        PgAlphaOn();
    }
    PgSetStrokeColor(PgRGBA(simple_random(255), simple_random(255), simple_random(255), simple_random(255)));

    for (jt=0; jt<2048/256; jt++)
    {
        for (it=0; it<256; it++)
        {
            spans[it].x1=simple_random(1024);
            spans[it].x2=simple_random(1024);
            spans[it].y=simple_random(600);
            if (spans[it].x1>spans[it].x2)
            {
                short temp;

                temp=spans[it].x1;
                spans[it].x1=spans[it].x2;
                spans[it].x2=temp;
            }
        }
        if (PgDrawSpanv(&spans[0], 256, &p, Pg_DRAW_STROKE)!=0)
        {
            fprintf(stderr, "Can't render spans\n");
        }
        PgFlush();
    }

    if (alpha)
    {
        PgAlphaOff();
    }

    frames++;

    return 0;
}

int create_window(int width, int height)
{
    PhDim_t winsize;
    PtArg_t winargs[32];
    uint32_t winargc = 0;
    int32_t status;

    PtSetArg(&winargs[winargc++], Pt_ARG_WINDOW_TITLE, "Spans benchmark test", 0);
    PtSetArg(&winargs[winargc++], Pt_ARG_BASIC_FLAGS, Pt_TRUE, Pt_BASIC_PREVENT_FILL);
    PtSetArg(&winargs[winargc++], Pt_ARG_WINDOW_MANAGED_FLAGS, Pt_FALSE, Ph_WM_APP_DEF_MANAGED);
    PtSetArg(&winargs[winargc++], Pt_ARG_WINDOW_MANAGED_FLAGS, Pt_TRUE,
             Ph_WM_BACKDROP | Ph_WM_TOFRONT | Ph_WM_COLLAPSE | Ph_WM_FFRONT |
             Ph_WM_FOCUS | Ph_WM_HELP | Ph_WM_HIDE | Ph_WM_MAX |
             Ph_WM_MENU | Ph_WM_MOVE | Ph_WM_RESTORE | Ph_WM_TASKBAR |
             Ph_WM_TOBACK | Ph_WM_RESIZE);
    PtSetArg(&winargs[winargc++], Pt_ARG_WINDOW_NOTIFY_FLAGS, Pt_FALSE,
             Ph_WM_RESIZE | Ph_WM_CLOSE | Ph_WM_HELP);
    PtSetArg(&winargs[winargc++], Pt_ARG_WINDOW_NOTIFY_FLAGS, Pt_TRUE,
             Ph_WM_CLOSE | Ph_WM_COLLAPSE | Ph_WM_FOCUS | Ph_WM_MAX |
             Ph_WM_MOVE | Ph_WM_RESIZE | Ph_WM_RESTORE | Ph_WM_HIDE);
    PtSetArg(&winargs[winargc++], Pt_ARG_WINDOW_RENDER_FLAGS, Pt_FALSE,
             Ph_WM_APP_DEF_RENDER);
    PtSetArg(&winargs[winargc++], Pt_ARG_WINDOW_RENDER_FLAGS, Pt_TRUE,
             Ph_WM_RENDER_CLOSE | Ph_WM_RENDER_MENU | Ph_WM_RENDER_MIN |
             Ph_WM_RENDER_TITLE | Ph_WM_RENDER_MOVE | Ph_WM_RENDER_BORDER);
    PtSetArg(&winargs[winargc++], Pt_ARG_WINDOW_STATE, Pt_TRUE,
             Ph_WM_STATE_ISFOCUS);
    PtSetArg(&winargs[winargc++], Pt_ARG_WINDOW_MANAGED_FLAGS, Pt_FALSE,
             Ph_WM_MAX | Ph_WM_RESTORE | Ph_WM_RESIZE);

    /* Set window dimension */
    winsize.w = width;
    winsize.h = height;
    PtSetArg(&winargs[winargc++], Pt_ARG_DIM, &winsize, 0);

    /* Finally create the window */
    phwindow = PtCreateWidget(PtWindow, Pt_NO_PARENT, winargc, winargs);
    if (phwindow == NULL)
    {
        return -1;
    }

    /* Show widget */
    status = PtRealizeWidget(phwindow);
    if (status != 0)
    {
        PtDestroyWidget(phwindow);
    }

    /* Flush all widget operations */
    PtFlush();

    /* Window has been successfully created */
    return 0;
}

void handle_window_event()
{
    uint8_t eventbuffer[8192];
    PhEvent_t *event = (PhEvent_t *) eventbuffer;
    int32_t status;
    uint32_t finish = 0;

    do {
        status = PhEventPeek(event, 8192);
        switch (status) {
        case Ph_RESIZE_MSG:
            {
                fprintf(stderr, "Photon: Event size too much for buffer\n");
                return;
            }
            break;
        case Ph_EVENT_MSG:
            {
                /* Event is ready */
                switch (event->type)
                {
                case Ph_EV_EXPOSE:
                    {
                        draw();
                    }
                    break;
                case Ph_EV_KEY:
                    {
                        PhKeyEvent_t *keyevent = NULL;
                        int pressed = 0;

                        keyevent = PhGetData(event);
                        if (keyevent == NULL) {
                            break;
                        }

                        /* Check if key is repeated */
                        if ((keyevent->key_flags & Pk_KF_Key_Repeat) ==
                            Pk_KF_Key_Repeat) {
                            /* Ignore such events */
                            break;
                        }

                        /* Check if key has its own scancode */
                        if ((keyevent->key_flags & Pk_KF_Scan_Valid) ==
                            Pk_KF_Scan_Valid)
                        {
                            if ((keyevent->key_flags & Pk_KF_Key_Down) ==
                                Pk_KF_Key_Down)
                            {
                                pressed = 1;
                            }
                            else
                            {
                                pressed = 0;
                            }

                            /* ESC is pressed */
                            if (keyevent->key_scan==1)
                            {
                                exit(1);
                            }
                        }
                    }
                    break;
                case Ph_EV_WM:
                    {
                        PhWindowEvent_t *wmevent = NULL;

                        /* Get associated event data */
                        wmevent = PhGetData(event);
                        if (wmevent == NULL) {
                            break;
                        }

                        switch (wmevent->event_f) {
                        case Ph_WM_CLOSE:
                            {
                                exit(0);
                            }
                            break;
                        }
                    }
                    break;
                }
                PtEventHandler(event);
            }
            break;
        case 0:
            {
                /* All events are read */
                finish = 1;
                break;
            }
        case -1:
            {
                /* Error occured in event reading */
                fprintf(stderr, "Photon: Can't read event\n");
                return;
            }
            break;
        }
        if (finish != 0)
        {
            break;
        }
    } while (1);
}

int main(int argc, char** argv)
{
    struct timespec starttime;
    struct timespec currtime;
    int timeelapsed;

    if (argc>1)
    {
        alpha=strtol(argv[1], 0, 0);
    }

    PtInit(NULL);

    create_window(1024, 600);

    clock_gettime(CLOCK_REALTIME, &starttime);

    do {
        clock_gettime(CLOCK_REALTIME, &currtime);
        timeelapsed=(currtime.tv_sec-starttime.tv_sec)*1000 + (currtime.tv_nsec-starttime.tv_nsec)/1000000L;
        if (timeelapsed>5000)
        {
            clock_gettime(CLOCK_REALTIME, &starttime);
            fps=frames/5;
            fprintf(stdout, "FPS=%d, average spans per second=%d\n", fps, frames*2048/5);
            frames=0;
        }

        draw();
        handle_window_event();
    } while(1);

    return 0;
}
