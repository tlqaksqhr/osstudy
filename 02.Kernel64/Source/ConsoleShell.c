#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"
#include "Task.h"
#include "Synchronization.h"
#include "DynamicMemory.h"
#include "HardDisk.h"

SHELLCOMMANDENTRY gs_vstCommandTable[] = {
    {"help","Show Help",kHelp},
    {"cls","Clear Screen",kCls},
    {"totalram","Show Total RAM Size",kShowTotalRAMSize},
    {"strtod","String To Decimal/Hex Convert",kStringToDecimalHexTest},
    {"shutdown","Shutdown And Reboot OS",kShutDown},
    {"settimer","Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)",kSetTimer},
    {"wait","Wait ms Using PIT, ex)wait 100(ms)",kWaitUsingPIT},
    {"rdtsc","Read Time Stamp Counter",kReadTimeStampCounter},
    {"cpuspeed","Measure Processor Speed",kMeasureProcessorSpeed},
    {"date","Show Date And Time",kShowDateAndTime},
    {"createtask", "Create Task, ex)createtask 1(type) 10(count)", kCreateTestTask },
    {"changepriority","Change Task Priority, ex)changepriority 1(ID) 2(Priority)",kChangeTaskPriority},
    {"tasklist","Show Task List",kShowTaskList},
    {"killtask","End Task ex)killtask 1(ID) or 0xffffffff(All Task)",kKillTask},
    {"cpuload","Show Processor Load",kCPULoad},
    {"testmutex","Test Mutex Function",kTestMutex},
    {"testthread","Test Thread And Process Function",kTestThread},
    {"showmatrix","Show Matrix Screen",kShowMatrix},
    {"testpie","TEST PIE Calculation",kTestPIE},
    {"dynamicmeminfo","Show Dynamic Memory Information",kShowDynamicMemoryInformation},
    {"testeqalloc","Test Sequential Allocation & Free",kTestSequentialAllocation},
    {"testranalloc","Test Random Allocation & Free",kTestRandomAllocation},
    {"hddinfo","Show HDD Information",kShowHDDInformation},
    {"readsector","Read HDD Sector, ex) readsector 0(LBA) 10(count)",kReadSector},
    {"writesector","Write HDD Sector, ex) writesector 0(LBA) 10(count)",kWriteSector}
};

void kStartConsoleShell(void)
{
    char vcCommandBuffer[CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
    int iCommandBufferIndex = 0;
    BYTE bKey;
    int iCursorX,iCursorY;

    kPrintf(CONSOLESHELL_PROMPTMESSAGE);

    while(1)
    {
        bKey = kGetCh();

        if(bKey == KEY_BACKSPACE)
        {
            if(iCommandBufferIndex > 0)
            {
                kGetCursor(&iCursorX,&iCursorY);
                kPrintStringXY(iCursorX - 1,iCursorY," ");
                kSetCursor(iCursorX - 1,iCursorY);
                iCommandBufferIndex--;
            }
        }
        else if(bKey == KEY_ENTER)
        {
            kPrintf("\n");

            if(iCommandBufferIndex > 0)
            {
                vcCommandBuffer[iCommandBufferIndex] = '\0';
                kExecuteCommand(vcCommandBuffer);
            }

            kPrintf("%s",CONSOLESHELL_PROMPTMESSAGE);
            kMemSet(vcCommandBuffer,'\0',CONSOLESHELL_MAXCOMMANDBUFFERCOUNT);
            iCommandBufferIndex = 0;
        }
        else if((bKey == KEY_LSHIFT) || (bKey == KEY_RSHIFT) || \
                (bKey == KEY_CAPSLOCK) || (bKey == KEY_NUMLOCK) || \
                (bKey == KEY_SCROLLLOCK))
        {
            ;
        }
        else
        {
            if(bKey == KEY_TAB)
            {
                bKey = ' ';
            }

            if(iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT)
            {
                vcCommandBuffer[iCommandBufferIndex++] = bKey;
                kPrintf("%c",bKey);
            }
        }
    }
}

void kExecuteCommand(const char *pcCommandBuffer)
{
    int i,iSpaceIndex;
    int iCommandBufferLength,iCommandLength;
    int iCount;

    iCommandBufferLength = kStrLen(pcCommandBuffer);
    for(iSpaceIndex =0;iSpaceIndex<iCommandBufferLength;iSpaceIndex++)
    {
        if(pcCommandBuffer[iSpaceIndex] == ' ')
        {
            break;
        }
    }

    iCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);
    for(i=0;i<iCount;i++)
    {
        iCommandLength = kStrLen(gs_vstCommandTable[i].pcCommand);
        
        if((iCommandLength == iSpaceIndex) && (kMemCmp(gs_vstCommandTable[i].pcCommand,pcCommandBuffer,iSpaceIndex) == 0))
        {
            gs_vstCommandTable[i].pfFunction(pcCommandBuffer + iSpaceIndex + 1);
            break;
        }
    }

    if(i >= iCount)
    {
        kPrintf("'%s' is not found.\n",pcCommandBuffer);
    }
}

