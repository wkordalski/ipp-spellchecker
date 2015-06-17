#include <gtk/gtk.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "editor.h"
#include "word_list.h"

void show_about () {
  GtkWidget *dialog = gtk_about_dialog_new();

  gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Text Editor");
  //gtk_window_set_title(GTK_WINDOW(dialog), "About Text Editor");
  
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), 
     "Text Editor for IPP exercises\n");
    
  gtk_dialog_run(GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
}

void show_help (void) {
  GtkWidget *help_window;
  GtkWidget *label;
  char help[5000];

  help_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW (help_window), "Help - Text Editor");
  gtk_window_set_default_size(GTK_WINDOW(help_window), 300, 300);
 
  strcpy(help,
         "\nAby podłączyć usługę spell-checkera do programu trzeba:\n\n"
         "Dołączyć ją do menu 'Spell' w menubar.\n\n"
         "Pobrać zawartość bufora tekstu z edytora: całą lub fragment,\n"
         "  zapamiętując pozycję.\n\n");
  strcat(help, "\0");

  label = gtk_label_new(help);
    
  gtk_container_add(GTK_CONTAINER(help_window), label); 

  gtk_widget_show_all(help_window);
}

// Zaślepeczki słownika (wchar_t i gunichar to prawie to samo)
//
// Oczywiście do zastąpienia prawdziwymi funkcjami

#include "dictionary.h"
#include "str.h"
#include <wctype.h>

struct dictionary *dict = NULL;
char *lang = NULL;

wchar_t *better_word(wchar_t *s)
{
    wchar_t *ss = s;
    struct string *st = string_make(L"");
    while(*ss != 0)
    {
        if(iswalpha(*ss))
            string_append(st, towlower(*ss));
        else break;
        ss++;
    }
    return string_undress(st);
}

// Procedurka obsługi

static void WhatCheck (GtkMenuItem *item, gpointer data) {
  GtkWidget *dialog;
  GtkTextIter start, end;
  char *word;
  gunichar *wwword;
  wchar_t *wword;
  
  if(dict == NULL)
  {
      dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
                                      GTK_BUTTONS_OK,
                                      "Najpierf wybież jenzyk aby zprafdzić poprafność tekstó.");
      gtk_dialog_run(GTK_DIALOG(dialog));
      gtk_widget_destroy(dialog);
      return;
  }
  
  // Znajdujemy pozycję kursora
  gtk_text_buffer_get_iter_at_mark(editor_buf, &start,
                                   gtk_text_buffer_get_insert(editor_buf));

  // Jeśli nie wewnątrz słowa, kończymy
  if (!gtk_text_iter_inside_word(&start)) {
    dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_OK,
                                    "Kursor musi być w środku słowa");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return;
  }

  // Znajdujemy początek i koniec słowa, a potem samo słowo 
  end = start;
  gtk_text_iter_forward_word_end(&end);
  start = end;
  gtk_text_iter_backward_word_start(&start);
   
  word = gtk_text_iter_get_text(&start, &end);

  // Zamieniamy na wide char (no prawie)
  wwword = g_utf8_to_ucs4_fast(word, -1, NULL);

  wword = better_word((wchar_t*)wwword);
  
  // Sprawdzamy
  if (dictionary_find(dict, (wchar_t *)wword)) {
    dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                    "Wszystko w porządku,\nśpij spokojnie");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
  else {
    // Czas korekty
    GtkWidget *vbox, *label, *combo;
    struct word_list hints;
    int i;
    const wchar_t * const *words;

    dictionary_hints(dict, (wchar_t *)wword, &hints);
    words = word_list_get(&hints);
    dialog = gtk_dialog_new_with_buttons("Korekta", NULL, 0, 
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_ACCEPT,
                                         GTK_STOCK_CANCEL,
                                         GTK_RESPONSE_REJECT,
                                         "Dodaj słofo",
                                         GTK_RESPONSE_APPLY,
                                         NULL);
    // W treści dialogu dwa elementy
    vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    // Tekst
    label = gtk_label_new("Coś nie tak, mam kilka propozycji");
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 1);

    // Spłukiwane menu
    combo = gtk_combo_box_text_new();
    for (i = 0; i < word_list_size(&hints); i++) {
      // Combo box lubi mieć Gtk
      char *uword = g_ucs4_to_utf8((gunichar *)words[i], -1, NULL, NULL, NULL);

      // Dodajemy kolejny element
      gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), uword);
      g_free(uword);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 1);
    gtk_widget_show(combo);

    int ret = gtk_dialog_run(GTK_DIALOG(dialog));
    if (ret == GTK_RESPONSE_ACCEPT) {
      char *korekta =
        gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));

      // Usuwamy stare
      gtk_text_buffer_delete(editor_buf, &start, &end);
      // Wstawiamy nowe
      gtk_text_buffer_insert(editor_buf, &start, korekta, -1);
      g_free(korekta);
    }
    else if (ret == GTK_RESPONSE_APPLY) {        
        GtkWidget *dialog_add;
        GtkWidget *entry;
        GtkWidget *vbox;


        dialog_add = gtk_dialog_new_with_buttons("Fprowadzańe wyrazu", NULL, 0,
                                                 GTK_STOCK_OK,
                                                 GTK_RESPONSE_ACCEPT,
                                                 GTK_STOCK_CANCEL,
                                                 GTK_RESPONSE_REJECT,
                                                 NULL);
        
        // W treści dialogu dwa elementy
        vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog_add));
        // Tekst
        label = gtk_label_new("Fpisz prosze wyras ktury, hćał byś dodać do słofńika.");
        gtk_widget_show(label);
        gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 1);

        entry = gtk_entry_new();

        gtk_box_pack_start(GTK_BOX(vbox), entry, TRUE, TRUE, 1);
        gtk_widget_show(entry);
        int ret2 = gtk_dialog_run(GTK_DIALOG(dialog_add));
        //
        if(ret2 == GTK_RESPONSE_ACCEPT)
        {
            const char *slowo = gtk_entry_get_text(GTK_ENTRY(entry));
            wchar_t *wslowo = (wchar_t*)g_utf8_to_ucs4_fast(slowo, -1, NULL);
            // Wstaw do słownika
            dictionary_insert(dict, wslowo);
            // Usuwamy stare
            gtk_text_buffer_delete(editor_buf, &start, &end);
            // Wstawiamy nowe
            gtk_text_buffer_insert(editor_buf, &start, slowo, -1);
            
            free(wslowo);
        }
        gtk_widget_destroy(dialog_add);
    }
    gtk_widget_destroy(dialog);
  }
  g_free(word);
  g_free(wwword);
  free(wword);
}


