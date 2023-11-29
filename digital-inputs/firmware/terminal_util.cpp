#include "ch.h"
#include "hal.h"
#include "terminal_util.h"

extern BaseSequentialStream *chp;
extern bool globalEverythingHappy;

void setRedText() {
    chprintf(chp, "\033[1;31m");
}

void setGreenText() {
    chprintf(chp, "\033[0;32m");
}

void setYellowText() {
    chprintf(chp, "\033[0;33m");
}

void setCyanText() {
    chprintf(chp, "\033[0;36m");
}

void setNormalText() {
    chprintf(chp, "\033[0m");
}

void setGlobalStatusText() {
    if (globalEverythingHappy) {
        setGreenText();
    } else {
        setRedText();
    }
}
