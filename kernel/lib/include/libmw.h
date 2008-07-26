/*
 * libmw.h
 *
 * common definitions for libmw
 */

#ifndef _LIBMW_H
#define _LIBMW_H

/* 
 * DEFINITIONS
 */

/* booleans */
/* TODO : remove?? */
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

/* M_PI is an Unix (XOPEN) specification */
/* TODO : remove?? */
#ifndef M_PI
#define M_PI 3.1415926535897931
#endif
#ifndef M_PI_2
#define M_PI_2 9.869604401089358
#endif
#ifndef M_PI_4
#define M_PI_4 97.409091034002429
#endif

/* error levels */
/* TODO : rename MW_XX */
/* TODO : use a better message lib */
#define WARNING  0
#define ERROR    1
#define FATAL    2
#define USAGE    3
#define INTERNAL 4

/*
 * SIZES
 */

/* max size of
 * - the megawave memory types (such as "Cimage")
 * - the megawave file types (such as "IMG")
 * - some string fields
 */
/* TODO : capital */
#define mw_mtype_size 20
#define mw_ftype_size 20
#define mw_cmtsize 255
#define mw_namesize 255

/*
 * STRUCTURES
 */

/* TODO : unsigned, typedef, simplify */
/** unsigned char (byte) gray level image */
typedef struct cimage
{
     int nrow;             /**< number of rows (dy)    */
     int ncol;             /**< number of columns (dx) */
     int allocsize;        /**< size allocated (in bytes) for the gray plane */
     unsigned char * gray; /**< the gray level plane (may be NULL)           */
     
     float scale;            /**< scale of the picture */
     char cmt[mw_cmtsize];   /**< comments */
     char name[mw_namesize]; /**< name of the image */
     
     /* defines the signifiant part of the picture */
     int firstcol;    /**< first col not affected by left side effect */
     int lastcol;     /**< last col not affected by right side effect */
     int firstrow;    /**< first row not aff. by upper side effect    */  
     int lastrow;     /**< last row not aff. by lower side effect     */  
     
     /* for use in movies */
     struct cimage * previous; /**< pointer to the prev image */
     struct cimage * next;     /**< pointer to the next image */
} * Cimage;

/** floating point gray level image */
typedef struct fimage
{
     int nrow;      /**< number of rows (dy)    */
     int ncol;      /**< number of columns (dx) */
     int allocsize; /**< size allocated (in bytes) for the gray plane */
     float * gray;  /**< the gray level plane (may be NULL)           */
     
     float scale;            /**< scale of the picture */
     char cmt[mw_cmtsize];   /**< comments */
     char name[mw_namesize]; /**< name of the image */
     
     /* defines the signifiant part of the picture */
     int firstcol;    /**< first col not affected by left side effect */
     int lastcol;     /**< last col not affected by right side effect */
     int firstrow;    /**< first row not aff. by upper side effect    */  
     int lastrow;     /**< last row not aff. by lower side effect     */  
     
     /* for use in movies */
     struct cimage * previous; /**< pointer to the prev image */
     struct cimage * next;     /**< pointer to the next image */
} * Fimage;


/* FIXME : abstract the color model */
/** unsigned char (byte) color image */
typedef struct ccimage
{
     int nrow;              /**< number of rows (dy)    */
     int ncol;              /**< number of columns (dx) */
     int allocsize;         /**< size allocated (in bytes) for the gray plane */
     unsigned char * red;   /**< the red   level plane (may be NULL)          */
     unsigned char * green; /**< the green level plane (may be NULL)          */
     unsigned char * blue;  /**< the blue  level plane (may be NULL)          */
     
     float scale;            /**< scale of the picture */
     char cmt[mw_cmtsize];   /**< comments             */
     char name[mw_namesize]; /**< name of the image    */
     
     /* defines the signifiant part of the picture */
     int firstcol;    /**< first col not affected by left side effect */
     int lastcol;     /**< last col not affected by right side effect */
     int firstrow;    /**< first row not aff. by upper side effect    */  
     int lastrow;     /**< last row not aff. by lower side effect     */  
     
     /* for use in movies */
     struct cimage * previous; /**< pointer to the prev image */
     struct cimage * next;     /**< pointer to the next image */
} * Ccimage;

/** floating point color image */
typedef struct cfimage {
     int nrow;      /**< number of rows (dy)    */
     int ncol;      /**< number of columns (dx) */
     int allocsize; /**< size allocated (in bytes) for the gray plane */
     float * red;   /**< the red   level plane (may be NULL)          */
     float * green; /**< the green level plane (may be NULL)          */
     float * blue;  /**< the blue  level plane (may be NULL)          */
     
     float scale;            /**< scale of the picture */
     char cmt[mw_cmtsize];   /**< comments */
     char name[mw_namesize]; /**< name of the image */
     
     /* defines the signifiant part of the picture */
     int firstcol;    /**< first col not affected by left side effect */
     int lastcol;     /**< last col not affected by right side effect */
     int firstrow;    /**< first row not aff. by upper side effect    */  
     int lastrow;     /**< last row not aff. by lower side effect     */  
     
     /* for use in movies */
     struct cimage * previous; /**< pointer to the prev image */
     struct cimage * next;     /**< pointer to the next image */
} * Cfimage;