void kInitializeParameter(PARAMETERLIST *pstList,const char *pcParameter)
{
    pstList->pcBuffer = pcParameter;
    pstList->iLength = kStrLen(pcParameter);
    pstList->iCurrentPosition = 0;
}

int kGetNextParameter(PARAMETERLIST *pstList,char *pcParameter)
{
    int i;
    int iLength;

    if(pstList->iLength <= pstList->iCurrentPosition)
    {
        return 0;
    }

    for(i=pstList->iCurrentPosition;i<pstList->iLength;i++)
    {
        if(pstList->pcBuffer[i] == ' ')
        {
            break;
        }
    }

    kMemCpy(pcParameter,pstList->pcBuffer + pstList->iCurrentPosition,i);
    iLength = i - pstList->iCurrentPosition;
    pcParameter[iLength] = '\0';

    pstList->iCurrentPosition += iLength + 1;
    return iLength;
}

void kHelp(const char *pcParameterBuffer)
{
    int i;
    int iCount;
    int iCursorX,iCursorY;
    int iLength,iMaxCommandLength = 0;

    kPrintf("=============================================================================\n");
    kPrintf("                             MINT64 Shell Help                               \n");
    kPrintf("=============================================================================\n");

    iCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);

    for(i=0;i<iCount;i++)
    {
        iLength = kStrLen(gs_vstCommandTable[i].pcCommand);
        if(iLength > iMaxCommandLength)
        {
            iMaxCommandLength = iLength;
        }
    }

    for(i=0;i<iCount;i++)
    {
        kPrintf("%s",gs_vstCommandTable[i].pcCommand);
        kGetCursor(&iCursorX,&iCursorY);
        kSetCursor(iMaxCommandLength,iCursorY);
        kPrintf("  - %s\n",gs_vstCommandTable[i].pcHelp);

        if((i!=0) && ((i%20)==0))
        {
            kPrintf("Press Any key to continue.... ('q' is exit) : ");
            if(kGetCh() == 'q')
            {
                kPrintf("\n");
                break;
            }
            kPrintf("\n");
        }
    }
}

void kCls(const char *pcParameterBuffer)
{
    kClearScreen();
    kSetCursor(0,1);
}

void kShowTotalRAMSize(const char *pcParameterBuffer)
{
    kPrintf("Total RAM Size = %d MB\n",kGetTotalRAMSize());
}

void kStringToDecimalHexTest(const char *pcParameterBuffer)
{
    char vcParameter[100];
    int iLength;
    PARAMETERLIST stList;
    int iCount = 0;
    long lValue;

    kInitializeParameter(&stList,pcParameterBuffer);

    while(1)
    {
        iLength = kGetNextParameter(&stList,vcParameter);

        if(iLength == 0)
        {
            break;
        }

        kPrintf("Param %d = '%s', Length = %d, ",iCount+1,vcParameter,iLength);

        if(kMemCmp(vcParameter,"0x",2)==0)
        {
            lValue = kAToI(vcParameter+2,16);
            kPrintf("HEX Value = %q\n",lValue);
        }
        else
        {
            lValue = kAToI(vcParameter,10);
            kPrintf("Decimal Value = %d\n",lValue);
        }

        iCount++;
    }

}

void kShutDown(const char *pcParameterBuffer)
{
    kPrintf("System Shutdown Start...\n");
    kPrintf("Press Any Key To Reboot PC...");
    kGetCh();
    kReboot();
}

