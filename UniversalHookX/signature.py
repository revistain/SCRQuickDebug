from eudplib import *
import eudplib.core.variable.eudv as ev
import eudplib.core.eudfunc.eudfuncn as ef
import eudplib.collections.eudarray as ea
import eudplib.epscript.helper as hp
import struct
import inspect
import os
eudplib_type = 0
def compare_versions(version1, version2):
    print(version1)
    print(version2)
    v1_parts = list(map(int, version1.split('.')[:2]))
    v2_parts = list(map(int, version2.split('.')[:2]))
    return v1_parts > v2_parts

if compare_versions(eudplibVersion(), "0.77.9"):
    eudplib_type = 1
else:
    ep_assert("euddraft 0.1.0.0 이상 버전을 사용하여 주세요 !!")
        
isEUDFunc = False
collected_vars = []
collected_arrays = []
collected_gvars = []
collected_garray = []
frames = {}
global_frames = {}
frames_array = {}
global_frames_array = {}
original_init = ev.EUDVariable.__init__
def _patched_init(self, *args, **kwargs):
    if eudplib_type == 0:
        ... # no
    elif eudplib_type == 1:
        prev_frame = inspect.currentframe().f_back
        fname = prev_frame.f_code.co_name
        if fname:
            if fname == "_TYLV":
                print("_TYLV: ", prev_frame.f_back.f_code.co_name)
                collected_vars.append(self)
                frames[self] = prev_frame.f_back
            else:
                # print("EUDVar: ", prev_frame.f_code.co_name)
                collected_vars.append(self)
                frames[self] = prev_frame                

        original_init(self, *args, **kwargs)
ev.EUDVariable.__init__ = _patched_init

original_tygv = hp._TYGV # typed global variable
def _patched_tygv(self, *args, **kwargs):
    if eudplib_type == 0:
        return original_tygv(self, *args, **kwargs)
    elif eudplib_type == 1:
        var_list = original_tygv(self, *args, **kwargs)
        if isinstance(var_list, list):
            for var in var_list:
                collected_gvars.append(var)
                print("_TYGL: ", var)
                prev_frame = inspect.currentframe().f_back
                global_frames[var] = prev_frame
        else:
            print("_TYGL: ", var_list)
            collected_gvars.append(var_list)
            global_frames[var_list] = inspect.currentframe().f_back
        return var_list
hp._TYGV = _patched_tygv

original_cgfw = hp._CGFW # const global forward?
def _patched_cgfw(self, *args, **kwargs):
    if eudplib_type == 0:
        ... # no
    elif eudplib_type == 1:
        rets = original_cgfw(self, *args, **kwargs)
        if not isinstance(rets[0], int):
            prev_frame = inspect.currentframe().f_back
            collected_garray.append(rets[0])
            global_frames_array[rets[0]] = prev_frame
    return rets
hp._CGFW = _patched_cgfw

# (Line 4) const someEUDArray = EUDArray(10);
# someEUDArray = _CGFW(lambda: [EUDArray(10)], 1)[0]
# so we prop dont need this
original_array_init = ea.EUDArray.__init__
def _patched_array_init(self, *args, **kwargs):
    if eudplib_type == 0:
        ... # nope
    elif eudplib_type == 1:
        prev_frame = inspect.currentframe().f_back
        collected_arrays.append(self)
        frames_array[self] = prev_frame
    original_array_init(self, *args, **kwargs)
ea.EUDArray.__init__    = _patched_array_init

vars = []
ignore_set = ("EPD", "EUDLoopRange", "_create_func_args", "_create_func_body",
            "caller", "RestorePUPx", "EUDTernary", "EUDLoopPlayer", "rot")

def find_var_names(var):
    if eudplib_type == 0:
        ... # nope
    elif eudplib_type == 1:
        try:
            for name, value in frames[var].f_locals.items():
                if inspect.getfile(frames[var]).endswith(".py"):
                    continue
                if unProxy(value) is unProxy(var):  # 내용 비교
                    file_path = inspect.getfile(frames[var])
                    file_name = os.path.basename(file_path)
                    # var_data(path, func_str_idx, var_str_idx, addr)
                    if frames[var].f_code.co_name == "<module>":
                        vars.append([file_name, "", name, var])
                    else:
                        vars.append([file_name, frames[var].f_code.co_name, name, var])
        except KeyError:
            ...

gvars = []
def find_global_var_names(gvar):
    if eudplib_type == 0:
        ... # nope
    elif eudplib_type == 1:
        try:
            for name, value in global_frames[gvar].f_locals.items():
                if unProxy(value) is unProxy(gvar):  # 내용 비교
                    file_path = inspect.getfile(global_frames[gvar])
                    file_name = os.path.basename(file_path)
                    # var_data(path, var_str_idx, addr)
                    print("gvar: ", name)
                    gvars.append([file_name, name, gvar])
        except KeyError:
            ...
            
