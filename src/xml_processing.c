/**
 * xml_processing.c - XML processing support for SciteProj
 *
 *  Copyright 2006 Roy Wood, 2008-2011 Andreas Ronnquist
 *
 * This file is part of SciteProj.
 * 
 * SciteProj is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SciteProj is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SciteProj.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>

#include <string.h>

#include "xml_processing.h"
#include "string_utils.h"
#include "tree_manipulation.h"
#include "clicked_node.h"
#include "gui.h"
#include "file_utils.h"

#include "prefs.h"

#define APP_SCITEPROJ_ERROR g_quark_from_static_string("APP_XML_ERROR")

/*
 * This module loads or saves an xml file, marshalling or unmarshalling data to/from a GtkTreeStore, 
 * including adding nodes to the tree.
 * The xml file is assumed to consist of <file> elements and/or <group> elements.
 * The pathnames of the files are contained in the text of the <file> elements, and the name of
 * the group elements are contained in a "name" attribute of the <group> element.
 */

/*
 *	Forward declare the static functions in the module
 */
static void start_element_cb(GMarkupParseContext *context, const gchar *element_name, const gchar **attribute_names, const gchar **attribute_values, gpointer user_data, GError **error);
static void end_element_cb(GMarkupParseContext *context, const gchar *element_name, gpointer user_data, GError **error);
static void text_cb(GMarkupParseContext *context, const gchar *text, gsize text_len, gpointer user_data, GError **error);
static void passthrough_cb(GMarkupParseContext *context, const gchar *passthrough_text, gsize text_len, gpointer user_data, GError **error);
static void error_cb(GMarkupParseContext *context, GError *error, gpointer user_data);

static gboolean marshal_subtree(GtkTreeModel *treeModel, GtkTreeIter *currentIter, gchar **xmlString, GError **err);



/*
 *	ParseStruct is used to store the state of a parsing operation
 */
struct ParseStruct {
	GtkTreeIter currentIter; // reference to the current group node
	
	int depth; // depth of parse
	gboolean inFileElement; // flag indicating whether we are processing a file node
	GtkTreeStore *treeStore; // pointer to the GtkTreeStore we are populating
	
	gboolean valid_file;
};

typedef struct ParseStruct ParseStruct;

GtkTreeIter prevFileIterArray[100];
int currentPrevFileIter=0;

gboolean prevFileIterValid[100];


/**
 * Handle the start of an element during parsing.
 *
 * @param context is the parse context
 * @param element_name is the name of the node being closed
 * @param attribute_names is the list of attribute names
 * @param attribute_values is the list of attribute values
 * @param user_data is a pointer to the ParseStruct struct used to store state info during parsing
 * @param error is the error object
 */
static void start_element_cb(GMarkupParseContext *context, const gchar *element_name, const gchar **attribute_names, const gchar **attribute_values, gpointer user_data, GError **error)
{
	ParseStruct *parseStruct = (ParseStruct *) user_data;
	int i=0;
	
	g_assert(parseStruct != NULL);
	g_assert(parseStruct->treeStore != NULL);
	
	gboolean expanded=FALSE;
	
	
	if (strcmp(element_name,"root")==0) {
		
		for (i=0;attribute_names[i]!=NULL;++i) {
			
			if (strcmp(attribute_names[i],"identifier")==0) {
				if (strcmp(attribute_values[i],"sciteproj project file")==0) {
					parseStruct->valid_file=TRUE;
				}
			}
		}
		
	} else if (strcmp(element_name, "group") == 0) {
		// Is it a "group" element?
		
		// Extract the "name" or "is_expanded" attribute, if they exist
		const gchar *groupname = "<anonymous>";
		
		for (i = 0; attribute_names[i] != NULL; ++i) {
			if (strcmp(attribute_names[i], "name") == 0) {
				groupname = attribute_values[i];
			}
			
			else if (strcmp(attribute_names[i], "expanded") == 0) {
				if (strcmp(attribute_values[i], "TRUE") == 0 || strcmp(attribute_values[i], "true") == 0) {
					expanded = TRUE;
					
				}
			}
		}
				
		// Add to the root of the tree, or as a child of an existing group?
		if (parseStruct->depth <= 0) {
			
			add_tree_group(NULL, ADD_CHILD, groupname, expanded, &(parseStruct->currentIter), error);
			
			set_tree_node_expanded(&(parseStruct->currentIter),expanded,error);
			
			prevFileIterValid[currentPrevFileIter]=FALSE;
			prevFileIterArray[currentPrevFileIter]=parseStruct->currentIter;
			currentPrevFileIter++;
			prevFileIterArray[currentPrevFileIter]=parseStruct->currentIter;

		}
		else {
			
			add_tree_group(&(parseStruct->currentIter), ADD_CHILD, groupname, expanded, &(parseStruct->currentIter), error);
			
			set_tree_node_expanded(&(parseStruct->currentIter),expanded,error);
			
			prevFileIterValid[currentPrevFileIter]=FALSE;
			prevFileIterArray[currentPrevFileIter]=parseStruct->currentIter;
			currentPrevFileIter++;

			prevFileIterValid[currentPrevFileIter]=prevFileIterValid[currentPrevFileIter];
		}
		
		// Increment the depth
		parseStruct->depth++;
		
		// Note that prevFileIter is not valid
		prevFileIterValid[currentPrevFileIter]=FALSE;
	}
	else if (strcmp(element_name, "file") == 0) {
		
		// Note that we're now in a file element, so text_cb() will extract and use subsequent text
		parseStruct->inFileElement = TRUE;
	}
}



