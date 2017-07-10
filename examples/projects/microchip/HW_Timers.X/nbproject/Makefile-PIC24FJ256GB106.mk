#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-PIC24FJ256GB106.mk)" "nbproject/Makefile-local-PIC24FJ256GB106.mk"
include nbproject/Makefile-local-PIC24FJ256GB106.mk
endif
endif

# Environment
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=PIC24FJ256GB106
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/HW_Timers.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/HW_Timers.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../../../../comms/uart/uart.c ../../../../logger/serial_log.c ../../../../processors/pic24FJ256GB106.c ../../../../processors/pic18f2680.c ../../../../processors/dsPIC33EP256MU806.c ../../../../processors/pic18f4585.c ../../../../timers/hw_timers.c ../../../../utils/clock.c ../../../main_hw_timers.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/252781366/uart.o ${OBJECTDIR}/_ext/2146973904/serial_log.o ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o ${OBJECTDIR}/_ext/566826655/pic18f2680.o ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o ${OBJECTDIR}/_ext/566826655/pic18f4585.o ${OBJECTDIR}/_ext/1924324082/hw_timers.o ${OBJECTDIR}/_ext/1030658703/clock.o ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/252781366/uart.o.d ${OBJECTDIR}/_ext/2146973904/serial_log.o.d ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o.d ${OBJECTDIR}/_ext/566826655/pic18f2680.o.d ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o.d ${OBJECTDIR}/_ext/566826655/pic18f4585.o.d ${OBJECTDIR}/_ext/1924324082/hw_timers.o.d ${OBJECTDIR}/_ext/1030658703/clock.o.d ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/252781366/uart.o ${OBJECTDIR}/_ext/2146973904/serial_log.o ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o ${OBJECTDIR}/_ext/566826655/pic18f2680.o ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o ${OBJECTDIR}/_ext/566826655/pic18f4585.o ${OBJECTDIR}/_ext/1924324082/hw_timers.o ${OBJECTDIR}/_ext/1030658703/clock.o ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o

# Source Files
SOURCEFILES=../../../../comms/uart/uart.c ../../../../logger/serial_log.c ../../../../processors/pic24FJ256GB106.c ../../../../processors/pic18f2680.c ../../../../processors/dsPIC33EP256MU806.c ../../../../processors/pic18f4585.c ../../../../timers/hw_timers.c ../../../../utils/clock.c ../../../main_hw_timers.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-PIC24FJ256GB106.mk dist/${CND_CONF}/${IMAGE_TYPE}/HW_Timers.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=24FJ256GB106
MP_LINKER_FILE_OPTION=,--script=p24FJ256GB106.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/252781366/uart.o: ../../../../comms/uart/uart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/252781366" 
	@${RM} ${OBJECTDIR}/_ext/252781366/uart.o.d 
	@${RM} ${OBJECTDIR}/_ext/252781366/uart.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../comms/uart/uart.c  -o ${OBJECTDIR}/_ext/252781366/uart.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/252781366/uart.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/252781366/uart.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/2146973904/serial_log.o: ../../../../logger/serial_log.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2146973904" 
	@${RM} ${OBJECTDIR}/_ext/2146973904/serial_log.o.d 
	@${RM} ${OBJECTDIR}/_ext/2146973904/serial_log.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../logger/serial_log.c  -o ${OBJECTDIR}/_ext/2146973904/serial_log.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/2146973904/serial_log.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/2146973904/serial_log.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o: ../../../../processors/pic24FJ256GB106.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/pic24FJ256GB106.c  -o ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/566826655/pic18f2680.o: ../../../../processors/pic18f2680.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f2680.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f2680.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/pic18f2680.c  -o ${OBJECTDIR}/_ext/566826655/pic18f2680.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/pic18f2680.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/pic18f2680.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o: ../../../../processors/dsPIC33EP256MU806.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/dsPIC33EP256MU806.c  -o ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/566826655/pic18f4585.o: ../../../../processors/pic18f4585.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f4585.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f4585.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/pic18f4585.c  -o ${OBJECTDIR}/_ext/566826655/pic18f4585.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/pic18f4585.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/pic18f4585.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1924324082/hw_timers.o: ../../../../timers/hw_timers.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1924324082" 
	@${RM} ${OBJECTDIR}/_ext/1924324082/hw_timers.o.d 
	@${RM} ${OBJECTDIR}/_ext/1924324082/hw_timers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../timers/hw_timers.c  -o ${OBJECTDIR}/_ext/1924324082/hw_timers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1924324082/hw_timers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1924324082/hw_timers.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1030658703/clock.o: ../../../../utils/clock.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1030658703" 
	@${RM} ${OBJECTDIR}/_ext/1030658703/clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/1030658703/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../utils/clock.c  -o ${OBJECTDIR}/_ext/1030658703/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1030658703/clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1030658703/clock.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/2124829536/main_hw_timers.o: ../../../main_hw_timers.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2124829536" 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o.d 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../main_hw_timers.c  -o ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/2124829536/main_hw_timers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/2124829536/main_hw_timers.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
