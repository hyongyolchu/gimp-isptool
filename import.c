/* GIMP Plugin
 * Import file with various color space
 * Copyright (C) 2012 HyongYol Chu <hyongyolchu@gmail.com>.
 * All Rights Reserved.
 */

#include <glib/gstdio.h>
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

/* MACRO */
#define IMPORT_PLUGIN_NAME "file-cstool-import"

/* Global variables */

/* Local function prototypes */
static void query(void);
static void run(const gchar* name,
		gint nparams,
		const GimpParam* param,
		gint* nreturn_vals,
		GimpParam** return_vals);

static gboolean format_dialog(const gchar* name);
static guchar* load_file(const gchar* fname);
static gboolean make_drawable(GimpDrawable* drawable, guchar* buf);

GimpPlugInInfo PLUG_IN_INFO =
{
	NULL,	/* init */
	NULL,	/* quit */
	query,	/* query */
	run		/* run */
};

MAIN()

static void query(void)
{
	static const GimpParamDef load_args[] =
	{
		{GIMP_PDB_INT32, "run-mode", "Interactive, non-interactive"},
		{GIMP_PDB_STRING, "filename", "The name of the file to load"},
		{GIMP_PDB_STRING, "raw-filename", "The name entered"}
	};
	static const GimpParamDef load_return_vals[] =
	{
		{GIMP_PDB_IMAGE, "image", "Output image"}
	};

	gimp_install_procedure(
			IMPORT_PLUGIN_NAME,
			"Import files of various color space",
			"Import files of various color space",
			"HyongYol Chu",
			"Copyright HyongYol Chu",
			"2012",
			"_00Some Image to import",
			NULL,
			GIMP_PLUGIN,
			G_N_ELEMENTS(load_args),
			G_N_ELEMENTS(load_return_vals),
			load_args, load_return_vals);

	gimp_register_file_handler_mime(IMPORT_PLUGIN_NAME, "image/raw");
	gimp_register_magic_load_handler(IMPORT_PLUGIN_NAME, "raw", "", "0, string,raw");
}

static void run(const gchar* name,
				gint nparams,
				const GimpParam* param,
				gint* nreturn_vals,
				GimpParam** return_vals)
{
	static GimpParam values[2];
	GimpPDBStatusType status = GIMP_PDB_SUCCESS;
	GimpRunMode run_mode;

	/* Setting mandatory output values */
	*nreturn_vals = 1;
	*return_vals = values;

	values[0].type = GIMP_PDB_STATUS;
	values[0].data.d_status = status;

	/* Getting run_mode - we won't display a dialog if
	 * we are in NONINTERACTIVE mode */
	run_mode = param[0].data.d_int32;

	if(0 == strcmp(name, IMPORT_PLUGIN_NAME))
	{
		switch(run_mode)
		{
			case GIMP_RUN_INTERACTIVE:
				/* Display the dialog */
				if(!format_dialog(param[1].data.d_string));
				{
					return;
				}
				break;

			case GIMP_RUN_NONINTERACTIVE:
				break;

			default:
				break;
		}
	}
	else
	{
		status = GIMP_PDB_CALLING_ERROR;
	}

	if(GIMP_PDB_SUCCESS != status)
	{
		*nreturn_vals = 2;
		values[1].type = GIMP_PDB_STRING;
		values[1].data.d_string = "";
	}

	values[0].data.d_status = status;	
}

static gboolean format_dialog(const gchar* fname)
{
	GtkWidget* 		dialog;
	GtkWidget* 		main_vbox;
	GtkWidget* 		preview;
	GimpDrawable* 	drawable;
	gboolean 		run;
	gint32 			image;
	guchar*			buf;

	/* Construct drawable */
	image = gimp_image_new(176, 144, GIMP_RGB);
	drawable = gimp_drawable_get(image);
	buf = load_file(fname);
	make_drawable(drawable, buf);

	gimp_ui_init("Format", FALSE);

	dialog = gimp_dialog_new(IMPORT_PLUGIN_NAME, IMPORT_PLUGIN_NAME,
							 NULL, 0,
							 gimp_standard_help_func, IMPORT_PLUGIN_NAME,
							 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							 GTK_STOCK_OK, GTK_RESPONSE_OK,
							 NULL);

	main_vbox = gtk_vbox_new(FALSE, 6);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), main_vbox);
	gtk_widget_show(main_vbox);

	preview = gimp_drawable_preview_new(drawable, NULL);
	gtk_box_pack_start(GTK_BOX(main_vbox), preview, TRUE, TRUE, 0);
	gtk_widget_show(preview);

	gtk_widget_show(dialog);
	run = (GTK_RESPONSE_OK == gimp_dialog_run(GIMP_DIALOG(dialog)));
	gtk_widget_destroy(dialog);

	gimp_drawable_detach(drawable);

	gimp_image_delete(image);

	return run;
}

static guchar* load_file(const gchar* fname)
{
	FILE* fd;
	guchar* buf;
	struct stat st;

	if(-1 == g_stat(fname, &st))
	{
		return NULL;
	}

	buf = g_malloc(st.st_size);
	if(NULL == buf)
	{
		return NULL;
	}
	
	fd = g_fopen(fname, "rb");
	if(!fd)
	{
		fclose(fd);
		return NULL;
	}

	if(fread(buf, st.st_size, 1, fd) <= 0)
	{
		fclose(fd);
		return NULL;
	}

	fclose(fd);

	return buf;
}
	
static gboolean make_drawable(GimpDrawable* drawable, guchar* buf)
{
	gint			x1, y1, x2, y2, width, height, channels;
	gint			i, j, k;
	GimpPixelRgn 	rgn;
	guchar			output[4];
	guchar*			ptr = buf;

	gimp_drawable_mask_bounds(drawable->drawable_id, &x1, &y1, &x2, &y2);
	width = x2 - x1;
	height = y2 - y1;
	channels = gimp_drawable_bpp(drawable->drawable_id);
	gimp_pixel_rgn_init(&rgn, drawable, x1, y1, width, height, TRUE, TRUE);

	for(i = x1;i < x2;i++)
	{
		for(j = y1;j < y2;j++)
		{
			for(k = 0;k < channels;k++)
			{
				output[k] = *ptr;
			}
			ptr++;
			gimp_pixel_rgn_set_pixel(&rgn, output, i, j);
		}
	}

	gimp_drawable_flush(drawable);
	gimp_drawable_merge_shadow(drawable->drawable_id, TRUE);
	gimp_drawable_update(drawable->drawable_id, x1, y1, width, height);

	return TRUE;
}
