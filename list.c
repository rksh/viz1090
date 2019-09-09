#include "dump1090.h"
#include "structs.h"
#include "parula.h"
#include "monokai.h"
#include "SDL2/SDL2_gfxPrimitives.h"

void drawList(int top) {
	struct aircraft *a = Modes.aircrafts;
    time_t now = time(NULL);
    int count = 0;
    char progress;
    char spinner[4] = "|/-\\";

    progress = spinner[time(NULL)%4];

	drawStringBG(" Flight  Alt(m) km/h D(km) H  S ", 0, top, appData.mapBoldFont, black, (SDL_Color){255,255,255,64});

    top = top + appData.mapFontHeight;

    int numNoDir = 0;
    while(a) {

        if(top > appData.screen_height) {
            return;
        }

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

                    if(fabs(a->lon) < .01 && fabs(a->lat) > fabs(a->lon)) {
                        cLon = ' ';
                    } else {
                        if(a->lon < 0) {
                            cLon = 'W';
                        } else {
                            cLon = 'E';
                        }      
                    }
                 
                   if(fabs(a->lat) < .01 && fabs(a->lon) > fabs(a->lat)) {
                        cLat = ' ';
                    } else {
                        if(a->lat < 0) {
                            cLat = 'S';
                        } else {
                            cLat = 'N';
                        }      
                    }

                    //snprintf(strDir,3,"%c%c",cLat,cLon);

                    //distance is borked during refactor

                    //d = sqrt(a->dx * a->dx + a->dy * a->dy);

                    //snprintf(strD, 5,"%4.01f", d);

                    if ((now - a->seen) > 30 ) {

                		drawString(a->flight, 0, top, appData.listFont, (SDL_Color){96,96,96,255});

                    } else {
                		drawString(a->flight, 10, top, appData.listFont, pink);

                		drawString(strFl, 90, top, appData.listFont, orange);

                		drawString(strGs, 160, top, appData.listFont, green);

                		drawString(strD, 210, top, appData.listFont, blue);

                		drawString(strDir, 270, top, appData.listFont, yellow);

                		// drawString(strS, 290, (count + 1) * 20, appData.listFont, (SDL_Color){255,9,96,255});                		

                        // printf("\x1B[1;31m%-8s\x1B[1;32m%5s \x1B[1;33m%4s \x1B[1;34m%5s  \x1B[1;36m%c%c \x1B[1;35m%d",
                        //     a->flight, 
                        //     strFl, 
                        //     strGs,
                        //     strD, 
                        //     cLat, cLon,
                        //     (int)((float)signalAverage/25.0f));
                    }
                    top = top + appData.mapFontHeight;
                    count++;
                } else {
                    numNoDir++;
                }
            }
        }
        a = a->next;
    }
}
