! RUN: %not_todo_cmd bbc -emit-fir -fopenmp -o - %s 2>&1 | FileCheck %s
! RUN: %not_todo_cmd %flang_fc1 -emit-fir -fopenmp -o - %s 2>&1 | FileCheck %s

! There's no definition of '+' for type(t)
! CHECK: The type of 'mt' is incompatible with the reduction operator.
subroutine reduction_allocatable
  type t
    integer :: x
  end type
  integer :: i = 1
  type(t) :: mt

  mt%x = 0

  !$omp parallel num_threads(4)
  !$omp do reduction(+:mt)
  do i = 1, 10
    mt%x = mt%x + i
  enddo
  !$omp end do
  !$omp end parallel
end subroutine
