/*
Copyright (c) 2012 Carsten Burstedde, Donna Calhoun
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

#include "amr_single_step.h"
#include "amr_forestclaw.H"

static
    void cb_single_step(fclaw2d_domain_t *domain,
                        fclaw2d_patch_t *this_patch,
                        int this_block_idx,
                        int this_patch_idx,
                        void *user)
{
    single_step_data_t *ss_data = (single_step_data_t *) user;

    double dt = ss_data->dt;
    double t = ss_data->t;

    fclaw2d_domain_data_t *ddata = get_domain_data(domain);
    double maxcfl = (ddata->f_single_step_update_patch_ptr)(domain,this_patch,
                                                            this_block_idx,
                                                            this_patch_idx,t,dt);
    ss_data->maxcfl = max(maxcfl,ss_data->maxcfl);
}


/* ---------------------------------------------------
   Advance the level using a single explicit time step
   Assumption is that the call back function 'cb_single_step'
   can update each patch independently of all the other
   patches at the level.
   This function is analogous to the MOL step solver
   fclaw_mol_step.cpp in that upon return, all the patches at
   the given level have been updated at the new time.
   --------------------------------------------------- */
double amr_single_step_level(fclaw2d_domain_t *domain,
                             int level,
                             double t, double dt)
{

    /* Iterate over every patch at this level */
    single_step_data_t ss_data;
    ss_data.t = t;
    ss_data.dt = dt;
    ss_data.maxcfl = 0;
    fclaw2d_domain_iterate_level(domain, level, cb_single_step,(void *) &ss_data);

    return ss_data.maxcfl;
}
