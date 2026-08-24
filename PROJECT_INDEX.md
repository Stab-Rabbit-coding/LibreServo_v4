# PROJECT_INDEX — LibreServo v4.0.0

Directory and file index of the **active** project, per the repository
governance in [`AGENTS.md`](AGENTS.md). Regenerate whenever files are added.

Archived files, if any, belong in `ARCHIVE_INDEX.md` — not here.

`PCB/datasheets/Infineon-SLB9672-TPM2.0-SPI-FW16.xx-datasheet.pdf` was removed
2026-08-23 with the SLB9672 itself; its `REFERENCES.md` entry **[2]** is
retained for design history per `AGENTS.md` §2.5.

Generated 2026-08-10 (124 paths, 21 directories); incrementally updated
2026-08-22 to add `PCB/servo-bus-security-protocol.md`, and 2026-08-23 to add
the `firmware/` tree, four Infineon documents in `PCB/datasheets/`, and
`.gitignore` (not a full regeneration/recount of the rest of the index).

`firmware/external/` is intentionally absent: it holds the upstream OPTIGA™
Trust M Host Library, fetched by `firmware/tools/fetch_optiga_host_library.sh`
at a pinned commit and excluded by `.gitignore`. It is a dependency, not a
project file.

---

## `(repo root)`

- `.cproject`
- `.gitignore`
- `.markdownlint-cli2.jsonc`
- `.mxproject`
- `.project`
- `AGENTS.md`
- `CLAUDE.md`
- `PROJECT_INDEX.md`
- `README.md`
- `REFERENCES.md`
- `STM32F302K8UX_FLASH.ld`
- `TODO.md`
- `Test_LibreServo_v2.ioc`
- `plus`
- `v2.0.zip`
- `v2.1.zip`
- `v2.2.zip`
- `v2.3.zip`

## `.vscode/`

- `extensions.json`

## `3D/`

- `LS_body.stl`
- `LS_shaft_a.stl`
- `LS_shaft_b.stl`
- `README.md`

## `Encoder/`

- `Main_Enc_6Layers.zip`
- `ReadMe.md`

## `Inc/`

- `LS_PID.h`
- `LS_boolean.h`
- `LS_curvas_motor.h`
- `LS_flash.h`
- `LS_funciones.h`
- `eeprom.h`
- `main.h`
- `stm32_assert.h`
- `stm32f30x_flash.h`
- `stm32f3xx_it.h`

## `PCB/`

- `LibreServo-v2.3.1.brd`
- `LibreServo-v2.3.1.sch`
- `LibreServo-v2.3_BOM.txt`
- `MSPM0G3507-MCU-swap.md`
- `MSPM0G3518-MCU-swap.md`
- `OPTIGA-Trust-M-secure-element.md`
- `RS485-CANFD-TPM-upgrade.md`
- `ReadMe.md`
- `S32K144-MCU-swap.md`
- `servo-bus-security-protocol.md`

## `PCB/Gerbers/`

- `LibreServo`
- `ReadMe.md`

## `PCB/Gerbers/old/`

- `LibreServo`

## `PCB/datasheets/`

- `ADM2582E-ADM2587E.pdf`
- `ADM3055E-ADM3057E.pdf`
- `Infineon_I2C_Protocol_v2.03.pdf`
- `OPTIGA_Trust_M_ConfigGuide_v2.2.pdf`
- `OPTIGA_Trust_M_Keys_And_Certificates_v3.10.pdf`
- `OPTIGA_Trust_M_Solution_Reference_Manual_v3.70.pdf`
- `S32K-RM.pdf`
- `S32K1xx.pdf`
- `infineon-optiga-trust-m-datasheet-en.pdf`
- `mspm0g3518-q1.pdf`
- `slaae29a.pdf`
- `slaae76e.pdf`
- `slaaet8a.pdf`
- `slau846e.pdf`
- `slaz742g.pdf`

## `PCB/kicad/`

- `LibreServo-v4.0.0-eagle-import.kicad_sym`
- `LibreServo-v4.0.0.kicad_dru`
- `LibreServo-v4.0.0.kicad_pcb`
- `LibreServo-v4.0.0.kicad_prl`
- `LibreServo-v4.0.0.kicad_pro`
- `LibreServo-v4.0.0.kicad_sch`
- `LibreServo-v4.0.0.kicad_sym`
- `fp-info-cache`
- `fp-lib-table`
- `sym-lib-table`