void kSetTimer(const char *pcParameterBuffer)
{
    char vcParameter[100];
    PARAMETERLIST stList;
    long lValue;
    BOOL bPeriodic;

    kInitializeParameter(&stList,pcParameterBuffer);

    if(kGetNextParameter(&stList,vcParameter) == 0)
    {
        kPrintf("ex)settimer 10(ms) 1(periodic)\n");
        return ;
    }
    lValue = kAToI(vcParameter,10);

    if(kGetNextParameter(&stList,vcParameter) == 0)
    {
        kPrintf("ex)settimer 10(ms) 1(periodic)\n");
        return ;        
    }
    bPeriodic = kAToI(vcParameter,10);

    kInitializePIT(MSTOCOUNT(lValue),bPeriodic);
    kPrintf("Time = %d ms, Periodic = %d Change Complete\n",lValue,bPeriodic);
}

void kWaitUsingPIT(const char *pcParameterBuffer)
{
    char vcParameter[100];
    int iLength;
    PARAMETERLIST stList;
    long lMiliSecond;
    int i;

    kInitializeParameter(&stList,pcParameterBuffer);

    if(kGetNextParameter(&stList,vcParameter) == 0)
    {
        kPrintf("ex)wait 100(ms)\n");
        return ;
    }

    lMiliSecond = kAToI(pcParameterBuffer,10);
    kPrintf("%d ms Sleep Start...\n",lMiliSecond);

    kDisableInterrupt();
    for(i=0;i<lMiliSecond/30;i++)
    {
        kWaitUsingDirectPIT(MSTOCOUNT(30));
    }
    kWaitUsingDirectPIT(MSTOCOUNT(lMiliSecond % 30));
    kEnableInterrupt();
    kPrintf("%d ms Sleep Complete\n",lMiliSecond);

    kInitializePIT(MSTOCOUNT(1),TRUE);
}

void kReadTimeStampCounter(const char *pcParameterBuffer)
{
    QWORD qwTSC;

    qwTSC = kReadTSC();
    kPrintf("Time Stamp Counter = %q\n",qwTSC);
}

void kMeasureProcessorSpeed(const char *pcParameterBuffer)
{
    int i;
    QWORD qwLastTSC,qwTotalTSC = 0;

    kPrintf("Now Measuring");

    kDisableInterrupt();

    for(i=0;i<200;i++)
    {
        qwLastTSC = kReadTSC();
        kWaitUsingDirectPIT(MSTOCOUNT(50));
        qwTotalTSC += kReadTSC() - qwLastTSC;

        kPrintf(".");
    }

    kInitializePIT(MSTOCOUNT(1),TRUE);
    kEnableInterrupt();

    kPrintf("\nCPU Speed = %d Mhz\n",qwTotalTSC / 10 / 1000 / 1000);
}

void kShowDateAndTime(const char *pcParameterBuffer)
{
    BYTE bSecond,bMinute,bHour;
    BYTE bDayOfWeek,bDayOfMonth,bMonth;
    WORD wYear;

    kReadRTCTime(&bHour,&bMinute,&bSecond);
    kReadRTCDate(&wYear,&bMonth,&bDayOfMonth,&bDayOfWeek);

    kPrintf("Date: %d/%d/%d %s, ",wYear,bMonth,bDayOfMonth,kConvertDayOfWeekToString(bDayOfWeek));
    kPrintf("Time: %d:%d:%d\n",bHour,bMinute,bSecond);
}

static TCB gs_vstTask[2] = {0,};
static QWORD gs_vstStack[1024] = {0,};

void kTestTask(void)
{
    int i = 0;

    while(1)
    {
        kPrintf("[%d] This message is from kTestTask, Press any key to switch kConsoleShell~!!\n",i++);

        kGetCh();

        kSwitchContext(&(gs_vstTask[1].stContext),&(gs_vstTask[0].stContext));
    }
}

void kTestTask1(void)
{
    BYTE bData;
    int i = 0, iX = 0, iY = 0, iMargin,j;
    CHARACTER *pstScreen = (CHARACTER *)CONSOLE_VIDEOMEMORYADDRESS;
    TCB *pstRunningTask;

    pstRunningTask = kGetRunningTask();
    iMargin = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) % 10;

    for(j=0;j<20000;j++)
    {
        switch(i)
        {
        case 0:
            iX++;
            if(iX >= (CONSOLE_WIDTH - iMargin))
            {
                i = 1;
            }
            break;
        case 1:
            iY++;
            if(iY >= (CONSOLE_HEIGHT - iMargin))
            {
                i = 2;
            }
            break;      
        case 2:
            iX--;
            if(iX < iMargin)
            {
                i = 3;
            }
            break;              
        case 3:
            iY--;
            if(iY < iMargin)
            {
                i = 0;
            }
            break; 
        }

        pstScreen[iY * CONSOLE_WIDTH + iX].bCharactor = bData;
        pstScreen[iY * CONSOLE_WIDTH + iX].bAttribute = bData & 0x0F;
        bData++;

        //kSchedule();
    }

    kExitTask();
}

