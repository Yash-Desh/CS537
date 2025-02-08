#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include "test_helper.h"


int
main(int argc, char* argv[])
{
    struct pstat ps;
    int pid = fork();
    int ch_tickets = 2;

    if (pid == 0) {
        
        ASSERT(settickets(ch_tickets) != -1, "settickets syscall failed in parent");

        int ch_idx = find_my_stats_index(&ps);
        
        ASSERT(ch_idx != -1, "Could not get process stats from pgetinfo");

        ASSERT(ps.inuse[ch_idx], "My slot in the ptable is not in use!");

        ASSERT(ps.tickets[ch_idx] == ch_tickets, "My tickets (%d) does not match \
            the modified number of tickets (%d)", ps.tickets[ch_idx], ch_tickets);
        // This just has to be large enough that we know it doesn't terminate
        // before the parent gets some information - note that they should 
        // run proportional to their tickets, so it's not completely random
        // One can pick the number and add some safe margin to that
        int rt = 100;
        run_until(rt);
        exit();
    }

    
    int my_idx = find_my_stats_index(&ps);
    ASSERT(my_idx != -1, "Could not get process stats from pgetinfo");

    ASSERT(ps.inuse[my_idx], "My slot in the ptable is not in use!");

    ASSERT(ps.tickets[my_idx] == DEFAULT_TICKETS, "My ticekts (%d) does not match \
            the default number of tickets (%d)", ps.tickets[my_idx], DEFAULT_TICKETS);

    test_passed();

    wait();


    exit();
}
