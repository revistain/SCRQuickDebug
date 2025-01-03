### 241019 signature 0.0.5
###################### version ##########################
from eudplib import *
eudplib_type = 0
def compare_versions(version1, version2):
    v1_parts = list(map(int, version1.split('.')[:2]))
    v2_parts = list(map(int, version2.split('.')[:2]))
    return v1_parts > v2_parts
if compare_versions(eudplibVersion(), "0.77.9"):
    eudplib_type = 1
ep_assert(eudplib_type != 0, "euddraft 0.10.0.0 이상 버전을 사용하여 주세요 !!")
#########################################################

##################### settings ##########################
# isTraced = int(settings.get("trace", 0))
#########################################################

import eudplib.core.variable.eudv as ev
import eudplib.core.eudfunc.eudfuncn as efn
import eudplib.core.eudfunc.eudf as ef
import eudplib.collections.eudarray as ea
import eudplib.epscript.helper as hp
import struct
import inspect
import sys
import os
        
isEUDFunc = False
collected_vars = []
collected_arrays = []
collected_gvars = []
collected_garray = []
frames = {}
global_frames = {}
frames_array = {}
global_frames_array = {}

######################## trace ###########################
functrace_stack = Db(1024*4)
functrace_count = EUDVariable(0)
functrace_trig = EUDXVariable(EPD(functrace_stack), SetTo, 0, 0xFFFFFFFF)
collected_functrace = []

isTimeTracing = EUDLightBool()
timestamp_stack = Db(12288) # TODO: add check
timestamp_count = EUDVariable(0)
timestamp_trig = EUDXVariable(EPD(timestamp_stack), SetTo, 0, 0xFFFFFFFF)
timestamp_time = EUDXVariable(EPD(timestamp_stack), SetTo, 0, 0xFFFFFFFF)
entry_trig = Forward()
jmp_branch = Forward()
_jmp_branch = Forward()
return_trig = Forward()

### deprecated: 0x51CE8C is not refreshed in triggerloop
# if PushTriggerScope():
#     entry_trig << NextTrigger()
#     VProc([timestamp_trig], [
#         timestamp_count.AddNumber(2),
#         timestamp_trig.AddDest(1),
#     ])

#     rev_system_time = f_dwread_epd(EPD(0x51CE8C))
#     VProc([rev_system_time, timestamp_trig], [
#         rev_system_time.SetDest(EPD(timestamp_trig.getValueAddr())),
#         timestamp_trig.AddDest(1),
#     ])
#     return_trig << RawTrigger()
# PopTriggerScope()

if PushTriggerScope():
    entry_trig << NextTrigger()
    VProc([timestamp_trig, timestamp_time], [
        timestamp_count.AddNumber(2),
        timestamp_trig.AddDest(1),
        timestamp_time.AddDest(2),
    ])
    return_trig << RawTrigger(actions=[timestamp_trig.AddDest(1)])
PopTriggerScope()

def profile(frame, event, arg):
    global stackdepth, entry_trig, jmp_branch, _jmp_branch
    if event == 'call':
        if not frame.f_code.co_filename.endswith(".eps") or frame.f_code.co_name.startswith("_"):
            return profile
        if len(collected_functrace) == 0:
            DoActions(
                timestamp_count.SetNumber(0),
                timestamp_trig.SetDest(EPD(timestamp_stack)),
                timestamp_time.SetDest(EPD(timestamp_stack)),
            )

        current_index = 0
        lineno = frame.f_lineno - 3
        msg = "%s|%s|%s" % (frame.f_code.co_filename.split('TriggerEditor', 1)[-1], frame.f_code.co_name, lineno)
        if msg in collected_functrace:
            current_index = collected_functrace.index(msg)
        else:
            collected_functrace.append(msg)
            current_index = len(collected_functrace) - 1
            
        end_trig = Forward()
        jmp_branch = Forward()
        jmp_branch << RawTrigger(
            conditions=[isTimeTracing.IsSet()],
            actions=[
                SetNextPtr(jmp_branch, entry_trig),
                SetNextPtr(return_trig, end_trig),
                timestamp_trig.SetNumber(current_index)
            ]
        )

        end_trig << NextTrigger()
        VProc([functrace_trig], [
            SetNextPtr(jmp_branch, end_trig),
            functrace_count.AddNumber(1),
            functrace_trig.AddDest(1),
            functrace_trig.SetNumber(current_index),
        ])
        _self = frame.f_back.f_back.f_locals['self']
        _self._signature_isprofiled = current_index
    elif event == 'return':
        if not frame.f_code.co_filename.endswith(".eps") or frame.f_code.co_name.startswith("_"):
            return profile
        lineno = frame.f_lineno

        _self = frame.f_back.f_back.f_locals['self']
        end_trig = Forward()
        _jmp_branch = Forward()
        _jmp_branch << RawTrigger(
            conditions=[isTimeTracing.IsSet()],
            actions=[
                SetNextPtr(_jmp_branch, entry_trig),
                SetNextPtr(return_trig, end_trig),
                timestamp_trig.SetNumber(_self._signature_isprofiled)
            ]
        )
        end_trig << NextTrigger()
        DoActions(
            SetNextPtr(_jmp_branch, end_trig),
            functrace_count.SubtractNumber(1),
            functrace_trig.SubtractDest(1)
        )
    return profile

