#!/usr/bin/env python3
"""Generate Ruqoom-owned Pyramid Arabic, a compact geometric TrueType font.

The source outlines are original Pyramid/Ruqoom work.  They deliberately avoid
third-party font data while providing a readable, connected Arabic fallback for
engine UI, diagnostics, and deterministic tests.  Contextual Arabic forms live
in Unicode Presentation Forms-B; Pyramid::Text selects the correct form before
font lookup.
"""
from __future__ import annotations

import math
import runpy
import struct
from pathlib import Path
from typing import Iterable

# (base, isolated, final, initial, medial, dot_count, dots_below, body_variant)
FORMS = [
    (0x0621,0xFE80,0,0,0,0,False,0),(0x0622,0xFE81,0xFE82,0,0,0,False,3),
    (0x0623,0xFE83,0xFE84,0,0,1,False,3),(0x0624,0xFE85,0xFE86,0,0,1,False,4),
    (0x0625,0xFE87,0xFE88,0,0,1,True,3),(0x0626,0xFE89,0xFE8A,0xFE8B,0xFE8C,1,False,5),
    (0x0627,0xFE8D,0xFE8E,0,0,0,False,3),(0x0628,0xFE8F,0xFE90,0xFE91,0xFE92,1,True,0),
    (0x0629,0xFE93,0xFE94,0,0,2,False,4),(0x062A,0xFE95,0xFE96,0xFE97,0xFE98,2,False,0),
    (0x062B,0xFE99,0xFE9A,0xFE9B,0xFE9C,3,False,0),(0x062C,0xFE9D,0xFE9E,0xFE9F,0xFEA0,1,True,1),
    (0x062D,0xFEA1,0xFEA2,0xFEA3,0xFEA4,0,False,1),(0x062E,0xFEA5,0xFEA6,0xFEA7,0xFEA8,1,False,1),
    (0x062F,0xFEA9,0xFEAA,0,0,0,False,2),(0x0630,0xFEAB,0xFEAC,0,0,1,False,2),
    (0x0631,0xFEAD,0xFEAE,0,0,0,False,2),(0x0632,0xFEAF,0xFEB0,0,0,1,False,2),
    (0x0633,0xFEB1,0xFEB2,0xFEB3,0xFEB4,0,False,6),(0x0634,0xFEB5,0xFEB6,0xFEB7,0xFEB8,3,False,6),
    (0x0635,0xFEB9,0xFEBA,0xFEBB,0xFEBC,0,False,7),(0x0636,0xFEBD,0xFEBE,0xFEBF,0xFEC0,1,False,7),
    (0x0637,0xFEC1,0xFEC2,0xFEC3,0xFEC4,0,False,8),(0x0638,0xFEC5,0xFEC6,0xFEC7,0xFEC8,1,False,8),
    (0x0639,0xFEC9,0xFECA,0xFECB,0xFECC,0,False,9),(0x063A,0xFECD,0xFECE,0xFECF,0xFED0,1,False,9),
    (0x0641,0xFED1,0xFED2,0xFED3,0xFED4,1,False,10),(0x0642,0xFED5,0xFED6,0xFED7,0xFED8,2,False,10),
    (0x0643,0xFED9,0xFEDA,0xFEDB,0xFEDC,0,False,11),(0x0644,0xFEDD,0xFEDE,0xFEDF,0xFEE0,0,False,3),
    (0x0645,0xFEE1,0xFEE2,0xFEE3,0xFEE4,0,False,4),(0x0646,0xFEE5,0xFEE6,0xFEE7,0xFEE8,1,False,0),
    (0x0647,0xFEE9,0xFEEA,0xFEEB,0xFEEC,0,False,12),(0x0648,0xFEED,0xFEEE,0,0,0,False,4),
    (0x0649,0xFEEF,0xFEF0,0,0,0,False,5),(0x064A,0xFEF1,0xFEF2,0xFEF3,0xFEF4,2,True,5),
]

FORM_INFO: dict[int, tuple[int, str, int, bool, int]] = {}
BASE_INFO: dict[int, tuple[int, int, bool, int]] = {}
for base, isolated, final, initial, medial, dots, below, variant in FORMS:
    BASE_INFO[base] = (dots, variant, below, isolated)
    for codepoint, form in ((isolated,"isolated"),(final,"final"),(initial,"initial"),(medial,"medial")):
        if codepoint:
            FORM_INFO[codepoint] = (base, form, dots, below, variant)

