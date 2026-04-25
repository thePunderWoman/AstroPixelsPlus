//-----------------------------------------------------------------------------------------------
// Dome Panel Servo Sequences — AstroPixelsPlus (ESP32 / PCA9685)
//
// Ported from DomeSequences.cpp (VarSpeedServo / Arduino Pro Mini)
// Jessica Janiuk (thePunderWoman) 1-2023, ported 2026
//
// Receives commands on COMMAND_SERIAL (Serial2, defined in AstroPixelsPlus.ino)
// with prefix "DM:" — e.g. "DM:PIES\r".
//
// Servo motion uses ServoDispatchPCA9685 (servoDispatch global from .ino).
// Panel indices below map 1-to-1 to the servoSettings[] array in the .ino.
//-----------------------------------------------------------------------------------------------

#pragma once

// =============================================================================
// Panel servo indices — these are servoSettings[] array positions, NOT pin numbers.
// =============================================================================
//   servoSettings[0]  door 4       → P4   (SMALL_PANEL  / PANEL_GROUP_4)
//   servoSettings[1]  door 3       → P3   (SMALL_PANEL  / PANEL_GROUP_3)
//   servoSettings[2]  door 2       → P2   (SMALL_PANEL  / PANEL_GROUP_2)
//   servoSettings[3]  door 1       → P1   (SMALL_PANEL  / PANEL_GROUP_1)
//   servoSettings[4]  door 5       → P7   (MEDIUM_PANEL / PANEL_GROUP_5)
//   servoSettings[5]  door 9       → P10  (BIG_PANEL    / PANEL_GROUP_6)
//   servoSettings[6]  mini door 2  → P11  (MINI_PANEL)
//   servoSettings[7]  mini PSI     → P13  (MINI_PANEL)
//   servoSettings[8]  pie panel 1  → PP1  (PIE_PANEL    / PANEL_GROUP_10)
//   servoSettings[9]  pie panel 2  → PP2  (PIE_PANEL    / PANEL_GROUP_9)
//   servoSettings[10] pie panel 3  → PP5  (PIE_PANEL    / PANEL_GROUP_8)
//   servoSettings[11] pie panel 4  → PP6  (PIE_PANEL    / PANEL_GROUP_7)
//   servoSettings[12] dome top     → (TOP_PIE_PANEL — not used in sequences below)

#define P4    0
#define P3    1
#define P2    2
#define P1    3
#define P7    4
#define P10   5
#define P11   6
#define P13   7
#define PP1   8
#define PP2   9
#define PP5  10
#define PP6  11

// =============================================================================
// Panel position values (pulse width in microseconds).
// servoSettings uses 800–2200 µs; tune PANEL_OPEN / PANEL_CLOSE for your panels.
// =============================================================================
#define DOME_PANEL_OPEN      2200
#define DOME_PANEL_CLOSE     1400
#define DOME_PANEL_RANGE     (DOME_PANEL_OPEN - DOME_PANEL_CLOSE)
#define DOME_PANEL_75_OPEN   (DOME_PANEL_CLOSE + (DOME_PANEL_RANGE * 3 / 4))
#define DOME_PANEL_50_OPEN   (DOME_PANEL_CLOSE + (DOME_PANEL_RANGE / 2))
#define DOME_PANEL_25_OPEN   (DOME_PANEL_CLOSE + (DOME_PANEL_RANGE / 4))

// =============================================================================
// Move-time constants (milliseconds). 
// Smaller = faster.  Tune to taste for your servos.
// =============================================================================
#define DOME_MOVE_SLOWSPEED    500   // deliberate/dramatic
#define DOME_MOVE_VLOWSPEED    350
#define DOME_MOVE_LOWSPEED     200
#define DOME_MOVE_SPEED        150   // normal open/close
#define DOME_MOVE_FASTSPEED    100   // burst/snap open
#define DOME_MOVE_OPENSPEED    100
#define DOME_MOVE_CLOSESPEED   100
#define DOME_MOVE_OVERLOAD     300   // intentionally very slow drift

// =============================================================================
// Panel open/close state (toggle tracking)
// =============================================================================
static bool dome_PiesOpen = false;
static bool dome_AllOpen  = false;
static bool dome_LowOpen  = false;