arrays = []
def find_array_names(arr):
    if eudplib_type == 0:
        ... # not support
    elif eudplib_type == 1:
        try:
            for name, value in frames_array[arr].f_locals.items():
                if inspect.getfile(frames_array[arr]).endswith(".py"):
                    continue
                if value is arr:  # 내용 비교
                    file_path = inspect.getfile(frames_array[arr])
                    file_name = os.path.basename(file_path)
                    # var_data(path, func_str_idx, var_str_idx, addr, size)
                    print("arr: ", name, arr)
                    arrays.append([file_name, frames_array[arr].f_code.co_name, name, arr, arr.length])
        except KeyError:
            ...
            
garrays = []
def find_cgfw_names(garr):
    if eudplib_type == 0:
        ... # not support
    elif eudplib_type == 1:
        try:
            for name, value in global_frames_array[garr].f_locals.items():
                if inspect.getfile(global_frames_array[garr]).endswith(".py"):
                    continue
                if value is garr:  # 내용 비교
                    file_path = inspect.getfile(global_frames_array[garr])
                    file_name = os.path.basename(file_path)
                    # var_data(path, type_str, var_str_idx, addr, size)
                    type_name = type(garr).__name__
                    if type_name == "EUDArray":
                        garrays.append([file_name, type_name, name, garr, garr.length])
                    elif type_name == "EUDVArray" or type_name == "PVariable":
                        garrays.append([file_name, type_name, name, garr, garr._size])
                    elif type_name == "StringBuffer":
                        garrays.append([file_name, type_name, name, garr, garr.capacity])
                    elif type_name == "EUDObject":
                        garrays.append([file_name, type_name, name, garr, garr.GetDataSize()])
                    elif type_name == "ConstExpr": # EPD has smashed the object, so cannot know the lengthh
                        garrays.append([file_name, type_name, name, garr, 0])
        except KeyError:
            ...

oldfbody = ef.EUDFuncN._create_func_body
def registerEUDFunc(self, *args, **kwargs):
    global isEUDFunc
    if eudplib_type == 0:
        isEUDFunc = True
        oldfbody(self, *args, **kwargs)
        isEUDFunc = False
    elif eudplib_type == 1:
        isEUDFunc = True
        oldfbody(self, *args, **kwargs)
        isEUDFunc = False
ef.EUDFuncN._create_func_body = registerEUDFunc

bufferEPD = EPD(Db(1008))
signatureEPD = EPD(Db("TEMPjknOSDIfnwlnlSNDKlnfkopqfnkLDNSFEDAC\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1")) # 40bytes
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
##  string
##  4b      : STRS
##  4b      : size of strings
##  strs    : func strings

### variable table
##  4b      : VART
##  4b      : size of variable
### 16b*vars: var_data(path, func_str_idx, var_str_idx, addr)

### global variable table
##  4b      : GVRT
##  4b      : size of variable
### 12b*vars: var_data(path, var_str_idx, addr)

### array table
##  4b      : ARRT
##  4b      : size of array
### 20b*vars: var_data(path, func_str_idx, var_str_idx, addr, size)

### cgfw table
##  4b      : GART
##  4b      : size of array
### 16b*vars: garr_data(path, type_str, var_str_idx, addr, size)

### location table
##  4b      : MRND
##  4b      : size of locations
##  8b*locs : loc_data(loc name, loc number)
#####

strs           = []
var_data       = []
gvar_data      = []
arr_data       = []
garr_data      = []
mrgn_data      = []
stringDb   = Forward()
varDataDb  = Forward()
gvarDataDb = Forward()
arrDataDb  = Forward()
garrDataDb = Forward()
MRGNDataDb = Forward()
def process_vars():
    # var_data(path, func_str_idx, var_str_idx, addr)
    for e in vars:
        path_idx = 0
        if e[0] not in strs:
            strs.append(e[0])
            path_idx = len(strs) - 1
        else:
            path_idx = strs.index(e[0])

        func_idx = 0
        if e[1] not in strs:
            strs.append(e[1])
            func_idx = len(strs) - 1
        else:
            func_idx = strs.index(e[1])
        strs.append(e[2])
        var_data.append([path_idx, func_idx, len(strs)-1, e[3]])
        
def process_gvars():
    # gvar_data(path, var_str_idx, addr)
    for e in gvars:
        path_idx = 0
        if e[0] not in strs:
            strs.append(e[0])
            path_idx = len(strs) - 1
        else:
            path_idx = strs.index(e[0])
            
        strs.append(e[1])
        gvar_data.append([path_idx, len(strs)-1, e[2]])
        
