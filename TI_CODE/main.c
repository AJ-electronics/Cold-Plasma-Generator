


#include "F28x_Project.h"
#include "F2837xD_device.h"
#include "math.h"

interrupt void epwm1_isr(void);


int compvalue=4000,TBPRD_VALUE=5000;
float theta=0, pi=3.1416, Tsw=2e-4;
int f=50;
float omega;
float m=0.8;


void main(void)


{
   InitSysCtrl(); // Configures Main Clock

    /* Enable Peripheral Clock */
    EALLOW;
    /* EPWMs*/
    CpuSysRegs.PCLKCR2.bit.EPWM1=1;


    EDIS;


    InitEPWM();



    DINT; //Disable interrupts globally
    InitPieCtrl();

    IER = 0x0000;   // Disable CPU interrupts
    IFR = 0x0000;   // Clear all CPU interrupt flags

    InitPieVectTable();

    EALLOW;
    /* EPWM2 Interrupt Configuration*/
    PieVectTable.EPWM1_INT = &epwm1_isr; //Writing the ISR vector for EPWM1 interrupt to the appropriate location in the PIE vector table
    IER = 0x0004; //Enabling Group 3 Interrupts in PIE which contains EPWM Interrupt
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1; // Enabling EPWM1 Interrupt in Group 3
    EDIS;




    EINT;  // Enable Global interrupt INTM
    ERTM;  //Clearing DBGM ensures that memory and register values can be accessed for use for updating debugger windows, essentially allowing halt requests within ISRs.

    //Enable TBCLKSYNC after configuring EPWMs before the start of while(1)
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
        while(1)
        {

        }
}
void InitEPWM(void)
{
    //Configure GPIO pins as EPWMs/
    EALLOW;
    //EPWM1A used as STROBE/
    GpioCtrlRegs.GPAMUX1.bit.GPIO0=0; //Before changing GPyGMUX always set the corresponding GPyMUX bits to zero first to avoid glitching in the muxes
    GpioCtrlRegs.GPAGMUX1.bit.GPIO0=0; //Set GMUX value to zero
    GpioCtrlRegs.GPAMUX1.bit.GPIO0=1; //Set MUX value to 1 to select EPWM1A as peripheral function
    GpioCtrlRegs.GPADIR.bit.GPIO0=1;  // selection of GPIO Direction as OUTPUT

    //EPWM1B used as STROBE/
        GpioCtrlRegs.GPAMUX1.bit.GPIO1=0; //Before changing GPyGMUX always set the corresponding GPyMUX bits to zero first to avoid glitching in the muxes
        GpioCtrlRegs.GPAGMUX1.bit.GPIO1=0; //Set GMUX value to zero
        GpioCtrlRegs.GPAMUX1.bit.GPIO1=1; //Set MUX value to 1 to select EPWM1A as peripheral function
        GpioCtrlRegs.GPADIR.bit.GPIO1=1;  // selection of GPIO Direction as OUTPUT






    /*TBCLKSYNC bit in the peripheral clock enable registers allows
    * all users to globally synchronize all enabled ePWM modules to
    * the time-base clock (TBCLK)
    * First Disable and configure EPWMs and then enable just before
    * the start of while(1) loop
    * */
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;
    EALLOW;
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV=1;
    EDIS;

    //EPWM1/
    //Time Base/
    EPwm1Regs.TBPRD= TBPRD_VALUE;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV= 1; //Peripheral Clock division
    EPwm1Regs.TBCTL.bit.CLKDIV = 0x0;  //Another Peripheral Clock division
    EPwm1Regs.TBCTR = 0x0000; // Clear counter

    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; //Up-down count mode
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;  //Do not load the time-base counter (TBCTR) from the time-base phase register (TBPHS)
    EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    //Counter Compare/
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;

    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // load on CTR=Zero

    //Action Qualifier/
    EPwm1Regs.AQCTLA.bit.CAU = 1;   //CLEAR When TBCTR = CMPA on Up Count
    EPwm1Regs.AQCTLA.bit.CAD = 2;     //SET When TBCTR = CMPA on Down Count

    EPwm1Regs.CMPA.bit.CMPA = 4000;

    EPwm1Regs.DBCTL.bit.IN_MODE    =   0;           // ePWM-A for both RED and FED
    EPwm1Regs.DBRED.bit.DBRED      =   75;          // RED = 75 TBCLKs, 75 for 1.5us delay
    EPwm1Regs.DBFED.bit.DBFED      =   75;          // FED = 75 TBCLKs, 75 for 1.5us delay
    EPwm1Regs.DBCTL.bit.POLSEL     =   2;           // Active High Complementary (ePwm-B inverted)
    EPwm1Regs.DBCTL.bit.OUT_MODE   =   3;           // Dead Band mode is fully enabled


    //Selection of the source for the EPWMSYNCPER signal that goes to the DAC/
   // EPwm2Regs.HRPCTL.bit.PWMSYNCSEL = 1; //Sync when Time-base counter equal to zero

    //EPWM Carrier Synchronization/
    //EPWM2 is Synchronized to EPWM1/
   // SyncSocRegs.SYNCSELECT.bit.SYNCOUT=0;
   // EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
   // EPwm1Regs.TBCTL.bit.SYNCOSEL = 0;
   // EPwm3Regs.TBCTL.bit.SYNCOSEL = 0;
   // EPwm11Regs.TBCTL.bit.SYNCOSEL= 0;

    //EPWM Interrupt Configuration/
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;     //INT at time-base counter equal to zero
    EPwm1Regs.ETSEL.bit.INTEN = 1;                // Enable INT
    EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;           // Generate INT on 1st event

    //EPWM SOCA Configuration/
  //  EALLOW;
  //  EPwm2Regs.ETSEL.bit.SOCAEN = 1;     // Enable SOC on A group
  //  EPwm2Regs.ETSEL.bit.SOCASEL = 1;    // Enable event time-base counter equal to zero
  //  EPwm2Regs.ETPS.bit.SOCAPRD = 1; // Generate pulse on 1st event
  //  EDIS;
}



 // PWM comparison subroutine
interrupt void epwm1_isr(void)
        {



            EPwm1Regs.CMPA.bit.CMPA  = 2500+(2500*m*sinf(theta));;
            omega=2*pi*f;
            theta=theta+omega*Tsw;
            if(theta > 6.2832)
                       {
                         theta = theta - 6.2832;
                       }
            PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;         // Clear the PIEACK bit for the interrupt's PIE group
            EPwm1Regs.ETCLR.bit.INT=1;                      // Clear the ETFLG flag to enable further interrupts



        }




