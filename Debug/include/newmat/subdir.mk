################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../include/newmat/bandmat.cc \
../include/newmat/cholesky.cc \
../include/newmat/evalue.cc \
../include/newmat/fft.cc \
../include/newmat/hholder.cc \
../include/newmat/jacobi.cc \
../include/newmat/myexcept.cc \
../include/newmat/newfft.cc \
../include/newmat/newmat1.cc \
../include/newmat/newmat2.cc \
../include/newmat/newmat3.cc \
../include/newmat/newmat4.cc \
../include/newmat/newmat5.cc \
../include/newmat/newmat6.cc \
../include/newmat/newmat7.cc \
../include/newmat/newmat8.cc \
../include/newmat/newmat9.cc \
../include/newmat/newmatex.cc \
../include/newmat/newmatnl.cc \
../include/newmat/newmatrm.cc \
../include/newmat/solution.cc \
../include/newmat/sort.cc \
../include/newmat/submat.cc \
../include/newmat/svd.cc 

CC_DEPS += \
./include/newmat/bandmat.d \
./include/newmat/cholesky.d \
./include/newmat/evalue.d \
./include/newmat/fft.d \
./include/newmat/hholder.d \
./include/newmat/jacobi.d \
./include/newmat/myexcept.d \
./include/newmat/newfft.d \
./include/newmat/newmat1.d \
./include/newmat/newmat2.d \
./include/newmat/newmat3.d \
./include/newmat/newmat4.d \
./include/newmat/newmat5.d \
./include/newmat/newmat6.d \
./include/newmat/newmat7.d \
./include/newmat/newmat8.d \
./include/newmat/newmat9.d \
./include/newmat/newmatex.d \
./include/newmat/newmatnl.d \
./include/newmat/newmatrm.d \
./include/newmat/solution.d \
./include/newmat/sort.d \
./include/newmat/submat.d \
./include/newmat/svd.d 

OBJS += \
./include/newmat/bandmat.o \
./include/newmat/cholesky.o \
./include/newmat/evalue.o \
./include/newmat/fft.o \
./include/newmat/hholder.o \
./include/newmat/jacobi.o \
./include/newmat/myexcept.o \
./include/newmat/newfft.o \
./include/newmat/newmat1.o \
./include/newmat/newmat2.o \
./include/newmat/newmat3.o \
./include/newmat/newmat4.o \
./include/newmat/newmat5.o \
./include/newmat/newmat6.o \
./include/newmat/newmat7.o \
./include/newmat/newmat8.o \
./include/newmat/newmat9.o \
./include/newmat/newmatex.o \
./include/newmat/newmatnl.o \
./include/newmat/newmatrm.o \
./include/newmat/solution.o \
./include/newmat/sort.o \
./include/newmat/submat.o \
./include/newmat/svd.o 


# Each subdirectory must supply rules for building sources it contributes
include/newmat/%.o: ../include/newmat/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__GXX_EXPERIMENTAL_CXX0X__ -I/usr/local/include/opencv -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