/** point in a curve */
typedef struct point_curve
{
     int x, y; /**< coordinates of the point */
     
     /* for use in curve */
     struct point_curve * previous; /**< pointer to the prev point */
     struct point_curve * next;     /**< pointer to the next point */
} * Point_curve;

/** curve(s) */
typedef struct curve
{
     Point_curve first; /** pointer to the first point of the curve */
     
     /* For use in curves */
     struct curve * previous; /** pointer to the prev curve */
     struct curve * next;     /** pointer to the next curve */
} * Curve;
typedef struct curves
{
     char cmt[mw_cmtsize];   /** comments                   */
     char name[mw_namesize]; /** name of the set            */
     Curve first;            /** pointer to the first curve */
} * Curves;

/** float point in a curve */
typedef struct point_fcurve
{
     float x, y; /**< coordinates of the point */
     
     /* for use in curve */
     struct point_fcurve * previous; /**< pointer to the prev point */
     struct point_fcurve * next;     /**< pointer to the next point */
} * Point_fcurve;

/** float curve(s) */
typedef struct fcurve
{
     Point_fcurve first; /** pointer to the first point of the fcurve */
     
     /* For use in curves */
     struct fcurve * previous; /** pointer to the prev curve */
     struct fcurve * next;     /** pointer to the next curve */
} * Fcurve;
typedef struct fcurves
{
     char cmt[mw_cmtsize];   /** comments                   */
     char name[mw_namesize]; /** name of the set            */
     Fcurve first;            /** pointer to the first curve */
} * Fcurves;

/** double point in a curve */
typedef struct point_dcurve
{
     double x, y; /**< coordinates of the point */
     
     /* for use in curve */
     struct point_dcurve * previous; /**< pointer to the prev point */
     struct point_dcurve * next;     /**< pointer to the next point */
} * Point_dcurve;

/** double curve(s) */
typedef struct dcurve
{
     Point_dcurve first; /** pointer to the first point of the fcurve */
     
     /* For use in curves */
     struct dcurve * previous; /** pointer to the prev curve */
     struct dcurve * next;     /** pointer to the next curve */
} * Dcurve;
typedef struct dcurves
{
     char cmt[mw_cmtsize];   /** comments                   */
     char name[mw_namesize]; /** name of the set            */
     Dcurve first;            /** pointer to the first curve */
} * Dcurves;


/** polygon(s) */
typedef struct polygon
{
     /* the number of elements is given by nb_channels */
     int nb_channels;   /**< number of channels */
     float * channel;   /**< tab to the channel */
     Point_curve first; /**< pointer to the first point of the curve */
     
     /* for use in polygons only */
     struct polygon * previous; /**< pointer to the prev poly. (may be NULL) */
     struct polygon * next;     /**< pointer to the next poly. (may be NULL) */
} * Polygon;
typedef struct polygons
{
     char cmt[mw_cmtsize];   /**< comments                     */
     char name[mw_namesize]; /**< name of the set              */
     Polygon first;          /**< pointer to the first polygon */
} * Polygons;

/** float polygon(s) */
typedef struct fpolygon
{
     /* the number of elements is given by nb_channels */
     int nb_channels;    /**< number of channels */
     float * channel;    /**< tab to the channel */
     Point_fcurve first; /**< pointer to the first point of the curve */
     
     /* for use in polygons only */
     struct polygon * previous; /**< pointer to the prev poly. (may be NULL) */
     struct polygon * next;     /**< pointer to the next poly. (may be NULL) */
} * Fpolygon;
typedef struct fpolygons
{
     char cmt[mw_cmtsize];   /**< comments                     */
     char name[mw_namesize]; /**< name of the set              */
     Fpolygon first;         /**< pointer to the first polygon */
} * Fpolygons;



/** type of a point in a curve */
typedef struct point_type 
{
     /* TODO : enum? */
     unsigned char type; /**< Type of the point
			  *   0 : regular point
			  *   1 : point in the image's border
			  *   2 : T-junction
			  *   3 : Tau-junction
			  *   4 : X-junction
			  *   5 : Y-junction
			  */
     struct point_type * previous; /**< pointer to the prev point */
     struct point_type * next;     /**< pointer to the next point */
} * Point_type;

