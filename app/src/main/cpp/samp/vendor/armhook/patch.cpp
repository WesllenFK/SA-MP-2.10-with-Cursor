#include "../main.h"
#include "patch.h"

#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>

// Máscaras de página por arquitetura
#if VER_x32
    #define PAGE_MASK 0xFFFFF000
#else
    #define PAGE_MASK 0xFFFFFFFFFFFFF000ULL
#endif

// Flag global para indicar se estamos em modo W^X estrito (Android 15+)
// Quando true, ReFuck() precisa ser chamado após escrever patches
static bool s_bWXStrictMode = false;

/**
 * Calcula o tamanho total das páginas afetadas por uma região de memória
 */
static size_t CalculatePageSpan(uintptr_t addr, size_t len)
{
    uintptr_t page_start = addr & PAGE_MASK;
    uintptr_t page_end = (addr + len + PAGE_SIZE - 1) & PAGE_MASK;
    size_t total = page_end - page_start;
    return (total < PAGE_SIZE) ? PAGE_SIZE : total;
}

/**
 * UnFuck - Desprotege memória para escrita
 * 
 * Fluxo W^X compatível:
 * - Android < 15: Tenta RWX (mais simples, memória fica executável)
 * - Android 15+:  Usa apenas RW (requer ReFuck() depois para restaurar X)
 * 
 * @param addr Endereço de memória a desproteger
 * @param len  Tamanho da região em bytes
 */
void CHook::UnFuck(uintptr_t addr, size_t len)
{
    uintptr_t page_start = addr & PAGE_MASK;
    size_t page_span = CalculatePageSpan(addr, len);
    
    // Tenta RWX primeiro (funciona em Android < 15)
    if(mprotect((void*)page_start, page_span, PROT_READ | PROT_WRITE | PROT_EXEC) == 0)
    {
        s_bWXStrictMode = false;
        return;  // Sucesso! Android antigo ou permissivo
    }
    
    // Fallback: Apenas RW (Android 15+ com W^X estrito)
    // O chamador DEVE chamar ReFuck() após escrever os patches
    if(mprotect((void*)page_start, page_span, PROT_READ | PROT_WRITE) == 0)
    {
        s_bWXStrictMode = true;
        return;
    }
    
    // Ambos falharam - log de erro
    LOGE("UnFuck failed for 0x%lx (len=%zu): %s", (unsigned long)addr, len, strerror(errno));
}

/**
 * ReFuck - Restaura memória para executável após escrita
 * 
 * Chamado após escrever patches para restaurar permissão de execução.
 * Essencial em modo W^X estrito (Android 15+).
 * 
 * Se não estamos em modo W^X estrito (Android < 15), esta função
 * não faz nada pois a memória já está RWX.
 * 
 * @param addr Endereço de memória a restaurar
 * @param len  Tamanho da região em bytes
 */
void CHook::ReFuck(uintptr_t addr, size_t len)
{
    // Se não estamos em modo W^X estrito, não precisa fazer nada
    // A memória já está RWX do UnFuck()
    if(!s_bWXStrictMode)
        return;
    
    uintptr_t page_start = addr & PAGE_MASK;
    size_t page_span = CalculatePageSpan(addr, len);
    
    // Restaura para RX (executável, não gravável)
    if(mprotect((void*)page_start, page_span, PROT_READ | PROT_EXEC) != 0)
    {
        LOGE("ReFuck failed for 0x%lx (len=%zu): %s", (unsigned long)addr, len, strerror(errno));
    }
}

void CHook::InitHookStuff()
{
    lib = dlopen("libGTASA.so", RTLD_LAZY);
}

