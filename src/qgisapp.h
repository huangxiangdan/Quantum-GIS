/***************************************************************************
                          qgisapp.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman@mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGISAPP_H
#define QGISAPP_H
class QCanvas;
class QRect;
class QCanvasView;
class QStringList;
class QScrollView;
class QgsPoint;
class QgsLegend;
class QVBox;
class QCursor;
class QListView;
class QListViewItem;
#include "qgisappbase.h"

class QgsMapCanvas;
/*! \class QgisApp
 * \brief Main window for the Qgis application
 */
class QgisApp : public QgisAppBase  {
	Q_OBJECT
public:
//! Constructor
	QgisApp(QWidget *parent=0, const char * name=0, WFlags fl = WType_TopLevel );
	
	~QgisApp();
	//public slots:
	//! Add a layer to the map
	void addLayer();
	//! Add a databaselayer to the map
	void addDatabaseLayer();
	//! Exit Qgis
	void fileExit();
	//! Set map tool to Zoom out
 	void zoomOut();
	//! Set map tool to Zoom in
  	void zoomIn();
	//! Zoom to full extent
	void zoomFull();
  //! Zoom to the previous extent
  void zoomPrevious();
	//! Set map tool to pan
	void pan();
	void identify();
	//! Read Well Known Binary stream from PostGIS
	//void readWKB(const char *, QStringList tables);
	//! Draw a point on the map canvas
	void drawPoint(double x, double y);
	//! draw layers
	void drawLayers();
	//! test function
	void testButton();
	//! About QGis
	void about();
public slots:
//! Slot to show the map coordinate position of the mouse cursor
	void showMouseCoordinate(QgsPoint &);
	//! Show layer properties for the selected layer
	void layerProperties(QListViewItem *);
	//! Show layer properties for selected layer (called by right-click menu)
	void layerProperties();
	//! Show the right-click menu for the legend
	void rightClickLegendMenu(QListViewItem *, const QPoint &, int );
	//! Remove a layer from the map and legend
	void removeLayer();
private:
//! Popup menu
QPopupMenu *popMenu;
//! Legend list view control
QListView *legendView;
	//! Map canvas
QgsMapCanvas *mapCanvas;
//! Table of contents (legend) for the map
QgsLegend *mapLegend;
QCursor *mapCursor;
//! scale factor
 double scaleFactor;
 //! Current map window extent in real-world coordinates
 QRect *mapWindow;
 //! Current map tool
 int mapTool;
 QCursor *cursorZoomIn;
};

#endif
