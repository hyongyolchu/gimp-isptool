/* GIMP Plugin
 * Import file with various color space
 * Copyright (C) 2012 HyongYol Chu <hyongyolchu@gmail.com>.
 * All Rights Reserved.
 */

#include <libgimp/gimp.h>

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
			"_Some Image to import",
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