void kTestTask2(void)
{
    int i = 0,iOffset;
    CHARACTER *pstScreen = (CHARACTER *)CONSOLE_VIDEOMEMORYADDRESS;
    TCB *pstRunningTask;
    char vcData[4] = {'-','\\','|','/'};

    pstRunningTask = kGetRunningTask();
    iOffset = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) * 2;
    iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

    while(1)
    {
        pstScreen[iOffset].bCharactor = vcData[i%4];
        pstScreen[iOffset].bAttribute = (iOffset % 15) + 1;
        i++;

        //kSchedule();
    }
}

void kCreateTestTask(const char *pcParameterBuffer)
{
    PARAMETERLIST stList;
    char vcType[30];
    char vcCount[30];
    int i;

    kInitializeParameter(&stList,pcParameterBuffer);
    kGetNextParameter(&stList,vcType);
    kGetNextParameter(&stList,vcCount);

    switch(kAToI(vcType,10))
    {
    case 1:
        for(i=0;i<kAToI(vcCount,10);i++)
        {
            if(kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD,0,0,(QWORD)kTestTask1) == NULL)
            {
                break;
            }
        }
        kPrintf("Task1 %d Created\n",i);
        break;
    case 2:
    default:
        for(i=0;i<kAToI(vcCount,10);i++)
        {
            if(kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD,0,0,(QWORD)kTestTask2) == NULL)
            {
                break;
            }
        }
        kPrintf("Task2 %d Created\n",i);
        break;  
    }
}

static void kChangeTaskPriority(const char *pcParameterBuffer)
{
    PARAMETERLIST stList;
    char vcID[30];
    char vcPriority[30];
    QWORD qwID;
    BYTE bPriority;

    kInitializeParameter(&stList,pcParameterBuffer);
    kGetNextParameter(&stList,vcID);
    kGetNextParameter(&stList,vcPriority);
    
    if(kMemCmp(vcID,"0x",2) == 0)
    {
        qwID = kAToI(vcID+2,16);
    }
    else
    {
        qwID = kAToI(vcID,10);
    }

    bPriority = kAToI(vcPriority,10);

    kPrintf("Change Task Priority ID [0x%q] Priority[%d] ",qwID,bPriority);

    if(kChangePriority(qwID,bPriority) == TRUE)
    {
        kPrintf("Success\n");
    }
    else
    {
        kPrintf("Fail\n");
    }
}

static void kShowTaskList(const char *pcParameterBuffer)
{
    int i;
    TCB *pstTCB;
    int iCount = 0;

    kPrintf("===================== Task Total Count [%d] =====================\n",kGetTaskCount());

    for(i=0;i<TASK_MAXCOUNT;i++)
    {
        pstTCB = kGetTCBInTCBPool(i);
        if((pstTCB->stLink.qwID >> 32) != 0)
        {
            if((iCount != 0) && ((iCount % 10) == 0))
            {
                kPrintf("Press any key to continue... ('q' is exit) : ");
                if(kGetCh() == 'q')
                {
                    kPrintf("\n");
                    break;
                }
                kPrintf("\n");
            }
            kPrintf("[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q], Thread[%d]\n",1 + iCount++,pstTCB->stLink.qwID,GETPRIORITY(pstTCB->qwFlags),pstTCB->qwFlags,kGetListCount(&(pstTCB->stChildThreadList)));
            kPrintf("   Parent PID[0x%Q], Memory Address[0x%Q], Size[0x%Q]\n",pstTCB->qwParentProcessID,pstTCB->pvMemoryAddress,pstTCB->qwMemorySize);
        }
    }
}

