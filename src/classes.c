#include "simple_logger.h"
#include "gfc_text.h"

#include "classes.h"

static SJson *_classJson = NULL;
static SJson *_classDefs = NULL;

void classes_close();

void classes_initialize(const char *filename)
{
    if(!filename)
    {
        slog("No filename provided");
        return;
    }

    _classJson = sj_load(filename);
    if(!_classJson)
    {
        slog("fialed to laod class file with filename provided");
        return;
    }
    _classDefs = sj_object_get_value(_classJson,"classes");
    if(!_classDefs)
    {
        slog("class def file %s does not contain classes list",filename);
        sj_free(_classJson);
        _classJson = NULL;
        return;
    }

    atexit(classes_close);
}

void classes_close()
{
    if(_classJson)
    {
        sj_free(_classJson);
    }
    _classJson = NULL;
    _classDefs = NULL;
}

SJson *classes_get_def_by_name(const char *name)
{
    int i,c;
    SJson *class;
    const char *classname = NULL;

    if(!name) return NULL;
    if(!_classDefs)
    {
        slog("No class definitions");
        return NULL;
    }
    c = sj_array_count(_classDefs);
    for(i=0;i<c;i++)
    {
        class = sj_array_get_nth(_classDefs,i);
        if(!class){
            slog("failed to get nth class");
            continue;
        }
        classname = sj_object_get_string(class, name);
        if(!classname)continue;
        if(gfc_strlcmp(classname,name)==0)
        {
            return class;
        }
        slog("%s not found",name);
        return NULL;
    }
}

/*eof@eof*/