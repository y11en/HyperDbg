/**
 * @file ManageRegs.c
 * @author Sina Karvandi (sina@rayanfam.com)
 * @author Alee Amini (aleeaminiz@gmail.com)
 * @brief Manage Registers
 * @details
 * @version 0.1
 * @date 2020-06-11
 * 
 * @copyright This project is released under the GNU Public License v3.
 * 
 */
#include "..\hprdbghv\pch.h"

/**
 * @brief Set just the Guest Cs selector
 * 
 * @param Cs The CS Selector for the guest
 * @return VOID 
 */
VOID
SetGuestCsSel(PSEGMENT_SELECTOR Cs)
{
    __vmx_vmwrite(GUEST_CS_SELECTOR, Cs->SEL);
}

/**
 * @brief Set the Guest Cs  
 * 
 * @param Cs The CS Selector for the guest
 * @return VOID 
 */

VOID
SetGuestCs(PSEGMENT_SELECTOR Cs)
{
    __vmx_vmwrite(GUEST_CS_BASE, Cs->BASE);
    __vmx_vmwrite(GUEST_CS_LIMIT, Cs->LIMIT);
    __vmx_vmwrite(GUEST_CS_AR_BYTES, Cs->ATTRIBUTES.UCHARs);
    __vmx_vmwrite(GUEST_CS_SELECTOR, Cs->SEL);
}

/**
 * @brief Get the Guest Cs Selector
 * 
 * @return SEGMENT_SELECTOR 
 */
SEGMENT_SELECTOR
GetGuestCs()
{
    SEGMENT_SELECTOR Cs;

    __vmx_vmread(GUEST_CS_BASE, &Cs.BASE);
    __vmx_vmread(GUEST_CS_LIMIT, &Cs.LIMIT);
    __vmx_vmread(GUEST_CS_AR_BYTES, &Cs.ATTRIBUTES.UCHARs);
    __vmx_vmread(GUEST_CS_SELECTOR, &Cs.SEL);

    return Cs;
}

/**
 * @brief Set just the Guest Ss selector
 * 
 * @param Ss The SS Selector for the guest
 * @return VOID 
 */
VOID
SetGuestSsSel(PSEGMENT_SELECTOR Ss)
{
    __vmx_vmwrite(GUEST_SS_SELECTOR, Ss->SEL);
}

/**
 * @brief Set the Guest Ss selector
 * 
 * @param Ss The SS Selector for the guest
 * @return VOID 
 */
VOID
SetGuestSs(PSEGMENT_SELECTOR Ss)
{
    __vmx_vmwrite(GUEST_SS_BASE, Ss->BASE);
    __vmx_vmwrite(GUEST_SS_LIMIT, Ss->LIMIT);
    __vmx_vmwrite(GUEST_SS_AR_BYTES, Ss->ATTRIBUTES.UCHARs);
    __vmx_vmwrite(GUEST_SS_SELECTOR, Ss->SEL);
}

/**
 * @brief Get the Guest Ss Selector
 * 
 * @return SEGMENT_SELECTOR 
 */
SEGMENT_SELECTOR
GetGuestSs()
{
    SEGMENT_SELECTOR Ss;

    __vmx_vmread(GUEST_SS_BASE, &Ss.BASE);
    __vmx_vmread(GUEST_SS_LIMIT, &Ss.LIMIT);
    __vmx_vmread(GUEST_SS_AR_BYTES, &Ss.ATTRIBUTES.UCHARs);
    __vmx_vmread(GUEST_SS_SELECTOR, &Ss.SEL);

    return Ss;
}

/**
 * @brief Set just the Guest Ds selector
 * 
 * @param Ds The DS Selector for the guest
 * @return VOID 
 */
VOID
SetGuestDsSel(PSEGMENT_SELECTOR Ds)
{
    __vmx_vmwrite(GUEST_DS_SELECTOR, Ds->SEL);
}

/**
 * @brief Set the Guest Ds selector
 * 
 * @param Ds The DS Selector for the guest
 * @return VOID 
 */
VOID
SetGuestDs(PSEGMENT_SELECTOR Ds)
{
    __vmx_vmwrite(GUEST_DS_BASE, Ds->BASE);
    __vmx_vmwrite(GUEST_DS_LIMIT, Ds->LIMIT);
    __vmx_vmwrite(GUEST_DS_AR_BYTES, Ds->ATTRIBUTES.UCHARs);
    __vmx_vmwrite(GUEST_DS_SELECTOR, Ds->SEL);
}

/**
 * @brief Get the Guest Ds Selector
 * 
 * @return SEGMENT_SELECTOR 
 */
