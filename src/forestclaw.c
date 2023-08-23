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

#include <forestclaw.h>
#include <p4est.h>
#include <p8est.h>

void
fclaw_domain_attribute_add (fclaw_domain_t * domain,
                              const char *name, void *attribute)
{
    sc_keyvalue_t *a = domain->attributes;

    FCLAW_ASSERT (a != NULL);
    FCLAW_ASSERT (!sc_keyvalue_exists (a, name));
    sc_keyvalue_set_pointer (a, name, attribute);
}

void *
fclaw_domain_attribute_access (fclaw_domain_t * domain,
                                 const char *name, void *default_attr)
{
    sc_keyvalue_t *a = domain->attributes;

    FCLAW_ASSERT (a != NULL);
    return sc_keyvalue_get_pointer (a, name, default_attr);
}

void
fclaw_domain_attribute_remove (fclaw_domain_t * domain, const char *name)
{
    sc_keyvalue_t *a = domain->attributes;
#ifdef FCLAW_ENABLE_DEBUG
    sc_keyvalue_entry_type_t et;
#endif

    FCLAW_ASSERT (a != NULL);
    FCLAW_ASSERT (sc_keyvalue_exists (a, name));
#ifndef FCLAW_ENABLE_DEBUG
    (void)
#else
    et =
#endif
        sc_keyvalue_unset (a, name);
    FCLAW_ASSERT (et == SC_KEYVALUE_ENTRY_POINTER);
}

void
fclaw_domain_iterate_level (fclaw_domain_t * domain, int level,
                              fclaw_patch_callback_t pcb, void *user)
{
    int i, j;
    fclaw_block_t *block;
    fclaw_patch_t *patch;

    FCLAW_ASSERT (0 <= level && level <= domain->possible_maxlevel);
    for (i = 0; i < domain->num_blocks; ++i)
    {
        block = domain->blocks + i;
        for (patch = block->patchbylevel[level];
             patch != NULL; patch = patch->u.next)
        {
            j = (int) (patch - block->patches);
            FCLAW_ASSERT (0 <= j && j < block->num_patches);
            FCLAW_ASSERT (patch->level == level);
            pcb (domain, patch, i, j, user);
        }
    }
}

void
fclaw_domain_iterate_patches (fclaw_domain_t * domain,
                                fclaw_patch_callback_t pcb, void *user)
{
    int i, j;
    fclaw_block_t *block;
    fclaw_patch_t *patch;

    for (i = 0; i < domain->num_blocks; i++)
    {
        block = domain->blocks + i;
        for (j = 0; j < block->num_patches; j++)
        {
            patch = block->patches + j;
            pcb (domain, patch, i, j, user);
        }
    }
}

void
fclaw_domain_iterate_families (fclaw_domain_t * domain,
                                 fclaw_patch_callback_t pcb, void *user)
{
    int i, j;
    fclaw_block_t *block;
    fclaw_patch_t *patch;

    for (i = 0; i < domain->num_blocks; i++)
    {
        block = domain->blocks + i;
        for (j = 0; j < block->num_patches; j++)
        {
            patch = block->patches + j;
            if (fclaw_patch_is_first_sibling (patch))
            {
#ifdef FCLAW_ENABLE_DEBUG
                int k;
                for (k = 0; k < fclaw_domain_num_children(domain); ++k)
                {
                    FCLAW_ASSERT (j + k < block->num_patches);
                    FCLAW_ASSERT (fclaw_patch_childid (patch + k) == k);
                }
#endif
                pcb (domain, patch, i, j, user);
                j += fclaw_domain_num_children(domain) - 1;
            }
        }
    }
}

int
fclaw_domain_dimension (const fclaw_domain_t * domain)
{
    return domain->dim;
}

int
fclaw_domain_refine_factor (const fclaw_domain_t * domain)
{
    return (domain->dim == 2) ? P4EST_CHILDREN/2 : P8EST_CHILDREN/2;
}