## `PCB/kicad/LibreServo-v2.3.1-backups/`

- `LibreServo-v2.3.1-2026-08-03_143618.zip`
- `LibreServo-v2.3.1-2026-08-03_221348.zip`
- `LibreServo-v2.3.1-2026-08-04_070800.zip`

## `PCB/kicad/LibreServo-v4.0.0.pretty/`

- `0402.kicad_mod`
- `0402_MIN.kicad_mod`
- `0603.kicad_mod`
- `1206.kicad_mod`
- `ACS711-QFN.kicad_mod`
- `DFN5X6.kicad_mod`
- `E2,5-6_MIN.kicad_mod`
- `FFC-5-0.5.kicad_mod`
- `HEADER_4_1.27_0.3.kicad_mod`
- `Infineon_PG-USON-10-2-4_3x3mm_P0.5mm_EP1.7x2.5mm.kicad_mod`
- `JST_PH-4_HOLE_B.kicad_mod`
- `MLP8.kicad_mod`
- `MPM36XX.kicad_mod`
- `MSOP-8.kicad_mod`
- `OSC-2X1.6.kicad_mod`
- `PLCC-4_2.2X2.kicad_mod`
- `POTEN_SERVO.kicad_mod`
- `QFN24.kicad_mod`
- `QFN32.kicad_mod`
- `SERVO_SLOT.kicad_mod`
- `SOT23.kicad_mod`
- `SOT25.kicad_mod`

## `PCB/kicad/tools/`

- `swap_slb9672_for_optiga.py`

## `PCB/old/`

- `LibreServo-v2.1.brd`
- `LibreServo-v2.1.sch`
- `LibreServo-v2.2.brd`
- `LibreServo-v2.2.sch`
- `LibreServo-v2.brd`
- `LibreServo-v2.sch`

## `Src/`

- `LICENSE.txt`
- `LS_PID.c`
- `LS_curvas_motor.c`
- `LS_flash.c`
- `LS_funciones.c`
- `eeprom.c`
- `main.c`
- `stm32f30x_flash.c`
- `stm32f3xx_it.c`
- `syscalls.c`
- `sysmem.c`
- `system_stm32f3xx.c`

## `firmware/`

- `CLAUDE.md`
- `README.md`

## `firmware/config/`

- `ls_optiga_lib_config.h`

## `firmware/pal/`

- `ls_board.h`
- `ls_crypto_backend.h`
- `ls_pal.c`
- `ls_pal_crypt.c`
- `ls_pal_gpio.c`
- `ls_pal_i2c.c`
- `ls_pal_ifx_i2c_config.c`
- `ls_pal_logger.c`
- `ls_pal_os_datastore.c`
- `ls_pal_os_event.c`
- `ls_pal_os_lock.c`
- `ls_pal_os_memory.c`
- `ls_pal_os_timer.c`
- `ls_secure_store.h`

## `firmware/pal/mspm0/`

- `ls_mspm0_i2c_regs.h`

## `firmware/tests/`

- `README.md`
- `generate_vectors.py`
- `ls_crypto_backend_openssl.c`
- `test_pal_crypt_ccm.c`
- `test_pal_crypt_prf.c`

## `firmware/tools/`

- `fetch_optiga_host_library.sh`

## `firmware/trust/`

- `ls_trust.c`
- `ls_trust.h`
- `ls_trust_internal.h`
- `ls_trust_oid.h`
- `ls_trust_pairing.c`

## `v2.3.1/`

- `Main`
- `ReadMe.md`

## `v2.3.1/Encoder/`

- `Enc_6Layers.zip`
- `ReadMe.md`

## `v2.3.1/Main/`

- `Main_6Layers.zip`
- `ReadMe.md`

## `v2.3.1/Main_Bottom/`

- `Main_Enc_6Layers_bottom.zip`
- `ReadMe.md`

## `v2.3.1/Main_Left/`

- `L_6Layers.zip`
- `ReadMe.md`

## `v2.3.1/Main_Right/`

- `R_6Layers.zip`
- `ReadMe.md`
