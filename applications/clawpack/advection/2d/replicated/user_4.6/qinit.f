      subroutine clawpack46_qinit(maxmx,maxmy,meqn,mbc,mx,my,
     &      xlower,ylower,dx,dy,q,maux,aux)
      implicit none

      integer maxmx, maxmy, meqn, mbc, mx, my, maux
      double precision xlower, ylower, dx, dy
      double precision q(1-mbc:mx+mbc, 1-mbc:my+mbc, meqn)
      double precision aux(1-mbc:mx+mbc, 1-mbc:my+mbc, maux)

      integer i,j
      integer blockno, fc2d_clawpack46_get_block
      double precision xlow, ylow, w

c     # Block number isn't used, but is passed to cellave2
      blockno = fc2d_clawpack46_get_block()

      do j = 1-mbc,my+mbc
         do i = 1-mbc,mx+mbc
            xlow = modulo(xlower,1.d0) + (i-1)*dx
            ylow = modulo(ylower,1.d0) + (j-1)*dy
            call cellave2(blockno,xlow,ylow,dx,dy,w)
            q(i,j,1) = w
         enddo
      enddo

      return
      end