################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/envs/BlockRoom.cc 

CC_DEPS += \
./src/envs/BlockRoom.d 

OBJS += \
./src/envs/BlockRoom.o 


# Each subdirectory must supply rules for building sources it contributes
src/envs/%.o: ../src/envs/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__GXX_EXPERIMENTAL_CXX0X__ -I/usr/local/include/opencv -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


