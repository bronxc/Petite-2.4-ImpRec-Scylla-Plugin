#include <Windows.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>

#pragma warning(disable: 4146)

#define DLLEXPORT                __declspec(dllexport)
#define IMP_REC_OK               200
#define IMP_REC_MAP_ERROR        201
#define IMP_REC_MAP_SIZE_INVALID 203
#define IMP_REC_BAD_READ_POINTER 205

DLLEXPORT DWORD Trace(DWORD hFileMap,
                      DWORD dwSizeMap,
                      DWORD dwTimeOut,
                      DWORD dwToTrace,
                      DWORD dwExactCall);

// Parameters:
// -----------
// <value>       : Value to be rotated
// <count>       : Number of bits to rotate
//
uint32_t rotate_left(uint32_t Value, unsigned int Count) {
  const unsigned int mask = (CHAR_BIT*sizeof(Value) - 1);
  Count &= mask;
  return (Value << Count) | (Value >> ((-Count) & mask));
}

// Parameters:
// -----------
// <hFileMap>    : HANDLE of the mapped file
// <dwSizeMap>   : Size of that mapped file
// <dwTimeOut>   : TimeOut of ImpREC in Options
// <dwToTrace>   : Pointer to trace (in VA)
// <dwExactCall> : EIP of the exact call (in VA)
//
// Petite 2.4 API redirection stub example
// =======================================
// 0093A005  68 BB3BD0BB  push BBD03BBB
// 0093A00A  C10424 11    rol dword ptr[esp], 11
// 0093A00E  C3           retn

DWORD Trace(DWORD hFileMap,
            DWORD dwSizeMap,
            DWORD dwTimeOut,
            DWORD dwToTrace,
            DWORD dwExactCall) {
  DWORD ret = 0;
  DWORD* dwPtrOutput = (DWORD*)MapViewOfFile((HANDLE)hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
  if (dwPtrOutput) {
    if (dwSizeMap >= 4) {
      if (!IsBadReadPtr((void*)dwToTrace, 4)) {
        BYTE* to_trace = (BYTE*)dwToTrace;
        if (*to_trace == 0x68) {
          DWORD address = *((DWORD*)(to_trace + 1));
          to_trace += 8;
          BYTE rot_val = *to_trace;
          char msg[64] = "";
          _snprintf(msg, sizeof(msg), "[Petite 2.4] Address: 0x%08x ROT: 0x%02x\n", address, rot_val);
          OutputDebugString(msg);
          *dwPtrOutput = rotate_left(address, rot_val);
          ret = IMP_REC_OK;
        }
      }  else {
        ret = IMP_REC_BAD_READ_POINTER;
      }
    } else {
      ret = IMP_REC_MAP_SIZE_INVALID;
    }
  } else {
    ret = IMP_REC_MAP_ERROR;
  }
  if (dwPtrOutput) {
    UnmapViewOfFile((LPCVOID)dwPtrOutput);
    CloseHandle((HANDLE)hFileMap);
  }
  return ret;
}

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved) {
  return TRUE;
}
