#ifndef __VARBOX_H__
#define __VARBOX_H__

#define VARBOXMAXNODES 128

struct varbox_node
{
    char *key;
    char *val;
};

struct varbox
{
    varbox_node *nodes[VARBOXMAXNODES];

    varbox();
    ~varbox();

    char*   get(const char *key);
    void    set(const char *key, char *val);
};

#endif
