/**
 * @file HypervisorRoutines.c
 * @author Sina Karvandi (sina@rayanfam.com)
 * @brief This file describes the routines in Hypervisor
 * @details vmx related routines
 * @version 0.1
 * @date 2020-04-11
 * 
 * @copyright This project is released under the GNU Public License v3.
 * 
 */
#include "..\hprdbghv\pch.h"

/**
 * @brief Adjust controls for VMCS based on processor capability
 * 
 * @param Ctl 
 * @param Msr 
 * @return ULONG Returns the Cpu Based and Secondary Processor Based Controls
 *  and other controls based on hardware support
 */
ULONG
HvAdjustControls(ULONG Ctl, ULONG Msr)
{
    MSR MsrValue = {0};

    MsrValue.Content = __readmsr(Msr);
    Ctl &= MsrValue.High; /* bit == 0 in high word ==> must be zero */
    Ctl |= MsrValue.Low;  /* bit == 1 in low word  ==> must be one  */
    return Ctl;
}

/**
 * @brief Set guest's selector registers
 * 
 * @param GdtBase 
 * @param SegmentRegister 
 * @param Selector 
 * @return BOOLEAN 
 */
BOOLEAN
HvSetGuestSelector(PVOID GdtBase, ULONG SegmentRegister, USHORT Selector)
{
    SEGMENT_SELECTOR SegmentSelector = {0};
    ULONG            AccessRights;

    GetSegmentDescriptor(&SegmentSelector, Selector, GdtBase);
    AccessRights = ((PUCHAR)&SegmentSelector.ATTRIBUTES)[0] + (((PUCHAR)&SegmentSelector.ATTRIBUTES)[1] << 12);

    if (!Selector)
        AccessRights |= 0x10000;

    __vmx_vmwrite(GUEST_ES_SELECTOR + SegmentRegister * 2, Selector);
    __vmx_vmwrite(GUEST_ES_LIMIT + SegmentRegister * 2, SegmentSelector.LIMIT);
    __vmx_vmwrite(GUEST_ES_AR_BYTES + SegmentRegister * 2, AccessRights);
    __vmx_vmwrite(GUEST_ES_BASE + SegmentRegister * 2, SegmentSelector.BASE);

    return TRUE;
}

/**
 * @brief Handle Cpuid Vmexits
 * 
 * @param RegistersState Guest's gp registers
 * @return VOID 
 */
VOID
HvHandleCpuid(PGUEST_REGS RegistersState)
{
    INT32  CpuInfo[4];
    ULONG  Mode    = 0;
    UINT64 Context = 0;

    //
    // Set the context (save eax for the debugger)
    //
    Context = RegistersState->rax;

    //
    // Otherwise, issue the CPUID to the logical processor based on the indexes
    // on the VP's GPRs.
    //
    __cpuidex(CpuInfo, (INT32)RegistersState->rax, (INT32)RegistersState->rcx);

    //
    // check whether we are in transparent mode or not
    // if we are in transparent mode then ignore the
    // cpuid modifications e.g. hyperviosr name or bit
    //
    if (!g_TransparentMode)
    {
        //
        // Check if this was CPUID 1h, which is the features request
        //
        if (RegistersState->rax == CPUID_PROCESSOR_AND_PROCESSOR_FEATURE_IDENTIFIERS)
        {
            //
            // Set the Hypervisor Present-bit in RCX, which Intel and AMD have both
            // reserved for this indication
            //
            CpuInfo[2] |= HYPERV_HYPERVISOR_PRESENT_BIT;
        }
        else if (RegistersState->rax == CPUID_HV_VENDOR_AND_MAX_FUNCTIONS)
        {
            //
            // Return a maximum supported hypervisor CPUID leaf range and a vendor
            // ID signature as required by the spec
            //

            CpuInfo[0] = HYPERV_CPUID_INTERFACE;
            CpuInfo[1] = 'epyH'; // [HyperDbg]
            CpuInfo[2] = 'gbDr';
            CpuInfo[3] = NULL;
        }
        else if (RegistersState->rax == HYPERV_CPUID_INTERFACE)
        {
            //
            // Return non Hv#1 value. This indicate that our hypervisor does NOT
            // conform to the Microsoft hypervisor interface.
            //

            CpuInfo[0] = '0#vH'; // Hv#0
            CpuInfo[1] = CpuInfo[2] = CpuInfo[3] = 0;
        }
    }

    //
    // Copy the values from the logical processor registers into the VP GPRs
    //
    RegistersState->rax = CpuInfo[0];
    RegistersState->rbx = CpuInfo[1];
    RegistersState->rcx = CpuInfo[2];
    RegistersState->rdx = CpuInfo[3];

    //
    // As the context to event trigger, we send the eax before the cpuid
    // so that the debugger can both read the eax as it's now changed by
    // the cpuid instruction and also can modify the results
    //
    if (g_TriggerEventForCpuids)
    {
        DebuggerTriggerEvents(CPUID_INSTRUCTION_EXECUTION, RegistersState, Context);
    }
}

