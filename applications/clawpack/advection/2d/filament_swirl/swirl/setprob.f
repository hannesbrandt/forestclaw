      subroutine swirl_setprob()
      implicit none

      double precision pi, pi2
      common /compi/ pi, pi2

      double precision tperiod
      common /comvt/ tperiod

      integer solver
      common /solver/ solver

      pi = 4.d0*datan(1.d0)
      pi2 = 2.d0*pi

      open(10,file='setprob.data')
      read(10,*) tperiod
      close(10)

      solver = 2

      end
