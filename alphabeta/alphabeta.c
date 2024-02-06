#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/time.h>

#define GT_UNEXPLORED 100000000
#define GT_WHITE 1
#define GT_BLACK -1
#define ABP_PINFTY 10000000
#define ABP_MINFTY -10000000
#define VALUE_RANGE 4000
#define WASTE 100000000

/* in microseconds (us) */
double get_elapsed_time(struct timeval *begin, struct timeval *end)
{
    return (end->tv_sec - begin->tv_sec) * 1000000
        + (end->tv_usec - begin->tv_usec);
}

typedef struct __gametree
{
    /*
        if this is a terminal node of the searching range,
        value = temporary_value.
        Otherwise, defined

    */

    //// value data

    int temporary_value;
    int value;

    //// board data

    // this expresses a identification number of the previous move. No meaning more.
    int previous_move;

    int turn_player;

    //// leaf data

    int no_leaves;
    struct __gametree *leaves;

    struct __gametree *next_from_bestmove;
} *gametree;

typedef unsigned long long ull;

typedef struct _randomizer
{
    ull old;
} randomizer;

static randomizer __rengine = {2906};

ull rand1();

gametree new_gametree();

void init(gametree tree, int maxdepth, int leaf_width);
void _init1(gametree tree, int depth, int maxdepth, int mean, int leaf_width);

void alphabeta(gametree tree, int maxdepth);
void _clear_value(gametree tree);
void _alphabeta(gametree tree, int depth, int maxdepth, int alpha, int beta);

int main(int argc, char *argv[])
{
    struct timeval t1, t2;

    int dep = 7, wid = 20;

    if (argc >= 3)
    {
        dep = atoi(argv[1]);
        wid = atoi(argv[2]);
    }

    gametree gt = new_gametree();
    init(gt, dep, wid);

    gettimeofday(&t1, NULL);

    alphabeta(gt, dep);

    gettimeofday(&t2, NULL);

    printf("%d\n", gt->value);

    {
        double us;
        double gflops;
        int op_per_point = 5; // 4 add & 1 multiply per point

        us = get_elapsed_time(&t1, &t2);
        printf("Elapsed time: %.3lf sec\n", us/1000000.0);
    }

    return 0;
}

ull rand1()
{
    __rengine.old ^= __rengine.old >> 12,
    __rengine.old ^= __rengine.old << 25,
    __rengine.old ^= __rengine.old >> 27;

    return (__rengine.old * 2685821657736338717LL);
}

gametree new_gametree()
{
    return (gametree)malloc(sizeof(struct __gametree));
}

void init(gametree tree, int maxdepth, int leaf_width)
{
    _init1(tree, 1, maxdepth, 0, leaf_width);
}

void _init1(gametree tree, int depth, int maxdepth, int mean, int leaf_width)
{
    tree->temporary_value = mean + rand1() % (VALUE_RANGE / (depth * depth)) - (VALUE_RANGE / (depth * depth * 2));
    tree->value = GT_UNEXPLORED;

    tree->previous_move = rand1() % 0xabc;
    tree->turn_player = 2 * (depth % 2) - 1;

    tree->no_leaves = 0;
    tree->leaves = NULL;

    tree->next_from_bestmove = NULL;

    int i;

    if (depth < maxdepth)
    {
        tree->no_leaves = leaf_width;
        tree->leaves = (gametree)malloc(sizeof(struct __gametree) * leaf_width);

        for (i = 0; i < leaf_width; ++i) 
        {
            _init1(tree->leaves + i, depth + 1, maxdepth, tree->temporary_value, leaf_width);
        }
    }
}

void _clear_value(gametree tree)
{
    tree->value = GT_UNEXPLORED;

    int i;

    for (i = 0; i < tree->no_leaves; ++i)
    {
        _clear_value(tree->leaves + i);
    }
}

void alphabeta(gametree tree, int maxdepth)
{
    // _clear_value(tree);
    _alphabeta(tree, 1, maxdepth, ABP_MINFTY, ABP_PINFTY);
}

void _alphabeta(gametree tree, int depth, int maxdepth, int alpha, int beta)
{
    usleep(100);
    
    // terminal process

    if (depth == maxdepth || tree->leaves == NULL)
    {
        tree->value = tree->temporary_value;
        return;
    }

    // alpha-beta pruning

    int i;
    gametree searching;

    for (i = 0; i < tree->no_leaves; ++i)
    {
        searching = tree->leaves + i;
        _alphabeta(searching, depth + 1, maxdepth, alpha, beta);
        if (tree->turn_player == GT_WHITE)
        {
            if (searching->value >= beta)
            {
                tree->value = searching->value;
                return;
            }
            else if (searching->value > alpha)
            {
                alpha = searching->value;
                tree->next_from_bestmove = searching;
            }
        }
        else if (tree->turn_player == GT_BLACK)
        {
            if (searching->value <= alpha)
            {
                tree->value = searching->value;
                return;
            }
            else if (searching->value < beta)
            {
                beta = searching->value;
                tree->next_from_bestmove = searching;
            }
        }
    }

    if (tree->turn_player == GT_WHITE)
    {
        tree->value = alpha;
    }
    else if (tree->turn_player == GT_BLACK)
    {
        tree->value = beta;
    }
}