int
fclaw_domain_num_children (const fclaw_domain_t * domain)
{
    return (domain->dim == 2) ? P4EST_CHILDREN : P8EST_CHILDREN;
}

int
fclaw_domain_num_faces (const fclaw_domain_t * domain)
{
    return (domain->dim == 2) ? P4EST_FACES : P8EST_FACES;
}

int
fclaw_domain_num_edges (const fclaw_domain_t * domain)
{
    return (domain->dim == 2) ? 0 : P8EST_EDGES;
}

int
fclaw_domain_num_corners (const fclaw_domain_t * domain)
{
    return (domain->dim == 2) ? P4EST_CHILDREN : P8EST_CHILDREN;
}

int
fclaw_domain_num_face_corners (const fclaw_domain_t * domain)
{
    return (domain->dim == 2) ? P4EST_HALF : P8EST_HALF;
}

int
fclaw_domain_num_orientations (const fclaw_domain_t * domain)
{
    return (domain->dim == 2) ? (P4EST_FACES * P4EST_HALF) : (P8EST_FACES * P8EST_HALF);
}

void
fclaw_domain_edge_faces (const fclaw_domain_t * domain,
                         int iedge, int faces[P4EST_DIM])
{
    FCLAW_ASSERT (0 <= iedge && iedge < fclaw_domain_num_edges(domain));
    faces[0] = p8est_edge_faces[iedge][0];
    faces[1] = p8est_edge_faces[iedge][1];
}

void
fclaw_domain_corner_faces (const fclaw_domain_t * domain,
                             int icorner, int faces[P4EST_DIM])
{
    FCLAW_ASSERT (0 <= icorner && icorner < fclaw_domain_num_children(domain));
    if(domain->dim == 2)
    {
        faces[0] = p4est_corner_faces[icorner][0];
        faces[1] = p4est_corner_faces[icorner][1];
    }
    else
    {
        faces[0] = p8est_corner_faces[icorner][0];
        faces[1] = p8est_corner_faces[icorner][1];
        faces[2] = p8est_corner_faces[icorner][2];
    }
}

int
fclaw_patch_corner_dimension (const fclaw_patch_t * patch, int cornerno)
{
    const int childid = fclaw_patch_childid (patch);

    if(patch->dim == 2)
    {
        FCLAW_ASSERT (0 <= cornerno && cornerno < P4EST_CHILDREN);
        return (patch->level == 0 ||
                cornerno == childid ||
                cornerno == P4EST_CHILDREN - 1 - childid) ? 0 : 1;
    }
    else
    {
        FCLAW_ASSERT (0 <= cornerno && cornerno < P8EST_CHILDREN);
        return (patch->level == 0 ||
                cornerno == childid ||
                cornerno == P8EST_CHILDREN - 1 - childid) ? 0 : 1;
    }
}

int
fclaw_patch_childid (const fclaw_patch_t * patch)
{
    int childid;
    if(patch->dim == 2)
    {
        childid = patch->flags & FCLAW2D_PATCH_CHILDID;
        FCLAW_ASSERT (0 <= childid && childid < P4EST_CHILDREN);
    } 
    else 
    {
        childid = patch->flags & FCLAW3D_PATCH_CHILDID;
        FCLAW_ASSERT (0 <= childid && childid < P8EST_CHILDREN);
    }

    return childid;
}

int
fclaw_patch_is_first_sibling (const fclaw_patch_t * patch)
{
    if(patch->dim == 2)
    {
        return patch->flags & FCLAW2D_PATCH_FIRST_SIBLING ? 1 : 0;
    }
    else
    {
        return patch->flags & FCLAW3D_PATCH_FIRST_SIBLING ? 1 : 0;
    }
}

int
fclaw_patch_is_ghost (const fclaw_patch_t * patch)
{
    if(patch->dim == 2)
    {
        return patch->flags & FCLAW2D_PATCH_IS_GHOST ? 1 : 0;
    }
    else
    {
        return patch->flags & FCLAW3D_PATCH_IS_GHOST ? 1 : 0;
    }
}

