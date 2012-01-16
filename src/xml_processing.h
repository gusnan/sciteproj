/**
 * xml_processing.h - XML processing support for SciteProj
 *
 *  Copyright 2006 Roy Wood, 2008-2012 Andreas Rönnquist
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
#ifndef __HEADER_XML_PROCESSING_
#define __HEADER_XML_PROCESSING_

// Save the contents of a GtkTreeModel into a file
extern gboolean save_tree_XML(GtkTreeModel *treeModel, const gchar *filepath, GError **err);

// Load and parse an XML file, populating a GtkTreeStore with the data
extern gboolean load_parse_XML(GtkTreeStore *treeStore, const char *filePath, GError **err);

#endif /*__HEADER_XML_PROCESSING_*/