ADVANCE = 620
JOIN_Y = 310
STROKE = 72
ASCENT = 860
DESCENT = -180

_LATIN_GENERATOR = runpy.run_path(str(Path(__file__).with_name("generate-pyramid-font.py")))
_LATIN_GLYPH = _LATIN_GENERATOR["glyph"]

Point = tuple[int, int]
Contour = list[Point]


def clockwise(points: Iterable[Point]) -> Contour:
    result = list(points)
    area = sum(
        result[i][0] * result[(i + 1) % len(result)][1] -
        result[(i + 1) % len(result)][0] * result[i][1]
        for i in range(len(result)))
    if area > 0:
        result.reverse()
    return result


def rect(x0: float, y0: float, x1: float, y1: float) -> Contour:
    return clockwise([
        (round(x0), round(y0)),
        (round(x0), round(y1)),
        (round(x1), round(y1)),
        (round(x1), round(y0)),
    ])


def ellipse(cx: float, cy: float, rx: float, ry: float, segments: int = 20) -> Contour:
    points = []
    for index in range(segments):
        angle = -2.0 * math.pi * index / segments
        points.append((round(cx + math.cos(angle) * rx), round(cy + math.sin(angle) * ry)))
    return clockwise(points)


def ring(cx: float, cy: float, rx: float, ry: float, thickness: float) -> list[Contour]:
    outer = ellipse(cx, cy, rx, ry)
    inner = ellipse(cx, cy, max(1.0, rx - thickness), max(1.0, ry - thickness))
    inner.reverse()
    return [outer, inner]


def segment(a: Point, b: Point, thickness: float = STROKE) -> Contour:
    ax, ay = a
    bx, by = b
    dx = bx - ax
    dy = by - ay
    length = math.hypot(dx, dy)
    if length == 0:
        return ellipse(ax, ay, thickness * 0.5, thickness * 0.5, 12)
    px = -dy / length * thickness * 0.5
    py = dx / length * thickness * 0.5
    return clockwise([
        (round(ax + px), round(ay + py)),
        (round(bx + px), round(by + py)),
        (round(bx - px), round(by - py)),
        (round(ax - px), round(ay - py)),
    ])


def polyline(points: list[Point], thickness: float = STROKE, rounded: bool = True) -> list[Contour]:
    contours: list[Contour] = []
    for left, right in zip(points, points[1:]):
        contours.append(segment(left, right, thickness))
    if rounded:
        radius = thickness * 0.5
        for x, y in points:
            contours.append(ellipse(x, y, radius, radius, 12))
    return contours


def connectors(form: str) -> list[Contour]:
    contours: list[Contour] = []
    if form in ("initial", "medial"):
        contours.append(rect(0, JOIN_Y - STROKE / 2, 220, JOIN_Y + STROKE / 2))
    if form in ("final", "medial"):
        contours.append(rect(400, JOIN_Y - STROKE / 2, ADVANCE, JOIN_Y + STROKE / 2))
    return contours


def dot(cx: int, cy: int, radius: int = 34) -> list[Contour]:
    # A soft diamond remains legible after small-size antialiasing.
    return [clockwise([(cx, cy + radius), (cx + radius, cy),
                       (cx, cy - radius), (cx - radius, cy)])]


def dots(count: int, below: bool) -> list[Contour]:
    if count == 0:
        return []
    y = 95 if below else 715
    if count == 1:
        positions = [(350, y)]
    elif count == 2:
        positions = [(295, y), (405, y)]
    else:
        positions = [(290, y - 6), (410, y - 6), (350, y + 82)]
    result: list[Contour] = []
    for x, py in positions:
        result.extend(dot(x, py))
    return result


def hamza(above: bool = True, x: int = 360) -> list[Contour]:
    y = 725 if above else 95
    return polyline([(x + 55, y + 30), (x, y + 55), (x - 38, y + 15),
                     (x + 8, y - 12), (x - 45, y - 45)], 40)