#define FCLAW2D_DOMAIN_TAG_SERIALIZE 4526
#define FCLAW3D_DOMAIN_TAG_SERIALIZE 4527

int get_tag(int dim)
{
    if(dim == 2)
    {
        return FCLAW2D_DOMAIN_TAG_SERIALIZE;
    }
    else
    {
        return FCLAW3D_DOMAIN_TAG_SERIALIZE;
    }
}

double
fclaw_domain_global_maximum (fclaw_domain_t * domain, double d)
{
    int mpiret;
    double gd;

    mpiret = sc_MPI_Allreduce (&d, &gd, 1, sc_MPI_DOUBLE, sc_MPI_MAX,
                               domain->mpicomm);
    SC_CHECK_MPI (mpiret);

    return gd;
}

double
fclaw_domain_global_sum (fclaw_domain_t * domain, double d)
{
    int mpiret;
    double gd;

    mpiret = sc_MPI_Allreduce (&d, &gd, 1, sc_MPI_DOUBLE, sc_MPI_SUM,
                               domain->mpicomm);
    SC_CHECK_MPI (mpiret);

    return gd;
}

void
fclaw_domain_barrier (fclaw_domain_t * domain)
{
    int mpiret;

    mpiret = sc_MPI_Barrier (domain->mpicomm);
    SC_CHECK_MPI (mpiret);
}

void
fclaw_domain_serialization_enter (fclaw_domain_t * domain)
{
    int mpiret;
    int i;
    sc_MPI_Status status;

    if (domain->mpirank > 0)
    {
        mpiret = sc_MPI_Recv (&i, 1, sc_MPI_INT, domain->mpirank - 1,
                              get_tag(domain->dim), domain->mpicomm,
                              &status);
        SC_CHECK_MPI (mpiret);
        FCLAW_ASSERT (i == 0);
    }
}

void
fclaw_domain_serialization_leave (fclaw_domain_t * domain)
{
    int mpiret;
    int i = 0;

    if (domain->mpirank + 1 < domain->mpisize)
    {
        mpiret = sc_MPI_Send (&i, 1, sc_MPI_INT, domain->mpirank + 1,
                              get_tag(domain->dim), domain->mpicomm);
        SC_CHECK_MPI (mpiret);
    }
}

////////////////////////////////////////
// adaptivity
///////////////////////////////////////

//definition of dimension specific functions

void fclaw2d_domain_set_refinement (fclaw_domain_t * domain,
                                    int smooth_refine, int smooth_level,
                                    int coarsen_delay);

void fclaw2d_patch_mark_refine (fclaw_domain_t * domain,
                                int blockno, int patchno);

void fclaw2d_patch_mark_coarsen (fclaw_domain_t * domain,
                                 int blockno, int patchno);

void fclaw2d_domain_iterate_adapted (fclaw_domain_t * old_domain,
                                     fclaw_domain_t * new_domain,
                                     fclaw_match_callback_t mcb,
                                     void *user);

void fclaw3d_domain_set_refinement (fclaw_domain_t * domain,
                                    int smooth_refine, int smooth_level,
                                    int coarsen_delay);

void fclaw3d_patch_mark_refine (fclaw_domain_t * domain,
                                int blockno, int patchno);

void fclaw3d_patch_mark_coarsen (fclaw_domain_t * domain,
                                 int blockno, int patchno);

void fclaw3d_domain_iterate_adapted (fclaw_domain_t * old_domain,
                                     fclaw_domain_t * new_domain,
                                     fclaw_match_callback_t mcb,
                                     void *user);

// dimension independent functions

