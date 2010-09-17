/*
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __CAIRO_DOCK_GAUGE__
#define  __CAIRO_DOCK_GAUGE__

//#include "cairo-dock-struct.h"
//#include "cairo-dock-data-renderer.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
G_BEGIN_DECLS

/**
*@file cairo-dock-gauge.h This class defines the Gauge, which derives from the DataRenderer.
* All you need to know is the attributes that define a Gauge, the API to use is the common API for DataRenderer, defined in cairo-dock-data-renderer.h.
*/

/// Attributes of a Gauge.
typedef struct _CairoGaugeAttribute CairoGaugeAttribute;
struct _CairoGaugeAttribute {
	/// General attributes of any DataRenderer.
	CairoDataRendererAttribute rendererAttribute;
	/// path to a gauge theme.
	const gchar *cThemePath;
};


typedef struct {
	RsvgHandle *pSvgHandle;
	cairo_surface_t *pSurface;
	gint sizeX;
	gint sizeY;
	GLuint iTexture;
} GaugeImage;

typedef struct {
	// needle
	gdouble posX, posY;
	gdouble posStart, posStop;
	gdouble direction;
	gint iNeedleRealWidth, iNeedleRealHeight;
	gdouble iNeedleOffsetX, iNeedleOffsetY;
	gdouble fNeedleScale;
	gint iNeedleWidth, iNeedleHeight;
	GaugeImage *pImageNeedle;
	// image list
	gint iNbImages;
	gint iNbImageLoaded;
	GaugeImage *pImageList;
	// value text zone
	CairoDataRendererTextParam textZone;
	// logo zone
	CairoDataRendererEmblemParam emblem;
	// label text zone
	CairoDataRendererTextParam labelZone;
} GaugeIndicator;

typedef struct {
	CairoDataRenderer dataRenderer;
	gchar *cThemeName;
	GaugeImage *pImageBackground;
	GaugeImage *pImageForeground;
	GList *pIndicatorList;
} Gauge;


void cairo_dock_register_data_renderer_gauge (void);


G_END_DECLS
#endif
