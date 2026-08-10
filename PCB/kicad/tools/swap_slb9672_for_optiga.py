#!/usr/bin/env python3
"""Replace the Infineon SLB9672 TPM 2.0 (U7) with an Infineon OPTIGA(TM)
Trust M V3 secure element in LibreServo-v4.0.0.kicad_sch.

This is NOT a drop-in substitution and must not be read as one. The SLB9672 is
a 32-pin SPI TPM; the Trust M is a 10-pin I2C secure element. They are
different device classes: a TPM is a platform-integrity module built around
PCRs and attestation, a secure element is a key vault with a crypto engine.
The servo needs an independent hardware root of trust, not a TPM stack, so the
SPI interface and all six of its TPM-specific nets are removed and replaced
with an I2C interface.

What this script does:
  1. Deletes the SLB9672_TPM embedded lib_symbol and the U7 instance.
  2. Deletes every wire with an endpoint on a former U7 pin (16 wires).
  3. Deletes the six TPM-side label stubs (TPM_CS#, TPM_PIRQ#, TPM_RST#,
     SPI_CLK, SPI_IN, SPI_OUT). The SPI nets themselves survive -- they are
     shared with other devices; only the TPM's tap on them is removed.
  4. Renames the surviving MCU-side labels so the sheet stops claiming a TPM
     exists: TPM_RST# -> SE_RST (kept, and genuinely reconnected to the new
     part), TPM_CS# -> SPARE_PA2, TPM_PIRQ# -> SPARE_PIRQ (both now free).
  5. Adds the OPTIGA_TRUST_M lib_symbol, the U7 instance, two 10k I2C
     pull-ups, one 100 nF decoupling cap, no-connect flags on the five NC
     contacts, and the SE_I2C_SDA / SE_I2C_SCL / SE_RST nets.

Circuit source: REFERENCES.md [45] p.12 Sec.3 Figure 2 (pull-ups + decoupling),
p.17 Table 6 (pin map, and the requirement that NC contacts be left floating).

KNOWN INCOMPLETE, deliberately: SE_I2C_SDA and SE_I2C_SCL are NOT attached to
MCU pins. Choosing those pins requires the MSPM0G351x-Q1 pin-multiplexing
table (Table 6-2 of the datasheet), whose column structure does not survive
PDF text extraction, so no I2C-capable pin pair could be confirmed. Assigning
them by guess would be a fabricated pin claim. SE_RST is attached, because it
needs only a plain GPIO and the freed PA14 is verified to be one. See TODO.md.
"""

import re
import sys
import uuid
from pathlib import Path

SCH = Path(__file__).resolve().parent.parent / "LibreServo-v4.0.0.kicad_sch"
OPTIGA_LIB = Path(__file__).resolve().parent.parent / "LibreServo-v4.0.0.kicad_sym"

T = "\t"
U7X, U7Y = 120.65, 410.21


def uid() -> str:
    return str(uuid.uuid4())


def close_paren(s: str, open_idx: int) -> int:
    """Index of the ')' matching the '(' at open_idx, skipping quoted strings."""
    depth, i, instr, esc = 0, open_idx, False, False
    while i < len(s):
        ch = s[i]
        if esc:
            esc = False
        elif ch == "\\" and instr:
            esc = True
        elif ch == '"':
            instr = not instr
        elif not instr:
            if ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
                if depth == 0:
                    return i
        i += 1
    sys.exit("unbalanced S-expression")


def drop_block(text: str, marker: str) -> str:
    """Remove the whole top-level node containing `marker`, plus its newline."""
    idx = text.index(marker)
    start = text.rindex("\n\t(", 0, idx) + 1
    end = close_paren(text, text.index("(", start))
    return text[:start] + text[end + 2 :]


def eff(ind: int, extra: str = "") -> str:
    i = T * ind
    return (
        f"{i}(effects\n{i}{T}(font\n{i}{T}{T}(size 1.27 1.27)\n{i}{T})\n"
        + (f"{i}{T}{extra}\n" if extra else "")
        + f"{i})\n"
    )