/** morpho_line on the discrete grid */
typedef struct morpho_line 
{
     Point_curve first_point; /**< pointer to the first point
			       *   of the morpho_line curve                 */
     Point_type first_type;   /**< pointer to the first Point_type          */
     float minvalue;          /**< min gray level value of this morpho line */
     float maxvalue;          /**< max gray level value of this morpho line */
     unsigned char open;      /**< 0 if the morpho line is closed
			       *   opened otherwise                 */
     float data;              /**< user-defined data field (saved)  */
     void *pdata;             /**< user-defined data field 
			       *   pointer to something (not saved) */
     
     /* for use in mimage */
     struct morpho_sets * morphosets; /**< pointer to the morpho sets */
     unsigned int num;                /**< morpho line number         */
     struct morpho_line * previous;   /**< pointer to the prev m.l.   */
     struct morpho_line * next;       /**< pointer to the next m.l.   */
} * Morpho_line;

/* TODO : morphosets? num? */
/** morpho_line on a pseudo-continuous plane */
typedef struct fmorpho_line 
{
     Point_fcurve first_point; /**< pointer to the first point
			        *   of the morpho_line curve                 */
     Point_type first_type;    /**< pointer to the first Point_type          */
     float minvalue;           /**< min gray level value of this morpho line */
     float maxvalue;           /**< max gray level value of this morpho line */
     unsigned char open;       /**< 0 if the morpho line is closed
			        *   opened otherwise                 */
     float data;               /**< user-defined data field (saved)  */
     void *pdata;              /**< user-defined data field 
			        *   pointer to something (not saved) */
     
     /* for use in mimage */
     struct morpho_line * previous;   /**< pointer to the prev m.l.   */
     struct morpho_line * next;       /**< pointer to the next m.l.   */
} * Fmorpho_line;

/** horizontal segment on the discrete grid */
typedef struct hsegment
{
     int xstart; /**< left  x-coordinate of the segment */
     int xend;   /**< right x-coordinate of the segment */
     int y;      /**< y-coordinate of the segment       */
     struct hsegment * previous; /**< pointer to the prev segment */
     struct hsegment * next;     /**< pointer to the next segment */
} * Hsegment;

/** morpho_set(s) on the discrete grid */
typedef struct morpho_set
{
     unsigned int num;       /**< morpho set number                */
     Hsegment first_segment; /**< pointer to the first segment     */
     Hsegment last_segment;  /**< pointer to the last segment      */  
     float minvalue;         /**< min gray level value of this set */
     float maxvalue;         /**< max gray level value of this set */
     unsigned char stated;   /**< 1 if this m.s. has already been stated,
			      *   0 otherwise                              */
     int area;               /**< area of the set
			      *   (number of pixels belonging to this set) */
     struct morpho_sets * neighbor; /**< pointer to a chain
				     *   of neighbor morpho sets */
} * Morpho_set;
typedef struct morpho_sets 
{
     Morpho_set morphoset;          /**< pointer to the current morpho set */
     struct morpho_sets * previous; /**< pointer to the prev morpho sets   */
     struct morpho_sets * next;     /**< pointer to the next morpho sets   */
     /* for use in mimage */
     struct morpho_line * morpholine;  /**< pointer to the morpho line */
} * Morpho_sets;

/** morphological image */
typedef struct mimage {
     char cmt[mw_cmtsize];   /**< comments                          */
     char name[mw_namesize]; /**< name of the set                   */
     int nrow;               /**< number of rows (dy)               */
     int ncol;               /**< number of columns (dx)            */
     float minvalue;         /**< min gray level value in the image */
     float maxvalue;         /**< max gray level value in the image */
     Morpho_line first_ml;   /**< pointer to the first morpho line  */
     Fmorpho_line first_fml; /**< Pointer to the first morpho line  */
     Morpho_sets first_ms;   /**< Pointer to the first morpho sets  */
} * Mimage;

/** color */
/* TODO : abstract the model */
/* TODO : n-channels         */
typedef struct color {
     unsigned char model; /**< model of the colorimetric system   */
     float red;           /**< the red   value if model=MODEL_RGB */
     float green;         /**< the green value if model=MODEL_RGB */
     float blue;          /**< the blue  value if model=MODEL_RGB */  
} Color;

/** cmorpho_line on the discrete grid */
typedef struct cmorpho_line 
{
     Point_curve first_point; /**< pointer to the first point
			       *   of the cmorpho_line curve        */
     Point_type first_type;   /**< pointer to the first Point_type  */
     Color minvalue;          /**< min color of this cmorpho line   */
     Color maxvalue;          /**< max color of this cmorpho line   */
     unsigned char open;      /**< 0 if the cmorpho line is closed
			       *   opened otherwise                 */
     float data;              /**< user-defined data field (saved)  */
     void *pdata;             /**< user-defined data field 
			       *   pointer to something (not saved) */
     
     /* for use in mimage */
     struct cmorpho_sets * cmorphosets; /**< pointer to the cmorpho sets */
     unsigned int num;                  /**< cmorpho line number         */
     struct cmorpho_line * previous;    /**< pointer to the prev cm.l.   */
     struct cmorpho_line * next;        /**< pointer to the next cm.l.   */
} * Cmorpho_line;

