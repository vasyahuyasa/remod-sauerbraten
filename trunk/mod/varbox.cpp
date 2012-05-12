#include "game.h"
#include "varbox.h"

varbox::varbox()
{
    loopi(VARBOXMAXNODES) nodes[i] = NULL;
}


varbox::~varbox()
{
    loopi(VARBOXMAXNODES)
    {
        if(nodes[i])
        {
            DELETEA(nodes[i]->val);
            DELETEA(nodes[i]->key);
            DELETEP(nodes[i]);
        }
        else
        {
            break;
        }
    }
}

// get value
char* varbox::get(const char *key)
{
    char *res = NULL;

    loopi(VARBOXMAXNODES)
    {
        if(nodes[i] == NULL) break;

        if(strcmp(key, nodes[i]->key) == 0)
        {
            res = nodes[i]->val;
            break;
        }
    }

    return res;
}

// set value
void varbox::set(const char *key, char *val)
{
    if(!key) return;

    loopi(VARBOXMAXNODES)
    {
        if((nodes[i] == NULL) || (strcmp(key, nodes[i]->key) == 0))
        {
            if(!nodes[i])
            {
                // new node
                nodes[i] = new varbox_node;
                nodes[i]->key = newstring(key);
            }
            else
            {
                // node already exists
                DELETEA(nodes[i]->val);
            }

            nodes[i]->val = newstring(val);
            break;
        }
    }
}
