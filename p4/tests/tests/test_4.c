#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include "test_helper.h"


int
main(int argc, char* argv[])
{
    struct pstat ps;
    

    // check tickets=0
    int pa_tickets = 0;
    ASSERT(settickets(pa_tickets) != -1, "settickets syscall failed in parent");

    int my_idx = find_my_stats_index(&ps);
    
    ASSERT(my_idx != -1, "Could not get process stats from pgetinfo");

    ASSERT(ps.inuse[my_idx], "My slot in the ptable is not in use!");

    ASSERT(ps.tickets[my_idx] == 8, "My tickets (%d) does not match \
the modified number of tickets (%d)", ps.tickets[my_idx], 8);

    // check tickets=1
    pa_tickets = 1;
    ASSERT(settickets(pa_tickets) != -1, "settickets syscall failed in parent");

    my_idx = find_my_stats_index(&ps);
    
    ASSERT(my_idx != -1, "Could not get process stats from pgetinfo");

    ASSERT(ps.inuse[my_idx], "My slot in the ptable is not in use!");

    ASSERT(ps.tickets[my_idx] == pa_tickets, "My tickets (%d) does not match \
            the modified number of tickets (%d)", ps.tickets[my_idx], pa_tickets);

   
    // check tickets = 32
    pa_tickets = 32;
    ASSERT(settickets(pa_tickets) != -1, "settickets syscall failed in parent");

    my_idx = find_my_stats_index(&ps);
    
    ASSERT(my_idx != -1, "Could not get process stats from pgetinfo");

    ASSERT(ps.inuse[my_idx], "My slot in the ptable is not in use!");

    ASSERT(ps.tickets[my_idx] == pa_tickets, "My tickets (%d) does not match \
            the modified number of tickets (%d)", ps.tickets[my_idx], pa_tickets);

        
        // check tickets = 33
    pa_tickets = 33;
    ASSERT(settickets(pa_tickets) != -1, "settickets syscall failed in parent");

    my_idx = find_my_stats_index(&ps);
    
    ASSERT(my_idx != -1, "Could not get process stats from pgetinfo");

    ASSERT(ps.inuse[my_idx], "My slot in the ptable is not in use!");

    ASSERT(ps.tickets[my_idx] == 32, "My tickets (%d) does not match \
            the modified number of tickets (%d)", ps.tickets[my_idx], 32);

    test_passed();

    exit();
}
