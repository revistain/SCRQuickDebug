from eudplib import *
import eudplib.core.variable.eudv as ev
import struct
import inspect

isEUDFunc = False
collected_vars = []
original_init = ev.EUDVariable.__init__
frames = {}
def patched_init(self, *args, **kwargs):
    prev_frame = inspect.currentframe().f_back
    if isEUDFunc == True or prev_frame.f_back.f_code.co_name in ("onPluginStart", "beforeTriggerExec", "afterTriggerExec"):
        fname = prev_frame.f_code.co_name
        if fname and (fname == "_LVAR" or not fname.startswith("_")):
            collected_vars.append(self)
        frames[self] = prev_frame
    original_init(self, *args, **kwargs)

vars = []
ignore_set = ("EPD", "EUDLoopRange", "_create_func_args", "_create_func_body",
            "caller", "RestorePUPx", "f", "EUDTernary")
def find_var_names(self):
    try:
        for name, value in frames[self].f_locals.items():
            if value is self:
                if frames[self].f_code.co_name in ignore_set:
                    continue
                elif frames[self].f_code.co_name == "_LVAR": 
                    frames[self] = frames[self].f_back
                    for n, v in frames[self].f_locals.items():
                        if v is self:
                            name = n
                vars.append([self, name, frames[self].f_code.co_name])
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
signatureEPD = EPD(Db("TEMPjknOSDIfnwlnlSNDKlnfkopqfnkLDNSFpwIn\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0")) # 40bytes

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
    # f_simpleprint(currentTailEPD]

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

# [self, varName, funcName]
### table func string
##  4b      : FUNS
##  4b      : size of strings
##  strs    : func strings

### table var string
##  4b      : VARS
##  4b      : size of strings
##  strs    : variable strings

### variable table
##  4b      : VART
##  4b      : size of variable
##  12b*vars: var_data(func_str_idx, var_str_idx, data)

func_str    = []
var_str     = []
var_data    = [] # func_str_idx, var_str_idx, data
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

def save_data():
    process_vars()

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

    DoActions(act)


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
    ])

## packet
# opcode    2byte
# pNumber   2bytes
# param1    4byte
# param2    4byte
# param3    4byte
# mask      4byte

## packet send
# 20byte * 50 = 1kb
# whoIsUsing    2byte
# isConnected   2byte
# currentHead   4byte
# currentEnd    4byte

sendCount = EUDVariable(0)
sendHeadEPD = EUDXVariable(0, SetTo, 0, 250)
sendTailEPD = EUDXVariable(0, SetTo, 0, 250)
def sendPacket(opCode, param1, param2, param3, mask=0xFFFFFFFF):
    DoActions([
        SetMemoryEPD(bufferEPD, SetTo, 1),
        SetMemoryEPD(bufferEPD+2, SetTo, sendTailEPD),
    ])
    baseEPD = (bufferEPD+sendTailEPD+3)
    DoActions([
        SetMemoryXEPD(baseEPD, SetTo, opCode<<24, 0xFF000000),
        SetMemoryEPD(baseEPD+1, SetTo, param1),
        SetMemoryEPD(baseEPD+2, SetTo, param2),
        SetMemoryEPD(baseEPD+3, SetTo, param3),
        SetMemoryEPD(baseEPD+4, SetTo, mask),
        SetMemoryEPD(bufferEPD, SetTo, 0),
        sendCount.AddNumber(1),
        sendTailEPD.AddNumber(5),
    ])
    f_maskwrite_epd(baseEPD, sendCount, 0xFFFFFF)
    #f_eprintln(hptr(f_dwread_epd(baseEPD)), " ",hptr(f_dwread_epd(baseEPD+1)), " ",f_dwread_epd(baseEPD+2), " ",hptr(f_dwread_epd(baseEPD+3)))

def onPluginStart():
    init_signature()
    ev.EUDVariable.__init__ = patched_init

def beforeTriggerExec():
    sendPacket(0xAB, 0x00FF00FF, 0xAAAABBBB, 0xBBBBCCCC)
    #f_eprintln(epd2s(signatureEPD))
    
def afterTriggerExec():
    for var in collected_vars:
        find_var_names(var)
    for var in vars:
        print(var)
    save_data()
    
