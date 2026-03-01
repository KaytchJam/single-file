#include <iostream>
#include <cmath>

/** VIRTUAL MEMORY EMULATOR */

// CONSTANTS
constexpr int PAGE_SIZE = 4;
constexpr int NUM_HARDWARE_PAGES = 4;
constexpr int PROCESS_SIZE = 8; // INDICES 

// MASKS
constexpr int VPN_MASK = 0x4;
constexpr int OFFSET_MASK = 0x3;

// MAXIMUM PROCESSES
constexpr int MAX_PROCESSES = 10;

// "REAL" "memory"
char memory[NUM_HARDWARE_PAGES * PAGE_SIZE] = { '-' };

// "DISK" "memory"
char disk[MAX_PROCESSES * PROCESS_SIZE] = { '-' };

// Helper for counting bits I might need
constexpr int count_two_multiples(int i) {
    int cnt = 0;
    while (i > 1) {
        i /= 2;
        cnt += 1;
    }
    return 2;
}

constexpr int NUM_VIRTUAL_PAGES = count_two_multiples(PROCESS_SIZE / PAGE_SIZE);
constexpr int VIRTUAL_OFFSET_BITS = count_two_multiples(PAGE_SIZE);

/** Meant to simulate a process. Has an id and a translation lookaside buffer */
struct process {
    int process_id;
    int tlb[NUM_VIRTUAL_PAGES];

    int get_real_index(const int virtual_idx);
    char read(const int virtual_idx);
    void write(const int virtual_idx, const char data);
    void write_from(const int virtual_idx, const char* data);
};

/** For implementing Second Chance LRU Algorithm for page swapping */
struct FreeListItem {
    int process_id;
    bool accessed;
    bool dirty;
};

/** Meant to simulate the kernel. Holds a data structure for all processes, a free list of pages */
struct kernel {
    FreeListItem free_list[NUM_HARDWARE_PAGES];
    process processes[MAX_PROCESSES];
    int swap_page_index = 0;

    /** Load a process identified by `process_id`, a virtual page index / number `vpn` and a target physical
     * page in memory to load to identified by `page_idx`, populates the contents of the physical page with
     * that of the virtual page */
    void load(const int process_id, const int vpn, const int page_idx) {
        for (int i = 0; i < PAGE_SIZE; i++) {
            memory[page_idx * PAGE_SIZE + i] = disk[process_id * PROCESS_SIZE + vpn * PAGE_SIZE + i];
        }
    }

    /** Takes a process identified by `process_id`, a virtual page index / number `vpn`. Copies the virtual page
     * of said process at that virtual page number to """disk""". */
    void store(const int process_id, const int vpn) {
        int* tlb = processes[process_id].tlb;
        for (int i = 0; i < PAGE_SIZE; i++) {
            disk[process_id * PROCESS_SIZE + vpn * PAGE_SIZE + i] = memory[tlb[vpn] * PAGE_SIZE + i];
        }
    }

    /** Given a target process and a physical page at `page_idx` it's currently using, find the virtual page index
     * that maps to the `page_idx` such that `p.tlb[virtual_index] = page_index` */
    int find_vpn(process& p, const int page_idx) {
        for (int i = 0; i < PROCESS_SIZE  / PAGE_SIZE; i++) {
            if (p.tlb[i] == page_idx) {
                return i;
            }
        }

        return -1;
    }

    void store_all() {
        for (int i = 0; i < NUM_HARDWARE_PAGES; i++) {
            FreeListItem& fli = free_list[i];
            if (fli.process_id != -1) {
                const int vpn = find_vpn(processes[fli.process_id], i);
                store(fli.process_id, vpn);
                fli.accessed = true;
            }
        }
    }

    /** Gives a page to a process upon request. If no free page is available a swap occurs */
    int request_page(const int process_id, const int vpn) {
        // search for a free page
        bool candidate = false;
        while (!candidate) {
            FreeListItem& fli = free_list[swap_page_index];
            // The page is vacant / free
            if (fli.process_id == -1) {
                load(fli.process_id, vpn, swap_page_index);
                fli.process_id = process_id;
                fli.accessed = true;
                processes[fli.process_id].tlb[vpn] = swap_page_index;
                return swap_page_index;
            // the page is occupied
            } else {
                if (fli.accessed) {
                    fli.accessed = false;
                    swap_page_index = (swap_page_index + 1) % NUM_HARDWARE_PAGES;
                } else {
                    candidate = true;
                }
            }
        }

        FreeListItem& swap_fli = free_list[swap_page_index];
        process& p = processes[swap_fli.process_id];
        int swap_vpn = find_vpn(p, swap_page_index);
        
        // swap
        std::cout << "> Swapping out process " << p.process_id << " (@ page " << swap_page_index << ") with process " << process_id << std::endl;
        if (swap_fli.dirty) {
            std::cout << "> Process " << p.process_id << " (Virtual Page " << swap_vpn << ") is DIRTY, storing to disk" << std::endl;
            store(p.process_id, swap_vpn);
        }

        load(process_id, vpn, swap_page_index);
        p.tlb[swap_vpn] = -1;
    
        FreeListItem& in_fli = free_list[swap_page_index];
        in_fli.process_id = process_id;
        in_fli.accessed = true;
        in_fli.dirty = false;

        return swap_page_index;
    }

    
    /** Clear the contents of a physical page in memory */
    void clear_page(const int page_idx) {
        for (int i = 0; i < PAGE_SIZE; i++) {
            memory[page_idx * PAGE_SIZE + i] = '-';
        }
    }

