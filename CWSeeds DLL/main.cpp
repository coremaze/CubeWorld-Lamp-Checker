#include "main.h"
#include <iostream>
#include <fstream>

UINT_PTR base;
char rarities[4] = {0, 0, 0, 0};

_declspec(naked) void DLL_EXPORT ForceChunkGen(){
    asm("mov ecx, [ebp-0x4C]"); //Getting World object
    asm("mov ecx, [ecx]");
    asm("push 0x8023"); //Coordinates for item shop chunk
    asm("push 0x8024");

    asm("mov edi, [_base]");
    asm("add edi, 0x118630"); //call GenerateChunk
    asm("call edi");

    asm("mov edi, [_base]"); //jump back
    asm("add edi, 0x1497DF");
    asm("jmp edi");


}

void DLL_EXPORT WriteRaritiesToFile(){
    std::ofstream rarityFile ("results.dat", std::ofstream::out | std::ofstream::binary);
    rarityFile.write (rarities, 4);
    rarityFile.close();
}
DWORD WriteRaritiesToFileAddr = (DWORD)&WriteRaritiesToFile;


_declspec(naked) void DLL_EXPORT SaveRarities(){
    asm("mov eax, [ebp - 0x350]"); //Rarity level
    asm("movb [_rarities-1 + eax], 1");
    asm("mov eax, 1");
    asm("mov [ebp - 0x10C], ax");

    asm("call [_WriteRaritiesToFileAddr]");

    asm("mov ecx, [_base]"); //jump back
    asm("add ecx, 0xFDA95");
    asm("jmp ecx");
}

_declspec(naked) void DLL_EXPORT ImmediatelyExit(){
    asm("mov eax, [ebx+8]");
    asm("add esp, 4");
    asm("mov word ptr [eax], 0"); //Set some bytes to cause shutdown


    asm("mov ecx, [_base]"); //jump back
    asm("add ecx, 0x149AF8");
    asm("cmp byte ptr [eax], 0"); //original comparison
    asm("jmp ecx");
}
void WriteJMP(BYTE* location, BYTE* newFunction){
	DWORD dwOldProtection;
	VirtualProtect(location, 5, PAGE_EXECUTE_READWRITE, &dwOldProtection);
    location[0] = 0xE9; //jmp
    *((DWORD*)(location + 1)) = (DWORD)(((unsigned INT32)newFunction - (unsigned INT32)location) - 5);
	VirtualProtect(location, 5, dwOldProtection, &dwOldProtection);
}


extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    base = (UINT_PTR)GetModuleHandle(NULL);
    switch (fdwReason)
    {

        case DLL_PROCESS_ATTACH:

            WriteJMP((BYTE*)(base + 0x1497D9), (BYTE*)&ForceChunkGen); //This code forces the server to generate the correct chunk

            WriteJMP((BYTE*)(base + 0xFDA89), (BYTE*)&SaveRarities); //This code writes the lamp rarities to a file

            WriteJMP((BYTE*)(base + 0x149AEF), (BYTE*)&ImmediatelyExit); //This code makes the server exit after its first terrain generation loop.
            break;
;
    }
    return TRUE;
}
