/** @file 
    Implementacja wrappera dictionary do pythona.
   
    @ingroup dictionary
    @author Wojciech Kordalski <wojtek.kordalski@gmail.com>
    @copyright Uniwersytet Warszawski
    @date 2015-06-18
 */

#include <Python.h>
#include <boost/python.hpp>
using namespace boost::python;

extern "C"
{
#include "dictionary.h"
}
#include <stdio.h>

#include <string>

/// Reprezentuje słownik.
class dict
{
    struct dictionary *d;   ///< Wewnętrzna reprezentacja.
    
private:
    /**
     * Tworzy obiekt słownika biorąc wskaźnik na wewnętrzną reprezentację.
     * @param[in] d Wewnętrzna reprezentacja.
     */
    dict(struct dictionary *d)
    {
        this->d = d;
    }
    
public:
    /**
     * Tworzy nowy słownik.
     */
    dict()
    {
        d = dictionary_new();
    }
    /**
     * Kopiuje obiekt słownika.
     * Właściwie, to powinno być przenoszenie,
     * ale to jest dopiero od C++11,
     * a w dodatku nie wiem, czy Boost.Python to wspiera.
     * Choć pewnie tak...
     */
    dict(const dict &orig)
    {
        d = orig.d;
    }

    /**
     * Wstawia słowo do słownika.
     * @param[in] s Słowo.
     * @return Wynik.
     */
    int insert(std::wstring s)
    {
        return dictionary_insert(d, s.c_str());
    }
    /**
     * Usuwa słowo ze słownika.
     * @param[in] s Słowo.
     * @return Wynik.
     */
    int remove(std::wstring s)
    {
        return dictionary_delete(d, s.c_str());
    }
    /**
     * Szuka słowo w słownika.
     * @param[in] s Słowo.
     * @return Wynik.
     */
    bool find(std::wstring s)
    {
        return dictionary_find(d, s.c_str());
    }
    /**
     * Finalizuje słownik.
     */
    void done()
    {
        dictionary_done(d);
        d = NULL;
    }
    /**
     * Zwraca listę podpowiedzi.
     * @param[in] s Słowo.
     * @return Lista podpowiedzi.
     */
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
    /**
     * Usuwa wszystkie reguły.
     */
    void clear_rules()
    {
        dictionary_rule_clear(d);
    }
    /**
     * Dodaje regułę lhs -> rhs, z kosztem cost i flagą flag.
     * @param[in] lhs lhs
     * @param[in] rhs rhs
     * @param[in] cost Koszt reguły.
     * @param[in] flag Flaga reguły.
     * @return Wynik.
     */
    int add_rule(std::wstring lhs, std::wstring rhs, int cost, char flag)
    {
        enum rule_flag fla = RULE_NORMAL;
        if(flag == 'n') fla = RULE_NORMAL;
        if(flag == 'e') fla = RULE_END;
        if(flag == 'b') fla = RULE_BEGIN;
        if(flag == 's') fla = RULE_SPLIT;
        return dictionary_rule_add(d, lhs.c_str(), rhs.c_str(), false, cost, fla);
    }
    /**
     * Ustawia maksymalny koszt podpowiedzi.
     * @param[in] value Nowy koszt.
     * @return Stary koszt.
     */
    int hints_max_cost(int value)
    {
        return dictionary_hints_max_cost(d, value);
    }
    
    /**
     * Zapisuje słownik.
     * @param[in] d Słownik.
     * @param[in] f Nazwa pliku.
     * @return Wynik.
     */
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
    
    /**
     * Wczytuje słownik.
     * @param[in] f Nazwa pliku.
     * @return Słownik.
     */
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
    
    /**
     * Zapisuje słownik.
     * @param[in] d Słownik.
     * @param[in] f Nazwa języka.
     * @return Wynik.
     */
    static int save_lang(dict d, std::string f)
    {
        return dictionary_save_lang(d.d, f.c_str());
    }
    
    /**
     * Wczytuje słownik.
     * @param[in] f Nazwa języka.
     * @return Słownik.
     */
    static dict load_lang(std::string f)
    {
        return dict(dictionary_load_lang(f.c_str()));
    }
    
    /**
     * Listuje języki.
     * @return Lista języków.
     */
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

/// Moduł pythona.
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