// Re-entrancy guard: prevents a second sequence from starting while one runs.
static bool dome_seqRunning = false;

// =============================================================================
// Core helpers
// =============================================================================

// Pump the full ReelTwo event loop while waiting.
// This lets PCA9685 servo tweens animate smoothly during blocking sequences.
static void domeWaitTime(unsigned long ms)
{
    unsigned long end = millis() + ms;
    while (millis() < end)
        AnimatedEvent::process();
}

// Send a command to the body controller via COMMAND_SERIAL (Serial2).
static void domeSendToBody(const char* cmd)
{
    COMMAND_SERIAL.print("BD:");
    COMMAND_SERIAL.print(cmd);
    COMMAND_SERIAL.print('\r');
}

// Notify the body controller that a dome sequence is starting (seconds = safety timeout).
static void domeBeginSequence(unsigned int seconds)
{
    dome_seqRunning = true;
    COMMAND_SERIAL.print("dome=seqon,");
    COMMAND_SERIAL.print(seconds);
    COMMAND_SERIAL.print('\r');
}

static void domeEndSequence()
{
    COMMAND_SERIAL.print("dome=seqoff\r");
    dome_seqRunning = false;
}

static inline void domeMove(uint8_t idx, uint16_t pos, uint32_t moveMs, bool wait = false)
{
    servoDispatch.moveToPulse(idx, moveMs, pos);
    if (wait) {
        // Blocking: move a servo and wait until it actually arrives.
        while (servoDispatch.isActive(idx))
            AnimatedEvent::process();
    }
}

// =============================================================================
// Easing helpers — drive multiple servos with ServoDispatch easing methods.
// =============================================================================
static void domeEaseSineInOut(const uint8_t* idx, uint8_t count,
                              int /*from*/, int to, unsigned int durationMs)
{
    for (uint8_t i = 0; i < count; i++)
        servoDispatch.setServoEasingMethod(idx[i], Easing::SineEaseInOut);
    for (uint8_t i = 0; i < count; i++)
        servoDispatch.moveToPulse(idx[i], durationMs, (uint16_t)to);
    domeWaitTime(durationMs + 50);
    for (uint8_t i = 0; i < count; i++)
        servoDispatch.setServoEasingMethod(idx[i], Easing::LinearInterpolation);
}

static void domeEaseOut(const uint8_t* idx, uint8_t count,
                        int /*from*/, int to, unsigned int durationMs)
{
    for (uint8_t i = 0; i < count; i++)
        servoDispatch.setServoEasingMethod(idx[i], Easing::SineEaseOut);
    for (uint8_t i = 0; i < count; i++)
        servoDispatch.moveToPulse(idx[i], durationMs, (uint16_t)to);
    domeWaitTime(durationMs + 50);
    for (uint8_t i = 0; i < count; i++)
        servoDispatch.setServoEasingMethod(idx[i], Easing::LinearInterpolation);
}

// =============================================================================
// Random panel selection (Fisher-Yates shuffle; indices = servoDispatch indices)
// =============================================================================
static uint8_t domeRandomPanels(uint8_t numLower, uint8_t numPie, uint8_t* out)
{
    static const uint8_t lowerPanels[] = { P1, P2, P3, P4, P7, P10, P11, P13 };
    static const uint8_t piePanels[]   = { PP1, PP2, PP5, PP6 };

    uint8_t lower[8], pie[4];
    memcpy(lower, lowerPanels, sizeof(lower));
    memcpy(pie,   piePanels,   sizeof(pie));

    for (uint8_t i = 7; i > 0; i--) { uint8_t j = random(i + 1); uint8_t t = lower[i]; lower[i] = lower[j]; lower[j] = t; }
    for (uint8_t i = 3; i > 0; i--) { uint8_t j = random(i + 1); uint8_t t = pie[i];   pie[i]   = pie[j];   pie[j]   = t; }

    if (numLower > 8) numLower = 8;
    if (numPie   > 4) numPie   = 4;

    uint8_t total = 0;
    for (uint8_t i = 0; i < numLower; i++) out[total++] = lower[i];
    for (uint8_t i = 0; i < numPie;   i++) out[total++] = pie[i];
    return total;
}

// =============================================================================
// Reset helpers — forward to local Marcduino command processor
// =============================================================================
static void domeResetHolos()
{
    CommandEvent::process(F("HPS9"));
}

