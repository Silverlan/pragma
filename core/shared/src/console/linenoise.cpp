// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#ifdef __linux__
#include "pragma/console/linenoise.hpp"
#include "pragma/engine.h"
#include <linenoise.h>

static void completion(const char *buf, linenoiseCompletions *lc);
static const char *hints(const char *buf, int *color, int *bold);
void pragma::console::impl::init_linenoise()
{
    char *line;
    int async = 1;

    /* Set the completion callback. This will be called every time the
     * user uses the <tab> key. */
    linenoiseSetCompletionCallback(completion);
    linenoiseSetHintsCallback(hints);
    linenoiseSetMultiLine(0);

    /* Load history from file. The history file is just a plain text file
     * where entries are separated by newlines. */
    linenoiseHistoryLoad("history.txt"); /* Load the history at startup */

    /* Now this is the main loop of the typical linenoise-based application.
     * The call to linenoise() will block as long as the user types something
     * and presses enter.
     *
     * The typed string is returned as a malloc() allocated string by
     * linenoise, so the user needs to free() it. */
}

void pragma::console::impl::update_linenoise() {
    static struct linenoiseState ls;
    static char buf[1024];
    static auto first = true;
    if (first) {
        first = false;
        linenoiseEditStart(&ls,-1,-1,buf,sizeof(buf),"> ");
    }

    fd_set readfds;
    struct timeval tv;
    int retval;

    FD_ZERO(&readfds);
    FD_SET(ls.ifd, &readfds);
    tv.tv_sec = 0; // 1 sec timeout
    tv.tv_usec = 0;

    retval = select(ls.ifd+1, &readfds, NULL, NULL, &tv);
    if (retval == -1) {
        perror("select()");
        exit(1);
    } else if (retval) {
        auto *line = linenoiseEditFeed(&ls);
        /* A NULL return means: line editing is continuing.
        * Otherwise the user hit enter or stopped editing
        * (CTRL+C/D). */
        if (line != linenoiseEditMore) {
            linenoiseEditStop(&ls);

            pragma::get_engine()->ConsoleInput(line);
            linenoiseHistoryAdd(line);

            //free(line); // TODO?
            linenoiseEditStart(&ls,-1,-1,buf,sizeof(buf),"> ");
        }
    } else {
        // Timeout occurred
        //       static int counter = 0;
        //      linenoiseHide(&ls);
        //printf("Async output %d.\n", counter++);
        //        linenoiseShow(&ls);
    }
}

void completion(const char *buf, linenoiseCompletions *lc) {
    if (buf[0] == 'h') {
        linenoiseAddCompletion(lc,"hello");
        linenoiseAddCompletion(lc,"hello there");
    }
}

const char *hints(const char *buf, int *color, int *bold) {
    if (!strcasecmp(buf,"hello")) {
        *color = 35;
        *bold = 0;
        return " World";
    }
    return NULL;
}

#endif
