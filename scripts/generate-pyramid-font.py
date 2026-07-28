#!/usr/bin/env python3
"""Generate Ruqoom-owned Pyramid Sans, a small TrueType font for engine examples.

The glyph designs derive from Pyramid's existing 5x7 debug patterns and are emitted
as original square TrueType outlines. No third-party font data is used.
"""
from __future__ import annotations
import struct
from pathlib import Path

PATTERNS = {
'A':["01110","10001","10001","11111","10001","10001","10001"],
'B':["11110","10001","10001","11110","10001","10001","11110"],
'C':["01111","10000","10000","10000","10000","10000","01111"],
'D':["11110","10001","10001","10001","10001","10001","11110"],
'E':["11111","10000","10000","11110","10000","10000","11111"],
'F':["11111","10000","10000","11110","10000","10000","10000"],
'G':["01111","10000","10000","10111","10001","10001","01111"],
'H':["10001","10001","10001","11111","10001","10001","10001"],
'I':["11111","00100","00100","00100","00100","00100","11111"],
'J':["00111","00010","00010","00010","10010","10010","01100"],
'K':["10001","10010","10100","11000","10100","10010","10001"],
'L':["10000","10000","10000","10000","10000","10000","11111"],
'M':["10001","11011","10101","10101","10001","10001","10001"],
'N':["10001","11001","10101","10011","10001","10001","10001"],
'O':["01110","10001","10001","10001","10001","10001","01110"],
'P':["11110","10001","10001","11110","10000","10000","10000"],
'Q':["01110","10001","10001","10001","10101","10010","01101"],
'R':["11110","10001","10001","11110","10100","10010","10001"],
'S':["01111","10000","10000","01110","00001","00001","11110"],
'T':["11111","00100","00100","00100","00100","00100","00100"],
'U':["10001","10001","10001","10001","10001","10001","01110"],
'V':["10001","10001","10001","10001","10001","01010","00100"],
'W':["10001","10001","10001","10101","10101","10101","01010"],
'X':["10001","10001","01010","00100","01010","10001","10001"],
'Y':["10001","10001","01010","00100","00100","00100","00100"],
'Z':["11111","00001","00010","00100","01000","10000","11111"],
'0':["01110","10001","10011","10101","11001","10001","01110"],
'1':["00100","01100","00100","00100","00100","00100","01110"],
'2':["01110","10001","00001","00010","00100","01000","11111"],
'3':["11110","00001","00001","01110","00001","00001","11110"],
'4':["00010","00110","01010","10010","11111","00010","00010"],
'5':["11111","10000","10000","11110","00001","00001","11110"],
'6':["01110","10000","10000","11110","10001","10001","01110"],
'7':["11111","00001","00010","00100","01000","01000","01000"],
'8':["01110","10001","10001","01110","10001","10001","01110"],
'9':["01110","10001","10001","01111","00001","00001","01110"],
'?':["01110","10001","00010","00100","00100","00000","00100"],
'!':["00100","00100","00100","00100","00100","00000","00100"],
'.':["00000","00000","00000","00000","00000","00000","00100"],
',':["00000","00000","00000","00000","00000","00100","01000"],
':':["00000","00100","00100","00000","00100","00100","00000"],
';':["00000","00100","00100","00000","00100","01000","00000"],
'-':["00000","00000","00000","11111","00000","00000","00000"],
'_':["00000","00000","00000","00000","00000","00000","11111"],
'+':["00000","00100","00100","11111","00100","00100","00000"],
'=':["00000","11111","00000","11111","00000","00000","00000"],
'/':["00001","00010","00010","00100","01000","01000","10000"],
'\\':["10000","01000","01000","00100","00010","00010","00001"],
'(':["00010","00100","01000","01000","01000","00100","00010"],
')':["01000","00100","00010","00010","00010","00100","01000"],
'[':["01110","01000","01000","01000","01000","01000","01110"],
']':["01110","00010","00010","00010","00010","00010","01110"],
'<':["00010","00100","01000","10000","01000","00100","00010"],
'>':["01000","00100","00010","00001","00010","00100","01000"],
'#':["01010","11111","01010","01010","11111","01010","01010"],
'%':["11001","11010","00100","01000","10110","00110","00000"],
'&':["01100","10010","10100","01000","10101","10010","01101"],
'@':["01110","10001","10111","10101","10111","10000","01110"],
'|':["00100","00100","00100","00100","00100","00100","00100"],
'*':["00000","10101","01110","11111","01110","10101","00000"],
"'":["00100","00100","00010","00000","00000","00000","00000"],
'"':["01010","01010","00100","00000","00000","00000","00000"],
'É':["00100","00010","11111","10000","11110","10000","11111"],
'Ω':["01110","10001","10001","10001","10001","01010","11011"],
'✓':["00000","00001","00010","10100","01000","00000","00000"],
}

