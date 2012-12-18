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

#include "amr_utils.H"
#include <p4est_base.h>
#include "amr_single_step.H"
#include "amr_mol.H"

int pow_int(int a, int n)
{
    int b = 1;
    for(int i = 0; i < n; i++)
    {
        b *= a;
    }
    return b;
}

/* -----------------------------------------------------------------
   Initialize data
   ----------------------------------------------------------------- */

// Should this maybe be 'allocate_domain_data' instead?  Reserve 'init'
// assigning default values to items?
void init_domain_data(fclaw2d_domain_t *domain)
{
    fclaw2d_domain_data_t *ddata = FCLAW2D_ALLOC_ZERO (fclaw2d_domain_data_t, 1);
    domain->user = (void *) ddata;

    ddata->amropts = NULL;
    ddata->curr_time = 0;

    /* Single step solver that is called. */
    ddata->f_single_step_level_ptr = &amr_single_step_level;

    /* Callback for single step solver */
    ddata->f_single_step_update_patch_ptr = NULL;

    /* Interface to (Fortran) ODE solver that is called */
    ddata->f_ode_solver_level_ptr = NULL;

    /* Right hand side for an MOL solver */
    ddata->f_ode_solver_rhs_patch_ptr = NULL;

    /* Setup patch */
    ddata->f_patch_setup_ptr = NULL;

    /* Initialize patch */
    ddata->f_patch_initialize_ptr = NULL;

    /* Boundary conditions */
    ddata->f_patch_physbc_ptr = NULL;

}


void init_block_data(fclaw2d_block_t *block)
{
    fclaw2d_block_data_t *bdata = FCLAW2D_ALLOC_ZERO (fclaw2d_block_data_t, 1);
    block->user = (void *) bdata;
}


void init_patch_data(fclaw2d_patch_t *patch)
{
    fclaw2d_patch_data_t *pdata = FCLAW2D_ALLOC(fclaw2d_patch_data_t, 1);
    patch->user = (void *) pdata;
}

// -----------------------------------------------------------------
// Return pointer to user data
// -----------------------------------------------------------------
fclaw2d_domain_data_t *get_domain_data(fclaw2d_domain_t *domain)
{
    return (fclaw2d_domain_data_t *) domain->user;
}


fclaw2d_block_data_t *get_block_data(fclaw2d_block_t *block)
{
    return (fclaw2d_block_data_t *) block->user;
}


fclaw2d_patch_data_t *get_patch_data(fclaw2d_patch_t *patch)
{
    return (fclaw2d_patch_data_t *) patch->user;
}


// -----------------------------------------------------------------
// Set user data with user defined variables, etc.
// -----------------------------------------------------------------
void set_domain_data(fclaw2d_domain_t *domain, const amr_options_t *gparms)
{
    fclaw2d_domain_data_t *ddata = get_domain_data(domain);
    ddata->amropts = gparms;
}

void copy_domain_data(fclaw2d_domain_t *old_domain, fclaw2d_domain_t *new_domain)
{
    fclaw2d_domain_data_t *ddata_old = get_domain_data(old_domain);

    /* Has the data already been allocated? */
    fclaw2d_domain_data_t *ddata_new = get_domain_data(new_domain);

    /* Copy data members */
    ddata_new->amropts = ddata_old->amropts;
    ddata_new->curr_time = ddata_old->curr_time;

    ddata_new->f_single_step_level_ptr = ddata_old->f_single_step_level_ptr;
    ddata_new->f_single_step_update_patch_ptr = ddata_old->f_single_step_update_patch_ptr;

    ddata_new->f_ode_solver_level_ptr = ddata_old->f_ode_solver_level_ptr;
    ddata_new->f_ode_solver_rhs_patch_ptr = ddata_old->f_ode_solver_rhs_patch_ptr;

    ddata_new->f_patch_setup_ptr = ddata_old->f_patch_setup_ptr;
    ddata_new->f_patch_initialize_ptr = ddata_old->f_patch_initialize_ptr;
    ddata_new->f_patch_physbc_ptr = ddata_old->f_patch_physbc_ptr;
}





void set_block_data(fclaw2d_block_t *block, const int mthbc[])
{
    fclaw2d_block_data_t *bdata = get_block_data(block);
    for(int i = 0; i < 4; i++)
    {
        bdata->mthbc[i] = mthbc[i];
    }
}

