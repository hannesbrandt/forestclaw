/*
Copyright (c) 2012-2023 Carsten Burstedde, Donna Calhoun
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

#include "sphere_user.h"

#include <fclaw_include_all.h>

#include <fclaw_clawpatch_pillow.h>
#include <fclaw_clawpatch.h>
#include <fclaw_clawpatch_options.h>

#include <fclaw2d_clawpatch_fort.h>

#include <fc2d_clawpack46.h> 
#include <fc2d_clawpack46_fort.h>
#include <fc2d_clawpack46_options.h>
#include <clawpack46_user_fort.h>




#if 0
// Fix syntax highlighting
#endif


static
void sphere_problem_setup(fclaw_global_t* glob)
{
    const user_options_t* user = sphere_get_options(glob);
    const fclaw_options_t* fclaw_opt = fclaw_get_options(glob);

    if (glob->mpirank == 0)
    {
        FILE *f = fopen("setprob_2d.data","w");
        fprintf(f,  "%-24d   %s",user->example,"\% example\n");
        fprintf(f,  "%-24.16f   %s",user->gravity,"\% gravity\n");
        fprintf(f,  "%-24d   %s",user->mapping,"\% mapping\n");
        fprintf(f,  "%-24d   %s",user->init_cond,"\% initial_condition\n");        

        fprintf(f,  "%-24.16f   %s",user->hin,"\% hin\n");
        fprintf(f,  "%-24.16f   %s",user->hout,"\% hout\n");

        fprintf(f,  "%-24.16f   %s",user->ring_inner,"\% ring-inner\n");
        fprintf(f,  "%-24.16f   %s",user->ring_outer,"\% ring-outer\n");
        fprintf(f,  "%-24d   %s",user->ring_units,"\% ring_units\n");

        fprintf(f,  "%-24.16f   %s",user->latitude[0],"\% latitude\n");
        fprintf(f,  "%-24.16f   %s",user->latitude[1],"\% latitude\n");
        fprintf(f,  "%-24.16f   %s",user->longitude[0],"\% longitude\n");
        fprintf(f,  "%-24.16f   %s",user->longitude[1],"\% longitude\n");

        fprintf(f,  "%-24.16f   %s",user->center[0],"\% center[0]\n");
        fprintf(f,  "%-24.16f   %s",user->center[1],"\% center[1]\n");
        fprintf(f,  "%-24.16f   %s",user->center[2],"\% center[2]\n");

        fprintf(f,  "%-24.16f   %s",user->theta_ridge,"\% theta_ridge\n");
        fprintf(f,  "%-24.16f   %s",user->ampl, "\% ampl\n");
        fprintf(f,  "%-24.16f   %s",user->alpha,"\% alpha\n");
        fprintf(f,  "%-24.16f   %s",user->speed,"\% speed\n");
        fprintf(f,  "%-24.16f   %s",user->bathy[0],"\% bathy[0]\n");
        fprintf(f,  "%-24.16f   %s",user->bathy[1],"\% bathy[1]\n");

        fprintf(f,  "%-24.16f   %s",fclaw_opt->refine_threshold,"\% refine_threshold\n");
        fprintf(f,  "%-24.16f   %s",fclaw_opt->coarsen_threshold,"\% coarsen_threshold\n");

        fclose(f);
    }
    fclaw_domain_barrier (glob->domain);
    SETPROB();
}

static
void sphere_patch_setup_manifold(fclaw_global_t *glob,
                                 fclaw_patch_t *patch,
                                 int blockno,
                                 int patchno)
{
    //const user_options_t *user = sphere_get_options(glob);

    int mx, my, mbc;
    double xlower, ylower, dx,dy;
    fclaw_clawpatch_2d_grid_data(glob,patch,&mx,&my,&mbc,
                                &xlower,&ylower,&dx,&dy);

    int maux;
    double *aux;
    fclaw_clawpatch_aux_data(glob,patch,&aux,&maux);

    double *xd,*yd,*zd,*area;
    double *xp,*yp,*zp;
    fclaw_clawpatch_2d_metric_data(glob,patch,&xp,&yp,&zp,
                                  &xd,&yd,&zd,&area);

    double *xnormals, *ynormals,*xtangents,*ytangents;
    double *surfnormals, *edgelengths, *curvature;
    fclaw_clawpatch_2d_metric_data2(glob,patch,
                                   &xnormals,&ynormals,
                                   &xtangents,&ytangents,
                                   &surfnormals,
                                   &edgelengths,&curvature);


    CLAWPACK46_SET_BLOCK(&blockno);
    SPHERE_SETAUX(&mx,&my,&mbc,&xlower,&ylower,
                  &dx,&dy,area,xnormals,ynormals,
                  xtangents,ytangents,surfnormals, curvature,
                  edgelengths,
                  aux, &maux);
    CLAWPACK46_UNSET_BLOCK();
}

static
void cb_sphere_output_ascii(fclaw_domain_t *domain,
                             fclaw_patch_t *patch,
                             int blockno, int patchno,
                             void *user)
{
    fclaw_global_iterate_t* s = (fclaw_global_iterate_t*) user;
    fclaw_global_t *glob = (fclaw_global_t*) s->glob;

    int iframe = *((int *) s->user);    

    /* Get info not readily available to user */
    int local_num, global_num, level;
    fclaw_patch_get_info(glob->domain,patch,
                           blockno,patchno,
                           &global_num, 
                           &local_num,&level);

    int mx,my,mbc;
    double xlower,ylower,dx,dy;
    fclaw_clawpatch_2d_grid_data(glob,patch,&mx,&my,&mbc,
                                &xlower,&ylower,&dx,&dy);

    double *q;
    int meqn;
    fclaw_clawpatch_soln_data(glob,patch,&q,&meqn);

    double *aux;
    int maux;
    fclaw_clawpatch_aux_data(glob,patch,&aux,&maux);

    SPHERE_FORT_WRITE_FILE(&mx,&my,&meqn, &maux,&mbc,&xlower,&ylower,
                                 &dx,&dy,q,aux,&iframe,&global_num,&level,
                                 &blockno,&glob->mpirank);
}

