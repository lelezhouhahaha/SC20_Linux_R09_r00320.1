/******************************************************************************
 *
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 *****************************************************************************/

#include "ll.h"

property_db* glisthead = NULL;

bool __list_is_empty()
{
  if (!glisthead)
    return true;
  else
    return false;
}

property_db* __list_matches_prop_name(const char* search_name)
{
    if(__list_is_empty())
    {
        LOG("List is empty, return \n");
        return NULL;
    }
    property_db *ln = glisthead;
    for (; ln !=NULL; ln  = ln ->next)
    {
        LOG("[%s] => search val=%s, curr val =%s\n", __func__,
                search_name,ln->unit.property_name);
        if (strcmp(ln->unit.property_name, search_name) == 0)
        {
            return ln;
        }
    }
    return NULL;
}

//supposed to create one node here once not from the property_ops
bool __create_list_and_add( const char* search_name, const char* property_value)
{
    property_db *ln = (property_db*) calloc(1, sizeof(property_db)*
        sizeof(unsigned char));

    strlcpy(ln->unit.property_name, search_name, sizeof(ln->unit.property_name));
    strlcpy(ln->unit.property_value, property_value, sizeof(ln->unit.property_value));

    ln->next = NULL;

    return __list_add(ln);
}

bool __update_prop_value(const char* search_name, const char* value)
{
    int retval = -1;
    char property_value[PROP_VALUE_MAX];

    // add trailing new line to property value
    memset(property_value, 0, sizeof property_value);
    strlcpy(property_value, value, sizeof property_value);
    property_value[strlen(property_value)] = '\n';

    property_db *ln = __list_matches_prop_name(search_name);
    if(ln != NULL)
    {
        LOG("List Matches property Updating Value\n");
        // ro.* properties are NEVER modified once set
        if(!strncmp(search_name, "ro.", 3)) {
            LOG("setprop(\"%s\", \"%s\") failed", search_name, property_value);
            retval = -1;
        } else {
            memset(ln->unit.property_value, 0,
                  sizeof(ln->unit.property_value));
            strlcpy(ln->unit.property_value,  property_value,
                   sizeof property_value);
            LOG("Value copied to the db prop name %s", search_name);
            retval = 0;
        }
    } else {
        LOG("New value not in the list, create a new node\n");
        retval = __create_list_and_add(search_name, property_value);
        LOG("Node Created Status =%d for prop name %s\n", retval, search_name);
    }
    return ((retval == 0)? true:false);
}

bool __retrive_prop_value(const char* search_name, const char* value)
{
    property_db *ln = NULL;
    char property_value[PROP_VALUE_MAX];

    memset(property_value, 0, sizeof property_value);

    ln = __list_matches_prop_name(search_name);
    if(NULL != ln)
    {
        LOG("Property: %s exist with value: %s\n",
            ln->unit.property_name, ln->unit.property_value);

        strlcpy(property_value, ln->unit.property_value,
                sizeof property_value);

        // Don't copy trailing new line added in update_prop_value
        strlcpy(value, property_value, strlen(property_value));
        return true;
    }
    return false;
}

bool __list_add(property_db* list)
{
    property_db *node;
    if (__list_is_empty())
    {
        LOG("Adding first Node\n");
        glisthead = list;//assumed list comes with NULL terminated next
    } else {
        LOG("First Node Present, add subsequent one\n");
        //check if already the property exists
        node = __list_matches_prop_name(list->unit.property_name);
        if (NULL == node)
        {
            //Add list to the end.
            property_db *ln;
            for (ln = glisthead; ln->next != NULL; ln = ln->next);
            ln->next = list;//assumed ln comes with NULL terminated next
        } else {
            //property exists update the value
            LOG("Node Present, updating value");
            memset(node->unit.property_value, 0,
                   sizeof(node->unit.property_value));
            strlcpy(node->unit.property_value, list->unit.property_value,
                    sizeof(list->unit.property_value));
            free(list);
        }
    }
}

bool __remove_node_from_list(unsigned char* property_name)
{
    bool retval;

    if (__list_is_empty())
    {
        LOG("List is Empty\n");
        retval = false;
    } else {
        property_db *ln_prev, *ln = glisthead;

        while(ln != NULL) {
            if (!strncmp(ln->unit.property_name, property_name,
                strlen(property_name))) {
                if (glisthead == ln) {
                    //first node matches.
                    glisthead = ln->next;
                    free(ln);
                    LOG("First Node Matched and head moved\n");
                    retval = true;
                    break;
                } else {
                    //remove the element here
                    LOG("Removing Property Entry for @prop name =%s\n",
                        ln->unit.property_name);
                    ln_prev->next = ln->next;
                    free(ln);
                    retval = true;
                    break;
                }
            }
            ln_prev = ln;
            ln = ln->next;
        }
    }
    return retval;
}

//to be called on deinit
bool __free_list()
{
    bool retval;
    property_db *ln = glisthead;
    if (__list_is_empty())
    {
        LOG("List is Empty\n");
        retval = false;
    } else {
        while(ln != NULL)
        {
            property_db *temp = ln;
            ln = ln->next;
            free(temp);
        }
        retval = true;
        LOG("List cleanup sucessfull\n");
    }
    return retval;
}

property_db* __get_list_head()
{
    return glisthead;
}

void __dump_nodes()
{
    property_db *ln = glisthead;
    if (__list_is_empty())
    {
        LOG("List is Empty\n");
        return;
    }
    for (; ln != NULL; ln = ln->next)
    {
        LOG("%s,%s,%s\n", ln->unit.property_name,
                ln->unit.property_value, ln->unit.callback_to_be_invoked);
    }
}
bool __save_persist_nodes_to_file()
{
    property_db *ln = glisthead;
    bool retval = false;
    int filewrite_status = -1;

    FILE *fp = fopen(PROP_FILE_PERSIST_PATH, "w");
    LOG("Save DS to persist file %s\n",PROP_FILE_PERSIST_PATH);

    if (NULL != fp)
    {
        LOG("File truncate mode\n");
        for (; ln != NULL; ln = ln->next)
        {
            if ( strncmp("persist.", ln->unit.property_name,
                 strlen("persist.")) == 0 ) {

                unsigned char stringtowrite[MAX_ALLOWED_LINE_LEN];
                memset(stringtowrite, 0 , sizeof(stringtowrite));

                snprintf(stringtowrite, MAX_ALLOWED_LINE_LEN, "%s=%s",
                     ln->unit.property_name,ln->unit.property_value );

                LOG("Writing to persist %s \n", stringtowrite);

                filewrite_status = fputs(stringtowrite, fp);
                if (filewrite_status < 0)
                {
                    retval = false;
                    break;
                } else {
                    retval = true;
                }
            }
        }
        fclose(fp);
    } else {
        ALOGE("Persist File doesnt exist");
        retval = false;
    }
    return retval;
}

