from eudplib import *
import eudplib.core.variable.eudv as ev
import struct
import inspect

# https://github.com/armoha/eudplib/commit/d44a65048d195592b29bd059e793488f53c25d28

isEUDFunc = False
collected_vars = []
original_init = ev.EUDVariable.__init__
frames = {}
def _patched_init(self, *args, **kwargs):
    prev_frame = inspect.currentframe().f_back
    if isEUDFunc == True or prev_frame.f_back.f_code.co_name in ("onPluginStart", "beforeTriggerExec", "afterTriggerExec"):
        fname = prev_frame.f_code.co_name
        if fname and (fname == "_LVAR" or not fname.startswith("_")):
            collected_vars.append(self)
        frames[self] = prev_frame
    original_init(self, *args, **kwargs)

original_lshift = ev.EUDVariable.__lshift__
def _patched_lshift(self, *args, **kwargs):
    prev_frame = inspect.currentframe().f_back
    if isEUDFunc == True:
        fname = prev_frame.f_code.co_name
        if fname:
            print("lshift: ", fname)
            collected_vars.append(self)
        frames[self] = prev_frame
    original_lshift(self, *args, **kwargs)

vars = []
ignore_set = ("EPD", "EUDLoopRange", "_create_func_args", "_create_func_body",
            "caller", "RestorePUPx", "f", "EUDTernary", "EUDLoopPlayer", "rot")
def find_var_names(var):
    try:
        for name, value in frames[var].f_locals.items():
            if value is var:
                if any(s[0] is name for s in vars):
                    continue
                if frames[var].f_code.co_name in ignore_set:
                    continue
                elif frames[var].f_code.co_name == "_LVAR": 
                    frames[var] = frames[var].f_back
                    for n, v in frames[var].f_locals.items():
                        if v is var:
                            name = n
                vars.append([var, name, frames[var].f_code.co_name])
    except KeyError:
        ...

import eudplib.core.eudfunc.eudfuncn as ef
oldfbody = ef.EUDFuncN._create_func_body
def registerEUDFunc(self, *args, **kwargs):
    global isEUDFunc
    isEUDFunc = True
    oldfbody(self, *args, **kwargs)
    isEUDFunc = False
ef.EUDFuncN._create_func_body = registerEUDFunc

bufferEPD = EPD(Db(1008))
signatureEPD = EPD(Db("TEMPjknOSDIfnwlnlSNDKlnfkopqfnkLDNSFEDAC\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0")) # 40bytes

# sendCount = EUDVariable(0, Add, 1)
# sendCount.SetDest(sendCount.getDestAddr())
# currentHeadEPD = EUDXVariable(EPD(0x6509B0), Add, 0, 200)
# currentTailEPD = EUDXVariable(EPD(0x6509B0), Add, 0, 200)
# def sendPacket(opCode, param1, param2, param3, mask=0xFFFFFFFF):
#     currentHeadEPD << f_dwread_epd(bufferEPD+1)
#     act = []
#     act.append(SetDeathsX(CurrentPlayer, SetTo, opCode<<24, 0, 0xFF000000))
#     act.append(SetMemoryEPD(EPD(0x6509B0), Add, 1))
#     act.append(SetDeaths(CurrentPlayer, SetTo, param1, 0))
#     act.append(SetMemoryEPD(EPD(0x6509B0), Add, 1))
    # act.append(SetDeaths(CurrentPlayer, SetTo, param2, 0))
    # act.append(SetMemoryEPD(EPD(0x6509B0), Add, 1))
    # act.append(SetDeaths(CurrentPlayer, SetTo, param3, 0))
    # act.append(SetMemoryEPD(EPD(0x6509B0), Add, 1))
    # act.append(SetDeaths(CurrentPlayer, SetTo, mask, 0))
    # act.append(SetMemoryEPD(EPD(0x6509B0), SetTo, 0))
    # VProc([currentTailEPD],
    #       [SetMemoryEPD(EPD(0x6509B0), SetTo, bufferEPD+3),
    #        currentTailEPD.AddNumber(5)])
    # VProc([sendCount], [act])
    # f_dwwrite_epd(bufferEPD+2, currentTailEPD)