static void kKillTask(const char *pcParameterBuffer)
{
    PARAMETERLIST stList;
    char vcID[30];
    QWORD qwID;
    TCB *pstTCB;
    int i;

    kInitializeParameter(&stList,pcParameterBuffer);
    kGetNextParameter(&stList,vcID);

    if(kMemCmp(vcID,"0x",2) == 0)
    {
        qwID = kAToI(vcID+2,16);
    }
    else
    {
        qwID = kAToI(vcID,10);
    }

    if(qwID != 0xFFFFFFFF)
    {
        pstTCB = kGetTCBInTCBPool(GETTCBOFFSET(qwID));
        qwID = pstTCB->stLink.qwID;
        
        if( ((qwID >> 32) != 0) && ((pstTCB->qwFlags & TASK_FLAGS_SYSTEM) == 0x00))
        {
            kPrintf("Kill Task ID [0x%q]",qwID);
            if(kEndTask(qwID) == TRUE)
            {
                kPrintf("Success\n");
            }
            else
            {
                kPrintf("Fail\n");
            }
        }
        else
        {
            kPrintf("Task does not exist or task is system task\n");
        }
    }
    else
    {
        for(i=2;i<TASK_MAXCOUNT;i++)
        {
            pstTCB = kGetTCBInTCBPool(i);
            qwID = pstTCB->stLink.qwID;
            if( ((qwID >> 32) != 0) && ((pstTCB->qwFlags & TASK_FLAGS_SYSTEM) == 0x00))
            {
                kPrintf("Kill Task ID [0x%q]",qwID);
                if(kEndTask(qwID) == TRUE)
                {
                    kPrintf("Success\n");
                }
                else
                {
                    kPrintf("Fail\n");
                }
            }
        }
    }
}

static void kCPULoad(const char *pcParameterBuffer)
{
    kPrintf("Processor Load : %d%%\n",kGetProcessorLoad());
}

static MUTEX gs_stMutex;
static volatile QWORD gs_qwAddr;

static void kPrintNumberTask(void)
{
    int i,j;
    QWORD qwTickCount;

    qwTickCount = kGetTickCount();

    while((kGetTickCount() - qwTickCount) < 50)
    {
        kSchedule();
    }

    for(i=0;i<5;i++)
    {
        kLock(&(gs_stMutex));
        kPrintf("Task ID [0x%Q] Value[%d]\n",kGetRunningTask()->stLink.qwID,gs_qwAddr);
        gs_qwAddr += 1;
        kUnlock(&(gs_stMutex));

        for(j=0;j<30000;j++);
    }

    qwTickCount = kGetTickCount();
    while((kGetTickCount() - qwTickCount) < 1000)
    {
        kSchedule();
    }

    kExitTask();
}

static void kTestMutex(const char *pcParameterBuffer)
{
    int i;

    gs_qwAddr = 1;

    kInitializeMutex(&(gs_stMutex));

    for(i=0;i<3;i++)
    {
        kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD,0,0,(QWORD)kPrintNumberTask);
    }
    kPrintf("Wait Until %d Task End...\n",i);
    kGetCh();
}

static void kCreateThreadTask(void)
{
    int i;

    for(i=0;i<3;i++)
    {
        kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD,0,0,(QWORD)kTestTask2);
    }

    while(1)
    {
        kSleep(1);
    }
}

static void kTestThread(const char *pcParameterBuffer)
{
    TCB *pstProcess;

    pstProcess = kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_PROCESS,(void *)0xEEEEEEEE,0x1000,(QWORD)kCreateThreadTask);

    if(pstProcess != NULL)
    {
        kPrintf("Process [0x%Q] Create Success\n",pstProcess->stLink.qwID);
    }
    else
    {
        kPrintf("Process Create Fail;\n");
    }
}


static volatile QWORD gs_qwRandomvalue = 0;

QWORD kRandom(void)
{
    gs_qwRandomvalue = (gs_qwRandomvalue * 412153 + 5571031) >> 16;
    return gs_qwRandomvalue;
}

static void kDropCharactorThread(void)
{
    int iX,iY;
    int i;
    char vcText[2] = {0,};

    iX = kRandom() % CONSOLE_WIDTH;

    while(1)
    {
        kSleep(kRandom()%20);

        if(kRandom() % 20 < 15)
        {
            vcText[0] = ' ';
            for(i=0;i<CONSOLE_HEIGHT-1;i++)
            {
                kPrintStringXY(iX,i,vcText);
                kSleep(50);
            }
        }
        else
        {
            for(i=0;i<CONSOLE_HEIGHT-1;i++)
            {
                vcText[0] = i+kRandom();
                kPrintStringXY(iX,i,vcText);
                kSleep(50);
            }
        }
    }
}