/**
 * @brief Handles Guest Access to control registers
 * 
 * @param GuestState Guest's gp registers
 * @param ProcessorIndex Index of processor
 * @return VOID 
 */
VOID
HvHandleControlRegisterAccess(PGUEST_REGS GuestState, UINT32 ProcessorIndex)
{
    ULONG                 ExitQualification = 0;
    PMOV_CR_QUALIFICATION CrExitQualification;
    PULONG64              RegPtr;
    UINT64                NewCr3;
    CR3_TYPE              NewCr3Reg;

    __vmx_vmread(EXIT_QUALIFICATION, &ExitQualification);

    CrExitQualification = (PMOV_CR_QUALIFICATION)&ExitQualification;

    RegPtr = (PULONG64)&GuestState->rax + CrExitQualification->Fields.Register;

    //
    // Because its RSP and as we didn't save RSP correctly (because of pushes)
    // so we have make it points to the GUEST_RSP
    //

    //
    // We handled it in vm-exit handler, commented
    //

    /*    
    if (CrExitQualification->Fields.Register == 4)
    {
        __vmx_vmread(GUEST_RSP, &GuestRsp);
        *RegPtr = GuestRsp;
    }
    */

    switch (CrExitQualification->Fields.AccessType)
    {
    case TYPE_MOV_TO_CR:
    {
        switch (CrExitQualification->Fields.ControlRegister)
        {
        case 0:
            __vmx_vmwrite(GUEST_CR0, *RegPtr);
            __vmx_vmwrite(CR0_READ_SHADOW, *RegPtr);
            break;
        case 3:

            NewCr3          = (*RegPtr & ~(1ULL << 63));
            NewCr3Reg.Flags = NewCr3;

            //
            // Check if we are in debugging thread's steppings or not
            //
            // if (g_EnableDebuggerSteppings)
            // {
            //    SteppingsHandleCr3Vmexits(NewCr3Reg, ProcessorIndex);
            // }

            //
            // Apply the new cr3
            //
            __vmx_vmwrite(GUEST_CR3, NewCr3Reg.Flags);

            //
            // Invalidate as we used VPID tags so the vm-exit won't
            // normally (automatically) flush the tlb, we have to do
            // it manually
            //
            InvvpidSingleContext(VPID_TAG);

            //
            //
            // Call kernel debugger handler for mov to cr3
            KdHandleMovToCr3(ProcessorIndex, GuestState, &NewCr3Reg);

            break;
        case 4:
            __vmx_vmwrite(GUEST_CR4, *RegPtr);
            __vmx_vmwrite(CR4_READ_SHADOW, *RegPtr);

            break;
        default:
            LogWarning("Unsupported register 0x%x in handling control registers access", CrExitQualification->Fields.ControlRegister);
            break;
        }
    }
    break;

    case TYPE_MOV_FROM_CR:
    {
        switch (CrExitQualification->Fields.ControlRegister)
        {
        case 0:
            __vmx_vmread(GUEST_CR0, RegPtr);
            break;
        case 3:
            __vmx_vmread(GUEST_CR3, RegPtr);
            break;
        case 4:
            __vmx_vmread(GUEST_CR4, RegPtr);
            break;
        default:
            LogWarning("Unsupported register 0x%x in handling control registers access", CrExitQualification->Fields.ControlRegister);
            break;
        }
    }
    break;

    default:
        LogWarning("Unsupported operation 0x%x in handling control registers access", CrExitQualification->Fields.AccessType);
        break;
    }
}

/**
 * @brief Fill the guest's selector data
 * 
 * @param GdtBase 
 * @param SegmentRegister 
 * @param Selector 
 * @return VOID 
 */
VOID
HvFillGuestSelectorData(PVOID GdtBase, ULONG SegmentRegister, USHORT Selector)
{
    SEGMENT_SELECTOR SegmentSelector = {0};
    ULONG            AccessRights;

    GetSegmentDescriptor(&SegmentSelector, Selector, GdtBase);
    AccessRights = ((PUCHAR)&SegmentSelector.ATTRIBUTES)[0] + (((PUCHAR)&SegmentSelector.ATTRIBUTES)[1] << 12);

    if (!Selector)
        AccessRights |= 0x10000;

    __vmx_vmwrite(GUEST_ES_SELECTOR + SegmentRegister * 2, Selector);
    __vmx_vmwrite(GUEST_ES_LIMIT + SegmentRegister * 2, SegmentSelector.LIMIT);
    __vmx_vmwrite(GUEST_ES_AR_BYTES + SegmentRegister * 2, AccessRights);
    __vmx_vmwrite(GUEST_ES_BASE + SegmentRegister * 2, SegmentSelector.BASE);
}

/**
 * @brief Handles in the cases when RDMSR causes a vm-exit
 * 
 * @param GuestRegs Guest's gp registers
 * @return VOID 
 */