/**
 * Handle the end of an element during parsing.
 *
 * @param context is the parse context
 * @param element_name is the name of the node being closed
 * @param user_data is a pointer to the ParseStruct struct used to store state info during parsing
 * @param error is the error object
 */
static void end_element_cb(GMarkupParseContext *context, const gchar *element_name, gpointer user_data, GError **error)
{
	ParseStruct *parseStruct = (ParseStruct *) user_data;
	
	g_assert(parseStruct != NULL);
	g_assert(parseStruct->treeStore != NULL);
	
	if (strcmp(element_name, "group") == 0) {
		
		// End of a group node, so decrement the depth and step back up the tree one level
		if (currentPrevFileIter>0) currentPrevFileIter--;
					
		if (parseStruct->depth > 0) {
			GtkTreeIter parentIter;
		
			GtkTreeIter tempIter=prevFileIterArray[currentPrevFileIter]; //parseStruct->currentIter;
			
			// Sets parentIter to the parent of tempIter if it returns TRUE, otherwise 
			if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(parseStruct->treeStore), &parentIter, &(tempIter))) {
				
				parseStruct->currentIter = parentIter;
				
				if (parseStruct->depth>0) parseStruct->depth -= 1;
				
			} else {
				parseStruct->currentIter = tempIter;
				
				if (parseStruct->depth>0) parseStruct->depth -= 1;
			}
		}
	}
	else if (strcmp(element_name, "file") == 0) {
		parseStruct->inFileElement = FALSE;
	}
}



/**
 * Handle text of an element data during parsing.
 *
 * @param context is the parse context
 * @param passthrough_text is the passthrough data (comments, etc.)
 * @param text_len is the length of the passthrough data
 * @param user_data is a pointer to the ParseStruct struct used to store state info during parsing
 * @param error is the error object
 */
static void text_cb(GMarkupParseContext *context, const gchar *text, gsize text_len, gpointer user_data, GError **error)
{
	ParseStruct *parseStruct = (ParseStruct *) user_data;
	
	g_assert(parseStruct != NULL);
	g_assert(parseStruct->treeStore != NULL);
	
	if (text == NULL || strlen(text) <= 0 || text[0] == '\n' || !parseStruct->inFileElement) {
		return;
	}
	
	// Append to root or an existing node?
	if (parseStruct->depth <= 0) {
		
		if (prevFileIterValid[currentPrevFileIter]) {
			add_tree_file(&(prevFileIterArray[currentPrevFileIter]), ADD_AFTER, text, &(prevFileIterArray[currentPrevFileIter]), FALSE, error);
			
		} else {
			add_tree_file(NULL, ADD_CHILD, text, &(prevFileIterArray[currentPrevFileIter]), FALSE, error);
		}
		
	} else {
			
		if (prevFileIterValid[currentPrevFileIter]) {
			add_tree_file(&(prevFileIterArray[currentPrevFileIter]), ADD_AFTER, text, &(prevFileIterArray[currentPrevFileIter]), FALSE, error);
		} else {
			add_tree_file(&(parseStruct->currentIter), ADD_CHILD,text, &(prevFileIterArray[currentPrevFileIter]), FALSE, error);
		}
	}
	
	prevFileIterValid[currentPrevFileIter]=TRUE;
}



