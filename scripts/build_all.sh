#! /bin/bash
#
# This script depends on the necessary build files being in place for each project
# so the IDE MPLAB-X must have already built each configuration to populate the
# necessary files

cd ../examples/projects/microchip/HW_Timers.X
make clean &> /dev/null
make -f nbproject/Makefile-dsPIC33EP256MU806.mk SUBPROJECTS= .build-conf &> /dev/null
if [ $? -ne 0 ]; then
    #
    # The build failed so human intervention is required.
    #
    echo "HW_Timers dsPIC33EP256MU806 - Make failed!"
    exit
fi

make clean &> /dev/null
make -f nbproject/Makefile-PIC24FJ256GB106.mk SUBPROJECTS= .build-conf &> /dev/null
if [ $? -ne 0 ]; then
    #
    # The build failed so human intervention is required.
    #
    echo "HW_Timers PIC24FJ256GB106 - Make failed!"
    exit
fi

make clean &> /dev/null
make -f nbproject/Makefile-PIC18F4585.mk SUBPROJECTS= .build-conf &> /dev/null
if [ $? -ne 0 ]; then
    #
    # The build failed so human intervention is required.
    #
    echo "HW_Timers PIC18F4585 - Make failed!"
    exit
fi
cd -

cd ../examples/projects/microchip/SW_Timers.X
make clean &> /dev/null
make -f nbproject/Makefile-dsPIC33EP256MU806.mk SUBPROJECTS= .build-conf &> /dev/null
if [ $? -ne 0 ]; then
    #
    # The build failed so human intervention is required.
    #
    echo "SW_Timers dsPIC33EP256MU806 - Make failed!"
    exit
fi

make clean &> /dev/null
make -f nbproject/Makefile-PIC24FJ256GB106.mk SUBPROJECTS= .build-conf &> /dev/null
if [ $? -ne 0 ]; then
    #
    # The build failed so human intervention is required.
    #
    echo "SW_Timers PIC24FJ256GB106 - Make failed!"
    exit
fi

make clean &> /dev/null
make -f nbproject/Makefile-PIC18F4585.mk SUBPROJECTS= .build-conf &> /dev/null
if [ $? -ne 0 ]; then
    #
    # The build failed so human intervention is required.
    #
    echo "SW_Timers PIC18F4585 Make failed!"
    exit
fi
cd -

cd ../examples/projects/microchip/uart_tx.X
make clean &> /dev/null
make -f nbproject/Makefile-dsPIC33EP256MU806.mk SUBPROJECTS= .build-conf &> /dev/null
if [ $? -ne 0 ]; then
    #
    # The build failed so human intervention is required.
    #
    echo "uart_tx dsPIC33EP256MU806 - Make failed!"
    exit
fi

make clean &> /dev/null
make -f nbproject/Makefile-PIC24FJ256GB106.mk SUBPROJECTS= .build-conf &> /dev/null
if [ $? -ne 0 ]; then
    #
    # The build failed so human intervention is required.
    #
    echo "uart_tx PIC24FJ256GB106 - Make failed!"
    exit
fi

make clean &> /dev/null
make -f nbproject/Makefile-PIC18F4585.mk SUBPROJECTS= .build-conf &> /dev/null
if [ $? -ne 0 ]; then
    #
    # The build failed so human intervention is required.
    #
    echo "uart_tx PIC18F4585 Make failed!"
    exit
fi
cd -

cd ../examples/projects/microchip/SerialLogging.X
make clean &> /dev/null
make -f nbproject/Makefile-dsPIC33EP256MU806.mk SUBPROJECTS= .build-conf &> /dev/null
if [ $? -ne 0 ]; then
    #
    # The build failed so human intervention is required.
    #
    echo "SerialLogging dsPIC33EP256MU806 - Make failed!"
    exit
fi

make clean &> /dev/null
make -f nbproject/Makefile-PIC24FJ256GB106.mk SUBPROJECTS= .build-conf &> /dev/null
if [ $? -ne 0 ]; then
    #
    # The build failed so human intervention is required.
    #
    echo "SerialLogging PIC24FJ256GB106 - Make failed!"
    exit
fi

make clean &> /dev/null
make -f nbproject/Makefile-PIC18F4585.mk SUBPROJECTS= .build-conf &> /dev/null
if [ $? -ne 0 ]; then
    #
    # The build failed so human intervention is required.
    #
    echo "SerialLogging PIC18F4585 Make failed!"
    exit
fi
cd -
