# <img src="https://www.libreservo.com/sites/libreservo.com/files/imagenes/LibreServo_logo_xs.png">

An Open source controller to convert any servo motor to the best smart servo.

This fork is focused improving the LibreServo design by moving to an TI MSPM0G3518-Q1 MCU, which provides simultaneous CAN-FD and RS485, and a robust cryptographic suite, along with an Infineon OPTIGA™ Trust M secure element as an independent hardware root of trust.

**Version 4.0.0.** Upstream LibreServo shipped v2.3.1 and had a v3 in mind. The changes in this fork — new MCU family, a hardware root of trust, CAN-FD alongside RS-485, an isolated transceiver pair, and a hardware security stack — go past what that v3 was scoped to be, so this fork takes the next major number rather than a v3.x that would collide with it.

**EDA tooling:** this fork has moved to **KiCad**. Autodesk EAGLE is end-of-life and no longer supported, so all new schematic and layout work is done in KiCad 9 under [`PCB/kicad/`](PCB/kicad/) as `LibreServo-v4.0.0`. The upstream EAGLE `.sch`/`.brd` files are kept in [`PCB/`](PCB/) for backward compatibility and design history; they stay frozen at v2.3.1 and are not renumbered. See [`PCB/ReadMe.md`](PCB/ReadMe.md).

This project was born as a necessity of mine to build a biped robot with intelligent servos to be able to “feel the muscles” to walk more human like. In the past we had OpenServo, and I think this project inherits something from it, but OpenServo died many years ago and it didn't reach my expectations anyway (power, communications...), and the commercial alternatives are way too expensive (Dynamixel, Herkulex, Lynxmotion...).

My goal with LibreServo is to make any standard servo the “smartest” one in the market. The "gold standard" nowadays is Robotis-Dynamixel, LibreServo should be better than that, that’s my goal at least.

A few characteristics of LibreServo:

    Compatible with standard servo motors (No need to change the bottom cover of them!)
    Voltage: From 4.5V up to 18V (Recommended: 5-14V) — UNVERIFIED — needs primary source (see TODO.md): board-level voltage rating with no cited component (regulator/MOSFET Vds/etc.) derivation on file.
    Communications: Isolated RS-485 (ADM2587E [47], 500 kbps) and isolated CAN-FD (ADM3055E [48], up to 12 Mbps demonstrated, ISO 11898-2:2016 rated to 5 Mbps). Daisy chained. CRC-16. (Corrected 2026-08-22 from an inherited, unsourced "Max Speed 9Mbps" claim that didn't match either transceiver's datasheet — see `TODO.md` 1.6.)
    Amp: Up to 16A continuous (WSD3069DN56) (Version >2.3) — UNVERIFIED — needs primary source (see TODO.md): no local datasheet for WSD3069DN56 has been intaken against this figure.
    Micro-Controller (this fork): TI MSPM0G3518-Q1 (cortex-M0+@80MHz, 256KB flash, 128KB SRAM, CAN-FD + 5 UART, AES-256 with CMAC/GCM, key store, CSC secure boot). Upstream uses an STM32F301k8 (cortex-M4@72MHz).
    Hardware root of trust: Infineon OPTIGA™ Trust M V3 secure element (I²C, `U7`) — device identity (ECDSA over a fab-provisioned key + X.509 certificate) and ephemeral session-key agreement (ECDHE). This is a **secure element, not a TPM**: the SLB9672 TPM 2.0 that previously occupied `U7` was removed 2026-08-10. A servo needs a key vault, not a platform-attestation stack. See [`PCB/OPTIGA-Trust-M-secure-element.md`](PCB/OPTIGA-Trust-M-secure-element.md).
    Position sensor: Magnetic encoder, 16 bits of resolution! 360 degrees (AEAT-8800). Using the servo motor potentiometer will be possible to lower the cost but will lost precision and some characteristics. — UNVERIFIED — needs primary source (see TODO.md): no local datasheet for AEAT-8800 has been intaken against this figure.
    For the encoder I have designed 3D parts to substitute the potentiometer and used the same hole/space than the original.
    LibreServo will generate their own curves (sine ramps, trapezoidal ramps, hermitian curves...)
    Current sensor: +-15A ACS711 — UNVERIFIED — needs primary source (see TODO.md): no local datasheet for ACS711 has been intaken against this figure.
Communication Protocol: <a href="https://www.libreservo.com/en/articulo/libreservo-commands-part-one">LibreServo Commands</a><BR>
<img src="https://www.libreservo.com/sites/libreservo.com/files/imagenes/Main-Encoder-PCB.jpg" width="550" height="412">

More info in <a href="https://www.libreservo.com/en">LibreServo</a>.

<p align="center">
<a href="http://creativecommons.org/licenses/by-sa/4.0/"><img src="https://user-images.githubusercontent.com/12425566/219942224-9a3bc76d-cd19-4feb-99f2-25e79919dd3e.png" alt="cc-by-sa" width="200px" height="69px"></a></p>