/**
 * Handle passthrough data during parsing.
 *
 * @param context is the parse context
 * @param passthrough_text is the passthrough data (comments, etc.)
 * @param text_len is the length of the passthrough data
 * @param user_data is a pointer to the ParseStruct struct used to store state info during parsing
 * @param error is the error object
 */
static void passthrough_cb(GMarkupParseContext *context, const gchar *passthrough_text, gsize text_len, gpointer user_data, GError **error)
{
	// Don't care right now....
}


/**
 * Handle errors during parsing.
 *
 * @param context is the parse context
 * @param error is the error object
 * @param user_data is a pointer to the ParseStruct struct used to store state info during parsing
 */
static void error_cb(GMarkupParseContext *context, GError *error, gpointer user_data)
{
	// Don't care right now....
}


/**
 *	
 */
gboolean for_each_open_close_function(GtkTreeModel *model,GtkTreePath *path,GtkTreeIter *iter,gpointer data)
{
	gchar *filename=NULL;
	gboolean expanded;
	
	gtk_tree_model_get(model, iter, COLUMN_FILEPATH, &filename, COLUMN_EXPANDED, &expanded, -1);
	
	if (path != NULL) {
				
		if (expanded) {
			expand_tree_row(path, TRUE);
		} else {
			collapse_tree_row(path);
		}
	}
	
	if (filename!=NULL) g_free(filename);
	
	return FALSE;
}


/**
 * Parse an XML file and populate a GtkTreeStore with the extracted data.
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 *
 * @param treeStore is the GtkTreeStore to populate
 * @param filepath is the path to the file to read and parse
 * @param err returns any errors
 */
gboolean load_parse_XML(GtkTreeStore *treeStore, const char *filepath, GError **err)
{
	g_assert(treeStore != NULL);
	g_assert(filepath != NULL);
	
	gboolean finalResult = FALSE;
	GMarkupParseContext *parseContext = NULL;
	GMarkupParser parser = { start_element_cb, end_element_cb, text_cb, passthrough_cb, error_cb };
	gsize xmlSize = 0;
	gchar *xml = NULL;
	ParseStruct parseStruct;
	
	// Read in the file
	if (!g_file_get_contents(filepath, &xml, &xmlSize, err)) {
		goto EXITPOINT;
	}
	
	// Clear any data that might already be in the tree
	gtk_tree_store_clear(treeStore);
	
	// Create an XML parser
	parseStruct.depth = 0;
	parseStruct.inFileElement = FALSE;
	parseStruct.treeStore = treeStore;
	parseStruct.valid_file=FALSE;
	
	parseContext = g_markup_parse_context_new(&parser, (GMarkupParseFlags) 0, &parseStruct, NULL);
	
	// Parse the data....
	if (!g_markup_parse_context_parse(parseContext, (char *) xml, xmlSize, err)) {
		goto EXITPOINT;
	}

	
	if (!parseStruct.valid_file) {
		if (gPrefs.identify_sciteproj_xml) {
			g_set_error(err, APP_SCITEPROJ_ERROR, -1, "%s: Not a valid sciteproj XML file!", __func__);
			goto EXITPOINT;
		}
	}
	
	// Check Open/Closed?
	gtk_tree_model_foreach(GTK_TREE_MODEL(treeStore),for_each_open_close_function,&parseStruct);
	
	finalResult = TRUE;
	
	
EXITPOINT:
	
	if (parseContext) g_markup_parse_context_free(parseContext);
	if (xml) g_free(xml);
	
	if (!finalResult) {
		// A parsing error occurred, so junk it all
		gtk_tree_store_clear(treeStore);
	}
	
	return finalResult;
}



/**
 * Marshal a subtree of a GtkTreeModel by appending its textual representation to an input string.
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 *
 * @param treeModel is the GtkTreeModel
 * @param subtreeIter is the iterator reference to the subtree to process
 * @param xmlString returns the marshaled output; the text is appended to this string
 * @param err returns any errors
 */