static void domeResetLogics()
{
    RLD.selectSequence(LogicEngineRenderer::NORMAL);
    FLD.selectSequence(LogicEngineRenderer::NORMAL);
}

static void domeResetPSIs()
{
    COMMAND_SERIAL.println("4T1");
    COMMAND_SERIAL.println("5T1");
}

static void domeResetBody()
{
    domeSendToBody("RESET");
}

// =============================================================================
// Open / Close Pie Panels
// =============================================================================
static void domeOpenClosePies()
{
    domeBeginSequence(12);

    if (dome_PiesOpen)
    {
        dome_PiesOpen = false;
        domeResetHolos();
        domeSendToBody("HAPPY");

        domeMove(PP1, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(PP2, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(PP5, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(PP6, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);

        domeWaitTime(800);
        servoDispatch.disable(PP1); servoDispatch.disable(PP2);
        servoDispatch.disable(PP5); servoDispatch.disable(PP6);
    }
    else
    {
        dome_PiesOpen = true;
        domeWaitTime(100);
        domeSendToBody("HAPPY");

        for (int i = 0; i < 2; i++)
        {
            // wave open
            domeMove(PP2, DOME_PANEL_OPEN,  DOME_MOVE_FASTSPEED, true);
            domeMove(PP1, DOME_PANEL_OPEN,  DOME_MOVE_FASTSPEED, true);
            domeMove(PP6, DOME_PANEL_OPEN,  DOME_MOVE_FASTSPEED, true);
            domeMove(PP5, DOME_PANEL_OPEN,  DOME_MOVE_FASTSPEED, true);
            // wave close
            domeMove(PP5, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED, true);
            domeMove(PP6, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED, true);
            domeMove(PP1, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED, true);
            domeMove(PP2, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED, true);
            // reopen
            domeMove(PP2, DOME_PANEL_OPEN,  DOME_MOVE_FASTSPEED, true);
            domeMove(PP1, DOME_PANEL_OPEN,  DOME_MOVE_FASTSPEED, true);
            domeMove(PP6, DOME_PANEL_OPEN,  DOME_MOVE_FASTSPEED, true);
            domeMove(PP5, DOME_PANEL_OPEN,  DOME_MOVE_FASTSPEED, true);
        }

        domeWaitTime(1000);
        servoDispatch.disable(PP1); servoDispatch.disable(PP2);
        servoDispatch.disable(PP5); servoDispatch.disable(PP6);
    }

    domeEndSequence();
}

// =============================================================================
// Open / Close Low Panels
// =============================================================================
static void domeOpenCloseLow()
{
    domeBeginSequence(15);

    if (dome_LowOpen)
    {
        dome_LowOpen = false;
        domeResetHolos();
        domeSendToBody("HAPPY");

        domeMove(P4,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P2,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P1,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P3,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P11, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P13, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P7,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P10, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);

        domeWaitTime(1000);
        servoDispatch.disable(P1);  servoDispatch.disable(P2);
        servoDispatch.disable(P3);  servoDispatch.disable(P4);
        servoDispatch.disable(P7);  servoDispatch.disable(P10);
        servoDispatch.disable(P11); servoDispatch.disable(P13);
    }
    else
    {
        dome_LowOpen = true;
        domeSendToBody("HAPPY");

        for (int i = 0; i < 2; i++)
        {
            // wave open — P11/P13/P10 non-blocking so they move with P1
            domeMove(P1,  DOME_PANEL_OPEN, DOME_MOVE_SPEED, true);
            domeMove(P11, DOME_PANEL_OPEN, DOME_MOVE_SPEED, true);
            domeMove(P13, DOME_PANEL_OPEN, DOME_MOVE_SPEED, true);
            domeMove(P10, DOME_PANEL_OPEN, DOME_MOVE_SPEED, true);
            domeMove(P2,  DOME_PANEL_OPEN, DOME_MOVE_SPEED, true);
            domeMove(P3,  DOME_PANEL_OPEN, DOME_MOVE_SPEED, true);
            domeMove(P4,  DOME_PANEL_OPEN, DOME_MOVE_SPEED, true);
            domeMove(P7,  DOME_PANEL_OPEN, DOME_MOVE_SPEED, true);

            // wave close
            domeMove(P7,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
            domeMove(P4,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
            domeMove(P3,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
            domeMove(P2,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
            domeMove(P1,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
            domeWaitTime(50);
            domeMove(P11, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
            domeMove(P13, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
            domeWaitTime(50);
            domeMove(P10, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        }

        // final open
        domeMove(P10, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
        domeMove(P11, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
        domeMove(P13, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
        domeMove(P1,  DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED, true);
        domeMove(P2,  DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED, true);
        domeMove(P3,  DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED, true);
        domeMove(P4,  DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED, true);
        domeMove(P7,  DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED, true);

        domeWaitTime(1000);
        servoDispatch.disable(P1);  servoDispatch.disable(P2);
        servoDispatch.disable(P3);  servoDispatch.disable(P4);
        servoDispatch.disable(P7);  servoDispatch.disable(P10);
        servoDispatch.disable(P11); servoDispatch.disable(P13);
    }

    domeEndSequence();
}

// =============================================================================
// Open / Close All Panels
// =============================================================================
static void domeOpenCloseAll()
{
    domeBeginSequence(10);

    if (dome_AllOpen)
    {
        dome_AllOpen = false;
        domeSendToBody("HAPPY");

        domeMove(PP2, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(PP6, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(PP5, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(PP1, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P10, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P7,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P4,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P3,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P2,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P1,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P11, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
        domeMove(P13, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);

        domeWaitTime(500);
        servoDispatch.disable(PP1); servoDispatch.disable(PP2);
        servoDispatch.disable(PP5); servoDispatch.disable(PP6);
        servoDispatch.disable(P1);  servoDispatch.disable(P2);
        servoDispatch.disable(P3);  servoDispatch.disable(P4);
        servoDispatch.disable(P7);  servoDispatch.disable(P10);
        servoDispatch.disable(P11); servoDispatch.disable(P13);
    }
    else
    {
        dome_AllOpen = true;
        domeSendToBody("HAPPY");

        // open pies
        domeMove(PP2, DOME_PANEL_OPEN, DOME_MOVE_SPEED, true);
        domeMove(PP5, DOME_PANEL_OPEN, DOME_MOVE_SPEED, true);
        domeMove(PP1, DOME_PANEL_OPEN, DOME_MOVE_SPEED, true);
        domeMove(PP6, DOME_PANEL_OPEN, DOME_MOVE_SPEED, true);

        // open lows (non-blocking except last)
        domeMove(P10, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
        domeMove(P11, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
        domeMove(P13, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
        domeMove(P1,  DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
        domeMove(P2,  DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
        domeMove(P3,  DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
        domeMove(P4,  DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
        domeMove(P7, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED, true);

        // twinkle P1/P2 and PP2/PP5
        for (int i = 0; i < 2; i++)
        {
            domeMove(P1, DOME_PANEL_75_OPEN, DOME_MOVE_FASTSPEED, true);
            domeMove(P1, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
            domeWaitTime(80);
            domeMove(P2, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED, true);
            domeMove(P2, DOME_PANEL_75_OPEN, DOME_MOVE_FASTSPEED);
            domeWaitTime(80);
            domeMove(P2, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
            domeWaitTime(100);

            domeMove(PP2, DOME_PANEL_75_OPEN, DOME_MOVE_FASTSPEED, true);
            domeMove(PP2, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED, true);
            domeWaitTime(80);
            domeMove(PP5, DOME_PANEL_75_OPEN, DOME_MOVE_FASTSPEED, true);
            domeMove(PP5, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED, true);
        }

        domeWaitTime(800);
        servoDispatch.disable(PP1); servoDispatch.disable(PP2);
        servoDispatch.disable(PP5); servoDispatch.disable(PP6);
        servoDispatch.disable(P1);  servoDispatch.disable(P2);
        servoDispatch.disable(P3);  servoDispatch.disable(P4);
        servoDispatch.disable(P7);  servoDispatch.disable(P10);
        servoDispatch.disable(P11); servoDispatch.disable(P13);
    }

    domeEndSequence();
}

// =============================================================================
// Flutter — quick open-close flap, all panels end closed
// =============================================================================
static void domeFlutter()
{
    domeBeginSequence(10);

    domeMove(P1,  DOME_PANEL_75_OPEN, DOME_MOVE_SPEED, true);
    domeMove(P2,  DOME_PANEL_75_OPEN, DOME_MOVE_SPEED, true);
    domeMove(P3,  DOME_PANEL_75_OPEN, DOME_MOVE_SPEED, true);
    domeWaitTime(20);
    domeMove(P4,  DOME_PANEL_75_OPEN, DOME_MOVE_SPEED, true);
    domeWaitTime(20);
    domeMove(P7,  DOME_PANEL_75_OPEN, DOME_MOVE_SPEED, true);
    domeMove(P10, DOME_PANEL_75_OPEN, DOME_MOVE_SPEED, true);
    domeMove(P11, DOME_PANEL_75_OPEN, DOME_MOVE_SPEED, true);
    domeMove(P13, DOME_PANEL_75_OPEN, DOME_MOVE_SPEED, true);

    domeMove(PP2, DOME_PANEL_75_OPEN, DOME_MOVE_SPEED, true);
    domeMove(PP1, DOME_PANEL_75_OPEN, DOME_MOVE_SPEED, true);
    domeMove(PP6, DOME_PANEL_75_OPEN, DOME_MOVE_SPEED, true);
    domeWaitTime(20);
    domeMove(PP5, DOME_PANEL_75_OPEN, DOME_MOVE_SPEED, true);

    domeMove(P1,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
    domeMove(P2,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
    domeMove(P3,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
    domeMove(P4,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
    domeMove(P7,  DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
    domeMove(P10, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
    domeMove(P11, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
    domeMove(P13, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);

    domeMove(PP2, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
    domeMove(PP1, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
    domeMove(PP6, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);
    domeWaitTime(20);
    domeMove(PP5, DOME_PANEL_CLOSE, DOME_MOVE_SPEED, true);

    domeWaitTime(500);

    servoDispatch.disable(PP1); servoDispatch.disable(PP2);
    servoDispatch.disable(PP5); servoDispatch.disable(PP6);
    servoDispatch.disable(P1);  servoDispatch.disable(P2);
    servoDispatch.disable(P3);  servoDispatch.disable(P4);
    servoDispatch.disable(P7);  servoDispatch.disable(P10);
    servoDispatch.disable(P11); servoDispatch.disable(P13);

    dome_PiesOpen = false;
    dome_AllOpen  = false;
    dome_LowOpen  = false;

    domeEndSequence();
}

// =============================================================================
// Bloom — pie panels ease open, wiggle, close
// =============================================================================
static void domeBloom()
{
    domeBeginSequence(8);

    static const uint8_t pies[] = { PP1, PP2, PP5, PP6 };

    domeEaseOut(pies, 4, DOME_PANEL_CLOSE, DOME_PANEL_OPEN, 1200);
    domeWaitTime(2000);

    for (uint8_t i = 0; i < 3; i++)
    {
        domeEaseSineInOut(pies, 4, DOME_PANEL_OPEN, 1900, 130);
        domeEaseSineInOut(pies, 4, 1900, DOME_PANEL_OPEN, 130);
    }

    domeWaitTime(1000);

    domeMove(PP1, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeMove(PP2, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeMove(PP5, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeMove(PP6, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);

    domeWaitTime(500);
    servoDispatch.disable(PP1); servoDispatch.disable(PP2);
    servoDispatch.disable(PP5); servoDispatch.disable(PP6);

    dome_PiesOpen = false;
    domeEndSequence();
}

// =============================================================================
// Scream — all panels burst open with random fluttering, then close
// =============================================================================
static void domeScream()
{
    domeBeginSequence(15);

    CommandEvent::process(F("HPA0070")); // all holos short circuit random color
    CommandEvent::process(F("HPA105|5")); // all holos wag 5 times
    
    FLD.selectSequence(LogicEngineRenderer::REDALERT, FLD.kDefault, 0, 15);
    RLD.selectSequence(LogicEngineRenderer::REDALERT, RLD.kDefault, 0, 15);

    COMMAND_SERIAL.println("4T5");
    COMMAND_SERIAL.println("5T5");
    domeSendToBody("SCREAM");

    dome_AllOpen = true;

    // burst open
    domeMove(PP2, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
    domeMove(PP5, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
    domeMove(PP1, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
    domeMove(PP6, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
    domeMove(P10, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
    domeMove(P11, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
    domeMove(P13, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
    domeMove(P1, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
    domeMove(P2, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
    domeMove(P3, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
    domeMove(P4, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
    domeMove(P7, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
    domeWaitTime(DOME_MOVE_FASTSPEED + 100);

    // random flutter
    static const uint8_t allPanels[] = { PP1, PP2, PP5, PP6, P1, P2, P3, P4, P7, P10, P11, P13 };
    randomSeed(analogRead(0));
    for (int i = 0; i < 10; i++)
    {
        uint8_t idx = allPanels[random(12)];
        domeMove(idx, DOME_PANEL_50_OPEN, DOME_MOVE_FASTSPEED, true);
        domeMove(idx, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
        domeWaitTime(80);
        domeMove(idx, DOME_PANEL_50_OPEN, DOME_MOVE_FASTSPEED, true);
        domeMove(idx, DOME_PANEL_OPEN, DOME_MOVE_FASTSPEED);
        domeWaitTime(100);
    }

    domeWaitTime(800);
    domeWaitTime(2000);

    // close
    dome_AllOpen = false;
    domeSendToBody("HAPPY");

    domeMove(PP2, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(PP6, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(PP5, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(PP1, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P10, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P7, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P4, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P3, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P2, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P1, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P11, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P13, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeWaitTime(DOME_MOVE_SPEED + 500);

    servoDispatch.disable(PP1); servoDispatch.disable(PP2);
    servoDispatch.disable(PP5); servoDispatch.disable(PP6);
    servoDispatch.disable(P1);  servoDispatch.disable(P2);
    servoDispatch.disable(P3);  servoDispatch.disable(P4);
    servoDispatch.disable(P7);  servoDispatch.disable(P10);
    servoDispatch.disable(P11); servoDispatch.disable(P13);

    domeResetHolos();
    domeResetLogics();
    domeResetPSIs();

    domeEndSequence();
}

// =============================================================================
// Overload — random panels sluggishly drift open, then snap closed
// =============================================================================
static void domeOverload()
{
    domeBeginSequence(12);

    FLD.selectSequence(LogicEngineRenderer::FAILURE);
    RLD.selectSequence(LogicEngineRenderer::FAILURE);
    
    CommandEvent::process(F("HPA0070")); // all holos short circuit random color

    COMMAND_SERIAL.println("4T4");
    COMMAND_SERIAL.println("5T4");

    domeSendToBody("OVERLOAD");

    uint8_t panels[6];
    uint8_t count = domeRandomPanels(4, 2, panels);

    for (uint8_t i = 0; i < count; i++)
    {
        int pos = random(DOME_PANEL_25_OPEN, DOME_PANEL_50_OPEN + 1);
        domeMove(panels[i], (uint16_t)pos, DOME_MOVE_OVERLOAD);
        domeWaitTime(random(400, 900));
    }

    domeWaitTime(2500);

    for (uint8_t i = 0; i < count; i++)
        domeMove(panels[i], DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeWaitTime(800);
    for (uint8_t i = 0; i < count; i++)
        servoDispatch.disable(panels[i]);

    domeResetHolos();
    domeResetLogics();
    domeResetPSIs();

    domeEndSequence();
}

// =============================================================================
// Heart — rainbow holos, sweet message on logics
// =============================================================================
static void domeHeart()
{
    domeBeginSequence(10);

    CommandEvent::process(F("HPF006|10"));
    CommandEvent::process(F("HPR006|10"));
    CommandEvent::process(F("HPT006|10"));
    FLD.selectScrollTextLeft("You're\nWonderful", FLD.kDefault, 0, 10);
    COMMAND_SERIAL.println("4T7");
    domeSendToBody("HEART");

    domeEndSequence();
}

// =============================================================================
// Alarm — pulsing red holos and logics
// =============================================================================
static void domeAlarm()
{
    domeBeginSequence(10);

    CommandEvent::process(F("HPA0021|10")); // all holos red flashes

    FLD.selectSequence(LogicEngineRenderer::ALARM, FLD.kDefault, 0, 10);
    RLD.selectSequence(LogicEngineRenderer::ALARM, RLD.kDefault, 0, 10);

    COMMAND_SERIAL.println("4T3");
    COMMAND_SERIAL.println("5T3");

    domeSendToBody("ALARM");

    domeEndSequence();
}

// =============================================================================
// Hello There — P1 waves a greeting
// =============================================================================
static void domeHelloThere()
{
    domeBeginSequence(4);

    FLD.selectScrollTextLeft("Hello\nThere", FLD.kDefault, 0, 10);
    RLD.selectScrollTextLeft("General Kenobi", RLD.randomColor());

    domeSendToBody("HELLO");

    domeMove(P1, DOME_PANEL_OPEN,     DOME_MOVE_SPEED, true);
    domeWaitTime(10);
    domeMove(P1, DOME_PANEL_50_OPEN,  DOME_MOVE_SPEED, true);
    domeWaitTime(10);
    domeMove(P1, DOME_PANEL_OPEN,     DOME_MOVE_SPEED, true);
    domeWaitTime(10);
    domeMove(P1, DOME_PANEL_50_OPEN,  DOME_MOVE_SPEED, true);
    domeWaitTime(10);
    domeMove(P1, DOME_PANEL_OPEN,     DOME_MOVE_SPEED, true);
    domeWaitTime(10);
    domeMove(P1, DOME_PANEL_CLOSE,    DOME_MOVE_SPEED, true);
    servoDispatch.disable(P1);

    domeEndSequence();
}

// =============================================================================
// Leia — front HP runs Leia LED sequence, all other HPs off, logics Leia mode
// =============================================================================
static void domeLeiaMode()
{
    domeBeginSequence(36);

    CommandEvent::process(F("HPS101|36")); // front holo leia sequence
    CommandEvent::process(F("HPR02|36")); // rear holo off
    CommandEvent::process(F("HPT02|36")); // top holo off
    
    FLD.selectSequence(LogicEngineRenderer::LEIA, FLD.kDefault, 0, 36);
    RLD.selectSequence(LogicEngineRenderer::LEIA, RLD.kDefault, 0, 36);
    
    COMMAND_SERIAL.println("4T6|36");
    COMMAND_SERIAL.println("5T6|36");
    domeSendToBody("LEIA");
    domeWaitTime(500);

    domeEndSequence();
}

// =============================================================================
// Reset All — close every panel, reset holos / logics / PSIs / body
// =============================================================================
static void domeResetAll()
{
    domeBeginSequence(4);

    domeMove(PP1, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(PP2, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(PP5, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(PP6, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P1, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P2, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P3, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P4, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P7, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P10, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P11, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeMove(P13, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
    domeWaitTime(DOME_MOVE_SPEED + 1000);

    servoDispatch.disable(PP1); servoDispatch.disable(PP2);
    servoDispatch.disable(PP5); servoDispatch.disable(PP6);
    servoDispatch.disable(P1);  servoDispatch.disable(P2);
    servoDispatch.disable(P3);  servoDispatch.disable(P4);
    servoDispatch.disable(P7);  servoDispatch.disable(P10);
    servoDispatch.disable(P11); servoDispatch.disable(P13);

    dome_PiesOpen = false;
    dome_AllOpen  = false;
    dome_LowOpen  = false;

    domeResetHolos();
    domeResetLogics();
    domeResetPSIs();
    domeResetBody();

    domeEndSequence();
}

// =============================================================================
// Cantina — 15-second alternating panel dance at 130 BPM
// =============================================================================
static void domeCantina()
{
    domeBeginSequence(17);
    domeSendToBody("CANTINA");

    Marcduino::processCommand(player, "@1T2|15");
    Marcduino::processCommand(player, "@2T2|15");
    Marcduino::processCommand(player, "@3T2|15");
    
    CommandEvent::process(F("HPA0029|15")); // all holos white flashes

    FLD.selectSequence(LogicEngineRenderer::FLASHCOLOR, FLD.kBlue, 0, 15);
    RLD.selectSequence(LogicEngineRenderer::FLASHCOLOR, RLD.kBlue, 0, 15);

    COMMAND_SERIAL.println("4T13|15");
    COMMAND_SERIAL.println("5T13|15");

    // 130 BPM ≈ 923 ms per beat
    const unsigned long BEAT_MS  = 923;
    const unsigned long DURATION = 15000;
    domeWaitTime(100);

    bool evenOpen = true;
    unsigned long endTime = millis() + DURATION;

    while (millis() < endTime)
    {
        if (evenOpen)
        {
            domeMove(PP1, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
            domeMove(PP5, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
            domeMove(PP2, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
            domeMove(PP6, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
            domeMove(P1, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
            domeMove(P3, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
            domeMove(P7, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
            domeMove(P11, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
            domeMove(P2, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
            domeMove(P4, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
            domeMove(P10, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
            domeMove(P13, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
        }
        else
        {
            domeMove(PP1, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
            domeMove(PP5, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
            domeMove(PP2, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
            domeMove(PP6, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
            domeMove(P1, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
            domeMove(P3, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
            domeMove(P7, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
            domeMove(P11, DOME_PANEL_CLOSE, DOME_MOVE_SPEED);
            domeMove(P2, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
            domeMove(P4, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
            domeMove(P10, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
            domeMove(P13, DOME_PANEL_OPEN, DOME_MOVE_SPEED);
        }
        evenOpen = !evenOpen;
        domeWaitTime(BEAT_MS);
    }

    domeMove(PP1, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeMove(PP2, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeMove(PP5, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeMove(PP6, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeMove(P1, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeMove(P2, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeMove(P3, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeMove(P4, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeMove(P7, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeMove(P10, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeMove(P11, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED);
    domeMove(P13, DOME_PANEL_CLOSE, DOME_MOVE_FASTSPEED, true);

    domeWaitTime(500);

    servoDispatch.disable(PP1); servoDispatch.disable(PP2);
    servoDispatch.disable(PP5); servoDispatch.disable(PP6);
    servoDispatch.disable(P1);  servoDispatch.disable(P2);
    servoDispatch.disable(P3);  servoDispatch.disable(P4);
    servoDispatch.disable(P7);  servoDispatch.disable(P10);
    servoDispatch.disable(P11); servoDispatch.disable(P13);

    dome_PiesOpen = false;
    dome_AllOpen  = false;
    dome_LowOpen  = false;

    domeResetPSIs();
    domeResetLogics();
    domeResetHolos();

    domeEndSequence();
}

// =============================================================================
// Marcduino serial command handlers — prefix "DM:" on COMMAND_SERIAL
//
// Send from any Marcduino-compatible device as:   DM:PIES\r
// =============================================================================

MARCDUINO_ACTION(DomeReset,    DM:RESET,   ({ if (!dome_seqRunning) domeResetAll();    }))
MARCDUINO_ACTION(DomePies,     DM:PIES,    ({ if (!dome_seqRunning) domeOpenClosePies(); }))
MARCDUINO_ACTION(DomeLow,      DM:LOW,     ({ if (!dome_seqRunning) domeOpenCloseLow();  }))
MARCDUINO_ACTION(DomeOpenAll,  DM:OPENALL, ({ if (!dome_seqRunning) domeOpenCloseAll();  }))
MARCDUINO_ACTION(DomeLeia,     DM:LEIA,    ({ if (!dome_seqRunning) domeLeiaMode();      }))
MARCDUINO_ACTION(DomeHeart,    DM:HEART,   ({ if (!dome_seqRunning) domeHeart();         }))
MARCDUINO_ACTION(DomeHello,    DM:HELLO,   ({ if (!dome_seqRunning) domeHelloThere();    }))
MARCDUINO_ACTION(DomeScream,   DM:SCREAM,  ({ if (!dome_seqRunning) domeScream();        }))
MARCDUINO_ACTION(DomeFlutter,  DM:FLUTTER, ({ if (!dome_seqRunning) domeFlutter();       }))
MARCDUINO_ACTION(DomeOverload, DM:OVERLOAD,({ if (!dome_seqRunning) domeOverload();      }))
MARCDUINO_ACTION(DomeBloom,    DM:BLOOM,   ({ if (!dome_seqRunning) domeBloom();         }))
MARCDUINO_ACTION(DomeCantina,  DM:CANTINA, ({ if (!dome_seqRunning) domeCantina();       }))
MARCDUINO_ACTION(DomeAlarm,    DM:ALARM,   ({ if (!dome_seqRunning) domeAlarm();         }))
