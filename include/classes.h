#ifndef __CLASSES_H__
#define __CLASSES_H__

#include "simple_json.h"

#include "gfc_text.h"

/*
@brief Initialize and load class definitions
@param filename json file with class defs
*/
void classes_initialize(const char *filename);

/*
@brief get a class def by its name
@param name name of the class to search for
@return NULL if not find, otherwise the def of the class
@note do not free that data
*/
SJson *classes_get_def_by_name(const char *name);


#endif