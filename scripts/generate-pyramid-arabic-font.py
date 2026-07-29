#!/usr/bin/env python3
"""Generate Ruqoom-owned Pyramid Arabic, a compact TrueType fallback font.

The glyphs are original geometric pixel outlines intended for engine diagnostics,
UI bring-up, and deterministic tests. They do not contain third-party font data.
Contextual Arabic forms are encoded in the Unicode Presentation Forms-B range;
Pyramid::Text selects those forms before glyph lookup.
"""
from __future__ import annotations

import struct
from pathlib import Path

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

FORM_INFO = {}
for base, isolated, final, initial, medial, dots, below, variant in FORMS:
    for codepoint, form in ((isolated,"isolated"),(final,"final"),(initial,"initial"),(medial,"medial")):
        if codepoint:
            FORM_INFO[codepoint]=(base,form,dots,below,variant)

DIGITS = {
    0x0660:["01110","10001","10001","10001","10001","10001","01110"],
    0x0661:["00100","01100","00100","00100","00100","00100","01110"],
    0x0662:["01110","10001","00001","00010","00100","01000","11111"],
    0x0663:["11110","00001","00001","01110","00001","00001","11110"],
    0x0664:["00010","00110","01010","10010","11111","00010","00010"],
    0x0665:["11111","10000","10000","11110","00001","00001","11110"],
    0x0666:["01110","10000","10000","11110","10001","10001","01110"],
    0x0667:["11111","00001","00010","00100","01000","01000","01000"],
    0x0668:["01110","10001","10001","01110","10001","10001","01110"],
    0x0669:["01110","10001","10001","01111","00001","00001","01110"],
}

PUNCT = {
    ord('?'):["01110","10001","00010","00100","00100","00000","00100"],
    0x060C:["00000","00000","00000","00000","00100","00100","01000"],
    0x061B:["00100","00100","00000","00100","00100","01000","00000"],
    0x061F:["01110","10001","01000","00100","00100","00000","00100"],
}

def blank(): return [[0]*7 for _ in range(7)]

def set_dot(grid, col, row):
    if 0 <= row < 7 and 0 <= col < 7: grid[row][col]=1

def arabic_pattern(codepoint: int):
    _, form, dots, below, variant = FORM_INFO[codepoint]
    g=blank()
    # A shared baseline and joining strokes create a deterministic connected script.
    for col in range(1,6): g[4][col]=1
    if form in ("initial","medial"): g[4][0]=1
    if form in ("final","medial"): g[4][6]=1

    if variant == 0: # shallow bowl
        g[3][5]=g[5][2]=g[5][3]=g[5][4]=1
    elif variant == 1: # deep bowl
        g[3][5]=g[5][1]=g[5][2]=g[5][3]=g[5][4]=g[5][5]=1
    elif variant == 2: # descending tail
        g[5][2]=g[5][3]=g[6][1]=1
    elif variant == 3: # tall stem
        for row in range(1,5): g[row][5]=1
    elif variant == 4: # loop
        for col,row in ((4,2),(5,2),(3,3),(5,3),(3,4),(4,5),(5,5)): g[row][col]=1
    elif variant == 5: # yeh-like tail
        g[5][1]=g[5][2]=g[5][3]=g[6][0]=g[6][4]=1
    elif variant == 6: # teeth
        for col in (2,4,5): g[3][col]=1
    elif variant == 7: # broad bowl with tooth
        g[2][5]=g[3][5]=1
        for col in range(1,6): g[5][col]=1
    elif variant == 8: # tall center
        for row in range(1,5): g[row][3]=1
    elif variant == 9: # eye
        for col,row in ((3,2),(4,2),(2,3),(4,3),(2,4),(3,5),(4,5)): g[row][col]=1
    elif variant == 10: # round head
        for col,row in ((3,2),(4,2),(2,3),(5,3),(3,4),(4,4)): g[row][col]=1
    elif variant == 11: # kaf
        for row in range(1,5): g[row][5]=1
        g[2][3]=g[3][4]=1
    elif variant == 12: # heh double loop
        for col,row in ((2,2),(3,2),(4,2),(1,3),(5,3),(2,4),(3,4),(4,4),(3,5)): g[row][col]=1

    if dots:
        row = 6 if below else 0
        positions={1:[3],2:[2,4],3:[1,3,5]}[dots]
        for col in positions: set_dot(g,col,row)
    return ["".join("1" if v else "0" for v in row) for row in g]

def pattern(codepoint: int):
    if codepoint in FORM_INFO: return arabic_pattern(codepoint)
    if codepoint in DIGITS: return DIGITS[codepoint]
    if codepoint in PUNCT: return PUNCT[codepoint]
    if 0x064B <= codepoint <= 0x0652:
        g=blank(); set_dot(g,3,0 if codepoint % 2 == 0 else 6)
        if codepoint in (0x064B,0x064D,0x064F): set_dot(g,4,0 if codepoint % 2 == 0 else 6)
        return ["".join("1" if v else "0" for v in row) for row in g]
    return PUNCT[ord('?')]

def be16(v): return struct.pack('>H',v&0xffff)
def bei16(v): return struct.pack('>h',v)
def be32(v): return struct.pack('>I',v&0xffffffff)