SEGMENT_SELECTOR
GetGuestDs()
{
    SEGMENT_SELECTOR Ds;

    __vmx_vmread(GUEST_DS_BASE, &Ds.BASE);
    __vmx_vmread(GUEST_DS_LIMIT, &Ds.LIMIT);
    __vmx_vmread(GUEST_DS_AR_BYTES, &Ds.ATTRIBUTES.UCHARs);
    __vmx_vmread(GUEST_DS_SELECTOR, &Ds.SEL);

    return Ds;
}

/**
 * @brief Set just the Guest Fs selector
 * 
 * @param Fs The FS Selector for the guest
 * @return VOID 
 */
VOID
SetGuestFsSel(PSEGMENT_SELECTOR Fs)
{
    __vmx_vmwrite(GUEST_FS_SELECTOR, Fs->SEL);
}

/**
 * @brief Set the Guest Fs selector
 * 
 * @param Fs The FS Selector for the guest
 * @return VOID 
 */
VOID
SetGuestFs(PSEGMENT_SELECTOR Fs)
{
    __vmx_vmwrite(GUEST_FS_BASE, Fs->BASE);
    __vmx_vmwrite(GUEST_FS_LIMIT, Fs->LIMIT);
    __vmx_vmwrite(GUEST_FS_AR_BYTES, Fs->ATTRIBUTES.UCHARs);
    __vmx_vmwrite(GUEST_FS_SELECTOR, Fs->SEL);
}

/**
 * @brief Get the Guest Fs Selector
 * 
 * @return SEGMENT_SELECTOR 
 */
SEGMENT_SELECTOR
GetGuestFs()
{
    SEGMENT_SELECTOR Fs;

    __vmx_vmread(GUEST_FS_BASE, &Fs.BASE);
    __vmx_vmread(GUEST_FS_LIMIT, &Fs.LIMIT);
    __vmx_vmread(GUEST_FS_AR_BYTES, &Fs.ATTRIBUTES.UCHARs);
    __vmx_vmread(GUEST_FS_SELECTOR, &Fs.SEL);

    return Fs;
}

/**
 * @brief Set just the Guest Gs selector
 * 
 * @param Gs The GS Selector for the guest
 * @return VOID 
 */
VOID
SetGuestGsSel(PSEGMENT_SELECTOR Gs)
{
    __vmx_vmwrite(GUEST_GS_SELECTOR, Gs->SEL);
}

/**
 * @brief Set the Guest Gs selector
 * 
 * @param Gs The GS Selector for the guest
 * @return VOID 
 */
VOID
SetGuestGs(PSEGMENT_SELECTOR Gs)
{
    __vmx_vmwrite(GUEST_GS_BASE, Gs->BASE);
    __vmx_vmwrite(GUEST_GS_LIMIT, Gs->LIMIT);
    __vmx_vmwrite(GUEST_GS_AR_BYTES, Gs->ATTRIBUTES.UCHARs);
    __vmx_vmwrite(GUEST_GS_SELECTOR, Gs->SEL);
}

/**
 * @brief Get the Guest Gs Selector
 * 
 * @return SEGMENT_SELECTOR 
 */
SEGMENT_SELECTOR
GetGuestGs()
{
    SEGMENT_SELECTOR Gs;

    __vmx_vmread(GUEST_GS_BASE, &Gs.BASE);
    __vmx_vmread(GUEST_GS_LIMIT, &Gs.LIMIT);
    __vmx_vmread(GUEST_GS_AR_BYTES, &Gs.ATTRIBUTES.UCHARs);
    __vmx_vmread(GUEST_GS_SELECTOR, &Gs.SEL);

    return Gs;
}

/**
 * @brief Set just the Guest Es selector
 * 
 * @param Es The ES Selector for the guest
 * @return VOID 
 */
VOID
SetGuestEsSel(PSEGMENT_SELECTOR Es)
{
    __vmx_vmwrite(GUEST_ES_SELECTOR, Es->SEL);
}

/**
 * @brief Set the Guest Es selector
 * 
 * @param Es The ES Selector for the guest
 * @return VOID 
 */
VOID
SetGuestEs(PSEGMENT_SELECTOR Es)
{
    __vmx_vmwrite(GUEST_ES_BASE, Es->BASE);
    __vmx_vmwrite(GUEST_ES_LIMIT, Es->LIMIT);
    __vmx_vmwrite(GUEST_ES_AR_BYTES, Es->ATTRIBUTES.UCHARs);
    __vmx_vmwrite(GUEST_ES_SELECTOR, Es->SEL);
}

/**
 * @brief Get the Guest Es Selector
 * 
 * @return SEGMENT_SELECTOR 
 */
