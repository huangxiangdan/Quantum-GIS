/***************************************************************************
                          qgsmaplayer.h  -  description
                             -------------------
    begin                : Fri Jun 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSMAPLAYER_H
#define QGSMAPLAYER_H

#include <vector>

#include <qwidget.h>
#include <qobject.h>
#include <qpixmap.h>

#include <qgsrect.h>
#include <qgis.h>
#include <qgscoordinatetransform.h>
#include <qgisapp.h>
#include <qgssymbol.h>
#include <qgsfield.h>

class QgsFeature;
class QPopupMenu;
class QgsLegendItem;
class QDomNode;
class QDomDocument;

/** \class QgsMapLayer
 * \brief Base class for all map layer types.
 * This class is the base class for all map layer types (shapefile,
 * raster, database). 
 */
class QgsMapLayer : public QObject
{
  Q_OBJECT;

public:

  /*! Constructor
   * @param type Type of layer as defined in LAYERS enum
   * @param lyrname Display Name of the layer
   */
  QgsMapLayer(int type = 0, QString lyrname = QString::null, QString source = QString::null);

  //! Destructor
    virtual ~ QgsMapLayer();

  /*! Get the type of the layer
   * @return Integer matching a value in the LAYERS enum
   */
  const int type();

  /*! Get this layer's unique ID */
  QString const & getLayerID() const;

  /*! Set the display name of the layer
     # @param name New name for the layer
   */
  void setLayerName(const QString & name);

  /*! Get the display name of the layer
   * @return the layer name
   */
  QString const & name() const;

  /*! Get the internal name of the layer. This is the name used to created the
   * layer from the data source
   * @return internal datasource name of the layer
   */
  QString const & sourceName() const;

  /*! Virtual function to calculate the extent of the current layer.
   * This function must be overridden in all child classes and implemented
   * based on the layer type
   */
  virtual QgsRect calculateExtent();

  /*! Accesor for mShowInOverviewFlag */
  bool showInOverviewStatus() {return mShowInOverview;}

  
  virtual void draw(QPainter *, QgsRect *, int);
  virtual void draw(QPainter *, QgsRect *, QgsCoordinateTransform * ,QPaintDevice *);
  virtual void drawLabels(QPainter *, QgsRect *, QgsCoordinateTransform * ,QPaintDevice *);
  /*! Identify the feature(s) in this layer that are contained in the search rectangle
   */
  virtual void identify(QgsRect *)
  {
  };

  /*!Select features on the map canvas by dragging a rectangle */
  virtual void select(QgsRect *, bool )
  {
  };

  /*! Display the attribute table for the layer
   */
  virtual void table()
  {
  };

  /*! Return the extent of the layer as a QRect
   */
  const QgsRect extent();

  /*! Returns the status of the layer. An invalid layer is one which has a bad datasource
   * or other problem. Child classes set this flag when intialized
   *@return True if the layer is valid and can be accessed
   */
  bool isValid();

  /** Write property of QString labelField. */
  virtual void setLabelField(const QString & _newVal);

  /** Read property of QString labelField. */
  virtual const QString & labelField();

  //! Visibility of the layer
  bool visible();

  //! Returns the source for the layer
  QString const & source() const;

  /** Write property of int featureType. */
  virtual void setFeatureType(const int &_newVal);

  /** Read property of int featureType. */
  virtual const int &featureType();

  /** Return the context menu for the layer */
  virtual QPopupMenu *contextMenu() = 0;

  /** 
  * Get the first feature resulting from a select operation
  * @return QgsFeature
  */
  virtual QgsFeature *getFirstFeature(bool fetchAttributes = false) const;

  /** 
  * Get the next feature resulting from a select operation
  * @return QgsFeature
  */
  virtual QgsFeature *getNextFeature(bool fetchAttributes = false) const;

  /**
   * Get the next feature using new method
   * TODO - make this pure virtual once it works and change existing providers
   *        to use this method of fetching features
   */
  virtual bool getNextFeature(QgsFeature & feature, bool fetchAttributes = false) const;

  /**
   * Number of features in the layer
   * @return long containing number of features
   */
  virtual long featureCount() const;

  /**
    Number of attribute fields for a feature in the layer
    @note
    
    Redundant since fields().size() gives same information.

   */
  virtual int fieldCount() const;

  /**
    Return a list of field names for this layer
   @return vector of field names
  */
  virtual std::vector < QgsField > const &fields() const;


  //! Layers enum defining the types of layers that can be added to a map
  enum LAYERS
  {
    VECTOR,
    RASTER,
    DATABASE
  };

  /**Shows the properties dialog for the map layer*/
  virtual void showLayerProperties() = 0;

  /**Returns a pointer to the legend pixmap*/
  virtual QPixmap *legendPixmap();

  /** 
      All inherited layers must be able to display a conext menu if requested 

      @note Calls initContextMenu_()
  */
  void initContextMenu(QgisApp * app);

  /** 
      Allows children to tailor context menu

      @note Calls initContextMenu_()
  */
  virtual void initContextMenu_(QgisApp * app)
  {
      // NOP; children can optionally over-ride
  }

  /**Returns a pointer to the legend item*/
  QgsLegendItem *legendItem();

  /**Sets the pointer to the legend item*/
  void setLegendItem(QgsLegendItem * li);

  /**True if the layer can be edited*/
  virtual bool isEditable() const =0;

