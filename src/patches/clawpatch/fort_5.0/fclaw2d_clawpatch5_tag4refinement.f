      subroutine fclaw2d_clawpatch5_fort_tag4refinement(mx,my,mbc,
     &      meqn, xlower,ylower,dx,dy,blockno,
     &      q, tag_threshold, init_flag,tag_patch)
      implicit none

      integer mx,my, mbc, meqn, tag_patch, init_flag
      integer blockno
      double precision xlower, ylower, dx, dy
      double precision tag_threshold
      double precision q(meqn,1-mbc:mx+mbc,1-mbc:my+mbc)

      integer i,j, mq, ii, jj
      double precision qmin, qmax, xc,yc,quad(-1:1,-1:1)
      logical(kind=4) :: fclaw2d_clawpatch_exceeds_threshold, exceeds_th

      logical(kind=4) :: is_ghost, fclaw2d_clawpatch5_is_ghost

      tag_patch = 0

c     # Refine based only on first variable in system.
      mq = 1
      qmin = q(mq,1,1)
      qmax = q(mq,1,1)
      do j = 1-mbc,my+mbc
         do i = 1-mbc,mx+mbc
            xc = xlower + (i-0.5)*dx
            yc = ylower + (j-0.5)*dy
            qmin = min(q(mq,i,j),qmin)
            qmax = max(q(mq,i,j),qmax)            
            is_ghost = fclaw2d_clawpatch5_is_ghost(i,j,mx,my)
            if (.not. is_ghost) then
               do ii = -1,1
                  do jj = -1,1
                     quad(ii,jj) = q(mq,i+ii,j+jj)
                  end do
               end do
            endif
            exceeds_th = fclaw2d_clawpatch_exceeds_threshold(
     &             blockno, q(mq,i,j),qmin,qmax,quad, dx,dy,xc,yc,
     &             tag_threshold,init_flag,is_ghost)
            if (exceeds_th) then
c              # Refine this patch               
               tag_patch = 1
               return
            endif
         enddo
      enddo

      end


c     # We may want to check ghost cells for tagging.  
      logical(kind=4) function fclaw2d_clawpatch5_is_ghost(i,j,mx,my)
         implicit none

         integer i, j, mx, my
         logical(kind=4) is_ghost

         is_ghost = .false.
         if (i .lt. 1 .or. j .lt. 1) then
            is_ghost = .true.
         elseif (i .gt. mx .or. j .gt. my) then
            is_ghost = .true.
         end if

         fclaw2d_clawpatch5_is_ghost = is_ghost

         return 

      end