def process_arrs():
    # arr_data(path, func_str_idx, var_str_idx, addr, size)
    for e in arrays:
        path_idx = 0
        if e[0] not in strs:
            strs.append(e[0])
            path_idx = len(strs) - 1
        else:
            path_idx = strs.index(e[0])
            
        func_idx = 0
        if e[1] not in strs:
            strs.append(e[1])
            func_idx = len(strs) - 1
        else:
            func_idx = strs.index(e[1])
        strs.append(e[2])
        arr_data.append([path_idx, func_idx, len(strs)-1, e[3], e[4]])
        
def process_garrs():
    # garr_data(path, type_str, var_str_idx, addr, size)
    for e in garrays:
        path_idx = 0
        if e[0] not in strs:
            strs.append(e[0])
            path_idx = len(strs) - 1
        else:
            path_idx = strs.index(e[0])
            
        type_idx = 0
        if e[1] not in strs:
            strs.append(e[1])
            type_idx = len(strs) - 1
        else:
            type_idx = strs.index(e[1])
            
        strs.append(e[2])
        print("CGFW: ", [path_idx, type_idx, len(strs)-1, e[3], e[4]])
        garr_data.append([path_idx, type_idx, len(strs)-1, e[3], e[4]])

import eudplib.core.mapdata.stringmap as sm
def process_mrgn():
    locmap = sm.locmap._s2id # {locname: locidx}
    mrgn_idx = 0
    for k, v in locmap.items():
        ks = k.decode('utf-8')
        if ks not in strs:
            strs.append(ks)
            mrgn_idx = len(strs) - 1
        else:
            mrgn_idx = strs.index(ks)
        mrgn_data.append([mrgn_idx, v])
    return mrgn_data

screenDbEPD = EPD(Db(8)) # screen top, left(0 ~ 0x1FFF) 4byte each
mapPathDbEPD = EPD(Db(260))
pathSignatureEPD = EPD(Db("TEMPNkdfhLpZmqWnRbZlfhInbpQYtZBwjeOqmPlW"))
def modify_map_name():
    f_repmovsd_epd(EPD(0x57FD3C), pathSignatureEPD, 40)
    header = struct.unpack("<I", b"Gong")[0]
    DoActions(SetMemoryEPD(EPD(0x57FD3C), SetTo, header))
        
