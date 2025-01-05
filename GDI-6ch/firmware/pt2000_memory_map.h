/**
 * see mc33816/rusefi/readme.md
*/

typedef enum {
    // see dram1.def values
    Iboost = 0,
    Ipeak = 1,
    Ihold = 2,
    Tpeak_off = 3,
    Tpeak_tot = 4,
    Tbypass = 5,
    Thold_off = 6,
    Thold_tot = 7,
    Tboost_max = 8,
    Tboost_min = 9,
    // see dram2.def values, base 64 for channel 2
    Iboost_2 = 64,
    Ipeak_2 = 65,
    Ihold_2 = 66,
    Tpeak_off_2 = 67,
    Tpeak_tot_2 = 68,
    Tbypass_2 = 69,
    Thold_off_2 = 70,
    Thold_tot_2 = 71,
    Tboost_max_2 = 72,
    Tboost_min_2 = 73,
    // see dram3.def values, base 64 for channel 3
    Vboost_high = 128,
    Vboost_low = 129,
    Isense4_high = 130,
    Isense4_low = 131,
    HPFP_Ipeak = 133,
    HPFP_Ihold = 134,
    HPFP_Thold_off = 135,
    HPFP_Thold_tot = 136,


    BLANK = 140,
} MC33PT2000Mem;
