#! /bin/bash
#
# This script depends on the necessary build files being in place for each project
# so the IDE MPLAB-X must have already built each configuration to populate the
# necessary files

make_dsPIC33EP256MU806() {
    make clean &> /dev/null
    make -f nbproject/Makefile-dsPIC33EP256MU806.mk SUBPROJECTS= .build-conf &> /dev/null
    if [ $? -ne 0 ]; then
        #
        # The build failed so human intervention is required.
        #
        echo " Make failed!"
    fi
}

make_PIC24FJ256GB106() {
    make clean &> /dev/null
    make -f nbproject/Makefile-PIC24FJ256GB106.mk SUBPROJECTS= .build-conf &> /dev/null
    if [ $? -ne 0 ]; then
        #
        # The build failed so human intervention is required.
        #
        echo " Make failed!"
    fi
}    

make_PIC18F4585() {
    make clean &> /dev/null
    make -f nbproject/Makefile-PIC18F4585.mk SUBPROJECTS= .build-conf &> /dev/null
    if [ $? -ne 0 ]; then
        #
        # The build failed so human intervention is required.
        #
        echo " Make failed!"
    fi
}    

#
# Build the BareBones Timers projets
#
cd ../examples/projects/microchip/BareBones.X
echo "BareBones dsPIC33EP256MU806"
make_dsPIC33EP256MU806
echo "BareBones PIC24FJ256GB1066"
make_PIC24FJ256GB106
#echo "BareBones PIC18F4585"
#make_PIC18F4585
cd - &> /dev/null

#
# Build the HW Timers projets
#
cd ../examples/projects/microchip/HW_Timers.X
echo "HW_Timers dsPIC33EP256MU806"
make_dsPIC33EP256MU806
echo "HW_Timers PIC24FJ256GB1066"
make_PIC24FJ256GB106
#echo "HW_Timers PIC18F4585"
#make_PIC18F4585
cd - &> /dev/null

#
# Build the SW Timers projets
#
cd ../examples/projects/microchip/SW_Timers.X
echo "SW_Timers dsPIC33EP256MU806"
make_dsPIC33EP256MU806
echo "SW_Timers PIC24FJ256GB1066"
make_PIC24FJ256GB106
#echo "SW_Timers PIC18F4585"
#make_PIC18F4585
cd - &> /dev/null

#
# Build the UART projets
#
cd ../examples/projects/microchip/uart_tx.X
echo "uart_tx dsPIC33EP256MU806"
make_dsPIC33EP256MU806
echo "uart_tx PIC24FJ256GB106"
make_PIC24FJ256GB106
#echo "uart_tx PIC18F4585"
#make_PIC18F4585
cd - &> /dev/null

cd ../examples/projects/microchip/uart_rx.X
echo "uart_rx dsPIC33EP256MU806"
make_dsPIC33EP256MU806
#echo "uart_tx PIC24FJ256GB106"
#make_PIC24FJ256GB106
#echo "uart_tx PIC18F4585"
#make_PIC18F4585
cd - &> /dev/null

#
# Build the Serial Logging projets
#
cd ../examples/projects/microchip/SerialLogging.X
echo "SerialLogging dsPIC33EP256MU806"
make_dsPIC33EP256MU806
echo "SerialLogging PIC24FJ256GB106"
make_PIC24FJ256GB106
#echo "SerialLogging PIC18F4585"
#make_PIC18F4585
cd - &> /dev/null

#
# Build the Change Notification projets
#
cd ../examples/projects/microchip/ChangeNotification.X
echo "EEPROM dsPIC33EP256MU806"
make_dsPIC33EP256MU806
cd - &> /dev/null

#
# Build the EEPROM projets
#
cd ../examples/projects/microchip/EEPROM.X
echo "EEPROM dsPIC33EP256MU806"
make_dsPIC33EP256MU806
echo "EEPROM PIC24FJ256GB106"
make_PIC24FJ256GB106
cd - &> /dev/null

#
# Build the Jobs projets
#
cd ../examples/projects/microchip/Jobs.X
echo "Jobs dsPIC33EP256MU806"
make_dsPIC33EP256MU806
echo "Jobs PIC24FJ256GB106"
make_PIC24FJ256GB106
cd - &> /dev/null

#
# Build the CAN-Bus projets
#
cd ../examples/projects/microchip/CAN-Bus.X
echo "CAN-Bus.x dsPIC33EP256MU806"
make_dsPIC33EP256MU806
#echo "CAN-Bus.x PIC24FJ256GB106"
#make_PIC24FJ256GB106
cd - &> /dev/null

cd ../examples/projects/microchip/CAN_BaudAutoDetect.X
echo "CAN_BaudAutoDetect.X dsPIC33EP256MU806"
make_dsPIC33EP256MU806
#echo "CAN_BaudAutoDetect.X PIC24FJ256GB106"
#make_PIC24FJ256GB106
cd - &> /dev/null

cd ../examples/projects/microchip/ChangeNotification.X
echo "ChangeNotification.X dsPIC33EP256MU806"
make_dsPIC33EP256MU806
cd - &> /dev/null

cd ../examples/projects/microchip/ISO15765.X
echo "ISO15765.X dsPIC33EP256MU806"
make_dsPIC33EP256MU806
#echo "ISO15765.X PIC24FJ256GB106"
#make_PIC24FJ256GB106
cd - &> /dev/null

cd ../examples/projects/microchip/LoRa.X
echo "LoRa.X dsPIC33EP256MU806"
make_dsPIC33EP256MU806
#echo "LoRa.X PIC24FJ256GB106"
#make_PIC24FJ256GB106
cd - &> /dev/null

#cd ../examples/projects/microchip/RTC.X
#echo "RTC.X dsPIC33EP256MU806"
#make_dsPIC33EP256MU806
#echo "RTC.X PIC24FJ256GB106"
#make_PIC24FJ256GB106
#cd - &> /dev/null

cd ../examples/projects/microchip/Modbus/Master.X
echo "Modbus Master.X dsPIC33EP256MU806"
make_dsPIC33EP256MU806
#echo "Modbus.X PIC24FJ256GB106"
#make_PIC24FJ256GB106
cd - &> /dev/null

cd ../examples/projects/microchip/Modbus/Slave.X
echo "Modbus Slave.X dsPIC33EP256MU806"
make_dsPIC33EP256MU806
#echo "Modbus.X PIC24FJ256GB106"
#make_PIC24FJ256GB106
cd - &> /dev/null

#cd ../examples/projects/microchip/DCNCP.X
#echo "DCNCP.X dsPIC33EP256MU806"
#make_dsPIC33EP256MU806
#echo "DCNCP.X PIC24FJ256GB106"
#make_PIC24FJ256GB106
#cd - &> /dev/null