VOID
HvHandleMsrRead(PGUEST_REGS GuestRegs)
{
    MSR Msr = {0};

    //
    // RDMSR. The RDMSR instruction causes a VM exit if any of the following are true:
    //
    // The "use MSR bitmaps" VM-execution control is 0.
    // The value of ECX is not in the ranges 00000000H - 00001FFFH and C0000000H - C0001FFFH
    // The value of ECX is in the range 00000000H - 00001FFFH and bit n in read bitmap for low MSRs is 1,
    //   where n is the value of ECX.
    // The value of ECX is in the range C0000000H - C0001FFFH and bit n in read bitmap for high MSRs is 1,
    //   where n is the value of ECX & 00001FFFH.
    //

    //
    // Execute WRMSR or RDMSR on behalf of the guest. Important that this
    // can cause bug check when the guest tries to access unimplemented MSR
    // even within the SEH block* because the below WRMSR or RDMSR raises
    // #GP and are not protected by the SEH block (or cannot be protected
    // either as this code run outside the thread stack region Windows
    // requires to proceed SEH). Hypervisors typically handle this by noop-ing
    // WRMSR and returning zero for RDMSR with non-architecturally defined
    // MSRs. Alternatively, one can probe which MSRs should cause #GP prior
    // to installation of a hypervisor and the hypervisor can emulate the
    // results.
    //

    //
    // Check for sanity of MSR if they're valid or they're for reserved range for WRMSR and RDMSR
    //
    if ((GuestRegs->rcx <= 0x00001FFF) || ((0xC0000000 <= GuestRegs->rcx) && (GuestRegs->rcx <= 0xC0001FFF)) || (GuestRegs->rcx >= RESERVED_MSR_RANGE_LOW && (GuestRegs->rcx <= RESERVED_MSR_RANGE_HI)))
    {
        Msr.Content = __readmsr(GuestRegs->rcx);
    }

    //
    // Check if it's EFER MSR then we show a false SCE state so the
    // patchguard won't cause BSOD
    //
    if (GuestRegs->rcx == MSR_EFER)
    {
        EFER_MSR MsrEFER;
        MsrEFER.Flags         = Msr.Content;
        MsrEFER.SyscallEnable = TRUE;
        Msr.Content           = MsrEFER.Flags;
    }

    GuestRegs->rax = Msr.Low;
    GuestRegs->rdx = Msr.High;
}

/**
 * @brief Handles in the cases when RDMSR causes a vm-exit
 * 
 * @param GuestRegs Guest's gp registers
 * @return VOID 
 */
VOID
HvHandleMsrWrite(PGUEST_REGS GuestRegs)
{
    MSR msr = {0};

    //
    // Execute WRMSR or RDMSR on behalf of the guest. Important that this
    // can cause bug check when the guest tries to access unimplemented MSR
    // even within the SEH block* because the below WRMSR or RDMSR raises
    // #GP and are not protected by the SEH block (or cannot be protected
    // either as this code run outside the thread stack region Windows
    // requires to proceed SEH). Hypervisors typically handle this by noop-ing
    // WRMSR and returning zero for RDMSR with non-architecturally defined
    // MSRs. Alternatively, one can probe which MSRs should cause #GP prior
    // to installation of a hypervisor and the hypervisor can emulate the
    // results.
    //

    //
    // Check for sanity of MSR if they're valid or they're for reserved range for WRMSR and RDMSR
    //
    if ((GuestRegs->rcx <= 0x00001FFF) || ((0xC0000000 <= GuestRegs->rcx) && (GuestRegs->rcx <= 0xC0001FFF)) || (GuestRegs->rcx >= RESERVED_MSR_RANGE_LOW && (GuestRegs->rcx <= RESERVED_MSR_RANGE_HI)))
    {
        msr.Low  = (ULONG)GuestRegs->rax;
        msr.High = (ULONG)GuestRegs->rdx;
        __writemsr(GuestRegs->rcx, msr.Content);
    }
}

/**
 * @brief Set bits in Msr Bitmap
 * 
 * @param Msr MSR Address
 * @param ProcessorID Processor to set MSR Bitmap for it
 * @param ReadDetection set read bit 
 * @param WriteDetection set write bit
 * @return BOOLEAN Returns true if the MSR Bitmap is succcessfully applied or false if not applied
 */
