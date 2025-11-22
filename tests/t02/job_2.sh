#!/bin/sh 

#SBATCH --job-name=mpitest
#SBATCH --partition=CudaTest
#SBATCH --nodes=2
#SBATCH --nodelist=cudanode[1-2]
#SBATCH --ntasks-per-node=1
#SBATCH --output=output-%j.log
#SBATCH --error=error-%j.err
#SBATCH --time=00:10:00


# Configure Extrae
export EXTRAE_CONFIG_FILE=./extrae.xml

# Load the tracing library (choose C/Fortran)
#export LD_PRELOAD=${EXTRAE_HOME}/lib/libmpitracef.so  # Fortran
export EXTRAE_PATH=/opt/ohpc/pub/libs/gnu12/openmpi4/extrae/3.8.3
export LD_PRELOAD=${EXTRAE_PATH}/lib/libmpitrace.so    # C

#module purge
module load openmpi4/4.1.4
module load extrae/3.8.3
module load papi/6.0.0

#TRACE_NAME=hellosh_t.prv

mpirun -np 2 ./ex_01 -i 10 -s 65