def glyph(codepoint: int) -> bytes:
    rows=pattern(codepoint); contours=[]
    for row,bits in enumerate(rows):
        for col,on in enumerate(bits):
            if on!='1': continue
            x0=30+col*80; x1=x0+66; y0=(6-row)*100+30; y1=y0+74
            contours.append([(x0,y0),(x0,y1),(x1,y1),(x1,y0)])
    if not contours: return bei16(0)+bei16(0)*4+be16(0)
    points=[p for c in contours for p in c]; xs=[p[0] for p in points]; ys=[p[1] for p in points]
    out=bytearray(bei16(len(contours))+bei16(min(xs))+bei16(min(ys))+bei16(max(xs))+bei16(max(ys)))
    end=-1
    for c in contours: end+=len(c); out+=be16(end)
    out+=be16(0)+bytes([1])*len(points); px=py=0
    for x,y in points: out+=bei16(x-px); px=x
    for x,y in points: out+=bei16(y-py); py=y
    if len(out)%2: out+=b'\0'
    return bytes(out)

def checksum(data: bytes) -> int:
    padded=data+b'\0'*((-len(data))%4)
    return sum(struct.unpack('>%dI'%(len(padded)//4),padded))&0xffffffff

def name_table():
    records=[]; strings=bytearray(); values={1:'Pyramid Arabic',2:'Regular',4:'Pyramid Arabic Regular',6:'PyramidArabic-Regular',16:'Pyramid Arabic'}
    for nid,text in values.items():
        raw=text.encode('utf-16-be'); off=len(strings); strings+=raw
        records.append(struct.pack('>HHHHHH',3,1,0x0409,nid,len(raw),off))
    return be16(0)+be16(len(records))+be16(6+12*len(records))+b''.join(records)+strings

def cmap_table(mapping):
    segs=[(cp,cp,gid-cp,0) for cp,gid in sorted(mapping.items())]+[(0xffff,0xffff,1,0)]
    n=len(segs); search=2*(2**(n.bit_length()-1)); entry=n.bit_length()-1; shift=2*n-search
    sub=bytearray(be16(4)+be16(16+8*n)+be16(0)+be16(2*n)+be16(search)+be16(entry)+be16(shift))
    sub+=b''.join(be16(e) for st,e,d,r in segs)+be16(0)
    sub+=b''.join(be16(st) for st,e,d,r in segs)
    sub+=b''.join(bei16(((d+32768)&0xffff)-32768) for st,e,d,r in segs)+b''.join(be16(r) for st,e,d,r in segs)
    return be16(0)+be16(1)+be16(3)+be16(1)+be32(12)+sub

def build(path: Path):
    codepoints=sorted({ord('?'),0x060C,0x061B,0x061F,*range(0x064B,0x0653),*range(0x0660,0x066A),*FORM_INFO.keys()})
    mapping={cp:index+1 for index,cp in enumerate(codepoints)}
    glyphs=[glyph(ord('?'))]+[glyph(cp) for cp in codepoints]
    glyf=b''; offsets=[]
    for g in glyphs: offsets.append(len(glyf)); glyf+=g
    offsets.append(len(glyf)); loca=b''.join(be32(x) for x in offsets)
    hmtx=be16(620)+bei16(30)
    for cp in codepoints:
        advance=0 if 0x064B <= cp <= 0x0652 else 620
        hmtx+=be16(advance)+bei16(30)
    head=bytearray(54); struct.pack_into('>I',head,0,0x00010000); struct.pack_into('>I',head,4,0x00010000)
    struct.pack_into('>I',head,12,0x5F0F3CF5); struct.pack_into('>H',head,18,1000)
    struct.pack_into('>hhhh',head,36,30,30,576,704); struct.pack_into('>H',head,46,8); struct.pack_into('>h',head,48,2); struct.pack_into('>h',head,50,1)
    hhea=bytearray(36); struct.pack_into('>Ihhh',hhea,0,0x00010000,800,-200,100); struct.pack_into('>H',hhea,10,620)
    struct.pack_into('>h',hhea,12,30); struct.pack_into('>h',hhea,14,576); struct.pack_into('>h',hhea,16,1); struct.pack_into('>H',hhea,34,len(glyphs))
    maxp=be32(0x00010000)+be16(len(glyphs))+be16(160)+be16(40)+be16(0)*12
    tables={'cmap':cmap_table(mapping),'glyf':glyf,'head':bytes(head),'hhea':bytes(hhea),'hmtx':hmtx,'loca':loca,'maxp':maxp,'name':name_table()}
    tags=sorted(tables); n=len(tags); p2=2**(n.bit_length()-1); search=p2*16; entry=n.bit_length()-1; shift=n*16-search
    directory=bytearray(be32(0x00010000)+be16(n)+be16(search)+be16(entry)+be16(shift)); offset=12+16*n; blobs=bytearray(); records=[]
    for tag in tags:
        data=tables[tag]; pad=(-len(data))%4; records.append((tag,checksum(data),offset,len(data))); blobs+=data+b'\0'*pad; offset+=len(data)+pad
    for tag,cs,off,length in records: directory+=tag.encode('ascii')+be32(cs)+be32(off)+be32(length)
    font=bytearray(directory+blobs); head_off=next(off for tag,cs,off,length in records if tag=='head')
    struct.pack_into('>I',font,head_off+8,(0xB1B0AFBA-checksum(font))&0xffffffff)
    path.parent.mkdir(parents=True,exist_ok=True); path.write_bytes(font)
    print(f'wrote {path} ({len(font)} bytes, {len(glyphs)} glyphs)')

if __name__=='__main__':
    import sys
    build(Path(sys.argv[1] if len(sys.argv)>1 else 'Examples/BasicGame/Assets/Fonts/PyramidArabic.ttf'))
