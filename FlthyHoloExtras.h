////////////////
// FlthyHoloExtras.h
//
// Additional Holoprojector commands inspired by the FlthyHPs sketch by Ryan Sondgeroth.
//
// Adds the following capability groups:
//   - Short Circuit: flickering flash effect, great for overload or scream sequences
//   - Solid Color: sets all LEDs to a fixed color
//   - All-HP Cycle: spinning single-LED cycle across all three holos simultaneously
//   - All-HP Dim Pulse: slow color breathe across all three holos simultaneously
//   - Holo Mode Loop: enables random cycling through lighting modes (ColorProjector,
//       DimPulse, Cycle, SolidColor, Rainbow) with random HP servo movement.
//       ShortCircuit is intentionally excluded — use the *SC commands for that.
//   - Reset to Mode Loop: resets all HPs into the mode loop instead of normal off state
//
// Color reference (matches FlthyHPs / HoloLights color table):
//   0=Random, 1=Red, 2=Yellow, 3=Green, 4=Cyan, 5=Blue,
//   6=Magenta, 7=Orange, 8=Purple, 9=White
//
// Command format: HP[designator][type][function##][color]
//   Designator: F=Front, R=Rear, T=Top, A=All
//   Type:       0=LED, 1=Servo
//   Function 07 = ShortCircuit, 05 = SolidColor
//   S9 sequence = enable random LED mode loop + HP servo twitch
//
////////////////

////////////////
// SHORT CIRCUIT
// Flashes the HP LEDs on and off with intervals that slow over time (20 cycles).
// Orange (color 7) matches the original FlthyHPs default short circuit color.
////////////////

MARCDUINO_ACTION(FrontHoloShortCircuit, *SC01, ({
    // Front Holo Short Circuit - orange flash
    CommandEvent::process(F("HPF0077"));
}))

////////////////

MARCDUINO_ACTION(RearHoloShortCircuit, *SC02, ({
    // Rear Holo Short Circuit - orange flash
    CommandEvent::process(F("HPR0077"));
}))

////////////////

MARCDUINO_ACTION(TopHoloShortCircuit, *SC03, ({
    // Top Holo Short Circuit - orange flash
    CommandEvent::process(F("HPT0077"));
}))

////////////////

MARCDUINO_ACTION(AllHoloShortCircuit, *SC00, ({
    // All Holos Short Circuit - orange flash (great for overload/scream effects)
    CommandEvent::process(F("HPA0077"));
}))

////////////////
// ALL-HP CYCLE
// Spinning single-LED cycle on all three holos simultaneously.
// Individual-HP cycle commands (*ON01/02/03) already exist in MarcduinoHolo.h.
////////////////

MARCDUINO_ACTION(AllHoloCycle, *CY00, ({
    // All Holos Cycle - random color
    CommandEvent::process(F("HPA0040"));
}))

////////////////
// ALL-HP DIM PULSE
// Slow color breathe on all three holos simultaneously.
// Individual-HP dim pulse commands (*HPS301/302/303) already exist in MarcduinoHolo.h.
////////////////

MARCDUINO_ACTION(AllHoloDimPulse, *PL00, ({
    // All Holos Dim Pulse - random color
    CommandEvent::process(F("HPA0030"));
}))

////////////////
// SOLID COLOR - BLUE (color 5)
// Sets all LEDs to a fixed blue. Useful for a calm, ambient holo state.
////////////////

MARCDUINO_ACTION(FrontHoloSolidBlue, *SB01, ({
    // Front Holo solid blue
    CommandEvent::process(F("HPF0055"));
}))

////////////////

MARCDUINO_ACTION(RearHoloSolidBlue, *SB02, ({
    // Rear Holo solid blue
    CommandEvent::process(F("HPR0055"));
}))

////////////////

MARCDUINO_ACTION(TopHoloSolidBlue, *SB03, ({
    // Top Holo solid blue
    CommandEvent::process(F("HPT0055"));
}))

////////////////

MARCDUINO_ACTION(AllHoloSolidBlue, *SB00, ({
    // All Holos solid blue
    CommandEvent::process(F("HPA0055"));
}))

////////////////
// SOLID COLOR - WHITE (color 9)
// Sets all LEDs to a fixed white. Useful for alert or scanning effects.
////////////////

MARCDUINO_ACTION(FrontHoloSolidWhite, *SW01, ({
    // Front Holo solid white
    CommandEvent::process(F("HPF0059"));
}))

////////////////

MARCDUINO_ACTION(RearHoloSolidWhite, *SW02, ({
    // Rear Holo solid white
    CommandEvent::process(F("HPR0059"));
}))

////////////////

MARCDUINO_ACTION(TopHoloSolidWhite, *SW03, ({
    // Top Holo solid white
    CommandEvent::process(F("HPT0059"));
}))

////////////////

MARCDUINO_ACTION(AllHoloSolidWhite, *SW00, ({
    // All Holos solid white
    CommandEvent::process(F("HPA0059"));
}))

////////////////
// HOLO MODE LOOP
// Enables the autonomous random LED mode loop on all holos simultaneously.
// Uses the S9 major sequence which:
//   - Enables random HP servo twitching
//   - Enables random LED sequence cycling, randomly choosing from:
//       ColorProjector (2), DimPulse (3), Cycle (4), SolidColor (5), Rainbow (6)
//   ShortCircuit (7) is intentionally excluded from the random loop.
// Each sequence runs for a random duration before switching to the next.
////////////////

MARCDUINO_ACTION(AllHoloModeLoop, *ML00, ({
    // All Holos: start autonomous random mode loop (excludes ShortCircuit)
    CommandEvent::process(F("HPS9"));
}))

////////////////
// RESET TO MODE LOOP
// Resets all holos and returns to the random mode loop instead of the normal
// AstroPixels off state. Use *ST00 (ResetAllHolos) to return to normal off state.
////////////////

MARCDUINO_ACTION(AllHoloResetToLoop, *RL00, ({
    // Reset all Holos to the random mode loop (not to normal off state)
    CommandEvent::process(F("HPS9"));
}))

////////////////