def madda() -> list[Contour]:
    return polyline([(270, 760), (325, 790), (380, 760), (435, 790)], 34)


def body_variant(base: int, form: str, variant: int) -> list[Contour]:
    c = connectors(form)
    isolated_or_final = form in ("isolated", "final")

    if base == 0x0621:  # standalone hamza
        return hamza(True, 330)

    if base in (0x0622, 0x0623, 0x0625, 0x0627):  # alef family
        c += polyline([(472, JOIN_Y), (472, 770)], 76)
        if base == 0x0622:
            c += madda()
        elif base == 0x0623:
            c += hamza(True, 420)
        elif base == 0x0625:
            c += hamza(False, 420)
        return c

    if base == 0x0644:  # lam
        c += polyline([(485, 770), (485, JOIN_Y), (430, 235)], 76)
        if isolated_or_final:
            c += polyline([(430, 235), (330, 175), (195, 195), (125, 260)], 76)
        else:
            c += polyline([(430, 235), (260, JOIN_Y), (160, JOIN_Y)], 76)
        return c

    if base == 0x0645:  # meem
        c += ring(405, 365, 118, 105, 58)
        c += polyline([(320, 315), (230, 250)], 72)
        if isolated_or_final:
            c += polyline([(230, 250), (185, 120)], 64)
        else:
            c += polyline([(230, 250), (140, JOIN_Y)], 64)
        return c

    if base == 0x0648:  # waw
        c += ring(410, 405, 110, 94, 56)
        c += polyline([(342, 350), (300, 245), (220, 170)], 68)
        return c

    if base == 0x0629:  # taa marbuta
        c += ring(365, 340, 170, 125, 60)
        c += polyline([(195, 330), (145, JOIN_Y)], 70)
        return c

    if base == 0x0647:  # heh
        if form in ("initial", "medial"):
            c += ring(370, 345, 105, 95, 52)
            c += polyline([(285, JOIN_Y), (190, JOIN_Y)], 66)
        else:
            c += ring(360, 365, 150, 130, 55)
            c += ring(360, 365, 72, 58, 30)
        return c

    if variant == 0:  # baa/taa/thaa/noon: shallow bowl
        c += polyline([(115, JOIN_Y), (180, 215), (325, 180),
                       (470, 205), (515, JOIN_Y), (505, 410)], 72)
        if not isolated_or_final:
            c += rect(90, JOIN_Y - 36, 230, JOIN_Y + 36)
    elif variant == 1:  # jeem/haa/khaa: open bowl
        c += ring(355, 310, 175, 145, 62)
        c += rect(170, 300, 360, 470)  # opens the upper-left counter into the joining stroke
        c += polyline([(500, 390), (515, JOIN_Y)], 70)
        if isolated_or_final:
            c += polyline([(265, 190), (205, 105), (115, 95)], 62)
    elif variant == 2:  # dal/thal/raa/zay
        c += polyline([(500, 485), (515, 350), (455, 250),
                       (340, 185), (210, 175)], 74)
    elif variant == 3:  # defensive fallback for tall stem family
        c += polyline([(480, JOIN_Y), (480, 760)], 76)
    elif variant == 4:  # round loop family fallback
        c += ring(390, 360, 125, 108, 56)
        c += polyline([(295, 300), (205, 235)], 68)
    elif variant == 5:  # yeh/alef maksura tail
        c += polyline([(500, 405), (500, JOIN_Y), (430, 225),
                       (295, 165), (145, 185), (90, 250)], 72)
        if not isolated_or_final:
            c += polyline([(275, 205), (175, JOIN_Y), (80, JOIN_Y)], 68)
    elif variant == 6:  # seen/sheen teeth
        c += polyline([(500, JOIN_Y), (470, 430), (410, JOIN_Y),
                       (355, 425), (300, JOIN_Y), (245, 415), (190, JOIN_Y)], 64)
        if isolated_or_final:
            c += polyline([(190, JOIN_Y), (160, 205), (235, 145),
                           (360, 145), (440, 200)], 68)
        else:
            c += polyline([(190, JOIN_Y), (95, JOIN_Y)], 64)
    elif variant == 7:  # sad/dad broad bowl
        c += ring(365, 300, 190, 125, 62)
        c += rect(175, 300, 355, 475)
        c += polyline([(505, JOIN_Y), (500, 455)], 72)
    elif variant == 8:  # tah/zah
        c += ring(350, 300, 180, 115, 58)
        c += polyline([(405, JOIN_Y), (405, 710)], 72)
        c += polyline([(405, 560), (500, 480)], 58)
    elif variant == 9:  # ain/ghain
        c += ring(405, 465, 120, 95, 52)
        c += rect(280, 405, 430, 570)
        c += polyline([(330, 385), (265, 300), (215, 210)], 68)
        if isolated_or_final:
            c += polyline([(215, 210), (135, 145)], 62)
        else:
            c += polyline([(215, 210), (125, JOIN_Y)], 62)
    elif variant == 10:  # faa/qaf
        c += ring(410, 485, 105, 92, 52)
        c += polyline([(330, 430), (300, JOIN_Y), (190, JOIN_Y)], 68)
        if isolated_or_final:
            c += polyline([(190, JOIN_Y), (170, 205), (260, 160),
                           (405, 170), (480, 230)], 68)
    elif variant == 11:  # kaf
        c += polyline([(485, 735), (485, JOIN_Y), (405, 225)], 74)
        c += polyline([(420, 520), (320, 455), (405, 390)], 52)
        if isolated_or_final:
            c += polyline([(405, 225), (260, 170), (130, 210)], 68)
        else:
            c += polyline([(405, 225), (260, JOIN_Y), (120, JOIN_Y)], 68)
    elif variant == 12:  # heh fallback
        c += ring(365, 365, 145, 120, 55)
    return c