def prop(ind: int, key: str, val: str, x: float, y: float, hide: bool = False) -> str:
    i = T * ind
    return (
        f'{i}(property "{key}" "{val}"\n{i}{T}(at {x} {y} 0)\n'
        + eff(ind + 1, "(hide yes)" if hide else "")
        + f"{i})\n"
    )


def wire(x1, y1, x2, y2) -> str:
    return (
        f"{T}(wire\n{T}{T}(pts\n{T}{T}{T}(xy {x1} {y1}) (xy {x2} {y2})\n{T}{T})\n"
        f"{T}{T}(stroke\n{T}{T}{T}(width 0.1524)\n{T}{T}{T}(type solid)\n{T}{T})\n"
        f'{T}{T}(uuid "{uid()}")\n{T})\n'
    )


def label(name, x, y) -> str:
    return (
        f'{T}(label "{name}"\n{T}{T}(at {x} {y} 0)\n'
        f"{T}{T}(effects\n{T}{T}{T}(font\n{T}{T}{T}{T}(size 1.5291 1.5291)\n{T}{T}{T})\n"
        f"{T}{T}{T}(justify right bottom)\n{T}{T})\n"
        f'{T}{T}(uuid "{uid()}")\n{T})\n'
    )


def noconn(x, y) -> str:
    return f'{T}(no_connect\n{T}{T}(at {x} {y})\n{T}{T}(uuid "{uid()}")\n{T})\n'


def instance(lib_id, x, y, ref, value, footprint, pin_nums, ref_dy, val_dy,
             extra_props=None) -> str:
    out = f"{T}(symbol\n{T}{T}(lib_id \"{lib_id}\")\n{T}{T}(at {x} {y} 0)\n{T}{T}(unit 1)\n"
    out += f"{T}{T}(exclude_from_sim no)\n{T}{T}(in_bom yes)\n{T}{T}(on_board yes)\n{T}{T}(dnp no)\n"
    out += f'{T}{T}(uuid "{uid()}")\n'
    out += prop(2, "Reference", ref, x, round(y + ref_dy, 2))
    out += prop(2, "Value", value, x, round(y + val_dy, 2))
    out += prop(2, "Footprint", footprint, x, y, hide=True)
    for k, v in (extra_props or {}).items():
        out += prop(2, k, v, x, y, hide=True)
    for n in pin_nums:
        out += f'{T}{T}(pin "{n}"\n{T}{T}{T}(uuid "{uid()}")\n{T}{T})\n'
    out += (
        f'{T}{T}(instances\n{T}{T}{T}(project ""\n{T}{T}{T}{T}(path "/{ROOT_UUID}"\n'
        f'{T}{T}{T}{T}{T}(reference "{ref}")\n{T}{T}{T}{T}{T}(unit 1)\n'
        f"{T}{T}{T}{T})\n{T}{T}{T})\n{T}{T})\n{T})\n"
    )
    return out


