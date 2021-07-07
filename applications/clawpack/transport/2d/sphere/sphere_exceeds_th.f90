!! # check to see if value exceeds threshold

logical(kind=4) function user_exceeds_threshold(blockno,& 
                                                qval,qmin,qmax,quad, & 
                                                dx,dy,xc,yc,threshold, &
                                                init_flag, is_ghost)
    implicit none
    
    double precision :: qval,qmin,qmax,threshold
    double precision :: quad(-1:1,-1:1)
    double precision :: dx,dy, xc, yc
    integer :: blockno, init_flag
    logical(kind=4) :: is_ghost

    integer :: initchoice
    common /initchoice_comm/ initchoice

    logical :: refine

    refine = .false.    
    if (initchoice .le. 3) then
        !! Not sure why we don't refine if initchoice .gt. 3
        if (qval .gt. threshold) then
            refine = qval .ge. threshold
        endif
    endif

    user_exceeds_threshold = refine

end function user_exceeds_threshold
