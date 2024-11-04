
#!/bin/bash
source /apps/profiles/modules_asax.sh.dyn
module load openmpi/4.1.4-gcc11
mpicc -g  -o conway i_send_conway.c

mpirun -np 20 conway 5000 5000 ./scratch/z201.txt
mpirun -np 20 conway 5000 5000 ./scratch/z202.txt
mpirun -np 20 conway 5000 5000 ./scratch/z203.txt

mpirun -np 16 conway 5000 5000 ./scratch/z161.txt
mpirun -np 16 conway 5000 5000 ./scratch/z162.txt
mpirun -np 16 conway 5000 5000 ./scratch/z163.txt

mpirun -np 1 conway 5000 5000 ./scratch/z11.txt
mpirun -np 1 conway 5000 5000 ./scratch/z12.txt
mpirun -np 1 conway 5000 5000 ./scratch/z13.txt

mpirun -np 2 conway 5000 5000 ./scratch/z21.txt
mpirun -np 2 conway 5000 5000 ./scratch/z22.txt
mpirun -np 2 conway 5000 5000 ./scratch/z23.txt

mpirun -np 4 conway 5000 5000 ./scratch/z41.txt
mpirun -np 4 conway 5000 5000 ./scratch/z42.txt
mpirun -np 4 conway 5000 5000 ./scratch/z43.txt

mpirun -np 8 conway 5000 5000 ./scratch/z81.txt
mpirun -np 8 conway 5000 5000 ./scratch/z82.txt
mpirun -np 8 conway 5000 5000 ./scratch/z83.txt

mpirun -np 10 conway 5000 5000 ./scratch/z101.txt
mpirun -np 10 conway 5000 5000 ./scratch/z102.txt
mpirun -np 10 conway 5000 5000 ./scratch/z103.txt