def build_optiga_lib_symbol(src: str) -> str:
    props = dict(re.findall(r'\(property "([^"]+)" "((?:[^"\\]|\\.)*)"', src))
    pins = re.findall(
        r'\(pin (\w+) line \(at ([-\d.]+) ([-\d.]+) (\d+)\) \(length ([\d.]+)\)\s*'
        r'\(name "([^"]+)"[\s\S]*?\(number "([^"]+)"',
        src,
    )
    if len(pins) != 10:
        sys.exit(f"expected 10 OPTIGA pins, parsed {len(pins)}")
    i2 = T * 2
    out = f'{i2}(symbol "LibreServo-v4.0.0:OPTIGA_TRUST_M"\n'
    out += f"{i2}{T}(pin_names\n{i2}{T}{T}(offset 1.016)\n{i2}{T})\n"
    out += f"{i2}{T}(exclude_from_sim no)\n{i2}{T}(in_bom yes)\n{i2}{T}(on_board yes)\n"
    out += prop(3, "Reference", "U", 0, 15.24)
    out += prop(3, "Value", "OPTIGA_TRUST_M", 0, 12.7)
    for key in ("Footprint", "Datasheet", "Description", "Citation", "Verification"):
        out += prop(3, key, props.get(key, ""), 0, 0, hide=True)
    out += (
        f'{i2}{T}(symbol "OPTIGA_TRUST_M_0_1"\n{i2}{T}{T}(rectangle\n'
        f"{i2}{T}{T}{T}(start -5.08 10.16)\n{i2}{T}{T}{T}(end 5.08 -10.16)\n"
        f"{i2}{T}{T}{T}(stroke\n{i2}{T}{T}{T}{T}(width 0)\n{i2}{T}{T}{T}{T}(type default)\n{i2}{T}{T}{T})\n"
        f"{i2}{T}{T}{T}(fill\n{i2}{T}{T}{T}{T}(type none)\n{i2}{T}{T}{T})\n{i2}{T}{T})\n{i2}{T})\n"
    )
    out += f'{i2}{T}(symbol "OPTIGA_TRUST_M_1_1"\n'
    for etype, px, py, rot, length, name, num in pins:
        out += (
            f"{i2}{T}{T}(pin {etype} line\n{i2}{T}{T}{T}(at {px} {py} {rot})\n"
            f"{i2}{T}{T}{T}(length {length})\n"
            f'{i2}{T}{T}{T}(name "{name}"\n' + eff(6) + f"{i2}{T}{T}{T})\n"
            f'{i2}{T}{T}{T}(number "{num}"\n' + eff(6) + f"{i2}{T}{T}{T})\n{i2}{T}{T})\n"
        )
    out += f"{i2}{T})\n{i2})\n"
    return out


# ---------------------------------------------------------------------------
text = SCH.read_text()
ROOT_UUID = re.search(r'\(path "/([0-9a-f-]+)"', text).group(1)
if "OPTIGA_TRUST_M" in text:
    sys.exit("OPTIGA already present -- refusing to double-apply")

# --- 1. former U7 pin coordinates, used to find the wires to delete ---------
old_pins = set()
for py in (30.48, 27.94, 25.4, 22.86, 20.32, 17.78, 15.24, 12.7):
    old_pins.add((round(U7X - 26.67, 2), round(U7Y - py, 2)))
for py in (30.48, 27.94, 25.4, 22.86, 20.32, 17.78, 15.24, 12.7, 10.16, 7.62,
           5.08, 2.54, 0.0, -2.54, -5.08, -7.62, -10.16, -12.7, -15.24, -17.78,
           -20.32, -22.86, -25.4, -27.94, -30.48):
    old_pins.add((round(U7X + 26.67, 2), round(U7Y - py, 2)))

removed_wires = 0
out, pos = [], 0
for m in re.finditer(r"\n\t\(wire\n", text):
    start = m.start() + 1
    end = close_paren(text, text.index("(", start))
    body = text[start : end + 1]
    pts = re.search(r"\(xy ([-\d.]+) ([-\d.]+)\) \(xy ([-\d.]+) ([-\d.]+)\)", body)
    a = (round(float(pts.group(1)), 2), round(float(pts.group(2)), 2))
    b = (round(float(pts.group(3)), 2), round(float(pts.group(4)), 2))
    if a in old_pins or b in old_pins:
        out.append(text[pos:start])
        pos = end + 2
        removed_wires += 1
text = "".join(out) + text[pos:]

# --- 2. drop the TPM-side label stubs (x = 76.2, the column beside U7) ------
removed_labels = 0
for name in ("TPM_CS#", "SPI_CLK", "SPI_OUT", "SPI_IN", "TPM_PIRQ#", "TPM_RST#"):
    while True:
        m = re.search(
            r'\n\t\(label "' + re.escape(name) + r'"\n\t\t\(at 76\.2 ', text
        )
        if not m:
            break
        start = m.start() + 1
        end = close_paren(text, text.index("(", start))
        text = text[:start] + text[end + 2 :]
        removed_labels += 1

