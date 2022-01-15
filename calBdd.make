CSRC_cal += 	cal.c calBddOp.c calBddManager.c calMemoryManagement.c\
		calHashTable.c calUtil.c calGC.c \
            	calTerminal.c calAssociation.c \
            	calBddSubstitute.c calReduce.c calQuant.c \
                calBddSwapVars.c calBddSatisfy.c calBddSize.c \
                calBddSupport.c calPrint.c calPrintProfile.c calDump.c\
		calHashTableOne.c calPipeline.c calPerformanceTest.c \
                calHashTableThree.c calBddITE.c calBddCompose.c\
		calCacheTableTwo.c calApplyReduce.c calBlk.c \
		calReorderBF.c calReorderDF.c calInteract.c\
		calBddVarSubstitute.c calReorderUtil.c calMem.c

HEADERS_cal += cal.h calInt.h calMem.h

MISC += calBddReorderTest.c calPerformanceTest.c calTest.c\
	calAllAbs.html  calAllByFile.html calAllByFunc.html\
	calAllDet.html  calAllFile.html calDesc.html\
 	calExt.html calExtAbs.html calExtDet.html calTitle.html credit.html\
 	calDoc.txt

DEPENDENCYFILES = $(CSRC_cal)