static void kMatrixProcess(void)
{
    int i;

    for(i=0;i<100;i++)
    {
        if(kCreateTask(TASK_FLAGS_THREAD | TASK_FLAGS_LOW,0,0,(QWORD)kDropCharactorThread) == NULL)
        {
            break;
        }

        kSleep(kRandom() % 5 + 5);
    }

    kPrintf("%d Thread is created\n");

    kGetCh();
}

static void kShowMatrix(const char *pcParameterBuffer)
{
    TCB *pstProcess;

    pstProcess = kCreateTask(TASK_FLAGS_PROCESS | TASK_FLAGS_LOW,(void *)0xE00000,0xE00000,(QWORD)kMatrixProcess);

    if(pstProcess != NULL)
    {
        kPrintf("Matrix Process [0x%Q] Create Success\n");

        while((pstProcess->stLink.qwID >> 32) != 0)
        {
            kSleep(100);
        }
    }
    else
    {
        kPrintf("Matrix Process Create Fail\n");
    }
}

static void kFPUTestTask(void)
{
    double dValue1,dValue2;
    TCB *pstRunningTask;
    QWORD qwCount;
    QWORD qwRandomValue;
    int i,iOffset;
    char vcData[5] = {'-','\\','|','/'};
    CHARACTER *pstScreen = (CHARACTER *)CONSOLE_VIDEOMEMORYADDRESS;

    pstRunningTask = kGetRunningTask();

    iOffset = (pstRunningTask->stLink.qwID & 0xFFFFFFFF);
    iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

    while(1)
    {
        dValue1 = 1;
        dValue2 = 1;
        for(i=0;i<10;i++)
        {
            qwRandomValue = kRandom();
            dValue1 *= (double)qwRandomValue;
            dValue2 *= (double)qwRandomValue;

            kSleep(1);

            qwRandomValue = kRandom();
            dValue1 /= (double)qwRandomValue;
            dValue2 /= (double)qwRandomValue;
        }
        if(dValue1 != dValue2)
        {
            kPrintf("Value Is Not Same~!!! [%f] != [%f]\n",dValue1,dValue2);
            break;
        }

        qwCount++;

        pstScreen[iOffset].bCharactor = vcData[qwCount % 4];
        pstScreen[iOffset].bAttribute = (iOffset % 15) + 1;
    }
    
}

static void kTestPIE(const char *pcParameterBuffer)
{
    double dResult;
    int i;

    kPrintf("PIE Calculation Test\n");
    kPrintf("Result: 355/113 = ");
    dResult = (double)355 / 113;
    kPrintf("%d.%d%d\n",(QWORD)dResult,((QWORD)(dResult*10)%10 ),((QWORD)(dResult * 100) % 10));

    for(i=0;i<100;i++)
    {
       kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD,0,0,(QWORD)kFPUTestTask);
    }
}

static void kShowDynamicMemoryInformation(const char *pcParameterBuffer)
{
    QWORD qwStartAddress,qwTotalSize,qwMetaSize,qwUsedSize;

    kGetDynamicMemoryInformation(&qwStartAddress,&qwTotalSize,&qwMetaSize,&qwUsedSize);

    kPrintf("============ Dynamic Memory Information ====================\n");
    kPrintf("Start Address: [0x%Q]\n",qwStartAddress);
    kPrintf("Total Size: [0x%Q]byte, [%d]MB\n",qwTotalSize,qwTotalSize/1024/1024);
    kPrintf("Meta Size: [0x%Q]byte, [%d]KB\n",qwMetaSize,qwMetaSize/1024);
    kPrintf("Used Size: [0x%Q]byte, [%d]KB\n",qwUsedSize,qwUsedSize/1024);
}

static void kTestSequentialAllocation(const char *pcParameterBuffer)
{
    DYNAMICMEMORY *pstMemory;
    long i,j,k;
    QWORD *pqwBuffer;

    kPrintf("============== Dynamic Memory Test ==================\n");
    pstMemory = kGetDynamicMemoryManager();

    for(i=0;i<pstMemory->iMaxLevelCount;i++)
    {
        kPrintf("Block List [%d] Test Start\n",i);
        kPrintf("Allocation And Compare: ");

        for(j=0;j<(pstMemory->iBlockCountOfSmallestBlock >> i);j++)
        {
            pqwBuffer = kAllocateMemory(DYNAMICMEMORY_MIN_SIZE << i);
            if(pqwBuffer == NULL)
            {
                kPrintf("\nAllocation Fail\n");
                return ;
            }

            for(k=0;k<(DYNAMICMEMORY_MIN_SIZE << i)/8;k++)
            {
                pqwBuffer[k] = k;
            }

            for(k=0;k<(DYNAMICMEMORY_MIN_SIZE << i)/8;k++)
            {
                if(pqwBuffer[k] != k)
                {
                    kPrintf("\nCompare Fail\n");
                    return ;
                }
            }
            kPrintf(".");
        }

        kPrintf("\nFree");

        for(j=0;j<(pstMemory->iBlockCountOfSmallestBlock >> i);j++)
        {
            if(kFreeMemory((void*)(pstMemory->qwStartAddress + (DYNAMICMEMORY_MIN_SIZE << i) * j)) == FALSE)
            {
                kPrintf("\nFree Fail\n");
                return ;
            }
            kPrintf(".");
        }
        kPrintf("\n");
    }
    kPrintf("Test Complete~!!!\n");
}