def arabic_contours(codepoint: int) -> list[Contour]:
    if codepoint in FORM_INFO:
        base, form, dot_count, below, variant = FORM_INFO[codepoint]
    elif codepoint in BASE_INFO:
        dot_count, variant, below, _ = BASE_INFO[codepoint]
        base, form = codepoint, "isolated"
    else:
        raise KeyError(codepoint)

    contours = body_variant(base, form, variant)
    if base == 0x0623:
        pass  # hamza already authored above the alef
    elif base == 0x0625:
        pass  # hamza already authored below the alef
    elif base == 0x0624:
        contours += hamza(True, 400)
    elif base == 0x0626:
        contours += hamza(True, 390)
    else:
        contours += dots(dot_count, below)
    return contours


DIGIT_STROKES = {
    0x0660: [[(250,250),(250,620)]],
    0x0661: [[(330,210),(330,650)]],
    0x0662: [[(210,580),(300,660),(410,610),(420,500),(300,410),(210,315),(410,210)]],
    0x0663: [[(190,620),(285,530),(360,620),(445,520),(400,380),(220,210)]],
    0x0664: [[(440,650),(220,380),(430,380),(300,210)]],
    0x0665: [[(210,610),(390,610),(435,480),(330,385),(205,440),(210,610)]],
    0x0666: [[(420,650),(260,510),(210,350),(300,210),(430,260)]],
    0x0667: [[(190,620),(315,210),(450,620)]],
    0x0668: [[(190,230),(315,640),(450,230)]],
    0x0669: [[(210,610),(370,610),(430,490),(350,400),(220,450),(210,610),(430,210)]],
}


def digit_contours(codepoint: int) -> list[Contour]:
    result: list[Contour] = []
    for stroke in DIGIT_STROKES[codepoint]:
        result += polyline(stroke, 66)
    return result


def punctuation_contours(codepoint: int) -> list[Contour]:
    if codepoint == ord('?'):
        return polyline([(185,590),(260,665),(390,650),(450,565),
                         (425,470),(315,405),(315,330)], 65) + dot(315,190,34)
    if codepoint == 0x060C:
        return polyline([(350,260),(325,175),(255,120)], 58)
    if codepoint == 0x061B:
        return dot(330,390,32) + polyline([(350,260),(325,175),(255,120)], 58)
    if codepoint == 0x061F:
        return polyline([(455,590),(385,665),(260,650),(195,565),
                         (220,470),(330,405),(330,330)], 65) + dot(330,190,34)
    return punctuation_contours(ord('?'))


