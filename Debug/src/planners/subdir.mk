################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/planners/ETUCT.cc \
../src/planners/ParallelETUCT.cc \
../src/planners/ValueIteration.cc 

CC_DEPS += \
./src/planners/ETUCT.d \
./src/planners/ParallelETUCT.d \
./src/planners/ValueIteration.d 

OBJS += \
./src/planners/ETUCT.o \
./src/planners/ParallelETUCT.o \
./src/planners/ValueIteration.o 


# Each subdirectory must supply rules for building sources it contributes
src/planners/%.o: ../src/planners/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__GXX_EXPERIMENTAL_CXX0X__ -I/usr/local/include/opencv -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


