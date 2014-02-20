#!/bin/bash
echo "generate test-matrices with python if no test data found"
echo
if [ ! -f data/mat_5_4.txt ]; then
    echo "generate 5x4 matrix..."
    python random_float_matrix.py 5 4 > data/mat_5_4.txt
fi

if [ ! -f data/mat_4_5.txt ]; then
    python random_float_matrix.py 4 5 > data/mat_4_5.txt
fi

if [ ! -f data/mat_100x100.txt ]; then
    echo "generate 100x100 matrix..."
    python random_float_matrix.py 100 100 > data/mat_100x100.txt
fi

if [ ! -f data/mat_100x100b.txt ]; then
    python random_float_matrix.py 100 100 > data/mat_100x100b.txt
fi

if [ ! -f data/mat_1000x1000.txt ]; then
    echo "generate 1000x1000 matrix..."
    python random_float_matrix.py 1000 1000 > data/mat_1000x1000.txt
fi
if [ ! -f data/mat_1000x1000b.txt ]; then
    python random_float_matrix.py 1000 1000 > data/mat_1000x1000b.txt
fi


if [ ! -f data/mat_5000x5000a.txt ]; then
    echo "generate 5000x5000 matrix..."
    python random_float_matrix.py 5000 5000 > data/mat_5000x5000a.txt
fi
if [ ! -f data/mat_5000x5000b.txt ]; then
    python random_float_matrix.py 5000 5000 > data/mat_5000x5000b.txt
fi

echo "compile..."
echo
make
echo
echo "calculate..."
echo
echo "* * * * * * * 100x100 Matrix"
cal_t=$((time bin/seq data/mat_100x100.txt data/mat_100x100b.txt)  2>&1 > /dev/null | grep real | awk '{print $2}')
echo "with sequential    $cal_t"

cal_t=$((time bin/omp data/mat_100x100.txt data/mat_100x100b.txt)  2>&1 > /dev/null | grep real | awk '{print $2}')
echo "with omp           $cal_t"

cal_t=$((time bin/thread2 data/mat_100x100.txt data/mat_100x100b.txt)  2>&1 > /dev/null | grep real | awk '{print $2}')
echo "with thread2       $cal_t"

cal_t=$((time mpirun -np 4 bin/mpi data/mat_100x100.txt data/mat_100x100b.txt)  2>&1 > /dev/null | grep real | awk '{print $2}')
echo "with mpi(4p)       $cal_t"
echo

echo "* * * * * * * 1000x1000 Matrix"
cal_t=$((time bin/seq data/mat_1000x1000.txt data/mat_1000x1000b.txt)  2>&1 > /dev/null | grep real | awk '{print $2}')
echo "with sequential    $cal_t"

cal_t=$((time bin/omp data/mat_1000x1000.txt data/mat_1000x1000b.txt)  2>&1 > /dev/null | grep real | awk '{print $2}')
echo "with omp           $cal_t"

cal_t=$((time bin/thread2 data/mat_1000x1000.txt data/mat_1000x1000b.txt)  2>&1 > /dev/null | grep real | awk '{print $2}')
echo "with thread2       $cal_t"

cal_t=$((time mpirun -np 4 bin/mpi data/mat_1000x1000.txt data/mat_1000x1000b.txt)  2>&1 > /dev/null | grep real | awk '{print $2}')
echo "with mpi(4p)       $cal_t"
echo

echo "* * * * * * * 5000x5000 Matrix"
cal_t=$((time bin/seq data/mat_5000x5000a.txt data/mat_5000x5000b.txt)  2>&1 > /dev/null | grep real | awk '{print $2}')
echo "with sequential    $cal_t"

cal_t=$((time bin/omp data/mat_5000x5000a.txt data/mat_5000x5000b.txt)  2>&1 > /dev/null | grep real | awk '{print $2}')
echo "with omp           $cal_t"

cal_t=$((time bin/thread2 data/mat_5000x5000a.txt data/mat_5000x5000b.txt)  2>&1 > /dev/null | grep real | awk '{print $2}')
echo "with thread2       $cal_t"

cal_t=$((time mpirun -np 4 bin/mpi ddata/mat_5000x5000a.txt data/mat_5000x5000b.txt)  2>&1 > /dev/null | grep real | awk '{print $2}')
echo "with mpi(4p)       $cal_t"
echo