  /** sets state from DOM document

     @param layer_node is DOM node corresponding to ``maplayer'' tag

     @note

     The DOM node corresponds to a DOM document project file XML element read
     by QgsProject.

     This, in turn, calls readXML_(), which is over-rideable by sub-classes so
     that they can read their own specific state from the given DOM node.

     Invoked by QgsProject::read().

     @returns true if successful

   */
  bool readXML( QDomNode & layer_node );


  /** stores state in DOM node

     @param layer_node is DOM node corresponding to ``projectlayers'' tag

     @note

     The DOM node corresponds to a DOM document project file XML element to be
     written by QgsProject.

     This, in turn, calls writeXML_(), which is over-rideable by sub-classes so
     that they can write their own specific state to the given DOM node.

     Invoked by QgsProject::write().

     @returns true if successful

  */
  bool writeXML( QDomNode & layer_node, QDomDocument & document );

public  slots:

  //! set visibility
  void setVisible(bool vis);

  /*! Slot connected to popup menus of derived classes. Used to indicate whether this layer
   * should be shown or hidden in the map overview. */
  //virtual void toggleShowInOverview();

  /**Copies the legend pixmap of this layer to the legenditem and adds an overview glasses if necessary*/
  void updateItemPixmap();

  /**Ensures that the overview item in the popup menu is checked/ unchecked correctly*/
  void updateOverviewPopupItem();

  /** set whether this layer is in the overview or not 

  @note this will hopefully eventually supercede toggleOverviewStatus() since
  this makes explicit the state change

  */
  virtual void inOverview( bool );

    /** Accessor and mutator for the minimum scale member */
    void setMinScale(float theMinScale) {mMinScale=theMinScale;};
    float minScale() {return mMinScale;};
    
    /** Accessor and mutator for the maximum scale member */
    void setMaxScale(float theMaxScale) {mMaxScale=theMaxScale;};
    float maxScale() {return mMaxScale;};
    
    /** Accessor and mutator for the scale based visilibility flag */
    void setScaleBasedVisibility( bool theVisibilityFlag) {mScaleBasedVisibility=theVisibilityFlag; };
    bool scaleBasedVisibility() { return mScaleBasedVisibility; };

signals:

  void visibilityChanged(void);

  /** \brief emit a signal to notify of a progress event */
  void setProgress(int theProgress, int theTotalSteps);

  /** \brief emit a signal to be caught by gisapp and display a msg on status bar */
  void setStatus(QString theStatusQString);

  /** This signal should be connected with the slot QgsMapCanvas::refresh() */
  virtual void repaintRequested();

  /** This is used to notify the application whether this layer should be shown in overview or not. */
  //@{
  //void showInOverview(QString theLayerId, bool);
  void showInOverview(QgsMapLayer * maplayer, bool);
  //@}

protected:

  /** called by readXML(), used by children to read state specific to them from 
      project files.
  */
  virtual bool readXML_( QDomNode & layer_node );


  /** called by writeXML(), used by children to write state specific to them to 
      project files.
  */
  virtual bool writeXML_( QDomNode & layer_node, QDomDocument & document );


  //! Extent of the layer
  QgsRect layerExtent;

  //! Indicates if the layer is valid and can be drawn
  bool valid;

  //! data source description string, varies by layer type
  QString dataSource;

  //! Geometry type as defined in enum WKBTYPE (qgis.h)
  int geometryType;

  /**Pointer to the legend item for this layer*/
  QgsLegendItem *m_legendItem;

  /**Pixmap used in the legend item*/
  QPixmap m_legendPixmap;
  //
  //! A little pixmap to show if this layer is represented in overview or now 
  QPixmap mInOverviewPixmap;

  QPixmap mEditablePixmap;

  /** Name of the layer - used for display  */
  QString layerName;

  /** Internal name of the layer. Derived from the datasource */
  QString internalName;

  //! context menu
  QPopupMenu *popMenu;

  //! label for popMenu
  QLabel * myPopupLabel;

  //! checkable item id in popmenu that sets overview status
  int mShowInOverviewItemId;

  /** Whether this layer is to be shown in the overview map or not */
  bool mShowInOverview;

//   /** action item for pop-up menu

//       @note obviously should be in synch with mShowInOverview

//       Is set in context menu.

//       @todo this is a GUI element and should not be here
//   */
//   QAction* mActionInOverview;

private:                       // Private attributes

  /// QgsMapLayer not copyable
  QgsMapLayer( QgsMapLayer const & );

  /// QgsMapLayer not copyable
  QgsMapLayer & operator=( QgsMapLayer const & );

  /** Unique ID of this layer - used to refer to this layer  in QGIS code */
  QString ID;

  /** Type of the layer (eg. vector, raster, database  */
  int layerType;

  //! Tag for embedding additional information
  QString tag;

  /**  true if visible ? */
  bool m_visible;

  /** debugging member
      invoked when a connect() is made to this object 
  */
  void connectNotify( const char * signal );


  /** Minimum scale at which this layer should be displayed */
  float mMinScale;
  /** Maximum scale at which this layer should be displayed */
  float mMaxScale;
  /** A flag that tells us whether to use the above vars to restrict layer visibility */
  bool mScaleBasedVisibility;
public:                        // Public attributes

  /** map label ? */
  QString m_labelField;

};

#endif
