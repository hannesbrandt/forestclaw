      subroutine fclaw2d_clawpatch46_fort_tag4refinement(mx,my,mbc,
     &      meqn, xlower,ylower,dx,dy,blockno,
     &      q, tag_threshold, init_flag, tag_patch)
      implicit none

      integer mx,my, mbc, meqn, tag_patch, init_flag
      integer blockno
      double precision xlower, ylower, dx, dy
      double precision tag_threshold
      double precision q(1-mbc:mx+mbc,1-mbc:my+mbc,meqn)

      integer i,j, mq
      double precision qmin, qmax

      logical exceeds_th, fclaw2d_clawpatch_exceeds_threshold
      integer ii,jj
      double precision xc,yc,quad(-1:1,-1:1), qval

      logical(kind=4) :: is_ghost, fclaw2d_clawpatch46_is_ghost

c     # Assume that we won't refine      
      tag_patch = 0

c     # Default : Refinement based only on first variable in system.  
c     # Users can modify this by creating a local copy of this routine
c     # and the corresponding tag4coarsening routine.

      mq = 1
      qmin = q(1,1,mq)
      qmax = q(1,1,mq)
      do j = 1-mbc,my+mbc
         do i = 1-mbc,mx+mbc
            xc = xlower + (i-0.5)*dx
            yc = ylower + (j-0.5)*dy
            qmin = min(qmin,q(i,j,mq))
            qmax = max(qmax,q(i,j,mq))
            qval = q(i,j,mq)
            is_ghost = fclaw2d_clawpatch46_is_ghost(i,j,mx,my)
            if (.not. is_ghost) then
               do jj = -1,1
                  do ii = -1,1
                     quad(ii,jj) = q(i+ii,j+jj,mq)
                  end do
               end do
            endif
            exceeds_th = fclaw2d_clawpatch_exceeds_threshold(
     &             blockno, qval,qmin,qmax,quad, dx,dy,xc,yc,
     &             tag_threshold,init_flag, is_ghost)
            if (exceeds_th) then
c              # Refine this patch               
               tag_patch = 1
               return
            endif
         enddo
      enddo

      end


c     # We may want to check ghost cells for tagging.  
      logical(kind=4) function fclaw2d_clawpatch46_is_ghost(i,j,mx,my)
         implicit none

         integer i, j, mx, my
         logical(kind=4) :: is_ghost

         is_ghost = .false.
         if (i .lt. 1 .or. j .lt. 1) then
            is_ghost = .true.
         elseif (i .gt. mx .or. j .gt. my) then
            is_ghost = .true.
         end if

         fclaw2d_clawpatch46_is_ghost = is_ghost

         return 

      end


