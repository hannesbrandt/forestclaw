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

#ifndef QUADRANTS_USER_H
#define QUADRANTS_USER_H

#include <fclaw2d_forestclaw.h>
#include <fc2d_clawpack46.h>
#include <fc2d_clawpack5.h>

#ifdef __cplusplus
extern "C"
{
#if 0
}
#endif
#endif

typedef struct user_options
{
    int example;
    double alpha;
    double gamma;
    int claw_version;

    int is_registered;

} user_options_t;

#define QUADRANTS_SETPROB FCLAW_F77_FUNC(quadrants_setprob, QUADRANTS_SETPROB)
void QUADRANTS_SETPROB(const double* gamma, const double* alpha);


#define RPN2EU3 FCLAW_F77_FUNC(rpn2eu3,RPN2EU3)
void RPN2EU3(const int* ixy,const int* maxm, const int* meqn, const int* mwaves,
             const int* mbc,const int* mx, double ql[], double qr[],
             double auxl[], double auxr[], double wave[],
             double s[], double amdq[], double apdq[]);

#define RPN2EU4 FCLAW_F77_FUNC(rpn2eu4,RPN2EU4)
void RPN2EU4(const int* ixy,const int* maxm, const int* meqn, const int* mwaves,
             const int* mbc,const int* mx, double ql[], double qr[],
             double auxl[], double auxr[], double wave[],
             double s[], double amdq[], double apdq[]);

const user_options_t* quadrants_user_get_options(fclaw2d_domain_t* domain);


void quadrants_link_solvers(fclaw2d_domain_t *domain);

void quadrants_problem_setup(fclaw2d_domain_t* domain);



fclaw2d_map_context_t* fclaw2d_map_new_nomap();

fclaw2d_map_context_t* fclaw2d_map_new_identity();

fclaw2d_map_context_t* fclaw2d_map_new_cart(fclaw2d_map_context_t* brick,
                                            const double scale[],
                                            const double shift[],
                                            const double rotate[]);

fclaw2d_map_context_t* fclaw2d_map_new_fivepatch(const double scale[],
                                                 const double shift[],
                                                 const double rotate[],
                                                 const double alpha);


#ifdef __cplusplus
#if 0
{
#endif
}
#endif

#endif