#include "dump1090.h"
#include "structs.h"
#include "parula.h"
#include "SDL/SDL_gfxPrimitives.h"

static uint64_t mstime(void) {
    struct timeval tv;
    uint64_t mst;

    gettimeofday(&tv, NULL);
    mst = ((uint64_t)tv.tv_sec)*1000;
    mst += tv.tv_usec/1000;
    return mst;
}

SDL_Color pink = {249,38,114,255};
SDL_Color blue = {102,217,239,255};
SDL_Color green = {166,226,46,255};
SDL_Color yellow = {253,250,31,255};
SDL_Color orange = {253,151,31,255};
SDL_Color lightGrey = {196,196,196,255};
SDL_Color grey = {127,127,127,255};
SDL_Color darkGrey = {64,64,64,255};
SDL_Color black = {0,0,0,255};
SDL_Color white = {255,255,255,255};

void drawList() {
	struct aircraft *a = Modes.aircrafts;
    time_t now = time(NULL);
    int count = 0;
    char progress;
    char spinner[4] = "|/-\\";

    // Refresh screen every (MODES_INTERACTIVE_REFRESH_TIME) miliseconde
    if ((mstime() - Modes.interactive_last_update) < MODES_INTERACTIVE_REFRESH_TIME)
       {return;}

    Modes.interactive_last_update = mstime();

    progress = spinner[time(NULL)%4];

    SDL_FillRect(game.screen, NULL, 0);

	drawStringBG(" Flight  Alt(m) km/h D(km) H  S ", 0, 0, game.listFont, black, white);

	// int xstride = 10;
	// for(int i = 0; i < 320 / xstride; i++) {
	// 	vlineRGBA (game.screen, i*xstride, 0, SCREEN_HEIGHT, 127, 127, 127, SDL_ALPHA_OPAQUE);		
	// }

	// int ystride = 20;
	// for(int i = 0; i < 240 / ystride; i++) {	
	// 	hlineRGBA (game.screen, 0, SCREEN_WIDTH, i * ystride, 127, 127, 127, SDL_ALPHA_OPAQUE);
	// }

    int numNoDir = 0;
    while(a && (count < 10)) {
        if ((now - a->seen) < Modes.interactive_display_ttl)
            {
            int msgs  = a->messages;
            int flags = a->modeACflags;

            if ( (((flags & (MODEAC_MSG_FLAG                             )) == 0                    )                 )
              || (((flags & (MODEAC_MSG_MODES_HIT | MODEAC_MSG_MODEA_ONLY)) == MODEAC_MSG_MODEA_ONLY) && (msgs > 4  ) ) 
              || (((flags & (MODEAC_MSG_MODES_HIT | MODEAC_MSG_MODEC_OLD )) == 0                    ) && (msgs > 127) ) 
              ) {
                int altitude = a->altitude, speed = a->speed;
                char strSquawk[5] = " ";
                char strFl[6]     = " ";
                char strTt[5]     = " ";
                char strGs[5]     = " ";

                // Convert units to metric if --metric was specified
                //if (Modes.metric) {
                    altitude = (int) (altitude / 3.2828);
                    speed    = (int) (speed    * 1.852);
                //}

                if (a->bFlags & MODES_ACFLAGS_SQUAWK_VALID) {
                    snprintf(strSquawk,5,"%04x", a->modeA);}

                if (a->bFlags & MODES_ACFLAGS_SPEED_VALID) {
                    snprintf (strGs, 5,"%3d", speed);}

                if (a->bFlags & MODES_ACFLAGS_HEADING_VALID) {
                    snprintf (strTt, 5,"%03d", a->track);}

                if (msgs > 99999) {
                    msgs = 99999;}

                  // Dump1090 display mode
                char strMode[5]               = "    ";
                // char strLat[8]                = " ";
                // char strLon[9]                = " ";

                char strD[5]                = " ";
                char strDir[3]                = " ";                
                char cLat = ' ';
                char cLon = ' ';

                char strS[3] = " ";
                double d;

                unsigned char * pSig       = a->signalLevel;
                unsigned int signalAverage = (pSig[0] + pSig[1] + pSig[2] + pSig[3] + 
                                              pSig[4] + pSig[5] + pSig[6] + pSig[7] + 3) >> 3;   

                if (a->bFlags & MODES_ACFLAGS_AOG) {
                    snprintf(strFl, 6," grnd");
                } else if (a->bFlags & MODES_ACFLAGS_ALTITUDE_VALID) {
                    snprintf(strFl, 6, "%5d", altitude);
                }

                if ((flags & MODEAC_MSG_FLAG) == 0) {
                    strMode[0] = 'S';
                } else if (flags & MODEAC_MSG_MODEA_ONLY) {
                    strMode[0] = 'A';
                }
                if (flags & MODEAC_MSG_MODEA_HIT) {strMode[2] = 'a';}
                if (flags & MODEAC_MSG_MODEC_HIT) {strMode[3] = 'c';}

                snprintf(strS, 3,"%3d", (int)((float)signalAverage/25.0f));

                if (a->bFlags & MODES_ACFLAGS_LATLON_VALID) {

                    if(fabs(a->dx) < .01 && fabs(a->dy) > fabs(a->dx)) {
                        cLon = ' ';
                    } else {
                        if(a->dx < 0) {
                            cLon = 'W';
                        } else {
                            cLon = 'E';
                        }      
                    }
                 
                   if(fabs(a->dy) < .01 && fabs(a->dx) > fabs(a->dy)) {
                        cLat = ' ';
                    } else {
                        if(a->dy < 0) {
                            cLat = 'S';
                        } else {
                            cLat = 'N';
                        }      
                    }

                    snprintf(strDir,3,"%c%c",cLat,cLon);

                    d = sqrt(a->dx * a->dx + a->dy * a->dy);

                    snprintf(strD, 5,"%4.01f", d);

                    if ((now - a->seen) > 30 ) {

                		drawString(a->flight, 0, (count + 1) * 20, game.listFont, (SDL_Color){96,96,96,255});

                        // printf("\x1B[1;30m%-8s%5s %4s %5s  %c%c %d",
                        //     a->flight, 
                        //     strFl, 
                        //     strGs,
                        //     strD, 
                        //     cLat, cLon,
                        //     (int)((float)signalAverage/25.0f));
                    } else {
                		drawString(a->flight, 10, (count + 1) * 20, game.listFont, pink);

                		drawString(strFl, 90, (count + 1) * 20, game.listFont, orange);

                		drawString(strGs, 160, (count + 1) * 20, game.listFont, green);

                		drawString(strD, 210, (count + 1) * 20, game.listFont, blue);

                		drawString(strDir, 270, (count + 1) * 20, game.listFont, yellow);

                		// drawString(strS, 290, (count + 1) * 20, game.listFont, (SDL_Color){255,9,96,255});                		

                        // printf("\x1B[1;31m%-8s\x1B[1;32m%5s \x1B[1;33m%4s \x1B[1;34m%5s  \x1B[1;36m%c%c \x1B[1;35m%d",
                        //     a->flight, 
                        //     strFl, 
                        //     strGs,
                        //     strD, 
                        //     cLat, cLon,
                        //     (int)((float)signalAverage/25.0f));
                    }
                    count++;
                } else {
                    numNoDir++;
                }
            }
        }
        a = a->next;
    }

    SDL_Flip(game.screen);

}