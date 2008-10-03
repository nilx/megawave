################################################################################
#                        MegaWave2 internal types                              #
################################################################################
#
# Type	Sens	Fonctions	Include

Cimage	read	_mwload_cimage	cimage.h# Lecture d'image de type entier
Cimage	write	_mwsave_cimage	cimage.h# Ecriture d'image de type entier

Fimage	read	_mwload_fimage	fimage.h# Lecture d'image de type flottant
Fimage	write	_mwsave_fimage	fimage.h# Ecriture d'image de type flottant

Cmovie	read	_mwload_cmovie	cmovie.h# Lecture de film de type entier
Cmovie	write	_mwsave_cmovie	cmovie.h# Ecriture de film de type entier

Fmovie	read	_mwload_fmovie	fmovie.h# Lecture de film de type float
Fmovie	write	_mwsave_fmovie	fmovie.h# Ecriture de film de type float

Polygon read	_mwload_polygon polygon.h# Lecture fichiers ASCII (Polygone) 
Polygon write	_mwsave_polygon polygon.h# Ecriture fichiers ASCII (Polygone)

Polygons read	_mwload_polygons polygon.h# Lecture fichiers ASCII (Polygones) 
Polygons write	_mwsave_polygons polygon.h# Ecriture fichiers ASCII (Polygones)

Fpolygon read	_mwload_fpolygon fpolygon.h# Lecture fichiers ASCII (fpolygone) 
Fpolygon write	_mwsave_fpolygon fpolygon.h# Ecriture fichiers ASCII (fpolygone)

Fpolygons read	_mwload_fpolygons fpolygon.h# Lecture fichiers ASCII (fpolygones) 
Fpolygons write	_mwsave_fpolygons fpolygon.h# Ecriture fichiers ASCII (fpolygones)

Curve read	_mwload_curve curve.h# Lecture fichiers MW2_CURVE 
Curve write	_mwsave_curve curve.h# Ecriture fichiers MW2_CURVE 

Fcurve read	_mwload_fcurve fcurve.h# Lecture fichiers MW2_FCURVE 
Fcurve write	_mwsave_fcurve fcurve.h# Ecriture fichiers MW2_FCURVE 

Dcurve read	_mwload_dcurve dcurve.h# Lecture fichiers MW2_DCURVE 
Dcurve write	_mwsave_dcurve dcurve.h# Ecriture fichiers MW2_DCURVE 

Curves read	_mwload_curves curve.h# Lecture fichiers MW2_CURVES 
Curves write	_mwsave_curves curve.h# Ecriture fichiers MW2_CURVES 

Fcurves read	_mwload_fcurves fcurve.h# Lecture fichiers MW2_FCURVES 
Fcurves write	_mwsave_fcurves fcurve.h# Ecriture fichiers MW2_FCURVES 

Dcurves read	_mwload_dcurves dcurve.h# Lecture fichiers MW2_DCURVES 
Dcurves write	_mwsave_dcurves dcurve.h# Ecriture fichiers MW2_DCURVES 

Fsignal	read	_mwload_fsignal	fsignal.h# Lecture de signaux de type flottant
Fsignal	write	_mwsave_fsignal	fsignal.h# Ecriture de signaux de type flottant

Wtrans1d read	_mwload_wtrans1d wtrans1d.h# Lecture de trans. en ondelettes 1D
Wtrans1d write	_mwsave_wtrans1d wtrans1d.h# Ecriture de tran. en ondelettes 1D

Wtrans2d read	_mwload_wtrans2d wtrans2d.h# Lecture de trans. en ondelettes 2D
Wtrans2d write	_mwsave_wtrans2d wtrans2d.h# Ecriture de tran. en ondelettes 2D

Vchain_wmax read _mwload_vchain_wmax wmax2d.h  # Lecture fichiers ASCII (Virtual Chain of 2D Wavelet Maxima) 
Vchain_wmax write _mwsave_vchain_wmax wmax2d.h # Ecriture fichiers ASCII (Virtual Chain of 2D Wavelet Maxima) 

Vchains_wmax read _mwload_vchains_wmax wmax2d.h  # Lecture fichiers ASCII (Set of Virtual Chains of 2D Wavelet Maxima) 
Vchains_wmax write _mwsave_vchains_wmax wmax2d.h # Ecriture fichiers ASCII (Set of Virtual Chains of 2D Wavelet Maxima) 

Ccimage	read	_mwload_ccimage	ccimage.h# Lecture d'image couleur de type entier
Ccimage	write	_mwsave_ccimage	ccimage.h# Ecriture d'image couleur de type entier