static gboolean marshal_subtree(GtkTreeModel *treeModel, GtkTreeIter *subtreeIter, gchar **xmlString, GError **err)
{
	g_assert(treeModel != NULL);
	g_assert(subtreeIter != NULL);
	g_assert(xmlString != NULL);
	
	gboolean finalResult = FALSE;
	gint itemType;
	gchar *filepathString = NULL;
	gchar *tmpXML = NULL;
	gchar *indentString = NULL;
	gint depth;
	
	gboolean expanded=FALSE;
	
	
	// Set up indent
	
	depth = gtk_tree_store_iter_depth(GTK_TREE_STORE(treeModel), subtreeIter) + 1;
	
	indentString = g_strnfill(depth, '\t');
	
	// Get relevant data for current node

	gchar *temp=NULL;

	gtk_tree_model_get(treeModel, subtreeIter, 
					COLUMN_FILEPATH, &temp, 
					COLUMN_ITEMTYPE, &itemType, 
					-1);

	filepathString=fix_separators(temp);

	g_free(temp);
	
	
	
	if (itemType == ITEMTYPE_FILE) {
		// Write out a simple file node
		
		tmpXML = g_markup_printf_escaped("%s<file>%s</file>\n", indentString, filepathString);
		
		if (!str_append(xmlString, tmpXML, err)) {
			goto EXITPOINT;
		}
	}
	else if (itemType == ITEMTYPE_GROUP) {

		GtkTreePath *path = gtk_tree_model_get_path(treeModel, subtreeIter);
		
		if (path != NULL) {
			expanded = tree_row_is_expanded(path);
			
			gtk_tree_path_free(path);
		}
		
		
		gchar *exp_string=NULL;
		if (expanded) {
			exp_string=g_strdup_printf("TRUE");
		} else {
			exp_string=g_strdup_printf("FALSE");
		}
				
		tmpXML = g_markup_printf_escaped("%s<group name='%s' expanded='%s'>\n", indentString, filepathString, exp_string);
				
		if (!str_append(xmlString, tmpXML, err)) {
			goto EXITPOINT;
		}
		
		GtkTreeIter childIter;
		if (gtk_tree_model_iter_children(treeModel, &childIter, subtreeIter)) {
		
			do {
				if (!marshal_subtree(treeModel, &childIter, xmlString, err)) {
					goto EXITPOINT;
				}
			} while (gtk_tree_model_iter_next(treeModel, &childIter));
		}
		
		// Don't forget to append the close tag
		if (!str_append(xmlString, indentString, err) || !str_append(xmlString, "</group>\n", err)) {
			goto EXITPOINT;
		}
	}
	
	finalResult = TRUE;
	

EXITPOINT:
	
	if (filepathString) g_free(filepathString);
	if (tmpXML) g_free(tmpXML);
	if (indentString) g_free(indentString);
	
	return finalResult;
}



/**
 * Save the contents of a GtkTreeModel to a file.
 *
 * @return TRUE on success, FALSE on failure (further details returned in err)
 * 
 * @param treeModel is the GtkTreeModel to save; the COLUMN_FILEPATH items are extracted and saved
 * @param filepath is the pathname for the target file
 * @param err returns any errors
 */
gboolean save_tree_XML(GtkTreeModel *treeModel, const gchar *filepath, GError **err)
{
	g_assert(treeModel != NULL);
	g_assert(filepath != NULL);
	
	gboolean finalResult = FALSE;
	GtkTreeIter iter;
	gchar *xmlString = NULL;
	
	
	// Add <root> of document
	
	if (!str_append(&xmlString, "<root identifier=\"sciteproj project file\">\n", err)) {
		goto EXITPOINT;
	}
	
	// Iterate over root level of tree
	
	if (gtk_tree_model_get_iter_first(treeModel, &iter)) {
		do {
			if (!marshal_subtree(treeModel, &iter, &xmlString, err)) {
				goto EXITPOINT;
			}
		} while (gtk_tree_model_iter_next(treeModel, &iter));
	}
	
	// Close <root>
	
	if (!str_append(&xmlString, "</root>\n", err)) {
		goto EXITPOINT;
	}
	
	// Write out file
	
	if (!g_file_set_contents(filepath, xmlString, -1, err)) {
		goto EXITPOINT;
	}
	
	finalResult = TRUE;
	
EXITPOINT:
	
	if (xmlString) g_free(xmlString);
	
	return finalResult;
}