# sendCount = EUDVariable(1)
# currentHeadEPD = EUDXVariable(0, SetTo, 0, 1000)
# currentTailEPD = EUDXVariable(0, SetTo, 0, 1000)
# def sendPacket(opCode, param1, param2, param3, mask=0xFFFFFFFF):
#     baseOffset = EUDVariable()
#     baseOffset << (currentHeadEPD + bufferEPD + 3)
#     DoActions([
#         SetMemoryXEPD(bufferEPD, SetTo, 1, 0xFFFF),
#         SetMemoryXEPD(baseOffset, SetTo, opCode, 0xFF000000),
#         SetMemoryXEPD(baseOffset, SetTo, sendCount, 0xFFFFFF),
#         SetMemoryEPD(baseOffset+1, SetTo, param1),
#         SetMemoryEPD(baseOffset+2, SetTo, param2),
#         SetMemoryEPD(baseOffset+3, SetTo, param3),
#         SetMemoryEPD(baseOffset+4, SetTo, mask),
#         currentTailEPD.AddNumber(5),
#         sendCount.AddNumber(1),
#     ])
#     DoActions([
#         SetMemoryXEPD(bufferEPD, SetTo, 0, 0xFFFF),
#         # SetMemoryEPD(bufferEPD+1, SetTo, currentHeadEPD),
#         SetMemoryEPD(bufferEPD+2, SetTo, currentTailEPD),
#     ])

##### custom defined sections
### func string
##  4b      : FUNS
##  4b      : size of strings
##  strs    : func strings

### var string
##  4b      : VARS
##  4b      : size of strings
##  strs    : variable strings

### variable table
##  4b      : VART
##  4b      : size of variable
##  12b*vars: var_data(func_str_idx, var_str_idx, data)

### location string
##  4b      : MRNS
##  4b      : size of strings
##  strs    : loc strings

### location table
##  4b      : MRND
##  4b      : size of locations
##  8b*locs : loc_data(loc name, loc str idex)
#####

func_str    = []
var_str     = []
var_data    = []
funcStrDb      = Forward()
funcVarDb      = Forward()
funcVarDataDb  = Forward()
funcMRGNDb     = Forward()
funcMRGNDataDb = Forward()
def process_vars():
    for e in vars:
        func_idx = 0
        if e[2] not in func_str:
            func_str.append(e[2])
            func_idx = len(func_str) - 1
        else:
            func_idx = func_str.index(e[2])
        var_str.append(e[1])
        var_data.append([func_idx, len(var_str)-1, e[0]])

screenDbEPD = EPD(Db(8)) # screen top, left(0 ~ 0x1FFF) 4byte each
mapPathDbEPD = EPD(Db(260))
pathSignatureEPD = EPD(Db("TEMPNkdfhLpZmqWnRbZlfhInbpQYtZBwjeOqmPlW"))
def modify_map_name():
    f_repmovsd_epd(EPD(0x57FD3C), pathSignatureEPD, 40)
    header = struct.unpack("<I", b"Gong")[0]
    DoActions(SetMemoryEPD(EPD(0x57FD3C), SetTo, header))
        