static void kRandomAllocationTask(void)
{
    TCB *pstTask;
    QWORD qwMemorySize;
    char vcBuffer[200];
    BYTE *pbAllocationBuffer;
    int i,j;
    int iY;

    pstTask = kGetRunningTask();
    iY = (pstTask->stLink.qwID)%15 + 9;

    for(j=0;j<10;j++)
    {
        do
        {
            qwMemorySize = ((kRandom() % (32 * 1024)) + 1) * 1024;
            pbAllocationBuffer = kAllocateMemory(qwMemorySize);

            if(pbAllocationBuffer == 0)
            {
                kSleep(1);
            }
        }
        while(pbAllocationBuffer == 0);

        kSPrintf(vcBuffer,"|Address: [0x%Q] Size: [0x%Q] Allocation Success",pbAllocationBuffer,qwMemorySize);

        kPrintStringXY(20,iY,vcBuffer);
        kSleep(200);

        kSPrintf(vcBuffer,"|Address: [0x%Q] Size: [0x%Q] Data Write...       ",pbAllocationBuffer,qwMemorySize);
        kPrintStringXY(20,iY,vcBuffer);

        for(i=0;i<qwMemorySize/2;i++)
        {
            pbAllocationBuffer[i] = kRandom() & 0xFF;
            pbAllocationBuffer[i+(qwMemorySize / 2)] = pbAllocationBuffer[i];
        }
        kSleep(200);

        kSPrintf(vcBuffer,"|Address: [0x%Q] Size: [0x%Q] Data Verify...       ",pbAllocationBuffer,qwMemorySize);
        kPrintStringXY(20,iY,vcBuffer);

        for(i=0;i<qwMemorySize/2;i++)
        {
            if(pbAllocationBuffer[i] != pbAllocationBuffer[i+(qwMemorySize/2)])
            {
                kPrintf("Task ID[0x%Q] Verify Fail\n",pstTask->stLink.qwID);
                kExitTask();
            }
        }
        kFreeMemory(pbAllocationBuffer);
        kSleep(200);
    }

    kExitTask();
}

static void kTestRandomAllocation(const char *pcParameterBuffer)
{
    int i;

    for(i=0;i<1000;i++)
    {
        kCreateTask(TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD,0,0,(QWORD)kRandomAllocationTask);
    }
}

static void kShowHDDInformation(const char *pcParameterBuffer)
{
    HDDINFORMATION stHDD;
    char vcBuffer[100];

    if(kReadHDDInformation(TRUE,TRUE,&stHDD) == FALSE)
    {
        kPrintf("HDD Information Read Fail\n");
        return ;
    }

    kPrintf("======================= Primary Master HDD Information =======================\n");

    kMemCpy(vcBuffer,stHDD.vwModelNumber,sizeof(stHDD.vwModelNumber));
    vcBuffer[sizeof(stHDD.vwModelNumber)-1] = '\0';
    kPrintf("Model Number:\t %s\n",vcBuffer);

    kMemCpy(vcBuffer,stHDD.vwSerialNumber,sizeof(stHDD.vwSerialNumber));
    vcBuffer[sizeof(stHDD.vwSerialNumber)-1] = '\0';
    kPrintf("Serial Number:\t %s\n",vcBuffer);

    kPrintf("Head Count:\t %d\n",stHDD.wNumberOfHead);
    kPrintf("Cylinder Count:\t %d\n",stHDD.wNumberOfCylinder);
    kPrintf("Sector Count:\t %d\n",stHDD.wNumberOfSectorPerCylinder);

    kPrintf("Total Sector:\t %d Sector, %dMB\n",stHDD.dwTotalSectors,stHDD.dwTotalSectors/2/1024);
}