else
${OBJECTDIR}/_ext/252781366/uart.o: ../../../../comms/uart/uart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/252781366" 
	@${RM} ${OBJECTDIR}/_ext/252781366/uart.o.d 
	@${RM} ${OBJECTDIR}/_ext/252781366/uart.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../comms/uart/uart.c  -o ${OBJECTDIR}/_ext/252781366/uart.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/252781366/uart.o.d"        -g -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/252781366/uart.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/2146973904/serial_log.o: ../../../../logger/serial_log.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2146973904" 
	@${RM} ${OBJECTDIR}/_ext/2146973904/serial_log.o.d 
	@${RM} ${OBJECTDIR}/_ext/2146973904/serial_log.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../logger/serial_log.c  -o ${OBJECTDIR}/_ext/2146973904/serial_log.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/2146973904/serial_log.o.d"        -g -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/2146973904/serial_log.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o: ../../../../processors/pic24FJ256GB106.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/pic24FJ256GB106.c  -o ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o.d"        -g -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/566826655/pic18f2680.o: ../../../../processors/pic18f2680.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f2680.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f2680.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/pic18f2680.c  -o ${OBJECTDIR}/_ext/566826655/pic18f2680.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/pic18f2680.o.d"        -g -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/pic18f2680.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o: ../../../../processors/dsPIC33EP256MU806.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/dsPIC33EP256MU806.c  -o ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o.d"        -g -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/566826655/pic18f4585.o: ../../../../processors/pic18f4585.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f4585.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f4585.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/pic18f4585.c  -o ${OBJECTDIR}/_ext/566826655/pic18f4585.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/pic18f4585.o.d"        -g -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/pic18f4585.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1924324082/hw_timers.o: ../../../../timers/hw_timers.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1924324082" 
	@${RM} ${OBJECTDIR}/_ext/1924324082/hw_timers.o.d 
	@${RM} ${OBJECTDIR}/_ext/1924324082/hw_timers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../timers/hw_timers.c  -o ${OBJECTDIR}/_ext/1924324082/hw_timers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1924324082/hw_timers.o.d"        -g -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1924324082/hw_timers.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1030658703/clock.o: ../../../../utils/clock.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1030658703" 
	@${RM} ${OBJECTDIR}/_ext/1030658703/clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/1030658703/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../utils/clock.c  -o ${OBJECTDIR}/_ext/1030658703/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1030658703/clock.o.d"        -g -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1030658703/clock.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/2124829536/main_hw_timers.o: ../../../main_hw_timers.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2124829536" 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o.d 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../main_hw_timers.c  -o ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/2124829536/main_hw_timers.o.d"        -g -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -DMCP -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/2124829536/main_hw_timers.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/HW_Timers.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/HW_Timers.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)   -mreserve=data@0x800:0x81F -mreserve=data@0x820:0x821 -mreserve=data@0x822:0x823 -mreserve=data@0x824:0x825 -mreserve=data@0x826:0x84F   -Wl,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK3=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST) 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/HW_Timers.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/HW_Timers.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DXPRJ_PIC24FJ256GB106=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -Wl,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST) 
	${MP_CC_DIR}/xc16-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/HW_Timers.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf  
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/PIC24FJ256GB106
	${RM} -r dist/PIC24FJ256GB106

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell "${PATH_TO_IDE_BIN}"mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