def save_data():
    process_vars()
    process_gvars()
    process_arrs()
    process_garrs()
    process_mrgn()

    # restore header
    act = []
    header = struct.unpack("<I", b"Gong")[0]
    act.append(SetMemoryEPD(signatureEPD, SetTo, header))
    # string table
    _stringBinaray = b''
    for str in strs:
        _stringBinaray += str.encode('utf-8')+ b'\x00'
    stringBinary = b'STRS' + struct.pack("<I", len(_stringBinaray))
    stringBinary += _stringBinaray
    stringDb << Db(stringBinary)

    #####
    ### variable table
    ##  4b      : VART
    ##  4b      : size of variable
    ### 16b*vars: var_data(path, func_str_idx, var_str_idx, addr)
    # var data table
    _funcVarDataBinaray = b''
    for i, var in enumerate(var_data):
        _funcVarDataBinaray += struct.pack("<I", var[0])
        _funcVarDataBinaray += struct.pack("<I", var[1])
        _funcVarDataBinaray += struct.pack("<I", var[2])
        _funcVarDataBinaray += b'\0\0\0\0'
        act.append(SetMemoryEPD(EPD(varDataDb)+2+3+4*i, SetTo, var[3].getValueAddr()))
    funcVarDataBinary = b'VART' + struct.pack("<I", len(_funcVarDataBinaray))
    funcVarDataBinary += _funcVarDataBinaray
    varDataDb << Db(funcVarDataBinary)

    ### global variable table
    ##  4b      : GVRT
    ##  4b      : size of variable
    ### 12b*vars: gvar_data(path, var_str_idx, addr)
    _funcGVarDataBinaray = b''
    for i, var in enumerate(gvar_data):
        _funcGVarDataBinaray += struct.pack("<I", var[0])
        _funcGVarDataBinaray += struct.pack("<I", var[1])
        _funcGVarDataBinaray += b'\0\0\0\0'
        act.append(SetMemoryEPD(EPD(gvarDataDb)+2+2+3*i, SetTo, var[2].getValueAddr()))
    funcGVarDataBinary = b'GVRT' + struct.pack("<I", len(_funcGVarDataBinaray))
    funcGVarDataBinary += _funcGVarDataBinaray
    gvarDataDb << Db(funcGVarDataBinary)

    ### array table
    ##  4b      : ARRT
    ##  4b      : size of array
    ### 20b*vars: arr_data(path, func_str_idx, var_str_idx, addr, size)
    _funcArrDataBinaray = b''
    for i, var in enumerate(arr_data):
        _funcArrDataBinaray += struct.pack("<I", var[0])
        _funcArrDataBinaray += struct.pack("<I", var[1])
        _funcArrDataBinaray += struct.pack("<I", var[2])
        _funcArrDataBinaray += b'\0\0\0\0'
        _funcArrDataBinaray += struct.pack("<I", var[4])
        act.append(SetMemoryEPD(EPD(arrDataDb)+2+3+5*i, SetTo, var[3]))
    funcArrDataBinaray = b'ARRT' + struct.pack("<I", len(_funcArrDataBinaray))
    funcArrDataBinaray += _funcArrDataBinaray
    arrDataDb << Db(funcArrDataBinaray)

    ### global array table
    ##  4b      : GART
    ##  4b      : size of array
    ### 16b*vars: garr_data(path, type_str, var_str_idx, addr, size)
    _funcGArrDataBinaray = b''
    for i, var in enumerate(garr_data):
        _funcGArrDataBinaray += struct.pack("<I", var[0])
        _funcGArrDataBinaray += struct.pack("<I", var[1])
        _funcGArrDataBinaray += struct.pack("<I", var[2])
        _funcGArrDataBinaray += b'\0\0\0\0'
        _funcGArrDataBinaray += struct.pack("<I", var[4])
        act.append(SetMemoryEPD(EPD(garrDataDb)+2+3+5*i, SetTo, var[3]))
    funcGArrDataBinaray = b'GART' + struct.pack("<I", len(_funcGArrDataBinaray))
    funcGArrDataBinaray += _funcGArrDataBinaray
    garrDataDb << Db(funcGArrDataBinaray)

    ### location table
    ##  4b      : MRNT
    ##  4b      : size of locations
    ##  8b*locs : loc_data(loc name, loc str index)
    # mrgn string mapping table
    _funcMRGNDataBinaray = b''
    for i, str in enumerate(mrgn_data):
        _funcMRGNDataBinaray += struct.pack("<I", str[0])
        _funcMRGNDataBinaray += struct.pack("<I", str[1])
    funcMRGNDataBinary = b'MRNT' + struct.pack("<I", len(_funcMRGNDataBinaray))
    funcMRGNDataBinary += _funcMRGNDataBinaray
    MRGNDataDb << Db(funcMRGNDataBinary)
    
    # save screen data
    f_repmovsd_epd(screenDbEPD, EPD(0x628470), 1)
    f_repmovsd_epd(screenDbEPD+1, EPD(0x628448), 1)
    if EUDExecuteOnce()():
        # save map path data
        # f_repmovsd_epd(mapPathDbEPD, EPD(0x57FD3C), 65)
        modify_map_name()
        DoActions(act)
    EUDEndExecuteOnce()


def dwread(ba, offset):
    return int.from_bytes(ba[offset:offset+4], byteorder="little")
def bread(ba, offset):
    return ba[offset]

def init_signature():
    DoActions([
        SetMemoryEPD(signatureEPD+10, SetTo, signatureEPD),
        SetMemoryEPD(signatureEPD+11, SetTo, bufferEPD),
        SetMemoryEPD(signatureEPD+12, SetTo, stringDb),
        SetMemoryEPD(signatureEPD+13, SetTo, varDataDb),
        SetMemoryEPD(signatureEPD+14, SetTo, gvarDataDb),
        SetMemoryEPD(signatureEPD+15, SetTo, arrDataDb),
        SetMemoryEPD(signatureEPD+16, SetTo, garrDataDb),
        SetMemoryEPD(signatureEPD+17, SetTo, MRGNDataDb),
        SetMemoryEPD(signatureEPD+18, SetTo, screenDbEPD),
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
    if compare_versions(eudplibVersion(), "0.77.9"):
        eudplib_type = 1
        
    init_signature()
    # ev.EUDVariable.__lshift__ = _patched_lshift
    sendPacket(0xAB, 0x00FF00FF, 0xAAAABBBB, 0xBBBBCCCC)

def beforeTriggerExec():
    ...
    #f_eprintln(epd2s(signatureEPD))
    
def afterTriggerExec():
    for var in collected_vars:
        find_var_names(var)
    for arr in collected_arrays:
        find_array_names(arr)
    for gvar in collected_gvars:
        find_global_var_names(gvar)
    for garr in collected_garray:
        find_cgfw_names(garr)
    save_data()
    
    # print screen
    # f_simpleprint(hptr(f_dwread_epd(screenDbEPD)), " ", hptr(f_dwread_epd(screenDbEPD+1)))
    
    # print locations
    # f_simpleprint(hptr(f_dwread(0x58DC60)), " ", hptr(f_dwread(0x58DC64)), " ", hptr(f_dwread(0x58DC68)), " ", hptr(f_dwread(0x58DC6C)), " ")
