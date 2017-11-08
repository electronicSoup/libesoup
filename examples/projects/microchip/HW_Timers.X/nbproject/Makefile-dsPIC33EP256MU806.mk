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
ifeq "$(wildcard nbproject/Makefile-local-dsPIC33EP256MU806.mk)" "nbproject/Makefile-local-dsPIC33EP256MU806.mk"
include nbproject/Makefile-local-dsPIC33EP256MU806.mk
endif
endif

# Environment
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=dsPIC33EP256MU806
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

ifdef SUB_IMAGE_ADDRESS
SUB_IMAGE_ADDRESS_COMMAND=--image-address $(SUB_IMAGE_ADDRESS)
else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../../../../processors/pic24FJ256GB106.c ../../../../processors/pic18f2680.c ../../../../processors/dsPIC33EP256MU806.c ../../../../processors/pic18f4585.c ../../../../timers/hw_timers.c ../../../../utils/clock.c ../../../main_hw_timers.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o ${OBJECTDIR}/_ext/566826655/pic18f2680.o ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o ${OBJECTDIR}/_ext/566826655/pic18f4585.o ${OBJECTDIR}/_ext/1924324082/hw_timers.o ${OBJECTDIR}/_ext/1030658703/clock.o ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o.d ${OBJECTDIR}/_ext/566826655/pic18f2680.o.d ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o.d ${OBJECTDIR}/_ext/566826655/pic18f4585.o.d ${OBJECTDIR}/_ext/1924324082/hw_timers.o.d ${OBJECTDIR}/_ext/1030658703/clock.o.d ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o ${OBJECTDIR}/_ext/566826655/pic18f2680.o ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o ${OBJECTDIR}/_ext/566826655/pic18f4585.o ${OBJECTDIR}/_ext/1924324082/hw_timers.o ${OBJECTDIR}/_ext/1030658703/clock.o ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o

# Source Files
SOURCEFILES=../../../../processors/pic24FJ256GB106.c ../../../../processors/pic18f2680.c ../../../../processors/dsPIC33EP256MU806.c ../../../../processors/pic18f4585.c ../../../../timers/hw_timers.c ../../../../utils/clock.c ../../../main_hw_timers.c


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
	${MAKE}  -f nbproject/Makefile-dsPIC33EP256MU806.mk dist/${CND_CONF}/${IMAGE_TYPE}/HW_Timers.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP256MU806
MP_LINKER_FILE_OPTION=,--script=p33EP256MU806.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o: ../../../../processors/pic24FJ256GB106.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/pic24FJ256GB106.c  -o ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/566826655/pic18f2680.o: ../../../../processors/pic18f2680.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f2680.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f2680.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/pic18f2680.c  -o ${OBJECTDIR}/_ext/566826655/pic18f2680.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/pic18f2680.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/pic18f2680.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o: ../../../../processors/dsPIC33EP256MU806.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/dsPIC33EP256MU806.c  -o ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/566826655/pic18f4585.o: ../../../../processors/pic18f4585.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f4585.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f4585.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/pic18f4585.c  -o ${OBJECTDIR}/_ext/566826655/pic18f4585.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/pic18f4585.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/pic18f4585.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1924324082/hw_timers.o: ../../../../timers/hw_timers.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1924324082" 
	@${RM} ${OBJECTDIR}/_ext/1924324082/hw_timers.o.d 
	@${RM} ${OBJECTDIR}/_ext/1924324082/hw_timers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../timers/hw_timers.c  -o ${OBJECTDIR}/_ext/1924324082/hw_timers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1924324082/hw_timers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1924324082/hw_timers.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1030658703/clock.o: ../../../../utils/clock.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1030658703" 
	@${RM} ${OBJECTDIR}/_ext/1030658703/clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/1030658703/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../utils/clock.c  -o ${OBJECTDIR}/_ext/1030658703/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1030658703/clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1030658703/clock.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/2124829536/main_hw_timers.o: ../../../main_hw_timers.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2124829536" 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o.d 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../main_hw_timers.c  -o ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/2124829536/main_hw_timers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/2124829536/main_hw_timers.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
else
${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o: ../../../../processors/pic24FJ256GB106.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/pic24FJ256GB106.c  -o ${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/pic24FJ256GB106.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/566826655/pic18f2680.o: ../../../../processors/pic18f2680.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f2680.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f2680.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/pic18f2680.c  -o ${OBJECTDIR}/_ext/566826655/pic18f2680.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/pic18f2680.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/pic18f2680.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o: ../../../../processors/dsPIC33EP256MU806.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/dsPIC33EP256MU806.c  -o ${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/dsPIC33EP256MU806.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/566826655/pic18f4585.o: ../../../../processors/pic18f4585.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/566826655" 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f4585.o.d 
	@${RM} ${OBJECTDIR}/_ext/566826655/pic18f4585.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../processors/pic18f4585.c  -o ${OBJECTDIR}/_ext/566826655/pic18f4585.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/566826655/pic18f4585.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/566826655/pic18f4585.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1924324082/hw_timers.o: ../../../../timers/hw_timers.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1924324082" 
	@${RM} ${OBJECTDIR}/_ext/1924324082/hw_timers.o.d 
	@${RM} ${OBJECTDIR}/_ext/1924324082/hw_timers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../timers/hw_timers.c  -o ${OBJECTDIR}/_ext/1924324082/hw_timers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1924324082/hw_timers.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1924324082/hw_timers.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1030658703/clock.o: ../../../../utils/clock.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1030658703" 
	@${RM} ${OBJECTDIR}/_ext/1030658703/clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/1030658703/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../utils/clock.c  -o ${OBJECTDIR}/_ext/1030658703/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1030658703/clock.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1030658703/clock.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/2124829536/main_hw_timers.o: ../../../main_hw_timers.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2124829536" 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o.d 
	@${RM} ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../main_hw_timers.c  -o ${OBJECTDIR}/_ext/2124829536/main_hw_timers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/2124829536/main_hw_timers.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -I"." -I"../../../../../" -msmart-io=1 -Wall -msfr-warn=off  
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
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/HW_Timers.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)   -mreserve=data@0x1000:0x101B -mreserve=data@0x101C:0x101D -mreserve=data@0x101E:0x101F -mreserve=data@0x1020:0x1021 -mreserve=data@0x1022:0x1023 -mreserve=data@0x1024:0x1027 -mreserve=data@0x1028:0x104F   -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK3=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST) 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/HW_Timers.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/HW_Timers.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DXPRJ_dsPIC33EP256MU806=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST) 
	${MP_CC_DIR}/xc16-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/HW_Timers.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf  
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/dsPIC33EP256MU806
	${RM} -r dist/dsPIC33EP256MU806

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell "${PATH_TO_IDE_BIN}"mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