/** cmorpho_line on a pseudo-continuous plane */
typedef struct cfmorpho_line 
{
     Point_fcurve first_point; /**< pointer to the first point
			        *   of the cmorpho_line curve        */
     Point_type first_type;    /**< pointer to the first Point_type  */
     Color minvalue;           /**< min color of this cmorpho line   */
     Color maxvalue;           /**< max color of this cmorpho line   */
     unsigned char open;       /**< 0 if the cmorpho line is closed
			        *   opened otherwise                 */
     float data;               /**< user-defined data field (saved)  */
     void * pdata;             /**< user-defined data field 
			        *   pointer to something (not saved) */
     
     /* for use in mimage */
     struct cmorpho_line * previous;  /**< pointer to the prev cm.l.  */
     struct cmorpho_line * next;      /**< pointer to the next cm.l.  */
} * Cfmorpho_line;

/** cmorpho_set(s) on the discrete grid */
typedef struct cmorpho_set
{
     unsigned int num;       /**< cmorpho set number               */
     Hsegment first_segment; /**< pointer to the first segment     */
     Hsegment last_segment;  /**< pointer to the last segment      */  
     Color minvalue;         /**< min color of this set            */
     Color maxvalue;         /**< max color of this set            */
     unsigned char stated;   /**< 1 if this cm.s. has already been stated,
			      *   0 otherwise                               */
     int area;               /**< area of the set
			      *   (number of pixels belonging to this set)  */
     struct morpho_sets * cneighbor; /**< pointer to a chain
				     *    of neighbor cmorpho sets */
} * Cmorpho_set;
typedef struct cmorpho_sets 
{
     Morpho_set cmorphoset;          /**< pointer to the current morpho set */
     struct cmorpho_sets * previous; /**< pointer to the prev morpho sets   */
     struct cmorpho_sets * next;     /**< pointer to the next morpho sets   */
     /* for use in mimage */
     struct cmorpho_line * cmorpholine;  /**< pointer to the morpho line */
} * Cmorpho_sets;

/** morphological cimage */
typedef struct cmimage {
     char cmt[mw_cmtsize];    /**< comments                          */
     char name[mw_namesize];  /**< name of the set                   */
     int nrow;                /**< number of rows (dy)               */
     int ncol;                /**< number of columns (dx)            */
     Color minvalue;          /**< min color in the image            */
     Color maxvalue;          /**< max color in the image            */
     Cmorpho_line first_ml;   /**< pointer to the first cmorpho line */
     Cfmorpho_line first_fml; /**< Pointer to the first cmorpho line */
     Cmorpho_sets first_ms;   /**< Pointer to the first cmorpho sets */
} * Cmimage;




/** float list(s) */
typedef struct flist
{
     int size;       /**< size (number of elements)                     */
     int max_size;   /**< currently allocated size (number of ELEMENTS) */
     int dim;        /**< dimension (number of components per element)  */
     
     float * values; /**< values = size * dim array
		      *   nth element = values[n*dim+i], i=0..dim-1 */

     int data_size;  /**< size of data[] in bytes    */
     void * data;    /**< user defined field (saved) */
} * Flist;
typedef struct flists
{
     char cmt[mw_cmtsize];   /**< comments */
     char name[mw_namesize]; /**< name     */
     
     int size;               /**< size (number of elements) */
     int max_size;           /**< currently alloc. size (number of ELEMENTS) */
     
     Flist * list;           /**< array of Flist             */
     
     int data_size;          /**< size of data[] in bytes    */
     void* data;             /**< user defined field (saved) */
} * Flists;
/** double list(s) */
typedef struct dlist
{
     int size;       /**< size (number of elements)                     */
     int max_size;   /**< currently allocated size (number of ELEMENTS) */
     int dim;        /**< dimension (number of components per element)  */
     
     double * values; /**< values = size * dim array
		      *   nth element = values[n*dim+i], i=0..dim-1 */

     int data_size;  /**< size of data[] in bytes    */
     void * data;    /**< user defined field (saved) */
} * Dlist;
typedef struct dlists
{
     char cmt[mw_cmtsize];   /**< comments */
     char name[mw_namesize]; /**< name     */
     
     int size;               /**< size (number of elements) */
     int max_size;           /**< currently alloc. size (number of ELEMENTS) */
     
     Dlist * list;           /**< array of Dlist             */
     
     int data_size;          /**< size of data[] in bytes    */
     void* data;             /**< user defined field (saved) */
} * Dlists;

#endif /* !_LIBMW_H */