def save_data():
    process_vars()
    modify_map_name()

    # restore header
    act = []
    header = struct.unpack("<I", b"Gong")[0]
    act.append(SetMemoryEPD(signatureEPD, SetTo, header))
    # func string table
    _funcFuncBinaray = b''
    for str in func_str:
        _funcFuncBinaray += str.encode('utf-8')+ b'\x00'
    funcFuncBinary = b'FUNS' + struct.pack("<I", len(_funcFuncBinaray))
    funcFuncBinary += _funcFuncBinaray
    funcStrDb << Db(funcFuncBinary)

    # var string table
    _funcVarBinaray = b''
    for str in var_str:
        _funcVarBinaray += str.encode('utf-8')+ b'\x00'
    funcVarBinary = b'VARS' + struct.pack("<I", len(_funcVarBinaray))
    funcVarBinary += _funcVarBinaray
    funcVarDb << Db(funcVarBinary)

    # var data table
    _funcVarDataBinaray = b''
    for i, var in enumerate(var_data):
        _funcVarDataBinaray += struct.pack("<I", var[0])
        _funcVarDataBinaray += struct.pack("<I", var[1])
        _funcVarDataBinaray += b'\0\0\0\0' # struct.pack("<I", var[2].getValueAddr())
        act.append(SetMemoryEPD(EPD(funcVarDataDb)+4+3*i, SetTo, var[2].getValueAddr()))
    funcVarDataBinary = b'VARD' + struct.pack("<I", len(_funcVarDataBinaray))
    funcVarDataBinary += _funcVarDataBinaray
    funcVarDataDb << Db(funcVarDataBinary)

    # mrgn string table
    mrgn = getMRGNArray()
    _funcMRGNBinaray = b''
    for i, str in enumerate(mrgn):
        print(str[0])
        _funcMRGNBinaray += str[0] + b'\x00'
        str[0] = i
    funcMRGNBinary = b'MRNS' + struct.pack("<I", len(_funcMRGNBinaray))
    funcMRGNBinary += _funcMRGNBinaray
    funcMRGNDb << Db(funcMRGNBinary)

    # mrgn string mapping table
    _funcMRGNDataBinaray = b''
    for i, str in enumerate(mrgn):
        _funcMRGNDataBinaray += struct.pack("<I", str[0])
        _funcMRGNDataBinaray += struct.pack("<I", str[1])
    funcMRGNDataBinary = b'MRND' + struct.pack("<I", len(_funcMRGNDataBinaray))
    funcMRGNDataBinary += _funcMRGNDataBinaray
    funcMRGNDataDb << Db(funcMRGNDataBinary)
    
    # save screen data
    f_repmovsd_epd(screenDbEPD, EPD(0x628470), 1)
    f_repmovsd_epd(screenDbEPD+1, EPD(0x628448), 1)
    if EUDExecuteOnce()():
        # save map path data
        # f_repmovsd_epd(mapPathDbEPD, EPD(0x57FD3C), 65)
        DoActions(act)
    EUDEndExecuteOnce()


def dwread(ba, offset):
    return int.from_bytes(ba[offset:offset+4], byteorder="little")
def bread(ba, offset):
    return ba[offset]

import eudplib.core.mapdata.stringmap as sm
def getMRGNArray():
    print()
    locmap = sm.locmap._s2id # {locname: locidx}
    mrgn_data = []
    for k, v in locmap.items():
        mrgn_data.append([k, v])
    return mrgn_data

def init_signature():
    DoActions([
        SetMemoryEPD(signatureEPD+10, SetTo, signatureEPD),
        SetMemoryEPD(signatureEPD+11, SetTo, bufferEPD),
        SetMemoryEPD(signatureEPD+12, SetTo, funcStrDb),
        SetMemoryEPD(signatureEPD+13, SetTo, funcVarDb),
        SetMemoryEPD(signatureEPD+14, SetTo, funcVarDataDb),
        SetMemoryEPD(signatureEPD+15, SetTo, funcMRGNDb),
        SetMemoryEPD(signatureEPD+16, SetTo, funcMRGNDataDb),
        SetMemoryEPD(signatureEPD+17, SetTo, screenDbEPD),
        SetMemoryEPD(signatureEPD+18, SetTo, mapPathDbEPD), # unused
    ])

## packet
# opcode    2byte
# pNumber   2bytes
# param1    4byte
# param2    4byte
# param3    4byte
# mask      4byte

## packet header
# bufferEPD
# 20byte * 50 = 1kb
# whoIsUsing    1byte
# isConnected   1byte
# stackedCount  2byte
# currentHead   4byte
# currentEnd    4byte