def mark_contours(codepoint: int) -> list[Contour]:
    above = codepoint not in (0x064D, 0x0650)
    y = 720 if above else 100
    if codepoint in (0x064B, 0x064D):
        return [segment((260,y),(345,y+40),34), segment((330,y),(415,y+40),34)]
    if codepoint in (0x064C, 0x064F):
        return ring(340,y,70,50,26)
    if codepoint in (0x064E,0x0650):
        return [segment((270,y),(410,y+45),34)]
    if codepoint == 0x0651:
        return polyline([(280,y),(330,y+55),(380,y),(430,y+55)],34)
    if codepoint == 0x0652:
        return ring(345,y,52,52,24)
    return dot(345,y,26)


def glyph_contours(codepoint: int) -> list[Contour]:
    if codepoint in FORM_INFO or codepoint in BASE_INFO:
        return arabic_contours(codepoint)
    if codepoint in DIGIT_STROKES:
        return digit_contours(codepoint)
    if codepoint in (ord('?'),0x060C,0x061B,0x061F):
        return punctuation_contours(codepoint)
    if 0x064B <= codepoint <= 0x0652:
        return mark_contours(codepoint)
    return punctuation_contours(ord('?'))


def be16(v): return struct.pack('>H', v & 0xffff)
def bei16(v): return struct.pack('>h', v)
def be32(v): return struct.pack('>I', v & 0xffffffff)


def glyph(codepoint: int) -> bytes:
    if 32 <= codepoint <= 126:
        return _LATIN_GLYPH(chr(codepoint))
    contours = [contour for contour in glyph_contours(codepoint) if len(contour) >= 3]
    if not contours:
        return bei16(0) + bei16(0) * 4 + be16(0)
    points = [point for contour in contours for point in contour]
    xs = [point[0] for point in points]
    ys = [point[1] for point in points]
    out = bytearray(
        bei16(len(contours)) + bei16(min(xs)) + bei16(min(ys)) +
        bei16(max(xs)) + bei16(max(ys)))
    end = -1
    for contour in contours:
        end += len(contour)
        out += be16(end)
    out += be16(0)
    out += bytes([1]) * len(points)  # all points are on-curve
    previous_x = previous_y = 0
    for x, _ in points:
        out += bei16(x - previous_x)
        previous_x = x
    for _, y in points:
        out += bei16(y - previous_y)
        previous_y = y
    if len(out) % 2:
        out += b'\0'
    return bytes(out)