# --- 3. rename the surviving MCU-side labels -------------------------------
renames = {"TPM_RST#": "SE_RST", "TPM_CS#": "SPARE_PA2", "TPM_PIRQ#": "SPARE_PIRQ"}
for old, new in renames.items():
    text = text.replace(f'(label "{old}"', f'(label "{new}"')

# --- 4. drop the SLB9672 symbol and instance -------------------------------
text = drop_block(text, '(lib_id "LibreServo-v4.0.0-eagle-import:SLB9672_TPM")')
sym_start = text.index('\t\t(symbol "LibreServo-v4.0.0-eagle-import:SLB9672_TPM"')
sym_end = close_paren(text, text.index("(", sym_start))
text = text[:sym_start] + text[sym_end + 2 :]

# --- 5. add the OPTIGA lib_symbol ------------------------------------------
anchor = '\t\t(symbol "LibreServo-v4.0.0-eagle-import:+3V3"\n'
text = text.replace(anchor, build_optiga_lib_symbol(OPTIGA_LIB.read_text()) + anchor, 1)

# --- 6. place the new parts ------------------------------------------------
FP_R = "LibreServo-v4.0.0:0402"
optiga_props = dict(
    re.findall(r'\(property "([^"]+)" "((?:[^"\\]|\\.)*)"', OPTIGA_LIB.read_text())
)
add = instance(
    "LibreServo-v4.0.0:OPTIGA_TRUST_M", U7X, U7Y, "U7", "OPTIGA_TRUST_M",
    optiga_props.get("Footprint", ""), [str(n) for n in range(1, 11)], -15.24, -12.7,
    extra_props={k: optiga_props.get(k, "") for k in
                 ("Datasheet", "Description", "Citation", "Verification")},
)
# two 10k I2C pull-ups (horizontal), +3V3 on the left, the I2C net on the right
for ref, y in (("R30", 407.67), ("R31", 410.21)):
    add += instance("LibreServo-v4.0.0-eagle-import:R-US0402", 90.17, y, ref,
                    "10k", FP_R, ["1", "2"], -5.08, -2.54)
# 100 nF VCC decoupling
add += instance("LibreServo-v4.0.0-eagle-import:C-EU0402", 139.7, 402.59, "C43",
                "100n", FP_R, ["1", "2"], -7.62, -5.08)

SDA_Y, SCL_Y, RST_Y = 407.67, 410.21, 412.75
add += wire(95.25, SDA_Y, 113.03, SDA_Y) + label("SE_I2C_SDA", 104.14, SDA_Y)
add += wire(95.25, SCL_Y, 113.03, SCL_Y) + label("SE_I2C_SCL", 104.14, SCL_Y)
add += wire(95.25, RST_Y, 113.03, RST_Y) + label("SE_RST", 95.25, RST_Y)
add += wire(85.09, SDA_Y, 80.01, SDA_Y) + label("+3V3", 80.01, SDA_Y)
add += wire(85.09, SCL_Y, 80.01, SCL_Y) + label("+3V3", 80.01, SCL_Y)
add += wire(U7X, 397.51, U7X, 392.43) + label("+3V3", U7X, 392.43)
add += wire(U7X, 422.91, U7X, 427.99) + label("GND", U7X, 427.99)
add += wire(139.7, 397.51, 139.7, 392.43) + label("+3V3", 139.7, 392.43)
add += wire(139.7, 405.13, 139.7, 417.83) + label("GND", 139.7, 417.83)
for ny in (405.13, 407.67, 410.21, 412.75, 415.29):
    add += noconn(128.27, ny)

cut = text.rstrip().rfind("\n)")
text = text[:cut] + "\n" + add.rstrip("\n") + text[cut:]
SCH.write_text(text)
print(f"removed {removed_wires} wires, {removed_labels} TPM-side labels")
print("SLB9672 TPM -> OPTIGA Trust M swap applied (U7)")
