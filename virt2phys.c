#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int log2(int n){
    int r=0;
    while(n>>=1) r++;
    return r;
}
int main(int argc, char* argv[]){
if(argc < 3){
       printf("Usage: virt2phys <page-table-file> <virtual-address>\n");
       return 0;
   }     
    else if(argc > 3){
        printf("Usage: virt2phys <page-table-file> <virtual-address>\n");
        return 0;
    }
    else{
        // Read File
        FILE *fpointer;
        fpointer = fopen(argv[1], "r");  
        int addr_bits;
        int page_size;
        int ppn;
        fscanf(fpointer, "%d", &addr_bits);
        fscanf(fpointer, "%d", &page_size);

        int page_off_bits = log2(page_size);
        int virt_pnum_bits = addr_bits - page_off_bits;
        int virt_pnum_pages = 1 << virt_pnum_bits;
        int virt_addr;
        sscanf(argv[2], "%x", &virt_addr);
        // int virt_addr = atoi(argv[2]);
        int mask = ((1<<page_off_bits)-1);
        int page_off = virt_addr & mask;
        int virt_pnum = (virt_addr >> page_off_bits);

        // Check virt_pnum
        for(int i = 0; i< virt_pnum_pages; i++){
            fscanf(fpointer, "%d", &ppn);
            if(i==virt_pnum){
                if(ppn == -1){
                    printf("PAGEFAULT\n");
                    return 0;
                }
                int phy_addr = (ppn << page_off_bits)| page_off;
                printf("%x\n", phy_addr);
                break;
            }
        }
        fclose(fpointer);
        return 0;
    }    
}