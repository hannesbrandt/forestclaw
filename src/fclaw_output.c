/*
Copyright (c) 2012-2022 Carsten Burstedde, Donna Calhoun, Scott Aiton
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <fclaw_output.h>
#include <fclaw_restart.h>
#include <fclaw_global.h>
#include <fclaw_options.h>
#include <fclaw_vtable.h>

/* -----------------------------------------------------------------------
    Public interface
    -------------------------------------------------------------------- */

void
fclaw_output_frame (fclaw_global_t * glob, int iframe)
{
    const fclaw_options_t *fclaw_opt = fclaw_get_options(glob);

    double time;
    time = glob->curr_time;


    if (fclaw_opt->output != 0)
    {
        fclaw_vtable_t *vt = fclaw_vt(glob);

        FCLAW_ASSERT(vt->output_frame != NULL);

        /* Record output time */
        fclaw_timer_start (&glob->timers[FCLAW_TIMER_OUTPUT]);

        /* User or solver set output file */
        fclaw_global_essentialf("Output Frame %4d  at time %16.8e\n\n",
                                iframe,time);  
        vt->output_frame(glob,iframe);

        /* Record output time */
        fclaw_timer_stop (&glob->timers[FCLAW_TIMER_OUTPUT]);
    }
    else
    {
        fclaw_global_essentialf("Time step %4d  at time %16.8e\n\n",
                                iframe,time);          
    }


    if (fclaw_opt->tikz_out)
    {
        fclaw2d_output_frame_tikz(glob,iframe);
    }

    if(fclaw_opt->restart_out)
    {
        fclaw_timer_start (&glob->timers[FCLAW_TIMER_OUTPUT]);

        fclaw_restart_output_frame(glob,iframe);

        fclaw_timer_stop (&glob->timers[FCLAW_TIMER_OUTPUT]);
    }
}