original_EUDReturn = efn.EUDFuncN._add_return
def _patched_EUDReturn(*args):
    if not hasattr(efn._current_compiled_func, '_signature_isprofiled'):
        return original_EUDReturn(*args)
    
    ################################################
    end_trig = Forward()
    _jmp_branch = Forward()
    _jmp_branch << RawTrigger(
        conditions=[isTimeTracing.IsSet()],
        actions=[
            SetNextPtr(_jmp_branch, entry_trig),
            SetNextPtr(return_trig, end_trig),
            timestamp_trig.SetNumber(efn._current_compiled_func._signature_isprofiled)
        ]
    )
    end_trig << NextTrigger()
    DoActions(
        SetNextPtr(_jmp_branch, end_trig),
        functrace_count.SubtractNumber(1),
        functrace_trig.SubtractDest(1)
    )
    original_EUDReturn(*args)
efn.EUDFuncN._add_return = _patched_EUDReturn
#########################################################

original_init = ev.EUDVariable.__init__
def _patched_init(self, *args, **kwargs):
    if eudplib_type == 0:
        ... # no
    elif eudplib_type == 1:
        prev_frame = inspect.currentframe().f_back
        fname = prev_frame.f_code.co_name
        if fname:
            if fname == "_TYLV":
                # print("_TYLV: ", prev_frame.f_back.f_code.co_name)
                collected_vars.append(self)
                frames[self] = prev_frame.f_back
            elif fname == "_TYSV":
                # print("_TYSV: ", prev_frame.f_back.f_code.co_name)
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
                # print("_TYGL: ", var)
                prev_frame = inspect.currentframe().f_back
                global_frames[var] = prev_frame
        else:
            # print("_TYGL: ", var_list)
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
                    # print("gvar: ", name)
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
                    # print("arr: ", name, arr)
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
                        garrays.append([file_name, type_name, name, garr.epd, garr.capacity])
                    elif type_name == "Db":
                        garrays.append([file_name, type_name, name, garr, garr.GetDataSize()])
                    elif type_name == "EUDObject":
                        garrays.append([file_name, type_name, name, garr, garr.GetDataSize()])
                    elif type_name == "ConstExpr": # EPD has smashed the object, so cannot know the lengthh
                        garrays.append([file_name, type_name, name, garr, 0])
        except KeyError:
            ...

## 중간에 있는 숫자 00??가 signature의 버전입니다. 이 버전과 dll의 버전이 일치해야함
signatureEPD = EPD(Db("TEMPjknOSDIfnwlnlSNDKlnfkopqYwZL0005EDAC\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1")) # 40bytes

##### custom defined sections
##  string
##  4b      : STRS
##  4b      : size of strings
##  strs    : all strings

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
##  4b      : MRNT
##  4b      : size of locations
##  8b*locs : loc_data(loc name, loc number)
####
strs           = []
var_data       = []
gvar_data      = []
arr_data       = []
garr_data      = []
mrgn_data      = []
bufferEPD = Db(0) #EPD(Db(2016)) # 1012, 1004 for send, receive
stringDb   = Forward()
varDataDb  = Forward()
gvarDataDb = Forward()
arrDataDb  = Forward()
garrDataDb = Forward()
MRGNDataDb = Forward()
SPRPDataDb = Forward()
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
        # print("CGFW: ", [path_idx, type_idx, len(strs)-1, e[3], e[4]])
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

def decode_map_title(map_title):
    try:
        # 먼저 UTF-8로 시도
        return map_title.decode('utf-8')
    except UnicodeDecodeError:
        try:
            # UTF-8이 실패하면 CP949로 시도
            return map_title.decode('cp949')
        except UnicodeDecodeError:
            # 두 가지 인코딩 모두 실패할 경우, 에러 처리 방식 선택
            return map_title.decode('utf-8', errors='replace')  # 혹은 'ignore'
        
