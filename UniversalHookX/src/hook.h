#ifndef HOOK_H
#define HOOK_H

void* FindFuncPattern(unsigned char* base, size_t size, const char* pattern, const char* mask);
void HookFunction(void* targetFunc, void* myFunc);

#endif