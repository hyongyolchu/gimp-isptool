/* GIMP Plugin
 * Import file with various color space
 * Copyright (C) 2012 HyongYol Chu <hyongyolchu@gmail.com>.
 * All Rights Reserved.
 */

#include <glib/gstdio.h>
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#if _WIN32
#pragma warning(disable:4996)
#endif

/* MACRO */
#define IMPORT_PLUGIN_NAME "file-cstool-import"
#define DEFAULT_WIDTH	176
#define DEFAULT_HEIGHT	144

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
static void yuv444_to_rgb888(guchar rgb[4], guchar y, guchar u, guchar v);

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

	if(0 == g_strcmp0(name, IMPORT_PLUGIN_NAME))
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
	GtkWidget* 		main_hbox;
	GtkWidget* 		preview;
	GtkWidget* 		frame;
	GtkWidget* 		width_ent;
	GtkWidget* 		height_ent;
	GtkWidget* 		button;
	//GSList*			group;
	GimpDrawable* 	drawable;
	gboolean 		run;
	gint32 			image, layer;
	guchar*			buf;
	guchar			str[100];

	/* Construct drawable */
	image = gimp_image_new(DEFAULT_WIDTH, DEFAULT_HEIGHT, GIMP_RGB);
	layer = gimp_layer_new(image, "default", DEFAULT_WIDTH, DEFAULT_HEIGHT, 
						   GIMP_RGB_IMAGE, 100, GIMP_NORMAL_MODE);
	gimp_image_add_layer(image, layer, -1);
	drawable = gimp_drawable_get(layer);
	buf = load_file(fname);
	make_drawable(drawable, buf);
	gimp_drawable_set_visible(layer, TRUE);

	/* Make UI from here */
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

	frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(main_vbox), frame);
	gtk_frame_set_label(GTK_FRAME(frame), "Width and Height");
	gtk_widget_show(frame);

	main_hbox = gtk_hbox_new(FALSE, 6);
	gtk_container_add(GTK_CONTAINER(frame), main_hbox);
	gtk_widget_show(main_hbox);

	width_ent = gtk_entry_new();
	sprintf(str, "%d", DEFAULT_WIDTH);
	gtk_entry_set_text(GTK_ENTRY(width_ent), str);
	gtk_box_pack_start(GTK_BOX(main_hbox), width_ent, TRUE, TRUE, 0);
	gtk_widget_show(width_ent);

	height_ent = gtk_entry_new();
	sprintf(str, "%d", DEFAULT_HEIGHT);
	gtk_entry_set_text(GTK_ENTRY(height_ent), str);
	gtk_box_pack_start(GTK_BOX(main_hbox), height_ent, TRUE, TRUE, 0);
	gtk_widget_show(height_ent);

	frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(main_vbox), frame);
	gtk_frame_set_label(GTK_FRAME(frame), "Y Component order");
	gtk_widget_show(frame);

	main_hbox = gtk_hbox_new(FALSE, 6);
	gtk_container_add(GTK_CONTAINER(frame), main_hbox);
	gtk_widget_show(main_hbox);

	button = gtk_radio_button_new_with_label(NULL, "First");
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button), "Second");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button), "Third");
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(main_vbox), frame);
	gtk_frame_set_label(GTK_FRAME(frame), "Cb Component number");
	gtk_widget_show(frame);

	main_hbox = gtk_hbox_new(FALSE, 6);
	gtk_container_add(GTK_CONTAINER(frame), main_hbox);
	gtk_widget_show(main_hbox);

	button = gtk_radio_button_new_with_label(NULL, "Zero");
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button), "One");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button), "Two");
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(main_vbox), frame);
	gtk_frame_set_label(GTK_FRAME(frame), "Cb Component order");
	gtk_widget_show(frame);

	main_hbox = gtk_hbox_new(FALSE, 6);
	gtk_container_add(GTK_CONTAINER(frame), main_hbox);
	gtk_widget_show(main_hbox);

	button = gtk_radio_button_new_with_label(NULL, "First");
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button), "Second");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button), "Third");
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(main_vbox), frame);
	gtk_frame_set_label(GTK_FRAME(frame), "Cr Component number");
	gtk_widget_show(frame);

	main_hbox = gtk_hbox_new(FALSE, 6);
	gtk_container_add(GTK_CONTAINER(frame), main_hbox);
	gtk_widget_show(main_hbox);

	button = gtk_radio_button_new_with_label(NULL, "Zero");
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button), "One");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button), "Two");
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(main_vbox), frame);
	gtk_frame_set_label(GTK_FRAME(frame), "Cr Component order");
	gtk_widget_show(frame);

	main_hbox = gtk_hbox_new(FALSE, 6);
	gtk_container_add(GTK_CONTAINER(frame), main_hbox);
	gtk_widget_show(main_hbox);

	button = gtk_radio_button_new_with_label(NULL, "First");
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button), "Second");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(button), "Second");
	gtk_box_pack_start(GTK_BOX(main_hbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

	button = gtk_button_new_with_label("Apply");
	gtk_box_pack_start(GTK_BOX(main_vbox), button, TRUE, TRUE, 0);
	gtk_widget_show(button);

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
	GStatBuf st;

	if(-1 == g_stat(fname, &st))
	{
		return NULL;
	}

	buf = (guchar*)g_malloc(st.st_size);
	if(NULL == buf)
	{
		return NULL;
	}
	
	fd = fopen(fname, "rb");
	if(!fd)
	{
		fclose(fd);
		return NULL;
	}

	if(fread(buf, 1, st.st_size, fd) != st.st_size)
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
	guchar*			y_ptr, *u_ptr, *v_ptr;
	guchar*			row;
	gint			mode;

	gimp_drawable_mask_bounds(drawable->drawable_id, &x1, &y1, &x2, &y2);
	width = x2 - x1;
	height = y2 - y1;
	channels = gimp_drawable_bpp(drawable->drawable_id);
	gimp_pixel_rgn_init(&rgn, drawable, x1, y1, width, height, TRUE, TRUE);

	row = g_new(guchar, channels * width);

	mode = 6;
	switch(mode)
	{
	case 0:	// uyv444_inter_packed
		break;
	case 1:	// uyv444_inter_packed
		break;
	case 2:	// uyv444_inter_planar
		break;
	case 3:	// uyv444_prog_packed
		break;
	case 4:	// uyv444_prog_planar
		break;
	case 5:	// uyvy422_inter_packed
		break;
	case 6:	// uyvy422_prog_packed
		y_ptr = buf + 1;
		u_ptr = buf;
		v_ptr = buf + 2;
		break;
	case 7:	// yuv420_inter_planar
		break;
	case 8:	// yuv420_prog_planar
		break;
	case 9:	// yuv422_inter_planar
		break;
	case 10:	// yuv422_prog_planar
		break;
	case 11:	// yuv444_inter_packed
		break;
	case 12:	// yuv444_inter_planar
		break;
	case 13:	// yuv444_prog_packed
		break;
	case 14:	// yuv444_prog_planar
		break;
	case 15:	// yuyv422_inter_packed
		break;
	case 16:	// yuyv422_prog_packed
		break;
	case 17:	// yvu420_inter_planar
		break;
	case 18:	// yvu420_prog_planar
		break;
	case 19:	// yvu422_inter_planar
		break;
	case 20:	// yvu422_prog_planar
		break;
	case 21:	// yvu444_inter_packed
		break;
	case 22:	// yvu444_inter_planar
		break;
	case 23:	// yvu444_prog_packed
		break;
	case 24:	// yvu444_prog_planar
		break;
	case 25:	// yvyu422_inter_packed
		break;
	case 26:	// yvyu422_prog_packed
		break;
	} // switch(mode)

	for(j = y1;j < y2;++j)
	{
		for(i = x1;i < x2;++i)
		{
			yuv444_to_rgb888(output, *y_ptr, *u_ptr, *v_ptr);
			for(k = 0;k < channels;k++)
			{
				row[channels * (i - x1) + k] = output[k];
			}

			switch(mode)
			{
			case 6:	// uyvy422_prog_packed
				y_ptr += 2;
				if(0 != (i % 2))
				{
					u_ptr += 4;
					v_ptr += 4;
				}
				break;
			} // switch(mode)
		} // i
		gimp_pixel_rgn_set_row(&rgn, row, x1, j, width);
	} // j

	g_free(row);

	gimp_drawable_flush(drawable);
	gimp_drawable_merge_shadow(drawable->drawable_id, TRUE);
	gimp_drawable_update(drawable->drawable_id, x1, y1, width, height);

	return TRUE;
}

static void yuv444_to_rgb888(guchar rgb[4], guchar y, guchar u, guchar v)
{
	gint c = y - 16;
	gint d = u - 128;
	gint e = v - 128;

	rgb[0] = CLAMP((298 * c + 409 * e + 128) >> 8, 0, 255);
	rgb[1] = CLAMP((298 * c - 100 * d - 208 * e + 128) >> 8, 0, 255);
	rgb[2] = CLAMP((298 * c + 516 * d + 128) >> 8, 0, 255);
	rgb[3] = 0;
}
