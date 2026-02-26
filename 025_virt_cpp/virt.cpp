#include <iostream>

/** VIRTUAL MEMORY EMULATOR */

// CONSTANTS
constexpr int PAGE_SIZE = 4;
constexpr int NUM_HARDWARE_PAGES = 8;
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

struct process {
    int process_id;
    int tlb[2] = {-1, -1};

    int get_real_index(const int virtual_idx);
    char read(const int virtual_idx);
    void write(const int virtual_idx, const char data);
    void write_from(const int virtual_idx, const char* data);
};

struct kernel {
    int free_list[NUM_HARDWARE_PAGES] = { -1 }; 
    process processes[MAX_PROCESSES] = {};
    int swap_page_index = 0;

    void load(const int process_id, const int vpn, const int page_idx) {
        for (int i = 0; i < PAGE_SIZE; i++) {
            memory[page_idx * PAGE_SIZE + i] = disk[process_id * PROCESS_SIZE + vpn * PAGE_SIZE + i];
        }
    }

    void store(const int process_id, const int vpn, const int page_idx) {
        for (int i = 0; i < PAGE_SIZE; i++) {
            disk[process_id * PROCESS_SIZE + vpn * PAGE_SIZE + i] = memory[page_idx * PAGE_SIZE + i];
        }
    }

    int find_swap_vpn(process& p, const int page_idx) {
        for (int i = 0; i < PROCESS_SIZE  / PAGE_SIZE; i++) {
            if (p.tlb[i] == page_idx) {
                return i;
            }
        }

        return -1;
    }

    void store_all() {
        for (int i = 0; i < NUM_HARDWARE_PAGES; i++) {
            const int process_id = free_list[i];
            if (process_id != -1) {
                std::cout << "Storing Process: " << process_id << " at page frame " << i << std::endl;
                const int vpn = find_swap_vpn(processes[process_id], i);

                for (int i : processes[process_id].tlb) {
                    std::cout << "tlb[" << i << "]: " << processes[process_id].tlb[i] << std::endl;
                }
                std::cout << "vpn: " << vpn << std::endl;
                store(process_id, vpn, i);
            }
        }
    }

    /** Gives a page to a process upon request. If no free page is available a swap occurs */
    int request_page(const int process_id, const int vpn) {
        // search for a free page
        for (int i = 0; i < NUM_HARDWARE_PAGES; i++) {
            if (free_list[i] == -1) {
                load(process_id, vpn, i);
                free_list[i] = process_id;
                processes[process_id].tlb[vpn] = i;
                return i;
            }
        }

        // find the disk memory that maps to the page in memory
        process& p = processes[free_list[swap_page_index]];
        int swap_vpn = find_swap_vpn(p, swap_page_index);
        
        // swap
        std::cout << "> Swapping out process " << p.process_id << " @ page " << swap_page_index << " with page " << process_id << std::endl;
        store(p.process_id, swap_vpn, swap_page_index);
        load(process_id, vpn, swap_page_index);

        p.tlb[swap_vpn] = -1;
    
        free_list[swap_page_index] = process_id;

        const int old_swap_index = swap_page_index;
        swap_page_index = (swap_page_index + 1) % NUM_HARDWARE_PAGES;
        return old_swap_index;
    }
};

/** global kernel object */
static kernel K;

int process::get_real_index(const int virtual_idx) {
    const int vpn = (virtual_idx & VPN_MASK) >> 2;
    const int page_frame_number = tlb[vpn];
    if (page_frame_number == -1) {
        std::cout << "> Process " << process_id << " is requesting page" << std::endl;
        tlb[vpn] = K.request_page(this->process_id, vpn);
    }

    return tlb[vpn] * PAGE_SIZE + (OFFSET_MASK & virtual_idx);
}

char process::read(const int virtual_idx) {
    const int real_idx = get_real_index(virtual_idx);
    return memory[real_idx];
}
    
/** Write to "memory" */
void process::write(const int virtual_idx, const char data) {
    const int real_idx = get_real_index(virtual_idx);
    // std::cout << "(Real Index: " << real_idx << ")" << std::endl;
    memory[real_idx] = data;
}

void process::write_from(const int virtual_start_idx, const char* data) {
    const int real_idx = get_real_index(virtual_start_idx);

    char c = *data;
    int i = 0;
    while (i < PROCESS_SIZE && c != '\0') {
        c = *(data + i);
        const int real_idx = get_real_index(virtual_start_idx + i);
        memory[real_idx] = c;
        i += 1;
    }
}

/** Zero out all indices in the buffer */
void clear_memory() {
    for (char &address : memory) {
        address = '-';
    }
}

void clear_disk() {
    for (char &address : disk) {
        address = '-';
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
    for (int i = 0; i < NUM_HARDWARE_PAGES; i++) {
        std::cout << K.free_list[i] << "-";
    }
    std::cout << std::endl;
}

void init_mock_os() {
    clear_memory();
    clear_disk();

    for (int i = 0; i < MAX_PROCESSES; i++) {
        K.processes[i].process_id = i;
        K.processes[i].tlb[0] = -1;
        K.processes[i].tlb[1] = -1;
    }

    for (int i = 0; i < NUM_HARDWARE_PAGES; i++) {
        K.free_list[i] = -1;
    }
}

int main() {
    std::cout << "Hello World!" << std::endl;
    init_mock_os();

    K.processes[0].write_from(0, "this_is_");
    K.processes[1].write_from(0, "me_testi");
    K.processes[2].write_from(0, "ng_virtu");
    K.processes[3].write_from(0, "alizatio");
    K.processes[4].write_from(0, "n.");

    std::cout << "\nPRINTING OUT THE MEMORY IN PROCESS '0'" << std::endl;
    for (int i = 0; i < PROCESS_SIZE; i++) {
        const char character = K.processes[0].read(i);
        std::cout << "P0: " << character << std::endl;
    }
    
    std::cout << "\n" << std::endl;


    // K.processes[0].write(0, 'h');
    // K.processes[0].write(1, 'e');
    // K.processes[0].write(2, 'l');
    // K.processes[0].write(3, 'l');

    // K.processes[1].write(0, 'w');

    // K.processes[0].write(4, 'c');
    // K.processes[0].write(5, 'o');
    // K.processes[0].write(6, 'r');
    // K.processes[0].write(7, 'n');

    // K.processes[2].write(5, 's');
    // K.processes[2].write(6, 'o');
    // K.processes[2].write(7, 'n');

    // K.processes[1].write(4, 'w');
    // K.processes[1].write(5, 'i');
    // K.processes[1].write(6, 'n');
    // K.processes[1].write(7, 'd');

    // K.processes[7].write_from(0, "blowhole");
    // K.processes[6].write_from(0, "coalmoon");

    // K.processes[1].write_from(1, "ink");

    K.store_all();

    print_memory();
    print_disk();
    print_free_list();
    return 0;
}