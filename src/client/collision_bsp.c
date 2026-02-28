#include "client/collision_bsp.h"
#include <stdlib.h>

cbsp_t *cbsp_create() {
    cbsp_t *cbsp = malloc(sizeof(*cbsp));

    cbsp->leaf_count  = 0;
    cbsp->node_count  = 0;
    cbsp->plane_count = 0;

    cbsp->leafs  = NULL;
    cbsp->nodes  = NULL;
    cbsp->planes = NULL;

    return cbsp;
}

void cbsp_load(cbsp_t *cbsp, bsp_t *bsp) {}