sendCount = EUDVariable(0)
sendHeadEPD = EUDVariable(0)
sendTailEPD = EUDVariable(0)
def sendPacket(opCode, param1, param2, param3, mask=0xFFFFFFFF):
    endTrigger = Forward()
    EUDJumpIfNot([MemoryXEPD(bufferEPD, Exactly, 0, 0xFF)], endTrigger) # program using / not connected // , MemoryXEPD(bufferEPD, AtLeast, 1<<8, 0xFF00)]
    
    baseEPD = (bufferEPD+sendTailEPD+3)
    DoActions([
        SetMemoryXEPD(bufferEPD, SetTo, 1, 0xFF),
        SetMemoryXEPD(bufferEPD, Add, 1 << 16, 0xFFFF0000),
        SetMemoryXEPD(baseEPD, SetTo, opCode, 0xFFFF),
        SetMemoryEPD(baseEPD+1, SetTo, param1),
        SetMemoryEPD(baseEPD+2, SetTo, param2),
        SetMemoryEPD(baseEPD+3, SetTo, param3),
        SetMemoryEPD(baseEPD+4, SetTo, mask),
        sendCount.AddNumber(1<<16),
        sendTailEPD.AddNumber(5),
    ])
    
    RawTrigger(
        conditions=sendTailEPD.AtLeast(250),
        actions=sendTailEPD.SubtractNumber(250)
    )
    f_maskwrite_epd(baseEPD, sendCount, 0xFFFF0000)
    f_dwwrite_epd(bufferEPD+2, sendTailEPD)
    
    DoActions(SetMemoryXEPD(bufferEPD, SetTo, 0, 0xFF))
    endTrigger << NextTrigger()


def receivePacket():
    ...

@EUDFunc
def debugDoTrigger(param1, param2, param3, param4):
    act = []
    EUDSwitch(param1, 0xFF)
    if EUDSwitchCase()(1): # Create Unit
        chkt = GetChkTokenized()
        dim = chkt.getsection("DIM ")
        mapX = b2i2(dim, 0) << 5
        mapY = b2i2(dim, 2) << 5
        
        # param1: AA param2: location YYXX, param3: unit_number, param4: target_player
        f_setloc_epd(EncodeLocation("Anywhere"), EPD(param2.getValueAddr))
        DoActions(CreateUnit(1, param3, EncodeLocation("Anywhere"), param4))
        f_setloc(EncodeLocation("Anywhere"), 0, 0, mapX*32, mapY*32)
        
        EUDBreak()
    if EUDSwitchCase()(10): # Get Clicked Unit
        ...
        EUDBreak()
    if EUDSwitchCase()(11): # Remove Clicked Unit
        ...
        EUDBreak()
    if EUDSwitchCase()(12): # Move Clicked Unit
        ...
        EUDBreak()
    EUDEndSwitch()


def modifiyMapName():
    chkt = GetChkTokenized()
    SPRP = bytearray(chkt.getsection("SPRP"))
    map_name_index = SPRP[0:2]
    prev_map_name = sm.strmap.GetString(map_name_index)
    signature = GetStringIndex(str("TEMPNkdfhLpZmqWnRbZlfhInbpQYtZ"))
    SPRP[0:2] = i2b2(signature)
    chkt.setsection("SPRP", SPRP)

def onPluginStart():
    init_signature()
    ev.EUDVariable.__init__ = _patched_init
    ev.EUDVariable.__lshift__ = _patched_lshift
    sendPacket(0xAB, 0x00FF00FF, 0xAAAABBBB, 0xBBBBCCCC)

def beforeTriggerExec():
    ...
    #f_eprintln(epd2s(signatureEPD))
    
def afterTriggerExec():
    for var in collected_vars:
        find_var_names(var)
    for var in vars:
        print(var)
    save_data()
    
    # print screen
    # f_simpleprint(hptr(f_dwread_epd(screenDbEPD)), " ", hptr(f_dwread_epd(screenDbEPD+1)))
    
    # print locations
    # f_simpleprint(hptr(f_dwread(0x58DC60)), " ", hptr(f_dwread(0x58DC64)), " ", hptr(f_dwread(0x58DC68)), " ", hptr(f_dwread(0x58DC6C)), " ")