void set_patch_data(fclaw2d_patch_t *patch, ClawPatch* cp)
{
    fclaw2d_patch_data_t *pdata = get_patch_data(patch);
    pdata->cp = cp;
}


// -----------------------------------------------------------------
// Some lazy helper functions that really do make things easier..
// -----------------------------------------------------------------
void allocate_user_data(fclaw2d_domain_t *domain)
{
    fclaw2d_block_t *block;
    fclaw2d_patch_t *patch;

    init_domain_data(domain);

    for (int i = 0; i < domain->num_blocks; i++)
    {
        block = &domain->blocks[i];
        init_block_data(block);
        for (int j = 0; j < block->num_patches; j++)
        {
            patch = &block->patches[j];
            init_patch_data(patch);
        }
    }
}


const amr_options_t* get_domain_parms(fclaw2d_domain_t *domain)
{
    fclaw2d_domain_data_t *ddata = get_domain_data (domain);
    return ddata->amropts;
}

void set_domain_time(fclaw2d_domain_t *domain, double time)
{
    fclaw2d_domain_data_t *ddata = get_domain_data (domain);
    ddata->curr_time = time;
}

double get_domain_time(fclaw2d_domain_t *domain)
{
    fclaw2d_domain_data_t *ddata = get_domain_data (domain);
    return ddata->curr_time;
}

// Will change the name of this to 'get_clawpatch' eventually
ClawPatch* get_clawpatch(fclaw2d_patch_t *patch)
{
    fclaw2d_patch_data_t *pdata = (fclaw2d_patch_data_t *) patch->user;

    return pdata->cp;
}
/* end of helper functions */

/*
const int get_refratio(fclaw2d_domain_t *domain)
{
    const amr_options_t* gparms = get_domain_parms(domain);
    return gparms->refratio;
}
*/

// int corners_per_patch = FCLAW_CORNERS_PER_PATCH;

/*
const int get_corners_per_patch(fclaw2d_domain_t *domain)
{
    // Number of patch corners, not the number of corners in the domain!
    return fclaw2d_domain_num_corners(domain);
}
*/

/*
const int get_faces_per_patch(fclaw2d_domain_t *domain)
{
    // Number of faces per patch, not the total number of faces in the domain!
    return fclaw2d_domain_num_faces(domain);
}
*/

/*
const int get_siblings_per_patch(fclaw2d_domain_t *domain)
{
    // Number of patch corners, not the number of corners in the domain!
    return fclaw2d_domain_num_corners(domain);
}
*/

/*
const int get_p4est_refineFactor(fclaw2d_domain_t *domain)
{
    return fclaw2d_domain_num_face_corners(domain);
}
*/




static void cb_num_patches(fclaw2d_domain_t *domain,
	fclaw2d_patch_t *patch, int block_no, int patch_no, void *user)
{
  (*(int *) user)++;
}

int num_patches(fclaw2d_domain_t *domain, int level, int include_shadow)
{
    int count = 0;
    if (include_shadow == 0)
    {
        fclaw2d_domain_iterate_level(domain, level,
                                     cb_num_patches,
                                     &count);
    }
    else
    {
        // Include shadow patches
    }
    return count;
}

/* Functions with C prototypes to use forestclaw from C code */

void
fclaw_mpi_init (int * argc, char *** argv, MPI_Comm mpicomm, int lp)
{
    int mpiret;

    mpiret = MPI_Init (argc, argv);
    SC_CHECK_MPI (mpiret);

    sc_init (mpicomm, 0, 0, NULL, lp);
    p4est_init (NULL, lp);
}

void
fclaw_mpi_finalize (void)
{
    int mpiret;

    sc_finalize ();

    mpiret = MPI_Finalize ();
    SC_CHECK_MPI (mpiret);
}

/*
void
fclaw2d_allocate_domain_data (fclaw2d_domain_t * domain,
                              amr_options_t * gparms,
                              fclaw2d_level_advance_t level_advance_cb,
                              fclaw2d_single_step_patch_t
                              single_step_patch_cb)
{
  allocate_user_data(domain);

  fclaw2d_domain_data_t *ddata = get_domain_data(domain);
  ddata->amropts = gparms;

  ddata->f_level_advance = level_advance_cb;
  ddata->f_single_step_patch = single_step_patch_cb;
}
*/