SEGMENT_SELECTOR
GetGuestEs()
{
    SEGMENT_SELECTOR Es;

    __vmx_vmread(GUEST_ES_BASE, &Es.BASE);
    __vmx_vmread(GUEST_ES_LIMIT, &Es.LIMIT);
    __vmx_vmread(GUEST_ES_AR_BYTES, &Es.ATTRIBUTES.UCHARs);
    __vmx_vmread(GUEST_ES_SELECTOR, &Es.SEL);

    return Es;
}

/**
 * @brief Set the Guest Idtr
 * 
 * @param Idtr The Idtr Selector for the guest
 * @return VOID 
 */
VOID
SetGuestIdtr(UINT64 Idtr)
{
    __vmx_vmwrite(GUEST_IDTR_BASE, Idtr);
}

/**
 * @brief Get the Guest Idtr
 * 
 * @return UINT64 
 */
UINT64
GetGuestIdtr()
{
    UINT64 Idtr;

    __vmx_vmread(GUEST_IDTR_BASE, &Idtr);

    return Idtr;
}

/**
 * @brief Set the Guest Ldtr
 * 
 * @param Ldtr The Idtr Selector for the guest
 * @return VOID 
 */
VOID
SetGuestLdtr(UINT64 Ldtr)
{
    __vmx_vmwrite(GUEST_LDTR_BASE, Ldtr);
}

/**
 * @brief Get the Guest Ldtr
 * 
 * @return UINT64 
 */
UINT64
GetGuestLdtr()
{
    UINT64 Ldtr;

    __vmx_vmread(GUEST_LDTR_BASE, &Ldtr);

    return Ldtr;
}

/**
 * @brief Set the Guest Gdtr
 * 
 * @param Gdtr The Gdtr Selector for the guest
 * @return VOID 
 */
VOID
SetGuestGdtr(UINT64 Gdtr)
{
    __vmx_vmwrite(GUEST_GDTR_BASE, Gdtr);
}

/**
 * @brief Get the Guest Gdtr
 * 
 * @return UINT64 
 */
UINT64
GetGuestGdtr()
{
    UINT64 Gdtr;

    __vmx_vmread(GUEST_GDTR_BASE, &Gdtr);

    return Gdtr;
}

/**
 * @param Tr The Tr Selector for the guest
 * @return VOID 
 */
VOID
SetGuestTr(UINT64 Tr)
{
    __vmx_vmwrite(GUEST_TR_BASE, Tr);
}

/**
 * @brief Get the Guest Tr
 * 
 * @return UINT64 
 */
UINT64
GetGuestTr()
{
    UINT64 Tr;

    __vmx_vmread(GUEST_TR_BASE, &Tr);

    return Tr;
}
/**
 * @brief Set the Guest RFLAGS Register
 * 
 * @param Rflags The Rflags Value for the guest
 * @return VOID 
 */
VOID
SetGuestRFlags(UINT64 RFlags)
{
    __vmx_vmwrite(GUEST_RFLAGS, RFlags);
}

/**
 * @brief Get the Guest Rflags value
 * 
 * @return UINT64
 */
UINT64
GetGuestRFlags()
{
    UINT64 RFlags;
    __vmx_vmread(GUEST_RFLAGS, &RFlags);
    return RFlags;
}

/**
 * @brief Set the Guest RIP Register
 * 
 * @param RIP The RIP Value for the guest
 * @return VOID 
 */
VOID
SetGuestRIP(UINT64 RIP)
{
    __vmx_vmwrite(GUEST_RIP, RIP);
}

/**
 * @brief Set the Guest RSP Register
 * 
 * @param RSP The RSP Value for the guest
 * @return VOID 
 */
VOID
SetGuestRSP(UINT64 RSP)
{
    __vmx_vmwrite(GUEST_RSP, RSP);
}

/**
 * @brief Get the Guest RIP value
 * 
 * @return UINT64
 */
UINT64
GetGuestRIP()
{
    UINT64 RIP;

    __vmx_vmread(GUEST_RIP, &RIP);
    return RIP;
}

/**
 * @brief Get the Guest Cr0 value
 * 
 * @return UINT64
 */
UINT64
GetGuestCr0()
{
    UINT64 Cr0;

    __vmx_vmread(GUEST_CR0, &Cr0);
    return Cr0;
}

/**
 * @brief Get the Guest Cr2 value
 * 
 * @return UINT64
 */
UINT64
GetGuestCr2()
{
    UINT64 Cr2;

    Cr2 = __readcr2();
    return Cr2;
}

/**
 * @brief Get the Guest Cr3 value
 * 
 * @return UINT64
 */
UINT64
GetGuestCr3()
{
    UINT64 Cr3;

    __vmx_vmread(GUEST_CR3, &Cr3);
    return Cr3;
}

/**
 * @brief Get the Guest Cr4 value
 * 
 * @return UINT64
 */