void
fclaw_domain_set_refinement (fclaw_domain_t * domain,
                             int smooth_refine, int smooth_level,
                             int coarsen_delay)
{
    if(domain->dim == 2)
    {
        fclaw2d_domain_set_refinement(domain,smooth_refine,smooth_level,
                                      coarsen_delay);
    }
    else if (domain->dim == 3)
    {
        fclaw3d_domain_set_refinement(domain,smooth_refine,smooth_level,
                                      coarsen_delay);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}

void
fclaw_patch_mark_refine(fclaw_domain_t *domain, int blockno, int patchno)
{
    if(domain->dim == 2)
    {
        fclaw2d_patch_mark_refine(domain,blockno,patchno);
    }
    else if (domain->dim == 3)
    {
        fclaw3d_patch_mark_refine(domain,blockno,patchno);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}

void
fclaw_patch_mark_coarsen(fclaw_domain_t *domain, int blockno, int patchno)
{
    if(domain->dim == 2)
    {
        fclaw2d_patch_mark_coarsen(domain,blockno,patchno);
    }
    else if (domain->dim == 3)
    {
        fclaw3d_patch_mark_coarsen(domain,blockno,patchno);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}

void
fclaw_domain_iterate_adapted(fclaw_domain_t *old_domain, fclaw_domain_t *new_domain, fclaw_match_callback_t mcb, void *user)
{
    if(old_domain->dim == 2)
    {
        fclaw2d_domain_iterate_adapted(old_domain,new_domain,mcb,user);
    }
    else if (old_domain->dim == 3)
    {
        fclaw3d_domain_iterate_adapted(old_domain,new_domain,mcb,user);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}

////////////////////////////////////////
// partition
///////////////////////////////////////

//definition of dimension specific functions
void fclaw2d_domain_allocate_before_partition (fclaw_domain_t * domain,
                                               size_t data_size,
                                               void ***patch_data);

void fclaw2d_domain_retrieve_after_partition (fclaw_domain_t * domain,
                                              void ***patch_data);

void fclaw2d_domain_iterate_partitioned (fclaw_domain_t * old_domain,
                                         fclaw_domain_t * new_domain,
                                         fclaw_transfer_callback_t tcb,
                                         void *user);

void fclaw2d_domain_free_after_partition (fclaw_domain_t * domain,
                                          void ***patch_data);


void fclaw3d_domain_allocate_before_partition (fclaw_domain_t * domain,
                                               size_t data_size,
                                               void ***patch_data);

void fclaw3d_domain_retrieve_after_partition (fclaw_domain_t * domain,
                                              void ***patch_data);

void fclaw3d_domain_iterate_partitioned (fclaw_domain_t * old_domain,
                                         fclaw_domain_t * new_domain,
                                         fclaw_transfer_callback_t tcb,
                                         void *user);

void fclaw3d_domain_free_after_partition (fclaw_domain_t * domain,
                                          void ***patch_data);


void fclaw_domain_allocate_before_partition(fclaw_domain_t *domain, size_t data_size, void ***patch_data)
{
    if(domain->dim == 2)
    {
        fclaw2d_domain_allocate_before_partition(domain,data_size,patch_data);
    }
    else if (domain->dim == 3)
    {
        fclaw3d_domain_allocate_before_partition(domain,data_size,patch_data);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}

void fclaw_domain_retrieve_after_partition(fclaw_domain_t *domain, void ***patch_data)
{
    if(domain->dim == 2)
    {
        fclaw2d_domain_retrieve_after_partition(domain,patch_data);
    }
    else if (domain->dim == 3)
    {
        fclaw3d_domain_retrieve_after_partition(domain,patch_data);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}

void fclaw_domain_iterate_partitioned(fclaw_domain_t *old_domain, fclaw_domain_t *new_domain, fclaw_transfer_callback_t tcb, void *user)
{
    if(old_domain->dim == 2)
    {
        fclaw2d_domain_iterate_partitioned(old_domain,new_domain,tcb,user);
    }
    else if (old_domain->dim == 3)
    {
        fclaw3d_domain_iterate_partitioned(old_domain,new_domain,tcb,user);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}

void fclaw_domain_free_after_partition(fclaw_domain_t *domain, void ***patch_data)
{
    if(domain->dim == 2)
    {
        fclaw2d_domain_free_after_partition(domain,patch_data);
    }
    else if (domain->dim == 3)
    {
        fclaw3d_domain_free_after_partition(domain,patch_data);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}

////////////////////////////////////////
// exchange
///////////////////////////////////////

//definition of dimension specific functions
void fclaw2d_domain_ghost_exchange_begin (fclaw_domain_t * domain,
                                          fclaw_domain_exchange_t * e,
                                          int exchange_minlevel,
                                          int exchange_maxlevel);

void fclaw2d_domain_ghost_exchange_end (fclaw_domain_t * domain,
                                        fclaw_domain_exchange_t * e);

void fclaw3d_domain_ghost_exchange_begin (fclaw_domain_t * domain,
                                          fclaw_domain_exchange_t * e,
                                          int exchange_minlevel,
                                          int exchange_maxlevel);

void fclaw3d_domain_ghost_exchange_end (fclaw_domain_t * domain,
                                        fclaw_domain_exchange_t * e);

fclaw_domain_exchange_t*
fclaw_domain_allocate_before_exchange(fclaw_domain_t *domain, size_t data_size)
{
    int i;
    char *m;
    fclaw_domain_exchange_t *e;

    e = FCLAW_ALLOC (fclaw_domain_exchange_t, 1);
    e->data_size = data_size;
    e->num_exchange_patches = domain->num_exchange_patches;
    e->num_ghost_patches = domain->num_ghost_patches;

    e->patch_data = FCLAW_ALLOC (void *, domain->num_exchange_patches);
    e->ghost_data = FCLAW_ALLOC (void *, domain->num_ghost_patches);
    e->ghost_contiguous_memory = m = FCLAW_ALLOC (char,
                                                  (size_t)
                                                  domain->num_ghost_patches *
                                                  data_size);
    for (i = 0; i < domain->num_ghost_patches; ++i)
    {
        e->ghost_data[i] = m;
        m += data_size;
    }

    e->async_state = NULL;
    e->by_levels = 0;
    e->inside_async = 0;

    return e;
}

void fclaw_domain_ghost_exchange(fclaw_domain_t *domain, fclaw_domain_exchange_t *e, int exchange_minlevel, int exchange_maxlevel)
{

    FCLAW_ASSERT (e->async_state == NULL);
    FCLAW_ASSERT (!e->inside_async);

    fclaw_domain_ghost_exchange_begin
        (domain, e, exchange_minlevel, exchange_maxlevel);
    fclaw_domain_ghost_exchange_end (domain, e);
}

void fclaw_domain_ghost_exchange_begin(fclaw_domain_t *domain, fclaw_domain_exchange_t *e, int exchange_minlevel, int exchange_maxlevel)
{
    if(domain->dim == 2)
    {
        fclaw2d_domain_ghost_exchange_begin(domain,e,exchange_minlevel,exchange_maxlevel);
    }
    else if (domain->dim == 3)
    {
        fclaw3d_domain_ghost_exchange_begin(domain,e,exchange_minlevel,exchange_maxlevel);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}

void fclaw_domain_ghost_exchange_end(fclaw_domain_t *domain, fclaw_domain_exchange_t *e)
{
    if(domain->dim == 2)
    {
        fclaw2d_domain_ghost_exchange_end(domain,e);
    }
    else if (domain->dim == 3)
    {
        fclaw3d_domain_ghost_exchange_end(domain,e);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}

void fclaw_domain_free_after_exchange(fclaw_domain_t *domain, fclaw_domain_exchange_t *e)
{
    FCLAW_FREE (e->ghost_contiguous_memory);
    FCLAW_FREE (e->ghost_data);
    FCLAW_FREE (e->patch_data);
    FCLAW_FREE (e);
}

////////////////////////////////////////
// neighbors
///////////////////////////////////////

//definition of dimension specific functions

void fclaw2d_patch_face_swap (int *faceno, int *rfaceno);
void fclaw3d_patch_face_swap (int *faceno, int *rfaceno);

void fclaw_patch_face_swap(int dim, int *faceno, int *rfaceno)
{
    if(dim == 2)
    {
        fclaw2d_patch_face_swap(faceno,rfaceno);
    }
    else if (dim == 3)
    {
        fclaw3d_patch_face_swap(faceno,rfaceno);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}

int fclaw2d_patch_boundary_type (fclaw_domain_t * domain,
                                 int blockno, int patchno, int boundaries[4]);

int fclaw2d_patch_normal_match (fclaw_domain_t * domain,
                                int blockno, int patchno, int faceno);

fclaw_patch_relation_t fclaw2d_patch_face_neighbors (fclaw_domain_t *
                                                       domain, int blockno,
                                                       int patchno,
                                                       int faceno,
                                                       int rproc[2],
                                                       int *rblockno,
                                                       int rpatchno[2],
                                                       int *rfaceno);

int fclaw2d_patch_corner_neighbors (fclaw_domain_t * domain,
                                    int blockno, int patchno, int cornerno,
                                    int *rproc, int *rblockno, int *rpatchno,
                                    int *rcorner,
                                    fclaw_patch_relation_t * neighbor_size);
int fclaw3d_patch_boundary_type (fclaw_domain_t * domain,
                                 int blockno, int patchno, int boundaries[6]);

int fclaw3d_patch_normal_match (fclaw_domain_t * domain,
                                int blockno, int patchno, int faceno);


fclaw_patch_relation_t fclaw3d_patch_face_neighbors (fclaw_domain_t *
                                                       domain, int blockno,
                                                       int patchno,
                                                       int faceno,
                                                       int rproc[4],
                                                       int *rblockno,
                                                       int rpatchno[4],
                                                       int *rfaceno);

int fclaw3d_patch_corner_neighbors (fclaw_domain_t * domain,
                                    int blockno, int patchno, int cornerno,
                                    int *rproc, int *rblockno, int *rpatchno,
                                    int *rcorner,
                                    fclaw_patch_relation_t * neighbor_size);


int fclaw_patch_boundary_type(fclaw_domain_t *domain, int blockno, int patchno, int boundaries[4])
{
    if(domain->dim == 2)
    {
        return fclaw2d_patch_boundary_type(domain,blockno,patchno,boundaries);
    }
    else if (domain->dim == 3)
    {
        return fclaw3d_patch_boundary_type(domain,blockno,patchno,boundaries);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}

int fclaw_patch_normal_match(fclaw_domain_t *domain, int blockno, int patchno, int faceno)
{
    if(domain->dim == 2)
    {
        return fclaw2d_patch_normal_match(domain,blockno,patchno,faceno);
    }
    else if (domain->dim == 3)
    {
        return fclaw3d_patch_normal_match(domain,blockno,patchno,faceno);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}

fclaw_patch_relation_t fclaw_patch_face_neighbors(fclaw_domain_t *domain, int blockno, int patchno, int faceno, int rproc[2], int *rblockno, int rpatchno[2], int *rfaceno)
{
    if(domain->dim == 2)
    {
        return fclaw2d_patch_face_neighbors(domain,blockno,patchno,faceno,rproc,rblockno,rpatchno,rfaceno);
    }
    else if (domain->dim == 3)
    {
        return fclaw3d_patch_face_neighbors(domain,blockno,patchno,faceno,rproc,rblockno,rpatchno,rfaceno);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}

int fclaw_patch_corner_neighbors(fclaw_domain_t *domain, int blockno, int patchno, int cornerno, int *rproc, int *rblockno, int *rpatchno, int *rcorner, fclaw_patch_relation_t *neighbor_size)
{
    if(domain->dim == 2)
    {
        return fclaw2d_patch_corner_neighbors(domain,blockno,patchno,cornerno,rproc,rblockno,rpatchno,rcorner,neighbor_size);
    }
    else if (domain->dim == 3)
    {
        return fclaw3d_patch_corner_neighbors(domain,blockno,patchno,cornerno,rproc,rblockno,rpatchno,rcorner,neighbor_size);
    }
    else
    {
        SC_ABORT_NOT_REACHED();
    }
}