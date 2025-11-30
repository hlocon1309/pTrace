#!/bin/sh 

#SBATCH --job-name=test
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --cpus-per-task=1
#SBATCH --output=output-%j.log
#SBATCH --error=error-%j.err
#SBATCH --time=00:10:00

module load openmpi4/4.1.4
module load extrae/3.8.3
module load papi/6.0.0

export EXTRAE_CONFIG_FILE=./extrae.xml
export EXTRAE_PATH=/opt/ohpc/pub/libs/gnu12/openmpi4/extrae/3.8.3
export LD_PRELOAD=${EXTRAE_PATH}/lib/libmpitrace.so

mpirun -np 8 mmatrix 8