    /** Given a process and a virtual page number presumed to be in main memory, clear out
     * its contents from the physical page. Note: no storing is done here. The physical page
     * the process was evicted from is returned. */
    int evict(const int process_id, const int vpn) {
        process& p = processes[process_id];
        const int page_index = p.tlb[vpn];
        std::cout << "> Evicting Process " << process_id << ", virtual page " << vpn << " from memory" << std::endl;
        
        const int MAX_VIRTUAL_PAGES = PROCESS_SIZE / PAGE_SIZE;
        if (page_index >= 0 && page_index < NUM_HARDWARE_PAGES) {
            clear_page(page_index);
            p.tlb[vpn] = -1;
            free_list[page_index].process_id = -1;
            std::cout << "> Process successfully evicted, page " << page_index << " is now free" << std::endl;
        } else {
            std::cout << "> Process " << process_id << "'s virtual page " << vpn << " could not be found in memory" << std::endl;
        }

        return page_index;
    }

    void write(const int process_id, const int virtual_index, const char data) {
        processes[process_id].write(virtual_index, data);
    }

    void write_from(const int process_id, const int virtual_start_index, const char* data) {
        processes[process_id].write_from(virtual_start_index, data);
    }

    /** Zero out all indices in "memory" */
    void clear_memory() {
        for (char &address : memory) {
            address = '-';
        }
    }

    /** Zero out all indices in the "disk" */
    void clear_disk() {
        for (char &address : disk) {
            address = '-';
        }
    }
};

/** global kernel object */
static kernel K;

int process::get_real_index(const int virtual_idx) {
    const int vpn = (virtual_idx & VPN_MASK) >> VIRTUAL_OFFSET_BITS;
    const int page_frame_number = tlb[vpn];

    if (page_frame_number == -1) {
        std::cout << "> Process " << process_id << " is requesting page" << std::endl;
        tlb[vpn] = K.request_page(this->process_id, vpn);
    }

    K.free_list[tlb[vpn]].accessed = true;
    return tlb[vpn] * PAGE_SIZE + (OFFSET_MASK & virtual_idx);
}

/** Read from "memory" given a virtual index */
char process::read(const int virtual_idx) {
    const int real_idx = get_real_index(virtual_idx);
    return memory[real_idx];
}
    
/** Write to "memory" given a virtual index and a byte of data */
void process::write(const int virtual_idx, const char data) {
    const int real_idx = get_real_index(virtual_idx);
    // std::cout << "real idx " << real_idx << ", page size " << PAGE_SIZE << ", " << (real_idx / PAGE_SIZE) << std::endl;
    K.free_list[real_idx / PAGE_SIZE].dirty = true;
    memory[real_idx] = data;
}

/** Write multiple bytes of data to "memory" given a starting virtual index */
void process::write_from(const int virtual_start_idx, const char* data) {
    for (int i = 0;  i < PROCESS_SIZE; i++) {
        const char c = *(data + i);
        if (c == '\0') {
            break;
        }

        const int real_idx = get_real_index(virtual_start_idx + i);
        // std::cout << "real idx " << real_idx << ", page size " << PAGE_SIZE << ", " << (real_idx / PAGE_SIZE) << std::endl;
        K.free_list[real_idx / PAGE_SIZE].dirty = true;
        memory[real_idx] = c;
    }

}

void print_memory() {
    std::cout << "MEMORY [";
    for (char address : memory) {
        std::cout << address;
    }
    std::cout << "]" << std::endl;
}

void print_disk() {
    std::cout << "DISK   [";
    for (char address : disk) {
        std::cout << address;
    }
    std::cout << "]" << std::endl;
}

void print_free_list() {
    std::cout << "FREE   [";
    for (int i = 0; i < NUM_HARDWARE_PAGES; i++) {
        FreeListItem& fli = K.free_list[i];
        std::cout << "(P:" << fli.process_id << ", A:" << fli.accessed << ", D:" << fli.dirty << ")";
    }
    std::cout << "]" << std::endl;
}

void print_diagnostics() {
    std::cout << "===================" << std::endl;
    print_memory();
    print_disk();
    print_free_list();
    std::cout << "===================" << std::endl;
}

/** Initializes all data structures in the kernel */
kernel& init_mock_os() {
    K.clear_memory();
    K.clear_disk();

    for (int i = 0; i < MAX_PROCESSES; i++) {
        K.processes[i].process_id = i;
        for (int v = 0; v < NUM_VIRTUAL_PAGES; v++) {
            K.processes[i].tlb[v] = -1;
        }
    }

    for (int i = 0; i < NUM_HARDWARE_PAGES; i++) {
        K.free_list[i].process_id = -1;
        K.free_list[i].accessed = false;
        K.free_list[i].dirty = false;
    }

    return K;
}

int main() {
    kernel& mother = init_mock_os();

    mother.processes[0].write_from(0, "this_is_");
    mother.processes[1].write_from(0, "me_testi");
    mother.processes[2].write_from(0, "ng_virtu");
    mother.processes[3].write_from(0, "alizatio");
    mother.processes[4].write_from(0, "n...");
    mother.processes[7].write_from(0, "monsoon.");

    // After this first of rounds we'll have had a 
    print_diagnostics();

    std::cout << "\n---\nPRINTING OUT THE MEMORY IN PROCESS '0'" << std::endl;
    for (int i = 0; i < PROCESS_SIZE; i++) {
        const char byte = mother.processes[0].read(i);
        std::cout << "P0: " << byte << std::endl;
    }
    std::cout << "DONE.\n---\n" << std::endl;

    print_diagnostics();

    mother.store_all();
    mother.evict(0, 1);
    mother.evict(7, 0);
    mother.evict(7, 1);
    mother.evict(3, 1);

    print_diagnostics();

    mother.write_from(5, 0, "12345678");
    mother.write_from(6, 0, "roygbiv.");
    mother.write_from(3, 0, "2CANNONS");
    mother.store_all();

    print_diagnostics();

    return 0;
}