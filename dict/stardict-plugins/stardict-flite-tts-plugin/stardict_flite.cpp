/*
 * Copyright 2016 Hu Zheng <huzheng001@mail.com>
 *
 * This file is part of StarDict.
 *
 * StarDict is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StarDict is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with StarDict.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stardict_flite.h"
#include <glib/gi18n.h>

static const StarDictPluginSystemInfo *plugin_info = NULL;
static IAppDirs* gpAppDirs = NULL;


#include <flite/flite.h>

extern "C" {
	cst_voice *register_cmu_us_kal(const char *voxdir);
	cst_voice *register_cmu_time_awb(const char *voxdir);
	cst_voice *register_cmu_us_kal16(const char *voxdir);
	cst_voice *register_cmu_us_awb(const char *voxdir);
	cst_voice *register_cmu_us_rms(const char *voxdir);
	cst_voice *register_cmu_us_slt(const char *voxdir);
}


static std::string voice_engine;
static cst_voice *now_voice = NULL;


/* concatenate path1 and path2 inserting a path separator in between if needed. */
static std::string build_path(const std::string& path1, const std::string& path2)
{
	std::string res;
	res.reserve(path1.length() + 1 + path2.length());
	res = path1;
	if(!res.empty() && res[res.length()-1] != G_DIR_SEPARATOR)
		res += G_DIR_SEPARATOR_S;
	if(!path2.empty() && path2[0] == G_DIR_SEPARATOR)
		res.append(path2, 1, std::string::npos);
	else
		res.append(path2);
	return res;
}

static std::string get_cfg_filename()
{
	return build_path(gpAppDirs->get_user_config_dir(), "flite.cfg");
}

static void saytext(const char *text)
{
	if (now_voice == NULL) {
		now_voice = flite_voice_select(NULL);
	}
	if (now_voice) {
		flite_text_to_speech(text, now_voice, "play");
	}
}

static void on_test_tts_button_clicked(GtkWidget *widget, GtkEntry *entry)
{
	const char *word = gtk_entry_get_text(entry);
	saytext(word);
}

static void on_tts_combobox_changed(GtkComboBox *widget, gpointer data)
{
	gint index = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
	if (index == 1)
		voice_engine = "kal";
	else if (index == 2)
		voice_engine = "awb_time";
	else if (index == 3)
		voice_engine = "kal16";
	else if (index == 4)
		voice_engine = "awb";
	else if (index == 5)
		voice_engine = "rms";
	else if (index == 6)
		voice_engine = "slt";
	else
		voice_engine.clear();
	if (!voice_engine.empty()) {
		now_voice = flite_voice_select(voice_engine.c_str());
	} else {
		now_voice = flite_voice_select(NULL);
	}
	gchar *data1 = g_strdup_printf("[flite]\nvoice=%s\n", voice_engine.c_str());
	std::string res = get_cfg_filename();
	g_file_set_contents(res.c_str(), data1, -1, NULL);
	g_free(data1);
}

static void configure()
{
	GtkWidget *window = gtk_dialog_new_with_buttons(_("Flite TTS configuration"), GTK_WINDOW(plugin_info->pluginwin), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
#if GTK_MAJOR_VERSION >= 3
	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
#else
	GtkWidget *vbox = gtk_vbox_new(FALSE, 10);
#endif
	gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area(GTK_DIALOG(window))), vbox);
#if GTK_MAJOR_VERSION >= 3
	GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
#else
	GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
#endif
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	GtkWidget *label = gtk_label_new(_("Voice type:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);
	GtkWidget *combobox = gtk_combo_box_text_new();
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), _("Default"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), "kal");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), "awb_time");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), "kal16");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), "awb");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), "rms");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combobox), "slt");
	gint old_index;
	if (voice_engine == "kal")
		old_index = 1;
	else if (voice_engine == "awb_time")
		old_index = 2;
	else if (voice_engine == "kal16")
		old_index = 3;
	else if (voice_engine == "awb")
		old_index = 4;
	else if (voice_engine == "rms")
		old_index = 5;
	else if (voice_engine == "slt")
		old_index = 6;
	else
		old_index = 0;
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), old_index);
	g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (on_tts_combobox_changed), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), combobox, false, false, 0);
#if GTK_MAJOR_VERSION >= 3
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
#else
	hbox = gtk_hbox_new(FALSE, 5);
#endif
	gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);
	GtkWidget *entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), "This is the test text");
	gtk_box_pack_start(GTK_BOX(hbox), entry, true, true, 0);
	GtkWidget *button = gtk_button_new_with_label(_("Test"));
	gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(on_test_tts_button_clicked), GTK_ENTRY(entry));
	gtk_widget_show_all(vbox);
	gtk_dialog_run(GTK_DIALOG(window));
	gtk_widget_destroy (window);
}

bool stardict_plugin_init(StarDictPlugInObject *obj, IAppDirs* appDirs)
{
	g_debug(_("Loading Flite plug-in..."));
	if (strcmp(obj->version_str, PLUGIN_SYSTEM_VERSION)!=0) {
		g_print(_("Error: Flite plugin version doesn't match!\n"));
		return true;
	}
	obj->type = StarDictPlugInType_TTS;
	obj->info_xml = g_strdup_printf("<plugin_info><name>%s</name><version>1.0</version><short_desc>%s</short_desc><long_desc>%s</long_desc><author>Hu Zheng &lt;huzheng001@gmail.com&gt;</author><website>http://stardict-4.sourceforge.net</website></plugin_info>", _("Flite"), _("Flite TTS."), _("Pronounce words by Flite TTS engine."));
	obj->configure_func = configure;
	plugin_info = obj->plugin_info;
	gpAppDirs = appDirs;
	return false;
}

void stardict_plugin_exit(void)
{
	gpAppDirs = NULL;
}

bool stardict_tts_plugin_init(StarDictTtsPlugInObject *obj)
{
	flite_init();

	register_cmu_us_kal(NULL);
	register_cmu_time_awb(NULL);
	register_cmu_us_kal16(NULL);
	register_cmu_us_awb(NULL);
	register_cmu_us_rms(NULL);
	register_cmu_us_slt(NULL);

	std::string res = get_cfg_filename();
	if (!g_file_test(res.c_str(), G_FILE_TEST_EXISTS)) {
		g_file_set_contents(res.c_str(), "[flite]\nvoice=\n", -1, NULL);
	}
	GKeyFile *keyfile = g_key_file_new();
	g_key_file_load_from_file(keyfile, res.c_str(), G_KEY_FILE_NONE, NULL);
	gchar *str = g_key_file_get_string(keyfile, "flite", "voice", NULL);
	g_key_file_free(keyfile);
	if (str) {
		voice_engine = str;
		g_free(str);
	}
	if (!voice_engine.empty()) {
		now_voice = flite_voice_select(voice_engine.c_str());
	} else {
		now_voice = flite_voice_select(NULL);
	}
	obj->saytext_func = saytext;
	obj->tts_name = _("Flite TTS");
	g_print(_("Flite TTS plug-in loaded.\n"));
	return false;
}