static void kReadSector(const char *pcParameterBuffer)
{
    PARAMETERLIST stList;
    char vcLBA[50],vcSectorCount[50];
    DWORD dwLBA;
    int iSectorCount;
    char *pcBuffer;
    int i,j;
    BYTE bData;
    BOOL bExit = FALSE;

    kInitializeParameter(&stList,pcParameterBuffer);

    if((kGetNextParameter(&stList,vcLBA)==0) || (kGetNextParameter(&stList,vcSectorCount)==0))
    {
        kPrintf("ex) readsector 0(LBA) 10(count)\n");
        return ;
    }
    dwLBA = kAToI(vcLBA,10);
    iSectorCount = kAToI(vcSectorCount,10);

    pcBuffer = kAllocateMemory(iSectorCount * 512);

    if(kReadHDDSector(TRUE,TRUE,dwLBA,iSectorCount,pcBuffer) == iSectorCount)
    {
        kPrintf("LBA [%d] [%d] Sector Read Success~!!",dwLBA,iSectorCount);

        for(j=0;j<iSectorCount;j++)
        {
            for(i=0;i<512;i++)
            {
                if(!((j==0) && (i==0)) && ((i%256)==0))
                {
                    kPrintf("\nPress any key to continue... ('q' is exit) : ");
                    if(kGetCh() == 'q')
                    {
                        bExit = TRUE;
                        break;
                    }
                }

                if((i%16) == 0)
                {
                    kPrintf("\n[LBA:%d, Offset:%d]\t| ",dwLBA+j,i);
                }

                bData = pcBuffer[j*512 + i] & 0xFF;
                if(bData < 16)
                {
                    kPrintf("0");
                }
                kPrintf("%X ",bData);
            }

            if(bExit == TRUE)
            {
                break;
            }
        }
        kPrintf("\n");
    }
    else
    {
        kPrintf("Read Fail\n");
    }

    kFreeMemory(pcBuffer);
}

static void kWriteSector(const char *pcParameterBuffer)
{
    PARAMETERLIST stList;
    char vcLBA[50],vcSectorCount[50];
    DWORD dwLBA;
    int iSectorCount;
    char *pcBuffer;
    int i,j;
    BOOL bExit = FALSE;
    BYTE bData;
    static DWORD s_dwWriteCount = 0;

    kInitializeParameter(&stList,pcParameterBuffer);

    if((kGetNextParameter(&stList,vcLBA) == 0) || (kGetNextParameter(&stList,vcSectorCount)==0))
    {
        kPrintf("ex) writesector 0(LBA) 10(count)\n");
        return ;
    }

    dwLBA = kAToI(vcLBA,10);
    iSectorCount = kAToI(vcSectorCount,10);

    s_dwWriteCount++;

    pcBuffer = kAllocateMemory(iSectorCount * 512);

    for(j=0;j<iSectorCount;j++)
    {
        for(i=0;i<512;i+=8)
        {
            *(DWORD *)&(pcBuffer[j * 512 + i]) = dwLBA + j;
            *(DWORD *)&(pcBuffer[j * 512 + i + 4]) = s_dwWriteCount;
        }
    }

    // This is problem
    if(kWriteHDDSector(TRUE,TRUE,dwLBA,iSectorCount,pcBuffer) != iSectorCount)
    {
        kPrintf("Write Fail\n");
        return ;
    }

    kPrintf("LBA [%d], [%d] Sector Read Success~!!",dwLBA,iSectorCount);

    for(j=0;j<iSectorCount;j++)
    {
        for(i=0;i<512;i++)
        {
            if(!((j==0) && (i==0)) && ((i%256)==0))
            {
                kPrintf("\nPress any key to continue... ('q' is exit): ");
                if(kGetCh() == 'q')
                {
                    bExit = TRUE;
                    break;
                }
            }

            if((i%16) == 0)
            {
                kPrintf("\n[LBA:%d, Offset:%d[\t]| ",dwLBA+j,i);
            }

            bData = pcBuffer[j*512 + i] & 0xFF;
            if(bData < 16)
            {
                kPrintf("0");
            }
            kPrintf("%X ",bData);
        }

        if(bExit == TRUE)
        {
            break;
        }
    }

    kPrintf("\n");
    kFreeMemory(pcBuffer);
}