BOOLEAN
HvSetMsrBitmap(ULONG64 Msr, INT ProcessorID, BOOLEAN ReadDetection, BOOLEAN WriteDetection)
{
    if (!ReadDetection && !WriteDetection)
    {
        //
        // Invalid Command
        //
        return FALSE;
    }

    if (Msr <= 0x00001FFF)
    {
        if (ReadDetection)
        {
            SetBit(Msr, g_GuestState[ProcessorID].MsrBitmapVirtualAddress);
        }
        if (WriteDetection)
        {
            SetBit(Msr, g_GuestState[ProcessorID].MsrBitmapVirtualAddress + 2048);
        }
    }
    else if ((0xC0000000 <= Msr) && (Msr <= 0xC0001FFF))
    {
        if (ReadDetection)
        {
            SetBit(Msr - 0xC0000000, g_GuestState[ProcessorID].MsrBitmapVirtualAddress + 1024);
        }
        if (WriteDetection)
        {
            SetBit(Msr - 0xC0000000, g_GuestState[ProcessorID].MsrBitmapVirtualAddress + 3072);
        }
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

/**
 * @brief UnSet bits in Msr Bitmap
 * 
 * @param Msr MSR Address
 * @param ProcessorID Processor to set MSR Bitmap for it
 * @param ReadDetection Unset read bit 
 * @param WriteDetection Unset write bit
 * @return BOOLEAN Returns true if the MSR Bitmap is succcessfully applied or false if not applied
 */
BOOLEAN
HvUnSetMsrBitmap(ULONG64 Msr, INT ProcessorID, BOOLEAN ReadDetection, BOOLEAN WriteDetection)
{
    if (!ReadDetection && !WriteDetection)
    {
        //
        // Invalid Command
        //
        return FALSE;
    }

    if (Msr <= 0x00001FFF)
    {
        if (ReadDetection)
        {
            ClearBit(Msr, g_GuestState[ProcessorID].MsrBitmapVirtualAddress);
        }
        if (WriteDetection)
        {
            ClearBit(Msr, g_GuestState[ProcessorID].MsrBitmapVirtualAddress + 2048);
        }
    }
    else if ((0xC0000000 <= Msr) && (Msr <= 0xC0001FFF))
    {
        if (ReadDetection)
        {
            ClearBit(Msr - 0xC0000000, g_GuestState[ProcessorID].MsrBitmapVirtualAddress + 1024);
        }
        if (WriteDetection)
        {
            ClearBit(Msr - 0xC0000000, g_GuestState[ProcessorID].MsrBitmapVirtualAddress + 3072);
        }
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

/**
 * @brief Add the current instruction length to guest rip to resume to next instruction
 * 
 * @return VOID 
 */
VOID
HvResumeToNextInstruction()
{
    ULONG64 ResumeRIP             = NULL;
    ULONG64 CurrentRIP            = NULL;
    ULONG   ExitInstructionLength = 0;

    __vmx_vmread(GUEST_RIP, &CurrentRIP);
    __vmx_vmread(VM_EXIT_INSTRUCTION_LEN, &ExitInstructionLength);

    ResumeRIP = CurrentRIP + ExitInstructionLength;

    __vmx_vmwrite(GUEST_RIP, ResumeRIP);
}

/**
 * @brief Set the monitor trap flag
 * 
 * @param Set Set or unset the MTFs
 * @return VOID 
 */
VOID
HvSetMonitorTrapFlag(BOOLEAN Set)
{
    ULONG CpuBasedVmExecControls = 0;

    //
    // Read the previous flags
    //
    __vmx_vmread(CPU_BASED_VM_EXEC_CONTROL, &CpuBasedVmExecControls);

    if (Set)
    {
        CpuBasedVmExecControls |= CPU_BASED_MONITOR_TRAP_FLAG;
    }
    else
    {
        CpuBasedVmExecControls &= ~CPU_BASED_MONITOR_TRAP_FLAG;
    }

    //
    // Set the new value
    //
    __vmx_vmwrite(CPU_BASED_VM_EXEC_CONTROL, CpuBasedVmExecControls);
}

/**
 * @brief Set LOAD DEBUG CONTROLS on Vm-entry controls
 * 
 * @param Set Set or unset 
 * @return VOID 
 */
VOID
HvSetLoadDebugControls(BOOLEAN Set)
{
    ULONG VmentryControls = 0;

    //
    // Read the previous flags
    //
    __vmx_vmread(VM_ENTRY_CONTROLS, &VmentryControls);

    if (Set)
    {
        VmentryControls |= VM_ENTRY_LOAD_DEBUG_CONTROLS;
    }
    else
    {
        VmentryControls &= ~VM_ENTRY_LOAD_DEBUG_CONTROLS;
    }

    //
    // Set the new value
    //
    __vmx_vmwrite(VM_ENTRY_CONTROLS, VmentryControls);
}

/**
 * @brief Set SAVE DEBUG CONTROLS on Vm-exit controls
 * 
 * @param Set Set or unset 
 * @return VOID 
 */
VOID
HvSetSaveDebugControls(BOOLEAN Set)
{
    ULONG VmexitControls = 0;

    //
    // Read the previous flags
    //
    __vmx_vmread(VM_EXIT_CONTROLS, &VmexitControls);

    if (Set)
    {
        VmexitControls |= VM_EXIT_SAVE_DEBUG_CONTROLS;
    }
    else
    {
        VmexitControls &= ~VM_EXIT_SAVE_DEBUG_CONTROLS;
    }

    //
    // Set the new value
    //
    __vmx_vmwrite(VM_EXIT_CONTROLS, VmexitControls);
}

/**
 * @brief Reset GDTR/IDTR and other old when you do vmxoff as the patchguard will detect them left modified
 * 
 * @return VOID 
 */
VOID
HvRestoreRegisters()
{
    ULONG64 FsBase;
    ULONG64 GsBase;
    ULONG64 GdtrBase;
    ULONG64 GdtrLimit;
    ULONG64 IdtrBase;
    ULONG64 IdtrLimit;

    //
    // Restore FS Base
    //
    __vmx_vmread(GUEST_FS_BASE, &FsBase);
    __writemsr(MSR_FS_BASE, FsBase);

    //
    // Restore Gs Base
    //
    __vmx_vmread(GUEST_GS_BASE, &GsBase);
    __writemsr(MSR_GS_BASE, GsBase);

    //
    // Restore GDTR
    //
    __vmx_vmread(GUEST_GDTR_BASE, &GdtrBase);
    __vmx_vmread(GUEST_GDTR_LIMIT, &GdtrLimit);

    AsmReloadGdtr(GdtrBase, GdtrLimit);

    //
    // Restore IDTR
    //
    __vmx_vmread(GUEST_IDTR_BASE, &IdtrBase);
    __vmx_vmread(GUEST_IDTR_LIMIT, &IdtrLimit);

    AsmReloadIdtr(IdtrBase, IdtrLimit);
}

/**
 * @brief Change MSR Bitmap for read
 * @details should be called in vmx-root mode
 * @param 
 * @return VOID 
 */
VOID
HvPerformMsrBitmapReadChange(UINT64 MsrMask)
{
    UINT32 CoreIndex = KeGetCurrentProcessorNumber();

    if (MsrMask == DEBUGGER_EVENT_MSR_READ_OR_WRITE_ALL_MSRS)
    {
        //
        // Means all the bitmaps should be put to 1
        //
        memset(g_GuestState[CoreIndex].MsrBitmapVirtualAddress, 0xff, 2048);
    }
    else
    {
        //
        // Means only one msr bitmap is target
        //
        HvSetMsrBitmap(MsrMask, CoreIndex, TRUE, FALSE);
    }
}

/**
 * @brief Reset MSR Bitmap for read
 * @details should be called in vmx-root mode
 * @return VOID 
 */
VOID
HvPerformMsrBitmapReadReset()
{
    UINT32 CoreIndex = KeGetCurrentProcessorNumber();

    //
    // Means all the bitmaps should be put to 0
    //
    memset(g_GuestState[CoreIndex].MsrBitmapVirtualAddress, 0x0, 2048);
}
/**
 * @brief Change MSR Bitmap for write
 * @details should be called in vmx-root mode
 * @param MsrMask MSR
 * @return VOID 
 */
VOID
HvPerformMsrBitmapWriteChange(UINT64 MsrMask)
{
    UINT32 CoreIndex = KeGetCurrentProcessorNumber();

    if (MsrMask == DEBUGGER_EVENT_MSR_READ_OR_WRITE_ALL_MSRS)
    {
        //
        // Means all the bitmaps should be put to 1
        //
        memset((UINT64)g_GuestState[CoreIndex].MsrBitmapVirtualAddress + 2048, 0xff, 2048);
    }
    else
    {
        //
        // Means only one msr bitmap is target
        //
        HvSetMsrBitmap(MsrMask, CoreIndex, FALSE, TRUE);
    }
}

/**
 * @brief Reset MSR Bitmap for write
 * @details should be called in vmx-root mode
 * @return VOID 
 */
VOID
HvPerformMsrBitmapWriteReset()
{
    UINT32 CoreIndex = KeGetCurrentProcessorNumber();

    //
    // Means all the bitmaps should be put to 0
    //
    memset((UINT64)g_GuestState[CoreIndex].MsrBitmapVirtualAddress + 2048, 0x0, 2048);
}

/**
 * @brief Set vm-exit for rdpmc instructions 
 * @details Should be called in vmx-root
 * 
 * @param Set Set or unset the vm-exits
 * @return VOID 
 */
VOID
HvSetPmcVmexit(BOOLEAN Set)
{
    ULONG CpuBasedVmExecControls = 0;

    //
    // Read the previous flags
    //
    __vmx_vmread(CPU_BASED_VM_EXEC_CONTROL, &CpuBasedVmExecControls);

    if (Set)
    {
        CpuBasedVmExecControls |= CPU_BASED_RDPMC_EXITING;
    }
    else
    {
        CpuBasedVmExecControls &= ~CPU_BASED_RDPMC_EXITING;
    }

    //
    // Set the new value
    //
    __vmx_vmwrite(CPU_BASED_VM_EXEC_CONTROL, CpuBasedVmExecControls);
}

/**
 * @brief Set vm-exit for mov-to-cr3 
 * @details Should be called in vmx-root
 * 
 * @param Set Set or unset the vm-exits
 * @return VOID 
 */
VOID
HvSetMovToCr3Vmexit(BOOLEAN Set)
{
    ULONG CpuBasedVmExecControls = 0;

    //
    // Read the previous flags
    //
    __vmx_vmread(CPU_BASED_VM_EXEC_CONTROL, &CpuBasedVmExecControls);

    if (Set)
    {
        CpuBasedVmExecControls |= CPU_BASED_CR3_LOAD_EXITING;
    }
    else
    {
        CpuBasedVmExecControls &= ~CPU_BASED_CR3_LOAD_EXITING;
    }

    //
    // Set the new value
    //
    __vmx_vmwrite(CPU_BASED_VM_EXEC_CONTROL, CpuBasedVmExecControls);
}

/**
 * @brief Write on exception bitmap in VMCS 
 * DO NOT CALL IT DIRECTLY, instead use HvSetExceptionBitmap
 * @details Should be called in vmx-root
 * 
 * @param BitmapMask The content to write on exception bitmap 
 * @return VOID 
 */
VOID
HvWriteExceptionBitmap(UINT32 BitmapMask)
{
    //
    // Set the new value
    //
    __vmx_vmwrite(EXCEPTION_BITMAP, BitmapMask);
}

/**
 * @brief Read exception bitmap in VMCS 
 * @details Should be called in vmx-root
 * 
 * @return UINT32 
 */
UINT32
HvReadExceptionBitmap()
{
    UINT32 ExceptionBitmap = 0;

    //
    // Read the current bitmap
    //
    __vmx_vmread(EXCEPTION_BITMAP, &ExceptionBitmap);

    return ExceptionBitmap;
}

/**
 * @brief Set Interrupt-window exiting
 * 
 * @param Set Set or unset the Interrupt Window-exiting
 * @return VOID 
 */
VOID
HvSetInterruptWindowExiting(BOOLEAN Set)
{
    ULONG CpuBasedVmExecControls = 0;

    //
    // Read the previous flags
    //
    __vmx_vmread(CPU_BASED_VM_EXEC_CONTROL, &CpuBasedVmExecControls);

    //
    // interrupt-window exiting
    //
    if (Set)
    {
        CpuBasedVmExecControls |= CPU_BASED_VIRTUAL_INTR_PENDING;
    }
    else
    {
        CpuBasedVmExecControls &= ~CPU_BASED_VIRTUAL_INTR_PENDING;
    }

    //
    // Set the new value
    //
    __vmx_vmwrite(CPU_BASED_VM_EXEC_CONTROL, CpuBasedVmExecControls);
}

/**
 * @brief Set the nmi-Window exiting
 * 
 * @param Set Set or unset the Interrupt Window-exiting
 * @return VOID 
 */
VOID
HvSetNmiWindowExiting(BOOLEAN Set)
{
    ULONG CpuBasedVmExecControls = 0;

    //
    // Read the previous flags
    //
    __vmx_vmread(CPU_BASED_VM_EXEC_CONTROL, &CpuBasedVmExecControls);

    //
    // nmi-window exiting
    //
    if (Set)
    {
        CpuBasedVmExecControls |= CPU_BASED_VIRTUAL_NMI_PENDING;
    }
    else
    {
        CpuBasedVmExecControls &= ~CPU_BASED_VIRTUAL_NMI_PENDING;
    }

    //
    // Set the new value
    //
    __vmx_vmwrite(CPU_BASED_VM_EXEC_CONTROL, CpuBasedVmExecControls);
}

/**
 * @brief Handle Mov to Debug Registers Exitings
 * 
 * @param ProcessorIndex Index of processor
 * @param Regs Registers of guest
 * @return VOID 
 */
VOID
HvHandleMovDebugRegister(UINT32 ProcessorIndex, PGUEST_REGS Regs)
{
    MOV_TO_DEBUG_REG_QUALIFICATION ExitQualification;
    CONTROL_REGISTER_4             Cr4;
    DEBUG_REGISTER_7               Dr7;
    SEGMENT_SELECTOR               Cs;
    UINT64 *                       GpRegs = Regs;
    //
    // The implementation is derived from Hvpp
    //
    __vmx_vmread(EXIT_QUALIFICATION, &ExitQualification);

    UINT64 GpRegister = GpRegs[ExitQualification.GpRegister];

    //
    // The MOV DR instruction causes a VM exit if the "MOV-DR exiting"
    // VM-execution control is 1.  Such VM exits represent an exception
    // to the principles identified in Section 25.1.1 (Relative Priority
    // of Faults and VM Exits) in that they take priority over the
    // following: general-protection exceptions based on privilege level;
    // and invalid-opcode exceptions that occur because CR4.DE = 1 and the
    // instruction specified access to DR4 or DR5.
    // (ref: Vol3C[25.1.3(Instructions That Cause VM Exits Conditionally)])
    //
    // TL;DR:
    //   CPU usually prioritizes exceptions.  For example RDMSR executed
    //   at CPL = 3 won't cause VM-exit - it causes #GP instead.  MOV DR
    //   is exception to this rule, as it ALWAYS cause VM-exit.
    //
    //   Normally, CPU allows you to write to DR registers only at CPL=0,
    //   otherwise it causes #GP.  Therefore we'll simulate the exact same
    //   behavior here.
    //

    Cs = GetGuestCs();

    if (Cs.ATTRIBUTES.Fields.DPL != 0)
    {
        EventInjectGeneralProtection();

        //
        // Redo the instruction
        //
        g_GuestState[ProcessorIndex].IncrementRip = FALSE;
        return;
    }

    //
    // Debug registers DR4 and DR5 are reserved when debug extensions
    // are enabled (when the DE flag in control register CR4 is set)
    // and attempts to reference the DR4 and DR5 registers cause
    // invalid-opcode exceptions (#UD).
    // When debug extensions are not enabled (when the DE flag is clear),
    // these registers are aliased to debug registers DR6 and DR7.
    // (ref: Vol3B[17.2.2(Debug Registers DR4 and DR5)])
    //

    //
    // Read guest cr4
    //
    __vmx_vmread(GUEST_CR4, &Cr4);

    if (ExitQualification.DrNumber == 4 || ExitQualification.DrNumber == 5)
    {
        if (Cr4.DebuggingExtensions)
        {
            //
            // re-inject #UD
            //
            EventInjectUndefinedOpcode(ProcessorIndex);
            return;
        }
        else
        {
            ExitQualification.DrNumber += 2;
        }
    }

    //
    // Enables (when set) debug-register protection, which causes a
    // debug exception to be generated prior to any MOV instruction
    // that accesses a debug register.  When such a condition is
    // detected, the BD flag in debug status register DR6 is set prior
    // to generating the exception.  This condition is provided to
    // support in-circuit emulators.
    // When the emulator needs to access the debug registers, emulator
    // software can set the GD flag to prevent interference from the
    // program currently executing on the processor.
    // The processor clears the GD flag upon entering to the debug
    // exception handler, to allow the handler access to the debug
    // registers.
    // (ref: Vol3B[17.2.4(Debug Control Register (DR7)])
    //

    //
    // Read the DR7
    //
    __vmx_vmread(GUEST_DR7, &Dr7);

    if (Dr7.GeneralDetect)
    {
        DEBUG_REGISTER_6 Dr6;
        Dr6.Flags                       = __readdr(6);
        Dr6.BreakpointCondition         = 0;
        Dr6.DebugRegisterAccessDetected = TRUE;
        __writedr(6, Dr6.Flags);

        Dr7.GeneralDetect = FALSE;

        __vmx_vmwrite(GUEST_DR7, Dr7.Flags);

        EventInjectDebugBreakpoint();

        //
        // Redo the instruction
        //
        g_GuestState[ProcessorIndex].IncrementRip = FALSE;
        return;
    }

    //
    // In 64-bit mode, the upper 32 bits of DR6 and DR7 are reserved
    // and must be written with zeros.  Writing 1 to any of the upper
    // 32 bits results in a #GP(0) exception.
    // (ref: Vol3B[17.2.6(Debug Registers and Intel� 64 Processors)])
    //
    if (ExitQualification.AccessType == AccessToDebugRegister &&
        (ExitQualification.DrNumber == 6 || ExitQualification.DrNumber == 7) &&
        (GpRegister >> 32) != 0)
    {
        EventInjectGeneralProtection();

        //
        // Redo the instruction
        //
        g_GuestState[ProcessorIndex].IncrementRip = FALSE;
        return;
    }

    switch (ExitQualification.AccessType)
    {
    case AccessToDebugRegister:
        switch (ExitQualification.DrNumber)
        {
        case 0:
            __writedr(0, GpRegister);
            break;
        case 1:
            __writedr(1, GpRegister);
            break;
        case 2:
            __writedr(2, GpRegister);
            break;
        case 3:
            __writedr(3, GpRegister);
            break;
        case 6:
            __writedr(6, GpRegister);
            break;
        case 7:
            __writedr(7, GpRegister);
            break;
        default:
            break;
        }
        break;

    case AccessFromDebugRegister:
        switch (ExitQualification.DrNumber)
        {
        case 0:
            GpRegister = __readdr(0);
            break;
        case 1:
            GpRegister = __readdr(1);
            break;
        case 2:
            GpRegister = __readdr(2);
            break;
        case 3:
            GpRegister = __readdr(3);
            break;
        case 6:
            GpRegister = __readdr(6);
            break;
        case 7:
            GpRegister = __readdr(7);
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
}

/**
 * @brief Set or unset the Mov to Debug Registers Exiting
 * 
 * @param Set Set or unset the Mov to Debug Registers Exiting
 * @return VOID 
 */
VOID
HvSetMovDebugRegsExiting(BOOLEAN Set)
{
    ULONG CpuBasedVmExecControls = 0;

    //
    // Read the previous flags
    //
    __vmx_vmread(CPU_BASED_VM_EXEC_CONTROL, &CpuBasedVmExecControls);

    if (Set)
    {
        CpuBasedVmExecControls |= CPU_BASED_MOV_DR_EXITING;
    }
    else
    {
        CpuBasedVmExecControls &= ~CPU_BASED_MOV_DR_EXITING;
    }

    //
    // Set the new value
    //
    __vmx_vmwrite(CPU_BASED_VM_EXEC_CONTROL, CpuBasedVmExecControls);
}

/**
 * @brief Set the NMI Exiting
 * 
 * @param Set Set or unset the NMI Exiting
 * @return VOID 
 */
VOID
HvSetNmiExiting(BOOLEAN Set)
{
    ULONG PinBasedControls = 0;
    ULONG VmExitControls   = 0;

    //
    // Read the previous flags
    //
    __vmx_vmread(PIN_BASED_VM_EXEC_CONTROL, &PinBasedControls);
    __vmx_vmread(VM_EXIT_CONTROLS, &VmExitControls);

    if (Set)
    {
        PinBasedControls |= PIN_BASED_VM_EXECUTION_CONTROLS_NMI_EXITING;
        VmExitControls |= VM_EXIT_ACK_INTR_ON_EXIT;
    }
    else
    {
        PinBasedControls &= ~PIN_BASED_VM_EXECUTION_CONTROLS_NMI_EXITING;
        VmExitControls &= ~VM_EXIT_ACK_INTR_ON_EXIT;
    }

    //
    // Set the new value
    //
    __vmx_vmwrite(PIN_BASED_VM_EXEC_CONTROL, PinBasedControls);
    __vmx_vmwrite(VM_EXIT_CONTROLS, VmExitControls);
}

/**
 * @brief Set bits in I/O Bitmap
 * 
 * @param Port Port
 * @param ProcessorID Processor ID
 * @return BOOLEAN Returns true if the MSR Bitmap is succcessfully applied or false if not applied
 */
BOOLEAN
HvSetIoBitmap(UINT64 Port, UINT32 ProcessorID)
{
    if (Port <= 0x7FFF)
    {
        SetBit(Port, g_GuestState[ProcessorID].IoBitmapVirtualAddressA);
    }
    else if ((0x8000 <= Port) && (Port <= 0xFFFF))
    {
        SetBit(Port - 0x8000, g_GuestState[ProcessorID].IoBitmapVirtualAddressB);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Change I/O Bitmap 
 * @details should be called in vmx-root mode
 * @param IoPort Port 
 * @return VOID 
 */
VOID
HvPerformIoBitmapChange(UINT64 Port)
{
    UINT32 CoreIndex = KeGetCurrentProcessorNumber();

    if (Port == DEBUGGER_EVENT_ALL_IO_PORTS)
    {
        //
        // Means all the bitmaps should be put to 1
        //
        memset(g_GuestState[CoreIndex].IoBitmapVirtualAddressA, 0xFF, PAGE_SIZE);
        memset(g_GuestState[CoreIndex].IoBitmapVirtualAddressB, 0xFF, PAGE_SIZE);
    }
    else
    {
        //
        // Means only one i/o bitmap is target
        //
        HvSetIoBitmap(Port, CoreIndex);
    }
}

/**
 * @brief Reset I/O Bitmap 
 * @details should be called in vmx-root mode
 * @return VOID 
 */
VOID
HvPerformIoBitmapReset()
{
    UINT32 CoreIndex = KeGetCurrentProcessorNumber();

    //
    // Means all the bitmaps should be put to 0
    //
    memset(g_GuestState[CoreIndex].IoBitmapVirtualAddressA, 0x0, PAGE_SIZE);
    memset(g_GuestState[CoreIndex].IoBitmapVirtualAddressB, 0x0, PAGE_SIZE);
}

/**
 * @brief Set exception bitmap in VMCS 
 * @details Should be called in vmx-root
 * 
 * @param IdtIndex Interrupt Descriptor Table index of exception 
 * @return VOID 
 */
VOID
HvSetExceptionBitmap(UINT32 IdtIndex)
{
    //
    // This is a wrapper to perform extra checks
    //
    ProtectedHvSetExceptionBitmap(IdtIndex);
}

/**
 * @brief Unset exception bitmap in VMCS 
 * @details Should be called in vmx-root
 * 
 * @param IdtIndex Interrupt Descriptor Table index of exception 
 * @return VOID 
 */
VOID
HvUnsetExceptionBitmap(UINT32 IdtIndex)
{
    //
    // This is a wrapper to perform extra checks
    //
    ProtectedHvUnsetExceptionBitmap(IdtIndex);
}

/**
 * @brief Set the External Interrupt Exiting
 * 
 * @param Set Set or unset the External Interrupt Exiting
 * @return VOID 
 */
VOID
HvSetExternalInterruptExiting(BOOLEAN Set)
{
    //
    // This is a wrapper to perform extra checks
    //
    ProtectedHvSetExternalInterruptExiting(Set);
}

/**
 * @brief Set the RDTSC/P Exiting
 * 
 * @param Set Set or unset the RDTSC/P Exiting
 * @return VOID 
 */
VOID
HvSetRdtscExiting(BOOLEAN Set)
{
    ProtectedHvSetRdtscExiting(Set);
}
