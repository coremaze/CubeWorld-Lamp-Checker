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

_declspec(naked) void DLL_EXPORT RememberRarities(){
    asm("mov eax, [ebp - 0x350]"); //Rarity level
    asm("movb [_rarities-1 + eax], 1");
    asm("mov eax, 1");
    asm("mov [ebp - 0x10C], ax");

    asm("mov ecx, [_base]"); //jump back
    asm("add ecx, 0xFDA95");
    asm("jmp ecx");
}

void DLL_EXPORT ExitAfterLamps(){
    WriteRaritiesToFile();
    ExitProcess(0);
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

            WriteJMP((BYTE*)(base + 0xFDA89), (BYTE*)&RememberRarities); //This code writes the lamp rarities to a file


            /*
            Exit as soon as the lamps are done being calculated.
            In addition, save the lamp data to a file.
            The server shutting down should guarantee that the lamp data was calculated, since it occurs only if they are.
            */
            WriteJMP((BYTE*)(base + 0xFDACE), (BYTE*)&ExitAfterLamps);

            break;
;
    }
    return TRUE;
}
