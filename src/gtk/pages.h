#include <gtk/gtk.h>
#include "baseball_objects.h"

struct roster_page{
	GtkWidget *main;
	GtkWidget *lineup;
	GtkWidget *rotation;
};

void roster_page_init(struct roster_page *page);
void roster_page_update(GtkWidget *widget, gpointer data);

struct setup_page{
	GtkWidget *main;
	GtkWidget *current_file;
	GtkWidget *current_user;
	GtkWidget *new_user_text;
	GtkWidget *new_user_team;
};

void setup_page_init(struct setup_page *page);
void setup_page_update(GtkWidget *widget, gpointer data);

struct standings_page{
	GtkWidget *main;
};

void standings_page_init(struct standings_page *page);
void standings_page_update(GtkWidget *widget, gpointer data);

struct simulate_page{
	GtkWidget *main;
};

void simulate_page_init(struct simulate_page *page);
void simulate_page_update(GtkWidget *widget, gpointer data);

struct stats_page{
	GtkWidget *main;
	GtkWidget *hitter_stats;
	GtkWidget *pitcher_stats;
};

void stats_page_init(struct stats_page *page);
void stats_page_update(GtkWidget *widget, gpointer data);
