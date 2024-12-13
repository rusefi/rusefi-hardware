/*
 * sent.h
 *
 * SENT protocol decoder header
 *
 * @date Dec 13, 2024
 * @author Andrey Gusakov <dron0gus@gmail.com>, (c) 2024
 */

#pragma once

#include "sent_decoder.h"

/* SENT decoder init */
void initSent();

/* decoder feed hook */
void SENT_ISR_Handler(uint8_t channels, uint16_t clocks, uint8_t flags);

int getSentValues(size_t index, uint16_t *sig0, uint16_t *sig1);

void sentDebug(void);