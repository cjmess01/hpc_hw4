all:
		mpicc -g  -o conway parallel_conway.c
i: 
		mpicc -g -o iconway i_send_conway.c
