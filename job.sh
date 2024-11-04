#!/bin/bash
source /apps/profiles/modules_asax.sh.dyn
module load openmpi/4.1.4-gcc11
mpicc -g  -o conway parallel_conway.c

mpirun -np 20 conway 5000 5000 ./scratch/201.txt
mpirun -np 20 conway 5000 5000 ./scratch/202.txt
mpirun -np 20 conway 5000 5000 ./scratch/203.txt

mpirun -np 16 conway 5000 5000 ./scratch/161.txt
mpirun -np 16 conway 5000 5000 ./scratch/162.txt
mpirun -np 16 conway 5000 5000 ./scratch/163.txt

mpirun -np 1 conway 5000 5000 ./scratch/11.txt
mpirun -np 1 conway 5000 5000 ./scratch/12.txt
mpirun -np 1 conway 5000 5000 ./scratch/13.txt

mpirun -np 2 conway 5000 5000 ./scratch/21.txt
mpirun -np 2 conway 5000 5000 ./scratch/22.txt
mpirun -np 2 conway 5000 5000 ./scratch/23.txt

mpirun -np 4 conway 5000 5000 ./scratch/41.txt
mpirun -np 4 conway 5000 5000 ./scratch/42.txt
mpirun -np 4 conway 5000 5000 ./scratch/43.txt

mpirun -np 8 conway 5000 5000 ./scratch/81.txt
mpirun -np 8 conway 5000 5000 ./scratch/82.txt
mpirun -np 8 conway 5000 5000 ./scratch/83.txt

mpirun -np 10 conway 5000 5000 ./scratch/101.txt
mpirun -np 10 conway 5000 5000 ./scratch/102.txt
mpirun -np 10 conway 5000 5000 ./scratch/103.txt