def pattern(ch: str):
    if ch == ' ': return ["00000"]*7
    if ch in PATTERNS: return PATTERNS[ch]
    key = ch.upper() if ch.isalpha() else ch
    return PATTERNS.get(key, PATTERNS['?'])

def be16(v): return struct.pack('>H', v & 0xffff)
def bei16(v): return struct.pack('>h', v)
def be32(v): return struct.pack('>I', v & 0xffffffff)

def glyph(ch: str) -> bytes:
    rows = pattern(ch)
    contours=[]
    for row,bits in enumerate(rows):
        for col,on in enumerate(bits):
            if on!='1': continue
            x0=40+col*100; x1=x0+78
            y0=(6-row)*100+40; y1=y0+78
            contours.append([(x0,y0),(x0,y1),(x1,y1),(x1,y0)])
    if not contours:
        return bei16(0)+bei16(0)*4+be16(0)
    points=[p for c in contours for p in c]
    xs=[p[0] for p in points]; ys=[p[1] for p in points]
    out=bytearray()
    out += bei16(len(contours))+bei16(min(xs))+bei16(min(ys))+bei16(max(xs))+bei16(max(ys))
    end=-1
    for c in contours:
        end += len(c); out += be16(end)
    out += be16(0)
    out += bytes([1])*len(points) # all on-curve, signed 16-bit deltas
    px=py=0
    for x,y in points:
        out += bei16(x-px); px=x
    for x,y in points:
        out += bei16(y-py); py=y
    if len(out)%2: out += b'\0'
    return bytes(out)

