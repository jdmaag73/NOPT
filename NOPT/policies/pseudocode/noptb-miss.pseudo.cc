//
//                                   
// NOPTb-miss replacement policy pseudo-code
// Javier Diaz, jdmaag@gmail.com 
//                                   
// This software and the methods and concepts it implements are available without charge to anyone for academic, research, experimental or personal use. 
// If you wish to distribute or make other use of the software and/or the methods, you may purchase a license to do so from the authors.
//

void initialize() {
    init_simulator();
    traces = load_traces(iteration-1, NUM_CORE, NUM_SETS);
    pointers_to_now = zero(NUM_SETS, NUM_CORE);
}

void build_global_sequence(int set) {
    my_set_pointers = pointers_to_now[set];
    do {
        next_req_cycle = MAX_VALUE;
        next_req_core = NONE;
        next_req_address = NONE;
        for (c=0; c<NUM_CORE; c++) {
            req_cycle = traces[c][set][my_set_pointers[c]].cycle;
            if (req_cycle != NONE && req_cycle < next_req_cycle) {
                next_req_cycle = req_cycle;
                next_req_core = c;
                next_req_address = traces[c][set][my_set_pointers[c]].address;;
            }
        }
        add_access(global_sequence, next_req_cycle, next_req_address);
        my_set_pointers[next_req_core]++;
    } while (!all_traces_finished())
}

int get_victim(int set, int core, int req_blk_address) {
    global_sequence = build_global_sequence(set);
    furthest_access_cycle = 0;
    furthest_access_way = NONE;
    for (i=0; i<SLLC_ASSOCIATIVITY; i++) {
        blk_address = cache_contents[set][i].blk_address;
        cycle = find_next_access(global_sequence, blk_address);
        if (cycle != NONE && cycle > furthest_access_cycle) {
            furthest_access_cycle = next_cycle;
            furthest_access_way = i;
        }
    }
    cycle = find_next_access(global_sequence, req_blk_address);
    if (cycle != NONE && cycle > furthest_access_cycle)
        return NONE;
    else
        return furthest_access_way;
}

int main() {
    initialize();
    do {
        sllc_req = run_simulation_until_sllc_request();
        if (is_miss(sllc_request)) {
            way = get_victim(sllc_req.set, sllc_req.core, sllc_req.blk_address);
            if (way != NONE) {
                evict(set, way);
             insert(sllc_sequest, way);
            } else {
                bypass(sllc_request);
            }
        }
        save_req_to_trace(iteration, sllc_req.core, sllc_req.set, sllc_req.blk_address, sllc_req.cycle);
    } while (!simulation_reached_end())
    simulation_save_stats(iteration);
}
