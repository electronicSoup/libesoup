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
ifeq "$(wildcard nbproject/Makefile-local-PIC18F4585.mk)" "nbproject/Makefile-local-PIC18F4585.mk"
include nbproject/Makefile-local-PIC18F4585.mk
endif
endif

# Environment
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=PIC18F4585
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/SerialLogging.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/SerialLogging.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=--mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../../../../comms/uart/uart.c ../../../main_serial_logging.c ../../../../logger/serial_log.c ../../../../processors/pic24FJ256GB106.c ../../../../processors/pic18f2680.c ../../../../processors/dsPIC33EP256MU806.c ../../../../processors/pic18f4585.c ../../../../utils/clock.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/252781366/uart.p1 ${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1 ${OBJECTDIR}/_ext/2146973904/serial_log.p1 ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1 ${OBJECTDIR}/_ext/566826655/pic18f2680.p1 ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1 ${OBJECTDIR}/_ext/566826655/pic18f4585.p1 ${OBJECTDIR}/_ext/1030658703/clock.p1
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/252781366/uart.p1.d ${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1.d ${OBJECTDIR}/_ext/2146973904/serial_log.p1.d ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1.d ${OBJECTDIR}/_ext/566826655/pic18f2680.p1.d ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1.d ${OBJECTDIR}/_ext/566826655/pic18f4585.p1.d ${OBJECTDIR}/_ext/1030658703/clock.p1.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/252781366/uart.p1 ${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1 ${OBJECTDIR}/_ext/2146973904/serial_log.p1 ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1 ${OBJECTDIR}/_ext/566826655/pic18f2680.p1 ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1 ${OBJECTDIR}/_ext/566826655/pic18f4585.p1 ${OBJECTDIR}/_ext/1030658703/clock.p1

# Source Files
SOURCEFILES=../../../../comms/uart/uart.c ../../../main_serial_logging.c ../../../../logger/serial_log.c ../../../../processors/pic24FJ256GB106.c ../../../../processors/pic18f2680.c ../../../../processors/dsPIC33EP256MU806.c ../../../../processors/pic18f4585.c ../../../../utils/clock.c


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
	${MAKE}  -f nbproject/Makefile-PIC18F4585.mk dist/${CND_CONF}/${IMAGE_TYPE}/SerialLogging.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=18F4585
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/252781366/uart.p1: ../../../../comms/uart/uart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/252781366" 
	@${RM} ${OBJECTDIR}/_ext/252781366/uart.p1.d 
	@${RM} ${OBJECTDIR}/_ext/252781366/uart.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=pickit3  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/252781366/uart.p1  ../../../../comms/uart/uart.c 
	@-${MV} ${OBJECTDIR}/_ext/252781366/uart.d ${OBJECTDIR}/_ext/252781366/uart.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/252781366/uart.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1: ../../../main_serial_logging.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2124829536" 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1.d 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=pickit3  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1  ../../../main_serial_logging.c 
	@-${MV} ${OBJECTDIR}/_ext/2124829536/main_serial_logging.d ${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/2146973904/serial_log.p1: ../../../../logger/serial_log.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2146973904" 
	@${RM} ${OBJECTDIR}/_ext/2146973904/serial_log.p1.d 
	@${RM} ${OBJECTDIR}/_ext/2146973904/serial_log.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=pickit3  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/2146973904/serial_log.p1  ../../../../logger/serial_log.c 
	@-${MV} ${OBJECTDIR}/_ext/2146973904/serial_log.d ${OBJECTDIR}/_ext/2146973904/serial_log.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/2146973904/serial_log.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1: ../../../../processors/pic24FJ256GB106.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=pickit3  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1  ../../../../processors/pic24FJ256GB106.c 
	@-${MV} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.d ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/566826655/pic18f2680.p1: ../../../../processors/pic18f2680.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f2680.p1.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f2680.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=pickit3  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/566826655/pic18f2680.p1  ../../../../processors/pic18f2680.c 
	@-${MV} ${OBJECTDIR}/_ext/566826655/pic18f2680.d ${OBJECTDIR}/_ext/566826655/pic18f2680.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/566826655/pic18f2680.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1: ../../../../processors/dsPIC33EP256MU806.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=pickit3  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1  ../../../../processors/dsPIC33EP256MU806.c 
	@-${MV} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.d ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/566826655/pic18f4585.p1: ../../../../processors/pic18f4585.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f4585.p1.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f4585.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=pickit3  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/566826655/pic18f4585.p1  ../../../../processors/pic18f4585.c 
	@-${MV} ${OBJECTDIR}/_ext/566826655/pic18f4585.d ${OBJECTDIR}/_ext/566826655/pic18f4585.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/566826655/pic18f4585.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/1030658703/clock.p1: ../../../../utils/clock.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1030658703" 
	@${RM} ${OBJECTDIR}/_ext/1030658703/clock.p1.d 
	@${RM} ${OBJECTDIR}/_ext/1030658703/clock.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=pickit3  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/1030658703/clock.p1  ../../../../utils/clock.c 
	@-${MV} ${OBJECTDIR}/_ext/1030658703/clock.d ${OBJECTDIR}/_ext/1030658703/clock.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/1030658703/clock.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
else
${OBJECTDIR}/_ext/252781366/uart.p1: ../../../../comms/uart/uart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/252781366" 
	@${RM} ${OBJECTDIR}/_ext/252781366/uart.p1.d 
	@${RM} ${OBJECTDIR}/_ext/252781366/uart.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/252781366/uart.p1  ../../../../comms/uart/uart.c 
	@-${MV} ${OBJECTDIR}/_ext/252781366/uart.d ${OBJECTDIR}/_ext/252781366/uart.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/252781366/uart.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1: ../../../main_serial_logging.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2124829536" 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1.d 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1  ../../../main_serial_logging.c 
	@-${MV} ${OBJECTDIR}/_ext/2124829536/main_serial_logging.d ${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/2124829536/main_serial_logging.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/2146973904/serial_log.p1: ../../../../logger/serial_log.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2146973904" 
	@${RM} ${OBJECTDIR}/_ext/2146973904/serial_log.p1.d 
	@${RM} ${OBJECTDIR}/_ext/2146973904/serial_log.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/2146973904/serial_log.p1  ../../../../logger/serial_log.c 
	@-${MV} ${OBJECTDIR}/_ext/2146973904/serial_log.d ${OBJECTDIR}/_ext/2146973904/serial_log.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/2146973904/serial_log.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1: ../../../../processors/pic24FJ256GB106.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1  ../../../../processors/pic24FJ256GB106.c 
	@-${MV} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.d ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/566826655/pic18f2680.p1: ../../../../processors/pic18f2680.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f2680.p1.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f2680.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/566826655/pic18f2680.p1  ../../../../processors/pic18f2680.c 
	@-${MV} ${OBJECTDIR}/_ext/566826655/pic18f2680.d ${OBJECTDIR}/_ext/566826655/pic18f2680.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/566826655/pic18f2680.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1: ../../../../processors/dsPIC33EP256MU806.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1  ../../../../processors/dsPIC33EP256MU806.c 
	@-${MV} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.d ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/566826655/pic18f4585.p1: ../../../../processors/pic18f4585.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f4585.p1.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f4585.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/566826655/pic18f4585.p1  ../../../../processors/pic18f4585.c 
	@-${MV} ${OBJECTDIR}/_ext/566826655/pic18f4585.d ${OBJECTDIR}/_ext/566826655/pic18f4585.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/566826655/pic18f4585.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/1030658703/clock.p1: ../../../../utils/clock.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1030658703" 
	@${RM} ${OBJECTDIR}/_ext/1030658703/clock.p1.d 
	@${RM} ${OBJECTDIR}/_ext/1030658703/clock.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib $(COMPARISON_BUILD)  --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"    -o${OBJECTDIR}/_ext/1030658703/clock.p1  ../../../../utils/clock.c 
	@-${MV} ${OBJECTDIR}/_ext/1030658703/clock.d ${OBJECTDIR}/_ext/1030658703/clock.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/1030658703/clock.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/SerialLogging.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) --chip=$(MP_PROCESSOR_OPTION) -G -mdist/${CND_CONF}/${IMAGE_TYPE}/SerialLogging.X.${IMAGE_TYPE}.map  -D__DEBUG=1 --debugger=pickit3  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"     --rom=default,-bd30-bfff --ram=default,-cf4-cff,-f9c-f9c,-fd4-fd4,-fdb-fdf,-fe3-fe7,-feb-fef,-ffd-fff  $(COMPARISON_BUILD) --memorysummary dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml -odist/${CND_CONF}/${IMAGE_TYPE}/SerialLogging.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}     
	@${RM} dist/${CND_CONF}/${IMAGE_TYPE}/SerialLogging.X.${IMAGE_TYPE}.hex 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/SerialLogging.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) --chip=$(MP_PROCESSOR_OPTION) -G -mdist/${CND_CONF}/${IMAGE_TYPE}/SerialLogging.X.${IMAGE_TYPE}.map  --double=24 --float=24 --emi=wordwrite --opt=default,+asm,+asmfile,-speed,+space,-debug --addrqual=ignore --mode=free -DMCP -P -N255 -I"." -I"../../../../../" --warn=-3 --asmlist -DXPRJ_PIC18F4585=$(CND_CONF)  --summary=default,-psect,-class,+mem,-hex,-file --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,-plib --output=-mcof,+elf:multilocs --stack=compiled:auto:auto:auto "--errformat=%f:%l: error: (%n) %s" "--warnformat=%f:%l: warning: (%n) %s" "--msgformat=%f:%l: advisory: (%n) %s"     $(COMPARISON_BUILD) --memorysummary dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml -odist/${CND_CONF}/${IMAGE_TYPE}/SerialLogging.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}     
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/PIC18F4585
	${RM} -r dist/PIC18F4585

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell "${PATH_TO_IDE_BIN}"mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
