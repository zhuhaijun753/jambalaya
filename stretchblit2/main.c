/*****************************************************************************/
/* Stretch blit regression test for photon                                   */
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
PdOffscreenContext_t* ball=NULL;
uint8_t* ball_ptr=NULL;
uint8_t* ball_copy_ptr=NULL;
uint32_t ball_copy_size=0;

int frames=0;
int fps=0;

void DrawSimpleText(int x, int y, char* text)
{
    PhPoint_t p = { x, y };
    char Helvetica[MAX_FONT_TAG];

    if (PfGenerateFontName("Helvetica", 0, 12, Helvetica) == NULL)
    {
        fprintf(stderr, "Can't find Helvetica font\n");
    }
    else
    {
        PgSetFont(Helvetica);
    }
    PgSetTextColor(Pg_WHITE);
    PgDrawText(text, strlen(text), &p, 0);
}

int init_draw()
{
    int handle;
    uint8_t* curr_ptr;
    uint16_t* curr_ptr16;
    uint8_t temp_ptr[256*4];
    int i, j;

    ball=PdCreateOffscreenContext(0, 256, 256,
         Pg_OSC_MEM_LINEAR_ACCESSIBLE | Pg_OSC_MEM_PAGE_ALIGN |
         Pg_OSC_MEM_2D_WRITABLE | Pg_OSC_MEM_2D_READABLE);

    if (ball==NULL)
    {
        return -1;
    }

    ball_ptr=PdGetOffscreenContextPtr(ball);

    /* Fill ball texture */
    curr_ptr=ball_ptr;
    curr_ptr16=(uint16_t*)ball_ptr;
    handle=open("./tennisball32.tga", O_RDONLY);
    if (handle==-1)
    {
        fprintf(stderr, "Can't open tennisball32.tga file\n");
        return -1;
    }
    read(handle, temp_ptr, 18);
    for (i=0; i<256; i++)
    {
        read(handle, temp_ptr, 256*4);
        switch (ball->format)
        {
            case Pg_IMAGE_DIRECT_8888:
                 memcpy(curr_ptr, temp_ptr, 256*4);
                 break;
            case Pg_IMAGE_DIRECT_565:
                 for (j=0; j<256; j++)
                 {
                     *(curr_ptr16+j)=((unsigned int)*(temp_ptr+j*4 + 2) >> 3) << 11 |
                                     ((unsigned int)*(temp_ptr+j*4 + 1) >> 2) << 5 |
                                     ((unsigned int)*(temp_ptr+j*4 + 0) >> 3);
                 }
                 break;
            case Pg_IMAGE_DIRECT_1555:
                 for (j=0; j<256; j++)
                 {
                     *(curr_ptr16+j)=((unsigned int)*(temp_ptr+j*4 + 3)>0 ? 0x8000 : 0x0000) |
                                     ((unsigned int)*(temp_ptr+j*4 + 2) >> 3) << 10 |
                                     ((unsigned int)*(temp_ptr+j*4 + 1) >> 3) << 5 |
                                     ((unsigned int)*(temp_ptr+j*4 + 0) >> 3);
                 }
                 break;
            default:
                 /* unknown pixel format */
                 fprintf(stderr, "Unknown pixel format: %08X\n", (unsigned int)ball->format);
                 return -1;
        }
        curr_ptr+=ball->pitch;
        curr_ptr16+=ball->pitch/2;
    }
    close(handle);

    ball_copy_size=ball->pitch*256;
    ball_copy_ptr=malloc(ball_copy_size);

    if (ball_copy_ptr==NULL)
    {
        fprintf(stderr, "Can't allocate memory!\n");
        return -1;
    }

    /* Store a copy of image in system RAM */
    memcpy(ball_copy_ptr, ball_ptr, ball_copy_size);

    return 0;
}

int draw()
{
    PhRect_t src_rect={{0, 0}, {255, 255}};
    PhRect_t dst_rect={{0, 0}, {0, 0}};
    char text[256];

    /* Draw alpha blended balls over bricks */
    dst_rect.ul.x = 0;
    dst_rect.ul.y = 16;
    dst_rect.lr.x = dst_rect.ul.x + 1024;
    dst_rect.lr.y = dst_rect.ul.y + 1024;
    PgContextBlit(ball, &src_rect, NULL, &dst_rect);

    sprintf(text, "fps = %d", fps);

    PgSetFillColor(PgRGBA(0, 0, 0, 0));
    PgDrawIRect(0, 0, 1024, 15, Pg_DRAW_FILL);

    DrawSimpleText(0, 13, text);

    frames++;

    PgFlush();
    PgWaitHWIdle();

    return 0;
}

int create_window(int width, int height)
{
    PhDim_t winsize;
    PtArg_t winargs[32];
    uint32_t winargc = 0;
    int32_t status;

    PtSetArg(&winargs[winargc++], Pt_ARG_WINDOW_TITLE, "Stretch blit regression test 2", 0);
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

    /* Flush all widget operations again */
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

int main()
{
    struct timespec starttime;
    struct timespec currtime;
    int timeelapsed;

    PtInit(NULL);

    create_window(1024, 1040);

    if (init_draw()==-1)
    {
        fprintf(stderr, "Can't create offscreen images\n");
        return 1;
    }

    clock_gettime(CLOCK_REALTIME, &starttime);

    do {
        clock_gettime(CLOCK_REALTIME, &currtime);
        timeelapsed=(currtime.tv_sec-starttime.tv_sec)*1000 + (currtime.tv_nsec-starttime.tv_nsec)/1000000L;
        if (timeelapsed>1000)
        {
            clock_gettime(CLOCK_REALTIME, &starttime);
            fps=frames;
            frames=0;
        }

        /* Update offscreen video memory each frame */
        /* pretend that it is a new video frame     */
        memcpy(ball_ptr, ball_copy_ptr, ball_copy_size);

        draw();
        handle_window_event();
    } while(1);

    return 0;
}