def checksum(data: bytes) -> int:
    padded=data + b'\0'*((-len(data))%4)
    return sum(struct.unpack('>%dI'%(len(padded)//4), padded)) & 0xffffffff

def name_table():
    records=[]; strings=bytearray()
    values={1:'Pyramid Sans',2:'Regular',4:'Pyramid Sans Regular',6:'PyramidSans-Regular',16:'Pyramid Sans'}
    for nid,text in values.items():
        raw=text.encode('utf-16-be'); off=len(strings); strings += raw
        records.append(struct.pack('>HHHHHH',3,1,0x0409,nid,len(raw),off))
    return be16(0)+be16(len(records))+be16(6+12*len(records))+b''.join(records)+strings

def cmap_table(mapping):
    segs=[(cp,cp,gid-cp,0) for cp,gid in sorted(mapping.items())]
    segs.append((0xffff,0xffff,1,0))
    n=len(segs); search=2*(2**((n.bit_length()-1))); entry=(n.bit_length()-1); shift=2*n-search
    sub=bytearray()
    sub += be16(4)+be16(16+8*n)+be16(0)+be16(2*n)+be16(search)+be16(entry)+be16(shift)
    sub += b''.join(be16(e) for st,e,d,r in segs)+be16(0)
    sub += b''.join(be16(st) for st,e,d,r in segs)
    sub += b''.join(bei16(d) for st,e,d,r in segs)
    sub += b''.join(be16(r) for st,e,d,r in segs)
    return be16(0)+be16(1)+be16(3)+be16(1)+be32(12)+sub

def kern_table(mapping):
    pairs=[(mapping[ord('A')],mapping[ord('V')],-80),(mapping[ord('A')],mapping[ord('W')],-60),(mapping[ord('T')],mapping[ord('O')],-50)]
    pairs.sort()
    n=len(pairs); p2=2**(n.bit_length()-1); search=p2*6; entry=n.bit_length()-1; shift=n*6-search
    sub=be16(0)+be16(14+n*6)+be16(1)+be16(n)+be16(search)+be16(entry)+be16(shift)
    sub += b''.join(be16(a)+be16(b)+bei16(v) for a,b,v in pairs)
    return be16(0)+be16(1)+sub

def build(path: Path):
    entries=[(c,chr(c)) for c in range(32,127)] + [(0x00E9,'É'),(0x03A9,'Ω'),(0x2713,'✓')]
    mapping={cp:index+1 for index,(cp,ch) in enumerate(entries)}
    glyphs=[glyph('?')]+[glyph(ch) for cp,ch in entries]
    glyf=b''; offsets=[]
    for g in glyphs:
        offsets.append(len(glyf)); glyf += g
    offsets.append(len(glyf))
    loca=b''.join(be32(x) for x in offsets)
    hmtx=be16(600)+bei16(40) + b''.join(be16(400 if ch==' ' else 600)+bei16(40) for cp,ch in entries)
    head=bytearray(54)
    struct.pack_into('>I',head,0,0x00010000); struct.pack_into('>I',head,4,0x00010000)
    struct.pack_into('>I',head,12,0x5F0F3CF5); struct.pack_into('>H',head,16,0)
    struct.pack_into('>H',head,18,1000); struct.pack_into('>hhhh',head,36,40,40,518,718)
    struct.pack_into('>H',head,44,0); struct.pack_into('>H',head,46,8); struct.pack_into('>h',head,48,2); struct.pack_into('>h',head,50,1)
    hhea=bytearray(36); struct.pack_into('>Ihhh',hhea,0,0x00010000,800,-200,100)
    struct.pack_into('>H',hhea,10,600); struct.pack_into('>h',hhea,12,40); struct.pack_into('>h',hhea,14,518)
    struct.pack_into('>h',hhea,16,1); struct.pack_into('>h',hhea,18,0); struct.pack_into('>h',hhea,20,0)
    struct.pack_into('>h',hhea,32,0); struct.pack_into('>H',hhea,34,len(glyphs))
    maxp=be32(0x00010000)+be16(len(glyphs))+be16(140)+be16(35)+be16(0)*12
    tables={'cmap':cmap_table(mapping),'glyf':glyf,'head':bytes(head),'hhea':bytes(hhea),'hmtx':hmtx,'kern':kern_table(mapping),'loca':loca,'maxp':maxp,'name':name_table()}
    tags=sorted(tables)
    n=len(tags); p2=2**(n.bit_length()-1); search=p2*16; entry=n.bit_length()-1; shift=n*16-search
    directory=bytearray(be32(0x00010000)+be16(n)+be16(search)+be16(entry)+be16(shift))
    offset=12+16*n; blobs=bytearray(); records=[]
    for tag in tags:
        data=tables[tag]; pad=(-len(data))%4
        records.append((tag,checksum(data),offset,len(data)))
        blobs += data+b'\0'*pad; offset += len(data)+pad
    for tag,cs,off,length in records:
        directory += tag.encode('ascii')+be32(cs)+be32(off)+be32(length)
    font=bytearray(directory+blobs)
    head_off=next(off for tag,cs,off,length in records if tag=='head')
    total=checksum(font); adjustment=(0xB1B0AFBA-total)&0xffffffff
    struct.pack_into('>I',font,head_off+8,adjustment)
    path.parent.mkdir(parents=True,exist_ok=True); path.write_bytes(font)
    print(f'wrote {path} ({len(font)} bytes, {len(glyphs)} glyphs)')

if __name__=='__main__':
    import sys
    target=Path(sys.argv[1] if len(sys.argv)>1 else 'Examples/BasicGame/Assets/Fonts/PyramidSans.ttf')
    build(target)