static
void sphere_header_ascii(fclaw_global_t* glob,int iframe)
{
    double time = glob->curr_time;
    int ngrids = glob->domain->global_num_patches;

    const fclaw_clawpatch_options_t *clawpatch_opt = fclaw_clawpatch_get_options(glob);
    int meqn = clawpatch_opt->meqn;
    int maux = clawpatch_opt->maux;

    SPHERE_FORT_WRITE_HEADER(&iframe,&time,&meqn,&maux,&ngrids);
}

void sphere_link_solvers(fclaw_global_t *glob)
{
    /* ForestClaw core functions */
    fclaw_vtable_t *vt = fclaw_vt(glob);
    vt->problem_setup = &sphere_problem_setup;  /* Version-independent */
    //vt->output_frame = &sphere_output;

    fclaw_patch_vtable_t *patch_vt = fclaw_patch_vt(glob);
    patch_vt->setup   = &sphere_patch_setup_manifold;


    const user_options_t* user_opt = sphere_get_options(glob);
    if (user_opt->mapping == 1)
        fclaw_clawpatch_use_pillowsphere(glob);

    if (user_opt->claw_version == 4)
    {
        fc2d_clawpack46_vtable_t  *clawpack46_vt = fc2d_clawpack46_vt(glob);
        // clawpack46_vt->b4step2        = sphere_b4step2;
        clawpack46_vt->fort_qinit     = CLAWPACK46_QINIT;

        fc2d_clawpack46_options_t *clawpack46_opt = fc2d_clawpack46_get_options(glob);

        if (clawpack46_opt->use_fwaves)
        {
            clawpack46_vt->fort_rpn2 = &CLAWPACK46_RPN2_FWAVE; 
            clawpack46_vt->fort_rpt2 = &CLAWPACK46_RPT2_FWAVE;                  
        }
        else
        {
            clawpack46_vt->fort_rpn2 = &CLAWPACK46_RPN2;
            clawpack46_vt->fort_rpt2 = &CLAWPACK46_RPT2;            
        }

        //clawpack46_vt->fort_rpn2_cons = &RPN2CONS_UPDATE_MANIFOLD;

        /* Clawpatch functions */    
        fclaw_clawpatch_vtable_t *clawpatch_vt = fclaw_clawpatch_vt(glob);
        clawpatch_vt->d2->fort_user_exceeds_threshold = &USER_EXCEEDS_THRESHOLD;

        clawpatch_vt->time_header_ascii = &sphere_header_ascii;
        clawpatch_vt->cb_output_ascii   = &cb_sphere_output_ascii;

    }
    else
    {
        printf("Clawpack 5 not yet implemented\n");
    }
 }