static void SelLang (GtkMenuItem *item, gpointer data) {
    GtkWidget *dialog;
    GtkWidget *vbox, *label, *combo;
    
    char   *list;
    size_t  llen;
    
    if(dictionary_lang_list(&list, &llen)<0)
    {
        dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                    "Niestety nie jestem f stanie stfierdzić\njakie słofniki zainstalofałeś.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
    
    dialog = gtk_dialog_new_with_buttons("Óztaf jenzyk zprafdzańa pisofńi", NULL, 0, 
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_ACCEPT,
                                         GTK_STOCK_CANCEL,
                                         GTK_RESPONSE_REJECT,
                                         "Nofy jenzyk",
                                         GTK_RESPONSE_APPLY,
                                         NULL);
    // W treści dialogu dwa elementy
    vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    // Tekst
    label = gtk_label_new("Wybież jenzyk spońirzszei listy.");
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 1);

    combo = gtk_combo_box_text_new();
    
    for (int i = 0; i < llen && llen > 1;) {
      int wlen = strlen(list+i);
      // Combo box lubi mieć Gtk
      char *uword = list+i;

      // Dodajemy kolejny element
      gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), uword);
      i += wlen + 1;
    }
    //gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 1);
    gtk_widget_show(combo);

    int ret = gtk_dialog_run(GTK_DIALOG(dialog));
    if (ret == GTK_RESPONSE_ACCEPT) {
      char *lang_name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
      if(lang != NULL)
      {
          dictionary_save_lang(dict, lang);
          dictionary_done(dict);
          free(lang);
      }
      int lang_len = strlen(lang_name);
      lang = malloc((lang_len+1)*sizeof(char));
      memcpy(lang, lang_name, lang_len+1);
      
      dict = dictionary_load_lang(lang);
          
      g_free(lang_name);
    }
    else if (ret == GTK_RESPONSE_APPLY) {        
        GtkWidget *dialog_add;
        GtkWidget *entry;
        GtkWidget *vbox;


        dialog_add = gtk_dialog_new_with_buttons("Fprowadzańe nazfy słofnika", NULL, 0,
                                                 GTK_STOCK_OK,
                                                 GTK_RESPONSE_ACCEPT,
                                                 GTK_STOCK_CANCEL,
                                                 GTK_RESPONSE_REJECT,
                                                 NULL);
        
        // W treści dialogu dwa elementy
        vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog_add));
        // Tekst
        label = gtk_label_new("Fpisz nazfe jenzyka ktury, hcerz ótfożyć.");
        gtk_widget_show(label);
        gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 1);

        entry = gtk_entry_new();

        gtk_box_pack_start(GTK_BOX(vbox), entry, TRUE, TRUE, 1);
        gtk_widget_show(entry);
        int ret2 = gtk_dialog_run(GTK_DIALOG(dialog_add));
        //
        if(ret2 == GTK_RESPONSE_ACCEPT)
        {
            const char *lang_name = gtk_entry_get_text(GTK_ENTRY(entry));
            if(lang != NULL)
            {
                dictionary_save_lang(dict, lang);
                dictionary_done(dict);
                free(lang);
            }
            int lang_len = strlen(lang_name);
            lang = malloc((lang_len+1)*sizeof(char));
            memcpy(lang, lang_name, lang_len+1);
            
            dict = dictionary_new();
            dictionary_save_lang(dict, lang);
            
            g_free((char*)lang_name);
        }
        gtk_widget_destroy(dialog_add);
    }
    gtk_widget_destroy(dialog);
}

