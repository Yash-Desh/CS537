#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include "test_helper.h"


int
main(int argc, char* argv[])
{
    struct pstat ps;
    int my_idx = find_my_stats_index(&ps);
    ASSERT(my_idx != -1, "Could not get process stats from pgetinfo");

    ASSERT(ps.inuse[my_idx], "My slot in the ptable is not in use!");

    ASSERT(ps.tickets[my_idx] == DEFAULT_TICKETS, "My ticekts (%d) does not match \
            the default number of tickets (%d)", ps.tickets[my_idx], DEFAULT_TICKETS);

    // run parent for 40 more ticks
    int old_rtime = ps.rtime[my_idx];
    int extra = 40;
    run_until(old_rtime + extra);

    int now_rtime = ps.rtime[my_idx];

    int diff_rtime = now_rtime - old_rtime;
    int margin = 2;
    ASSERT(diff_rtime <= 40 + margin && diff_rtime >= 40 - margin,
            "Expected %d ticks,  got %d ticks, should be within a \
%d margin ticks", 40, diff_rtime, margin, margin);

    test_passed();

    exit();
}