WireFrameDb = Forward()# Db(8) #12b: isSingle(bool), map_title_offset, map_title_idx
def process_wf_colors():
    chkt = GetChkTokenized()
    tileset_index = b2i2(chkt.getsection("ERA ")) & 7
    dimensions_are_256_256 = list(chkt.getsection("DIM ")) == [0, 1, 0, 1]
    lobby_tile_offset = 123 + (0 if tileset_index == 0 else 1) + (0 if dimensions_are_256_256 else 2) - 2
    
    SPRP = bytearray(chkt.getsection("SPRP"))
    map_index = b2i2(SPRP[0:2])
    map_title = sm.get_string_map().GetString(map_index)
    strs.append(decode_map_title(map_title))
    map_title_idx = len(strs) - 1
    
    isSingle = f_strlen(0x6D0F78)
    _wfDataBinary = b'\0\0\0\0'
    _wfDataBinary += struct.pack("<I", lobby_tile_offset)
    _wfDataBinary += struct.pack("<I", map_title_idx)
    f_dwwrite_epd(EPD(WireFrameDb)+2, isSingle)
    
    wfDataBinary = b'WFST' + struct.pack("<I", len(_wfDataBinary))
    wfDataBinary += _wfDataBinary
    WireFrameDb << Db(wfDataBinary)

saved_functrace_str_offset = 0
def process_functrace():
    global saved_functrace_str_offset
    saved_functrace_str_offset = len(strs)
    for functrace in collected_functrace:
        strs.append(functrace)

FuncTraceDb = Forward() # Db: traceStackDb, traceCount, strs offset, timestampDb, timestampCount, timestamptimetrig
screenDbEPD = EPD(Db(56)) # screen top, left(0 ~ 0x1FFF), selected unitindex(x12)
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
    process_wf_colors()
    process_functrace()
    
    # restore header
    act = []
    header = struct.unpack("<I", b"Gong")[0]
    act.append(SetMemoryEPD(signatureEPD, SetTo, header))
    
    ### string
    ##  4b      : STRS
    ##  4b      : size of strings
    ### strs    : func strings
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
    
    ### FTCD
    global saved_functrace_str_offset
    _functraceBinary = b'\0\0\0\0'
    _functraceBinary += b'\0\0\0\0'
    _functraceBinary += struct.pack("<I", saved_functrace_str_offset)
    _functraceBinary += b'\0\0\0\0'
    _functraceBinary += b'\0\0\0\0'
    _functraceBinary += b'\0\0\0\0'
    act.append(SetMemoryEPD(EPD(FuncTraceDb)+2, SetTo, functrace_stack))
    act.append(SetMemoryEPD(EPD(FuncTraceDb)+3, SetTo, functrace_count.getValueAddr()))
    act.append(SetMemoryEPD(EPD(FuncTraceDb)+5, SetTo, timestamp_stack))
    act.append(SetMemoryEPD(EPD(FuncTraceDb)+6, SetTo, timestamp_count.getValueAddr()))
    act.append(SetMemoryEPD(EPD(FuncTraceDb)+7, SetTo, timestamp_time.getValueAddr()))
    
    functraceBinary = b'FTCD' + struct.pack("<I", len(_functraceBinary))
    functraceBinary += _functraceBinary
    FuncTraceDb << Db(functraceBinary)
    
    
    # save screen data
    f_repmovsd_epd(screenDbEPD, EPD(0x628470), 1)
    f_repmovsd_epd(screenDbEPD+1, EPD(0x628448), 1)
    # save selected units offset
    f_repmovsd_epd(screenDbEPD+2, EPD(0x6284B8), 12)
    if EUDExecuteOnce()():
        modify_map_name()
        DoActions(act)
    EUDEndExecuteOnce()


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
        SetMemoryEPD(signatureEPD+19, SetTo, WireFrameDb),
        SetMemoryEPD(signatureEPD+20, SetTo, FuncTraceDb),
    ])
    

def onPluginStart():
    init_signature()
    DoActions(isTimeTracing.Set())

def beforeTriggerExec():
    sys.setprofile(profile)
    ...
    
def afterTriggerExec():
    sys.setprofile(None)
    for var in collected_vars:
        find_var_names(var)
    for arr in collected_arrays:
        find_array_names(arr)
    for gvar in collected_gvars:
        find_global_var_names(gvar)
    for garr in collected_garray:
        find_cgfw_names(garr)
    save_data()
    