UINT64
GetGuestCr4()
{
    UINT64 Cr4;

    __vmx_vmread(GUEST_CR4, &Cr4);
    return Cr4;
}

/**
 * @brief Get the Guest Cr8 value
 * 
 * @return UINT64
 */
UINT64
GetGuestCr8()
{
    UINT64 Cr8;

    Cr8 = __readcr8();
    return Cr8;
}

/**
 * @brief Set the Guest Cr0 Register
 * 
 * @param Cr0 The Cr0 Value for the guest
 * @return VOID 
 */
VOID
SetGuestCr0(UINT64 Cr0)
{
    __vmx_vmwrite(GUEST_CR0, Cr0);
}

/**
 * @brief Set the Guest Cr2 Register
 * 
 * @param Cr2 The Cr2 Value for the guest
 * @return VOID 
 */
VOID
SetGuestCr2(UINT64 Cr2)
{
    __writecr2(Cr2);
}

/**
 * @brief Set the Guest Cr3 Register
 * 
 * @param Cr3 The Cr3 Value for the guest
 * @return VOID 
 */
VOID
SetGuestCr3(UINT64 Cr3)
{
    __vmx_vmwrite(GUEST_CR3, Cr3);
}

/**
 * @brief Set the Guest Cr4 Register
 * 
 * @param Cr4 The Cr4 Value for the guest
 * @return VOID 
 */
VOID
SetGuestCr4(UINT64 Cr4)
{
    __vmx_vmwrite(GUEST_CR4, Cr4);
}

/**
 * @brief Set the Guest Cr8 Register
 * 
 * @param Cr8 The Cr8 Value for the guest
 * @return VOID 
 */
VOID
SetGuestCr8(UINT64 Cr8)
{
    __writecr8(Cr8);
}

/**
 * @brief Set the Guest Dr0 Register
 * 
 * @param Dr0 The Dr0 Value for the guest
 * @return VOID 
 */
VOID
SetGuestDr0(UINT64 value)
{
    __writedr(0, value);
}

/**
 * @brief Set the Guest Dr1 Register
 * 
 * @param Dr1 The Dr1 Value for the guest
 * @return VOID 
 */
VOID
SetGuestDr1(UINT64 value)
{
    __writedr(1, value);
}

/**
 * @brief Set the Guest Dr2 Register
 * 
 * @param Dr2 The Dr2 Value for the guest
 * @return VOID 
 */
VOID
SetGuestDr2(UINT64 value)
{
    __writedr(2, value);
}

/**
 * @brief Set the Guest Dr3 Register
 * 
 * @param Dr3 The Dr3 Value for the guest
 * @return VOID 
 */
VOID
SetGuestDr3(UINT64 value)
{
    __writedr(3, value);
}

/**
 * @brief Set the Guest Dr6 Register
 * 
 * @param Dr6 The Dr6 Value for the guest
 * @return VOID 
 */
VOID
SetGuestDr6(UINT64 value)
{
    __writedr(6, value);
}

/**
 * @brief Set the Guest Dr7 Register
 * 
 * @param Dr7 The Dr7 Value for the guest
 * @return VOID 
 */
VOID
SetGuestDr7(UINT64 value)
{
    __writedr(7, value);
}

/**
 * @brief Get the Guest Dr0 value
 * 
 * @return UINT64
 */
UINT64
GetGuestDr0()
{
    UINT64 Dr0 = 0;
    Dr0        = __readdr(0);
    return Dr0;
}

/**
 * @brief Get the Guest Dr1 value
 * 
 * @return UINT64
 */
UINT64
GetGuestDr1()
{
    UINT64 Dr1 = 0;
    Dr1        = __readdr(1);
    return Dr1;
}

/**
 * @brief Get the Guest Dr2 value
 * 
 * @return UINT64
 */
UINT64
GetGuestDr2()
{
    UINT64 Dr2 = 0;
    Dr2        = __readdr(2);
    return Dr2;
}

/**
 * @brief Get the Guest Dr3 value
 * 
 * @return UINT64
 */
UINT64
GetGuestDr3()
{
    UINT64 Dr3 = 0;
    Dr3        = __readdr(3);
    return Dr3;
}

/**
 * @brief Get the Guest Dr6 (breakpoint status) value
 * 
 * @return UINT64
 */
UINT64
GetGuestDr6()
{
    UINT64 Dr6 = 0;
    Dr6        = __readdr(6);
    return Dr6;
}

/**
 * @brief Get the Guest Dr7 (breakpoint trigger) value
 * 
 * @return UINT64
 */
UINT64
GetGuestDr7()
{
    UINT64 Dr7 = 0;
    Dr7        = __readdr(7);
    return Dr7;
}
