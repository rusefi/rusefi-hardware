* we use the same timings across all injectors but the DRAM space is not sharable 
#include "dram2.def";

* uc0ch2 uses DAC6 (low?)
* uc1ch2 not used


* ### Initialization phase ###
init0:      dfcsct dac6l;
            stgn gain8.68 osoc;                     * Set the gain of the opamp of the current measure block 1
            ldjr1 eoinj0;                           * Load the eoinj line label Code RAM address into the register jr1 
            ldjr2 idle0;                            * Load the idle line label Code RAM address into the register jr2
            cwef jr1 _start row1;                   * If the start signal goes low, go to eoinj phase

* ### Idle phase- the uPC loops here until start signal is present ###
idle0:      joslr inj5_start start5;                * Jump to inj1 if start 1 is high
            joslr inj6_start start6;                * Jump to inj2 if start 2 is high
            jmpf jr1;

* ### Shortcuts definition per the injector to be actuated ###
inj5_start: dfsct hs5 hs6 ls5;                      * Set the 3 shortcuts: VBAT, VBOOST, LS2
            jmpr boost0;                            * Jump to launch phase
inj6_start: dfsct hs5 hs6 ls6;                      * Set the 3 shortcuts: VBAT, VBOOST, LS2
            jmpr boost0;                            * Jump to launch phase

* ### Launch phase enable boost ###
* Row1 - !start: injection ended
* Row2 - Overcurrent, jump to error case (threshold is reached early)
* Row3 - Overcurrent, jump to peak phase (threshold is reached on time)
* Row4 - timer1: Minimum boost time reached - switch to row3 instead of row2 to allow peak phase
* Row5 - timer2: Boost phase timeout - boost took too long, go to error phase
boost0:     load Iboost dac_osoc _ofs;              * Load the boost phase current threshold in the current DAC
            cwer boost0_err ocur row2;              * On overcurrent, go to boost error case
            cwer peak0 ocur row3;                   * Jump to peak phase when current is over threshold

            ldcd rst _ofs keep keep Tboost_min c1;  * Start boost counter to switch out of error behavior if threshold reached
            cwer boost0_mintime tc1 row4;           * On timer timeout, go allow overcurrent without error (ie, end of boost)

            ldcd rst _ofs keep keep Tboost_max c2;  * Start boost counter in case Iboost never reached
            cwer boost0_err tc2 row5;               * Jump to boost0_err in case boost phase takes too long

            stf low b1;                             * set flag1 low to force the DC-DC converter in idle mode
            stos off on on;                         * Turn VBAT off, BOOST on, LS on
            wait row1245;                           * Wait for one of the previously defined conditions
             
* ### Boost phase - minimum time reached ###
boost0_mintime: wait row135;                        * Minimum time for boost phase has been reached, now wait for !start, overcurrent or timeout

boost0_err: stos off off off;                       * Turn off all drivers
            stf low b0;                             * Set ch1 error flag0 to signal MCU
            stf high b1;                            * set flag1 high to release the DC-DC converter idle mode
            wait row1;                              * Wait for start signal to go low for the next injection attempt
            
            
* ### Peak phase continue on Vbat ###
peak0:      stos keep off keep;                      * Turn VBAT off (keep), BOOST off, LS on (keep)
            ldcd rst _ofs keep keep Tpeak_tot c1;   * Load the length of the total peak phase in counter 1
            load Ipeak dac_osoc _ofs;               * Load the peak current threshold in the current DAC            
            cwer bypass0 tc1 row2;                  * Jump to bypass phase when tc1 reaches end of count
            cwer peak_on0 tc2 row3;                 * Jump to peak_on when tc2 reaches end of count
            cwer peak_off0 ocur row4;               * Jump to peak_off when own-current is over threshold
                   
            stf high b1;                            * set flag1 high to release the DC-DC converter idle mode
            
            * discharge down to a known current
            cwer peak_off0 _ocur row5;              * Jump to peak_off when own-current is discharged
            wait row125;
            
* attempt to toggle this later            
peak_on0:   stos on off on;                         * Turn VBAT on, BOOST off, LS on
            wait row124;                            * Wait for one of the previously defined conditions
            
peak_off0:  ldcd rst ofs keep keep Tpeak_off c2;    * Load in the counter 2 the length of the peak_off phase
            stos off off on;                        * Turn VBAT off, BOOST off, LS on
            wait row123;



* ### Bypass phase ###
bypass0:    ldcd rst ofs keep keep Tbypass c3;      * Load in the counter 3 the length of the off_phase phase
            stos off off off;                       * Turn VBAT off, BOOST off, LS off
            cwer hold0 tc3 row4;                    * Jump to hold when tc3 reaches end of count
            wait row14;                             * Wait for one of the previously defined conditions

* ### Hold phase on Vbat ###
hold0:      ldcd rst _ofs keep keep Thold_tot c1;   * Load the length of the total hold phase in counter 2
            load Ihold dac_osoc _ofs;               * Load the hold current threshold in the DAC
            cwer eoinj0 tc1 row2;                   * Jump to eoinj phase when tc1 reaches end of count
            cwer hold_on0 tc2 row3;                 * Jump to hold_on when tc2 reaches end of count
            cwer hold_off0 ocur row4;               * Jump to hold_off when current is over threshold

hold_on0:   stos on off on;                         * Turn VBAT on, BOOST off, LS on
            wait row124;                            * Wait for one of the previously defined conditions

hold_off0:  ldcd rst _ofs keep keep Thold_off c2;   * Load the length of the hold_off phase in counter 1
            stos off off on;                        * Turn VBAT off, BOOST off, LS on
            wait row123;


* ### End of injection phase ###
eoinj0:     stos off off off;                       * Turn VBAT off, BOOST off, LS off
            stf high b1;                            * set flag1 to high to release the DC-DC converter idle mode
            jmpf jr2;                               * Jump back to idle phase

* ### End of Channel 2 - uCore0 code ###





* ### CHANNEL2 UCORE1 useless code (only here to avoid issue when dual core are enabled) #####
* Unable to find reference of enabling only a single core

init1:  ldirh 52h _rst;
        ldirl 52h _rst;
        ldirh 52h _rst;
        jmpr init1;