Cfimage	read	_mwload_cfimage	cfimage.h# Lecture d'image couleur de type float
Cfimage	write	_mwsave_cfimage	cfimage.h# Ecriture d'image couleur de type float

Modules	read	_mwload_modules	module.h# Lecture d'une hierarchie de modules
Modules	write	_mwsave_modules	module.h# Ecriture d'un source XMegaWave2

Ccmovie	read	_mwload_ccmovie	ccmovie.h# Lecture de film couleur de type entier
Ccmovie	write	_mwsave_ccmovie	ccmovie.h# Ecriture de film couleur de type entier

Cfmovie	read	_mwload_cfmovie	cfmovie.h# Lecture de film couleur de type float
Cfmovie	write	_mwsave_cfmovie	cfmovie.h# Ecriture de film couleur de type float

Morpho_line read	_mwload_morpho_line mimage.h# Lecture fichiers MW2_MORPHO_LINE 
Morpho_line write	_mwsave_morpho_line mimage.h# Ecriture fichiers MW2_MORPHO_LINE 

Fmorpho_line read	_mwload_fmorpho_line mimage.h# Lecture fichiers MW2_FMORPHO_LINE 
Fmorpho_line write	_mwsave_fmorpho_line mimage.h# Ecriture fichiers MW2_FMORPHO_LINE 

Morpho_set read	_mwload_morpho_set mimage.h# Lecture fichiers MW2_MORPHO_SET
Morpho_set write	_mwsave_morpho_set mimage.h# Ecriture fichiers MW2_MORPHO_SET

Morpho_sets read	_mwload_morpho_sets mimage.h# Lecture fichiers MW2_MORPHO_SETS
Morpho_sets write	_mwsave_morpho_sets mimage.h# Ecriture fichiers MW2_MORPHO_SETS

Mimage read	_mwload_mimage mimage.h# Lecture fichiers MW2_MIMAGE 
Mimage write	_mwsave_mimage mimage.h# Ecriture fichiers MW2_MIMAGE 

Cmorpho_line read	_mwload_cmorpho_line cmimage.h# Lecture fichiers MW2_CMORPHO_LINE 
Cmorpho_line write	_mwsave_cmorpho_line cmimage.h# Ecriture fichiers MW2_CMORPHO_LINE 

Cfmorpho_line read	_mwload_cfmorpho_line cmimage.h# Lecture fichiers MW2_CFMORPHO_LINE 
Cfmorpho_line write	_mwsave_cfmorpho_line cmimage.h# Ecriture fichiers MW2_CFMORPHO_LINE 

Cmorpho_set read	_mwload_cmorpho_set cmimage.h# Lecture fichiers MW2_CMORPHO_SET
Cmorpho_set write	_mwsave_cmorpho_set cmimage.h# Ecriture fichiers MW2_CMORPHO_SET

Cmorpho_sets read	_mwload_cmorpho_sets cmimage.h# Lecture fichiers MW2_CMORPHO_SETS
Cmorpho_sets write	_mwsave_cmorpho_sets cmimage.h# Ecriture fichiers MW2_CMORPHO_SETS

Cmimage read	_mwload_cmimage cmimage.h# Lecture fichiers MW2_CMIMAGE 
Cmimage write	_mwsave_cmimage cmimage.h# Ecriture fichiers MW2_CMIMAGE 

Shape read	_mwload_shape shape.h# Lecture fichiers MW2_SHAPE
Shape write	_mwsave_shape shape.h# Ecriture fichiers MW2_SHAPE

Shapes read	_mwload_shapes shape.h# Lecture fichiers MW2_SHAPES
Shapes write	_mwsave_shapes shape.h# Ecriture fichiers MW2_SHAPES

Rawdata read	_mwload_rawdata rawdata.h# Lecture fichiers Rawdata
Rawdata write	_mwsave_rawdata rawdata.h# Ecriture fichiers Rawdata

Flist read	_mwload_flist list.h# Lecture fichiers Flist
Flist write	_mwsave_flist list.h# Ecriture fichiers Flist

Flists read	_mwload_flists list.h# Lecture fichiers Flists
Flists write	_mwsave_flists list.h# Ecriture fichiers Flists

Dlist read	_mwload_dlist list.h# Lecture fichiers Dlist
Dlist write	_mwsave_dlist list.h# Ecriture fichiers Dlist

Dlists read	_mwload_dlists list.h# Lecture fichiers Dlists
Dlists write	_mwsave_dlists list.h# Ecriture fichiers Dlists