extern GtkTextBuffer *editor_buf;

void Blooden(GtkMenuItem *item, gpointer data)
{
    if(dict == NULL)
    {
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_OK,
                                        "Najpierf wybież jenzyk aby zprafdzić poprafność tekstó.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(editor_buf, &end);
    while(1) {
        if(gtk_text_iter_is_end(&end)) break;
        char *word;
        gtk_text_iter_forward_word_end(&end); 
        start = end;
        gtk_text_iter_backward_word_start(&start); 
        word = gtk_text_iter_get_text(&start, &end);

        // Zamieniamy na wide char (no prawie)
        wchar_t *wwword = (wchar_t*)g_utf8_to_ucs4_fast(word, -1, NULL);

        wchar_t *wword = better_word(wwword);
        
        // Sprawdzamy
        if (dictionary_find(dict, (wchar_t *)wword) == false)
        {
            gtk_text_buffer_apply_tag_by_name(editor_buf, "blooded", &start, &end);
        }
        g_free(word);
        g_free(wwword);
        free(wword);
    }
}

void Cleanse(GtkMenuItem *item, gpointer data)
{
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(editor_buf, &start);
    gtk_text_buffer_get_end_iter(editor_buf, &end);
    gtk_text_buffer_remove_all_tags(editor_buf, &start, &end);
}

static void destroy (GtkWidget *widget, gpointer data) {
  // For security save the dictionary
  if(lang != NULL && dict != NULL)
  {
      dictionary_save_lang(dict, lang);
      dictionary_done(dict);
      free(lang);
  }
  else if(lang != NULL || dict != NULL) assert(false);
  gtk_main_quit();
}

// Tutaj dodacie nowe pozycje menu

void extend_menu (GtkWidget *menubar) {
  GtkWidget *spell_menu_item, *spell_menu, *check_item, *choose_lang, *blooden, *cleanse;

  spell_menu_item = gtk_menu_item_new_with_label("Spell");
  spell_menu = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(spell_menu_item), spell_menu);
  gtk_widget_show(spell_menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), spell_menu_item);

  check_item = gtk_menu_item_new_with_label("Check Word");
  g_signal_connect(G_OBJECT(check_item), "activate", 
                   G_CALLBACK(WhatCheck), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(spell_menu), check_item);  
  gtk_widget_show(check_item);
  
  choose_lang = gtk_menu_item_new_with_label("Czuz langłydż");
  g_signal_connect(G_OBJECT(choose_lang), "activate",
                   G_CALLBACK(SelLang), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(spell_menu), choose_lang);
  gtk_widget_show(choose_lang);
  
  blooden = gtk_menu_item_new_with_label("Pomalói błendy na krfafy kolor");
  g_signal_connect(G_OBJECT(blooden), "activate",
                   G_CALLBACK(Blooden), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(spell_menu), blooden);
  gtk_widget_show(blooden);
  
  cleanse = gtk_menu_item_new_with_label("Zmyi farbe z tekstó");
  g_signal_connect(G_OBJECT(cleanse), "activate",
                   G_CALLBACK(Cleanse), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(spell_menu), cleanse);
  gtk_widget_show(cleanse);
  
  g_signal_connect(editor_window, "destroy",
                   G_CALLBACK(destroy), NULL);
}

/*EOF*/