def checksum(data: bytes) -> int:
    padded = data + b'\0' * ((-len(data)) % 4)
    return sum(struct.unpack('>%dI' % (len(padded) // 4), padded)) & 0xffffffff


def name_table():
    records = []
    strings = bytearray()
    values = {
        1:'Pyramid Arabic', 2:'Regular', 4:'Pyramid Arabic Regular',
        6:'PyramidArabic-Regular', 16:'Pyramid Arabic'}
    for name_id, text in values.items():
        raw = text.encode('utf-16-be')
        offset = len(strings)
        strings += raw
        records.append(struct.pack('>HHHHHH',3,1,0x0409,name_id,len(raw),offset))
    return be16(0) + be16(len(records)) + be16(6 + 12 * len(records)) + b''.join(records) + strings


def cmap_table(mapping):
    segments = [(cp,cp,gid-cp,0) for cp,gid in sorted(mapping.items())] + [(0xffff,0xffff,1,0)]
    count = len(segments)
    search = 2 * (2 ** (count.bit_length() - 1))
    entry = count.bit_length() - 1
    shift = 2 * count - search
    sub = bytearray(
        be16(4) + be16(16 + 8 * count) + be16(0) + be16(2 * count) +
        be16(search) + be16(entry) + be16(shift))
    sub += b''.join(be16(end) for start,end,delta,offset in segments) + be16(0)
    sub += b''.join(be16(start) for start,end,delta,offset in segments)
    sub += b''.join(bei16(((delta + 32768) & 0xffff) - 32768)
                    for start,end,delta,offset in segments)
    sub += b''.join(be16(offset) for start,end,delta,offset in segments)
    return be16(0) + be16(1) + be16(3) + be16(1) + be32(12) + sub


SUPPORTED_CODEPOINTS = sorted({
    *range(32, 127),
    0x060C, 0x061B, 0x061F,
    *range(0x064B, 0x0653), *range(0x0660, 0x066A),
    *BASE_INFO.keys(), *FORM_INFO.keys()})


def build(path: Path):
    codepoints = SUPPORTED_CODEPOINTS
    mapping = {codepoint:index + 1 for index, codepoint in enumerate(codepoints)}
    glyphs = [glyph(ord('?'))] + [glyph(codepoint) for codepoint in codepoints]
    glyf = b''
    offsets = []
    for glyph_data in glyphs:
        offsets.append(len(glyf))
        glyf += glyph_data
    offsets.append(len(glyf))
    loca = b''.join(be32(offset) for offset in offsets)

    hmtx = be16(ADVANCE) + bei16(0)
    for codepoint in codepoints:
        if 0x064B <= codepoint <= 0x0652:
            advance = 0
        elif codepoint == ord(' '):
            advance = 400
        elif codepoint in (0x060C,0x061B,0x061F):
            advance = 520
        elif 32 <= codepoint <= 126:
            advance = 600
        else:
            advance = ADVANCE
        hmtx += be16(advance) + bei16(0)

    head = bytearray(54)
    struct.pack_into('>I',head,0,0x00010000)
    struct.pack_into('>I',head,4,0x00010000)
    struct.pack_into('>I',head,12,0x5F0F3CF5)
    struct.pack_into('>H',head,18,1000)
    struct.pack_into('>hhhh',head,36,0,40,ADVANCE,860)
    struct.pack_into('>H',head,46,8)
    struct.pack_into('>h',head,48,2)
    struct.pack_into('>h',head,50,1)

    hhea = bytearray(36)
    struct.pack_into('>Ihhh',hhea,0,0x00010000,ASCENT,DESCENT,90)
    struct.pack_into('>H',hhea,10,ADVANCE)
    struct.pack_into('>h',hhea,12,0)
    struct.pack_into('>h',hhea,14,ADVANCE)
    struct.pack_into('>h',hhea,16,1)
    struct.pack_into('>H',hhea,34,len(glyphs))

    arabic_codepoints = [cp for cp in codepoints if not 32 <= cp <= 126]
    max_points = max(140, max(len([point for contour in glyph_contours(cp) for point in contour])
                              for cp in arabic_codepoints))
    max_contours = max(35, max(len(glyph_contours(cp)) for cp in arabic_codepoints))
    maxp = (be32(0x00010000) + be16(len(glyphs)) + be16(max_points) +
            be16(max_contours) + be16(0) * 12)
    tables = {
        'cmap':cmap_table(mapping), 'glyf':glyf, 'head':bytes(head),
        'hhea':bytes(hhea), 'hmtx':hmtx, 'loca':loca,
        'maxp':maxp, 'name':name_table()}
    tags = sorted(tables)
    count = len(tags)
    power = 2 ** (count.bit_length() - 1)
    search = power * 16
    entry = count.bit_length() - 1
    shift = count * 16 - search
    directory = bytearray(
        be32(0x00010000) + be16(count) + be16(search) + be16(entry) + be16(shift))
    offset = 12 + 16 * count
    blobs = bytearray()
    records = []
    for tag in tags:
        data = tables[tag]
        padding = (-len(data)) % 4
        records.append((tag,checksum(data),offset,len(data)))
        blobs += data + b'\0' * padding
        offset += len(data) + padding
    for tag, table_checksum, table_offset, length in records:
        directory += tag.encode('ascii') + be32(table_checksum) + be32(table_offset) + be32(length)
    font = bytearray(directory + blobs)
    head_offset = next(table_offset for tag,table_checksum,table_offset,length in records if tag == 'head')
    struct.pack_into('>I',font,head_offset + 8,(0xB1B0AFBA - checksum(font)) & 0xffffffff)
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_bytes(font)
    print(f'wrote {path} ({len(font)} bytes, {len(glyphs)} glyphs)')


if __name__ == '__main__':
    import sys
    build(Path(sys.argv[1] if len(sys.argv) > 1 else
               'Examples/BasicGame/Assets/Fonts/PyramidArabic.ttf'))
