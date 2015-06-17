#include <Python.h>
#include <boost/python.hpp>
using namespace boost::python;

extern "C"
{
#include "dictionary.h"
}
#include <stdio.h>

#include <string>

class dict
{
    struct dictionary *d;
    
private:
    dict(struct dictionary *d)
    {
        this->d = d;
    }
    
public:
    dict()
    {
        d = dictionary_new();
    }
    dict(const dict &orig)
    {
        d = orig.d;
    }

    int insert(std::wstring s)
    {
        return dictionary_insert(d, s.c_str());
    }
    int remove(std::wstring s)
    {
        return dictionary_delete(d, s.c_str());
    }
    bool find(std::wstring s)
    {
        return dictionary_find(d, s.c_str());
    }
    void done()
    {
        dictionary_done(d);
        d = NULL;
    }
    boost::python::list hints(std::wstring s)
    {
        struct word_list list;
        dictionary_hints(d, s.c_str(), &list);
        boost::python::list ret;
        for(int i = 0; i < word_list_size(&list); i++)
        {
            ret.append(std::wstring(word_list_get(&list)[i]));
        }
        return ret;
    }
    void clear_rules()
    {
        dictionary_rule_clear(d);
    }
    int add_rule(std::wstring lhs, std::wstring rhs, int cost, char flag)
    {
        enum rule_flag fla = RULE_NORMAL;
        if(flag == 'n') fla = RULE_NORMAL;
        if(flag == 'e') fla = RULE_END;
        if(flag == 'b') fla = RULE_BEGIN;
        if(flag == 's') fla = RULE_SPLIT;
        return dictionary_rule_add(d, lhs.c_str(), rhs.c_str(), false, cost, fla);
    }
    int hints_max_cost(int value)
    {
        return dictionary_hints_max_cost(d, value);
    }
    
    static int save(dict d, std::string f)
    {
        FILE *fl = fopen(f.c_str(), "w");
        if (!fl || dictionary_save(d.d, fl))
        {
            return -1;
        }
        fclose(fl);
        return 0;
    }
    
    static dict load(std::string f)
    {
        FILE *fl = fopen(f.c_str(), "r");
        struct dictionary *new_dict;
        if (!fl || !(new_dict = dictionary_load(fl)))
        {
            return dict();
        }
        fclose(fl);
        return dict(new_dict);
    }
    
    static int save_lang(dict d, std::string f)
    {
        return dictionary_save_lang(d.d, f.c_str());
    }
    
    static dict load_lang(std::string f)
    {
        return dict(dictionary_load_lang(f.c_str()));
    }
    
    static list list_lang()
    {
        char   *mlist;
        size_t  llen;
        
        if(dictionary_lang_list(&mlist, &llen)<0)
        {
            return list();
        }
        list ret;
        for (int i = 0; i < llen && llen > 1;)
        {
            int wlen = strlen(mlist+i);
            // Combo box lubi mieć Gtk
            char *uword = mlist+i;

            ret.append(std::string(uword));
            i += wlen + 1;
        }
        return ret;
    }
};

BOOST_PYTHON_MODULE(libpyippdict)
{
    class_< ::dict>("dict")
        .def("done", &::dict::done)
        .def("insert", &::dict::insert)
        .def("remove", &::dict::remove)
        .def("find", &::dict::find)
        .def("hints", &::dict::hints)
        .def("clear_rules", &::dict::clear_rules)
        .def("add_rule", &::dict::add_rule)
        .def("hints_max_cost", &::dict::hints_max_cost);
    def("load_file", &::dict::load);
    def("load_lang", &::dict::load_lang);
    def("save_file", &::dict::save);
    def("save_lang", &::dict::save_lang);
    def("list_lang", &::dict::list_lang);
}