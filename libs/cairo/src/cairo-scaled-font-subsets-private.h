/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAIRO_SCALED_FONT_SUBSETS_PRIVATE_H
#define CAIRO_SCALED_FONT_SUBSETS_PRIVATE_H

#include "cairoint.h"

#if CAIRO_HAS_FONT_SUBSET

typedef struct _cairo_scaled_font_subsets_glyph {
  unsigned int font_id;
  unsigned int subset_id;
  unsigned int subset_glyph_index;
  cairo_bool_t is_scaled;
  cairo_bool_t is_composite;
  double     x_advance;
  double     y_advance;
  cairo_bool_t utf8_is_mapped;
  uint32_t      unicode;
} cairo_scaled_font_subsets_glyph_t;

/**
 * _cairo_scaled_font_subsets_create_scaled:
 *
 * Create a new #cairo_scaled_font_subsets_t object which can be used
 * to create subsets of any number of #cairo_scaled_font_t
 * objects. This allows the (arbitrarily large and sparse) glyph
 * indices of a #cairo_scaled_font_t to be mapped to one or more font
 * subsets with glyph indices packed into the range
 * [0 .. max_glyphs_per_subset).
 *
 * Return value: a pointer to the newly creates font subsets. The
 * caller owns this object and should call
 * _cairo_scaled_font_subsets_destroy() when done with it.
 **/
cairo_private cairo_scaled_font_subsets_t *
_cairo_scaled_font_subsets_create_scaled (void);

/**
 * _cairo_scaled_font_subsets_create_simple:
 *
 * Create a new #cairo_scaled_font_subsets_t object which can be used
 * to create font subsets suitable for embedding as Postscript or PDF
 * simple fonts.
 *
 * Glyphs with an outline path available will be mapped to one font
 * subset for each font face. Glyphs from bitmap fonts will mapped to
 * separate font subsets for each #cairo_scaled_font_t object.
 *
 * The maximum number of glyphs per subset is 256. Each subset
 * reserves the first glyph for the .notdef glyph.
 *
 * Return value: a pointer to the newly creates font subsets. The
 * caller owns this object and should call
 * _cairo_scaled_font_subsets_destroy() when done with it.
 **/
cairo_private cairo_scaled_font_subsets_t *
_cairo_scaled_font_subsets_create_simple (void);

/**
 * _cairo_scaled_font_subsets_create_composite:
 *
 * Create a new #cairo_scaled_font_subsets_t object which can be used
 * to create font subsets suitable for embedding as Postscript or PDF
 * composite fonts.
 *
 * Glyphs with an outline path available will be mapped to one font
 * subset for each font face. Each unscaled subset has a maximum of
 * 65536 glyphs except for Type1 fonts which have a maximum of 256 glyphs.
 *
 * Glyphs from bitmap fonts will mapped to separate font subsets for
 * each #cairo_scaled_font_t object. Each unscaled subset has a maximum
 * of 256 glyphs.
 *
 * Each subset reserves the first glyph for the .notdef glyph.
 *
 * Return value: a pointer to the newly creates font subsets. The
 * caller owns this object and should call
 * _cairo_scaled_font_subsets_destroy() when done with it.
 **/
cairo_private cairo_scaled_font_subsets_t *
_cairo_scaled_font_subsets_create_composite (void);

/**
 * _cairo_scaled_font_subsets_destroy:
 * @font_subsets: a #cairo_scaled_font_subsets_t object to be destroyed
 *
 * Destroys @font_subsets and all resources associated with it.
 **/
cairo_private void
_cairo_scaled_font_subsets_destroy (cairo_scaled_font_subsets_t *font_subsets);

/**
 * _cairo_scaled_font_subsets_map_glyph:
 * @font_subsets: a #cairo_scaled_font_subsets_t
 * @scaled_font: the font of the glyph to be mapped
 * @scaled_font_glyph_index: the index of the glyph to be mapped
 * @utf8: a string of text encoded in UTF-8
 * @utf8_len: length of @utf8 in bytes
 * @subset_glyph_ret: return structure containing subset font and glyph id
 *
 * Map a glyph from a #cairo_scaled_font to a new index within a
 * subset of that font. The mapping performed is from the tuple:
 *
 *    (scaled_font, scaled_font_glyph_index)
 *
 * to the tuple:
 *
 *    (font_id, subset_id, subset_glyph_index)
 *
 * This mapping is 1:1. If the input tuple has previously mapped, the
 * the output tuple previously returned will be returned again.
 *
 * Otherwise, the return tuple will be constructed as follows:
 *
 * 1) There is a 1:1 correspondence between the input scaled_font
 *  value and the output font_id value. If no mapping has been
 *  previously performed with the scaled_font value then the
 *  smallest unused font_id value will be returned.
 *
 * 2) Within the set of output tuples of the same font_id value the
 *  smallest value of subset_id will be returned such that
 *  subset_glyph_index does not exceed max_glyphs_per_subset (as
 *  passed to _cairo_scaled_font_subsets_create()) and that the
 *  resulting tuple is unique.
 *
 * 3) The smallest value of subset_glyph_index is returned such that
 *  the resulting tuple is unique.
 *
 * The net result is that any #cairo_scaled_font_t will be represented
 * by one or more font subsets. Each subset is effectively a tuple of
 * (scaled_font, font_id, subset_id) and within each subset there
 * exists a mapping of scaled_glyph_font_index to subset_glyph_index.
 *
 * This final description of a font subset is the same representation
 * used by #cairo_scaled_font_subset_t as provided by
 * _cairo_scaled_font_subsets_foreach.
 *
 * @utf8 and @utf8_len specify a string of unicode characters that the
 * glyph @scaled_font_glyph_index maps to. If @utf8_is_mapped in
 * @subset_glyph_ret is %TRUE, the font subsetting will (where index to
 * unicode mapping is supported) ensure that @scaled_font_glyph_index
 * maps to @utf8. If @utf8_is_mapped is %FALSE,
 * @scaled_font_glyph_index has already been mapped to a different
 * unicode string.
 *
 * The returned values in the #cairo_scaled_font_subsets_glyph_t struct are:
 *
 * @font_id: The font ID of the mapped glyph
 * @subset_id : The subset ID of the mapped glyph within the @font_id
 * @subset_glyph_index: The index of the mapped glyph within the @subset_id subset
 * @is_scaled: If true, the mapped glyph is from a bitmap font, and separate font
 * subset is created for each font scale used. If false, the outline of the mapped glyph
 * is available. One font subset for each font face is created.
 * @x_advance, @y_advance: When @is_scaled is true, @x_advance and @y_advance contain
 * the x and y advance for the mapped glyph in device space.
 * When @is_scaled is false, @x_advance and @y_advance contain the x and y advance for
 * the the mapped glyph from an unhinted 1 point font.
 * @utf8_is_mapped: If true the utf8 string provided to _cairo_scaled_font_subsets_map_glyph()
 * is (or already was) the utf8 string mapped to this glyph. If false the glyph is already
 * mapped to a different utf8 string.
 * @unicode: the unicode character mapped to this glyph by the font backend.
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful, or a non-zero
 * value indicating an error. Possible errors include
 * %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_status_t
_cairo_scaled_font_subsets_map_glyph (cairo_scaled_font_subsets_t    *font_subsets,
                    cairo_scaled_font_t        *scaled_font,
                    unsigned long             scaled_font_glyph_index,
                    const char *              utf8,
                    int                 utf8_len,
                    cairo_scaled_font_subsets_glyph_t *subset_glyph_ret);

typedef cairo_status_t
(*cairo_scaled_font_subset_callback_func_t) (cairo_scaled_font_subset_t    *font_subset,
                       void            *closure);

/**
 * _cairo_scaled_font_subsets_foreach:
 * @font_subsets: a #cairo_scaled_font_subsets_t
 * @font_subset_callback: a function to be called for each font subset
 * @closure: closure data for the callback function
 *
 * Iterate over each unique scaled font subset as created by calls to
 * _cairo_scaled_font_subsets_map_glyph(). A subset is determined by
 * unique pairs of (font_id, subset_id) as returned by
 * _cairo_scaled_font_subsets_map_glyph().
 *
 * For each subset, @font_subset_callback will be called and will be
 * provided with both a #cairo_scaled_font_subset_t object containing
 * all the glyphs in the subset as well as the value of @closure.
 *
 * The #cairo_scaled_font_subset_t object contains the scaled_font,
 * the font_id, and the subset_id corresponding to all glyphs
 * belonging to the subset. In addition, it contains an array providing
 * a mapping between subset glyph indices and the original scaled font
 * glyph indices.
 *
 * The index of the array corresponds to subset_glyph_index values
 * returned by _cairo_scaled_font_subsets_map_glyph() while the
 * values of the array correspond to the scaled_font_glyph_index
 * values passed as input to the same function.
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful, or a non-zero
 * value indicating an error. Possible errors include
 * %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_status_t
_cairo_scaled_font_subsets_foreach_scaled (cairo_scaled_font_subsets_t          *font_subsets,
                       cairo_scaled_font_subset_callback_func_t  font_subset_callback,
                       void                      *closure);

/**
 * _cairo_scaled_font_subsets_foreach_unscaled:
 * @font_subsets: a #cairo_scaled_font_subsets_t
 * @font_subset_callback: a function to be called for each font subset
 * @closure: closure data for the callback function
 *
 * Iterate over each unique unscaled font subset as created by calls to
 * _cairo_scaled_font_subsets_map_glyph(). A subset is determined by
 * unique pairs of (font_id, subset_id) as returned by
 * _cairo_scaled_font_subsets_map_glyph().
 *
 * For each subset, @font_subset_callback will be called and will be
 * provided with both a #cairo_scaled_font_subset_t object containing
 * all the glyphs in the subset as well as the value of @closure.
 *
 * The #cairo_scaled_font_subset_t object contains the scaled_font,
 * the font_id, and the subset_id corresponding to all glyphs
 * belonging to the subset. In addition, it contains an array providing
 * a mapping between subset glyph indices and the original scaled font
 * glyph indices.
 *
 * The index of the array corresponds to subset_glyph_index values
 * returned by _cairo_scaled_font_subsets_map_glyph() while the
 * values of the array correspond to the scaled_font_glyph_index
 * values passed as input to the same function.
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful, or a non-zero
 * value indicating an error. Possible errors include
 * %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_status_t
_cairo_scaled_font_subsets_foreach_unscaled (cairo_scaled_font_subsets_t        *font_subsets,
                       cairo_scaled_font_subset_callback_func_t  font_subset_callback,
                       void                    *closure);

/**
 * _cairo_scaled_font_subsets_foreach_user:
 * @font_subsets: a #cairo_scaled_font_subsets_t
 * @font_subset_callback: a function to be called for each font subset
 * @closure: closure data for the callback function
 *
 * Iterate over each unique scaled font subset as created by calls to
 * _cairo_scaled_font_subsets_map_glyph(). A subset is determined by
 * unique pairs of (font_id, subset_id) as returned by
 * _cairo_scaled_font_subsets_map_glyph().
 *
 * For each subset, @font_subset_callback will be called and will be
 * provided with both a #cairo_scaled_font_subset_t object containing
 * all the glyphs in the subset as well as the value of @closure.
 *
 * The #cairo_scaled_font_subset_t object contains the scaled_font,
 * the font_id, and the subset_id corresponding to all glyphs
 * belonging to the subset. In addition, it contains an array providing
 * a mapping between subset glyph indices and the original scaled font
 * glyph indices.
 *
 * The index of the array corresponds to subset_glyph_index values
 * returned by _cairo_scaled_font_subsets_map_glyph() while the
 * values of the array correspond to the scaled_font_glyph_index
 * values passed as input to the same function.
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful, or a non-zero
 * value indicating an error. Possible errors include
 * %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_status_t
_cairo_scaled_font_subsets_foreach_user (cairo_scaled_font_subsets_t          *font_subsets,
                     cairo_scaled_font_subset_callback_func_t  font_subset_callback,
                     void                      *closure);

/**
 * _cairo_scaled_font_subset_create_glyph_names:
 * @font_subsets: a #cairo_scaled_font_subsets_t
 *
 * Create an array of strings containing the glyph name for each glyph
 * in @font_subsets. The array as store in font_subsets->glyph_names.
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful,
 * %CAIRO_INT_STATUS_UNSUPPORTED if the font backend does not support
 * mapping the glyph indices to unicode characters. Possible errors
 * include %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_int_status_t
_cairo_scaled_font_subset_create_glyph_names (cairo_scaled_font_subset_t *subset);

typedef struct _cairo_cff_subset {
  char *font_name;
  char *ps_name;
  double *widths;
  double x_min, y_min, x_max, y_max;
  double ascent, descent;
  char *data;
  unsigned long data_length;
} cairo_cff_subset_t;

/**
 * _cairo_cff_subset_init:
 * @cff_subset: a #cairo_cff_subset_t to initialize
 * @font_subset: the #cairo_scaled_font_subset_t to initialize from
 *
 * If possible (depending on the format of the underlying
 * #cairo_scaled_font_t and the font backend in use) generate a
 * cff file corresponding to @font_subset and initialize
 * @cff_subset with information about the subset and the cff
 * data.
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful,
 * %CAIRO_INT_STATUS_UNSUPPORTED if the font can't be subset as a
 * cff file, or an non-zero value indicating an error.  Possible
 * errors include %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_status_t
_cairo_cff_subset_init (cairo_cff_subset_t      *cff_subset,
            const char          *name,
            cairo_scaled_font_subset_t  *font_subset);

/**
 * _cairo_cff_subset_fini:
 * @cff_subset: a #cairo_cff_subset_t
 *
 * Free all resources associated with @cff_subset.  After this
 * call, @cff_subset should not be used again without a
 * subsequent call to _cairo_cff_subset_init() again first.
 **/
cairo_private void
_cairo_cff_subset_fini (cairo_cff_subset_t *cff_subset);

/**
 * _cairo_cff_fallback_init:
 * @cff_subset: a #cairo_cff_subset_t to initialize
 * @font_subset: the #cairo_scaled_font_subset_t to initialize from
 *
 * If possible (depending on the format of the underlying
 * #cairo_scaled_font_t and the font backend in use) generate a cff
 * file corresponding to @font_subset and initialize @cff_subset
 * with information about the subset and the cff data.
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful,
 * %CAIRO_INT_STATUS_UNSUPPORTED if the font can't be subset as a
 * cff file, or an non-zero value indicating an error.  Possible
 * errors include %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_status_t
_cairo_cff_fallback_init (cairo_cff_subset_t      *cff_subset,
              const char          *name,
              cairo_scaled_font_subset_t  *font_subset);

/**
 * _cairo_cff_fallback_fini:
 * @cff_subset: a #cairo_cff_subset_t
 *
 * Free all resources associated with @cff_subset.  After this
 * call, @cff_subset should not be used again without a
 * subsequent call to _cairo_cff_subset_init() again first.
 **/
cairo_private void
_cairo_cff_fallback_fini (cairo_cff_subset_t *cff_subset);

typedef struct _cairo_truetype_subset {
  char *font_name;
  char *ps_name;
  double *widths;
  double x_min, y_min, x_max, y_max;
  double ascent, descent;
  unsigned char *data;
  unsigned long data_length;
  unsigned long *string_offsets;
  unsigned long num_string_offsets;
} cairo_truetype_subset_t;

/**
 * _cairo_truetype_subset_init:
 * @truetype_subset: a #cairo_truetype_subset_t to initialize
 * @font_subset: the #cairo_scaled_font_subset_t to initialize from
 *
 * If possible (depending on the format of the underlying
 * #cairo_scaled_font_t and the font backend in use) generate a
 * truetype file corresponding to @font_subset and initialize
 * @truetype_subset with information about the subset and the truetype
 * data.
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful,
 * %CAIRO_INT_STATUS_UNSUPPORTED if the font can't be subset as a
 * truetype file, or an non-zero value indicating an error.  Possible
 * errors include %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_status_t
_cairo_truetype_subset_init (cairo_truetype_subset_t  *truetype_subset,
               cairo_scaled_font_subset_t    *font_subset);

/**
 * _cairo_truetype_subset_fini:
 * @truetype_subset: a #cairo_truetype_subset_t
 *
 * Free all resources associated with @truetype_subset.  After this
 * call, @truetype_subset should not be used again without a
 * subsequent call to _cairo_truetype_subset_init() again first.
 **/
cairo_private void
_cairo_truetype_subset_fini (cairo_truetype_subset_t *truetype_subset);



typedef struct _cairo_type1_subset {
  char *base_font;
  double *widths;
  double x_min, y_min, x_max, y_max;
  double ascent, descent;
  char *data;
  unsigned long header_length;
  unsigned long data_length;
  unsigned long trailer_length;
} cairo_type1_subset_t;


#if CAIRO_HAS_FT_FONT

/**
 * _cairo_type1_subset_init:
 * @type1_subset: a #cairo_type1_subset_t to initialize
 * @font_subset: the #cairo_scaled_font_subset_t to initialize from
 * @hex_encode: if true the encrypted portion of the font is hex encoded
 *
 * If possible (depending on the format of the underlying
 * #cairo_scaled_font_t and the font backend in use) generate a type1
 * file corresponding to @font_subset and initialize @type1_subset
 * with information about the subset and the type1 data.
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful,
 * %CAIRO_INT_STATUS_UNSUPPORTED if the font can't be subset as a type1
 * file, or an non-zero value indicating an error.  Possible errors
 * include %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_status_t
_cairo_type1_subset_init (cairo_type1_subset_t        *type_subset,
              const char            *name,
              cairo_scaled_font_subset_t    *font_subset,
              cairo_bool_t           hex_encode);

/**
 * _cairo_type1_subset_fini:
 * @type1_subset: a #cairo_type1_subset_t
 *
 * Free all resources associated with @type1_subset.  After this call,
 * @type1_subset should not be used again without a subsequent call to
 * _cairo_truetype_type1_init() again first.
 **/
cairo_private void
_cairo_type1_subset_fini (cairo_type1_subset_t *subset);

#endif /* CAIRO_HAS_FT_FONT */


/**
 * _cairo_type1_scaled_font_is_type1:
 * @scaled_font: a #cairo_scaled_font_t
 *
 * Return %TRUE if @scaled_font is a Type 1 font, otherwise return %FALSE.
 **/
cairo_private cairo_bool_t
_cairo_type1_scaled_font_is_type1 (cairo_scaled_font_t    *scaled_font);

/**
 * _cairo_type1_fallback_init_binary:
 * @type1_subset: a #cairo_type1_subset_t to initialize
 * @font_subset: the #cairo_scaled_font_subset_t to initialize from
 *
 * If possible (depending on the format of the underlying
 * #cairo_scaled_font_t and the font backend in use) generate a type1
 * file corresponding to @font_subset and initialize @type1_subset
 * with information about the subset and the type1 data.  The encrypted
 * part of the font is binary encoded.
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful,
 * %CAIRO_INT_STATUS_UNSUPPORTED if the font can't be subset as a type1
 * file, or an non-zero value indicating an error.  Possible errors
 * include %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_status_t
_cairo_type1_fallback_init_binary (cairo_type1_subset_t        *type_subset,
                   const char            *name,
                   cairo_scaled_font_subset_t *font_subset);

/**
 * _cairo_type1_fallback_init_hexencode:
 * @type1_subset: a #cairo_type1_subset_t to initialize
 * @font_subset: the #cairo_scaled_font_subset_t to initialize from
 *
 * If possible (depending on the format of the underlying
 * #cairo_scaled_font_t and the font backend in use) generate a type1
 * file corresponding to @font_subset and initialize @type1_subset
 * with information about the subset and the type1 data. The encrypted
 * part of the font is hex encoded.
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful,
 * %CAIRO_INT_STATUS_UNSUPPORTED if the font can't be subset as a type1
 * file, or an non-zero value indicating an error.  Possible errors
 * include %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_status_t
_cairo_type1_fallback_init_hex (cairo_type1_subset_t       *type_subset,
                const char           *name,
                cairo_scaled_font_subset_t *font_subset);

/**
 * _cairo_type1_fallback_fini:
 * @type1_subset: a #cairo_type1_subset_t
 *
 * Free all resources associated with @type1_subset.  After this call,
 * @type1_subset should not be used again without a subsequent call to
 * _cairo_truetype_type1_init() again first.
 **/
cairo_private void
_cairo_type1_fallback_fini (cairo_type1_subset_t *subset);

typedef struct _cairo_type2_charstrings {
  int *widths;
  long x_min, y_min, x_max, y_max;
  long ascent, descent;
  cairo_array_t charstrings;
} cairo_type2_charstrings_t;

/**
 * _cairo_type2_charstrings_init:
 * @type2_subset: a #cairo_type2_subset_t to initialize
 * @font_subset: the #cairo_scaled_font_subset_t to initialize from
 *
 * If possible (depending on the format of the underlying
 * #cairo_scaled_font_t and the font backend in use) generate type2
 * charstrings to @font_subset and initialize @type2_subset
 * with information about the subset.
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful,
 * %CAIRO_INT_STATUS_UNSUPPORTED if the font can't be subset as a type2
 * charstrings, or an non-zero value indicating an error.  Possible errors
 * include %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_status_t
_cairo_type2_charstrings_init (cairo_type2_charstrings_t   *charstrings,
                 cairo_scaled_font_subset_t  *font_subset);

/**
 * _cairo_type2_charstrings_fini:
 * @subset: a #cairo_type2_charstrings_t
 *
 * Free all resources associated with @type2_charstring.  After this call,
 * @type2_charstring should not be used again without a subsequent call to
 * _cairo_type2_charstring_init() again first.
 **/
cairo_private void
_cairo_type2_charstrings_fini (cairo_type2_charstrings_t *charstrings);

/**
 * _cairo_truetype_create_glyph_to_unicode_map:
 * @font_subset: the #cairo_scaled_font_subset_t to initialize from
 *
 * If possible (depending on the format of the underlying
 * #cairo_scaled_font_t and the font backend in use) assign
 * the unicode character of each glyph in font_subset to
 * fontsubset->to_unicode.
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful,
 * %CAIRO_INT_STATUS_UNSUPPORTED if the unicode encoding of
 * the glyphs is not available.  Possible  errors include
 * %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_int_status_t
_cairo_truetype_create_glyph_to_unicode_map (cairo_scaled_font_subset_t    *font_subset);

/**
 * _cairo_truetype_index_to_ucs4:
 * @scaled_font: the #cairo_scaled_font_t
 * @index: the glyph index
 * @ucs4: return value for the unicode value of the glyph
 *
 * If possible (depending on the format of the underlying
 * #cairo_scaled_font_t and the font backend in use) assign
 * the unicode character of the glyph to @ucs4.
 *
 * If mapping glyph indices to unicode is supported but the unicode
 * value of the specified glyph is not available, @ucs4 is set to -1.
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful,
 * %CAIRO_INT_STATUS_UNSUPPORTED if mapping glyph indices to unicode
 * is not supported.  Possible errors include %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_int_status_t
_cairo_truetype_index_to_ucs4 (cairo_scaled_font_t *scaled_font,
                 unsigned long    index,
                 uint32_t      *ucs4);

/**
 * _cairo_truetype_read_font_name:
 * @scaled_font: the #cairo_scaled_font_t
 * @ps_name: returns the PostScript name of the font
 *       or %NULL if the name could not be found.
 * @font_name: returns the font name or %NULL if the name could not be found.
 *
 * If possible (depending on the format of the underlying
 * #cairo_scaled_font_t and the font backend in use) read the
 * PostScript and Font names from a TrueType/OpenType font.
 *
 * The font name is the full name of the font eg "DejaVu Sans Bold".
 * The PostScript name is a shortened name with spaces removed
 * suitable for use as the font name in a PS or PDF file eg
 * "DejaVuSans-Bold".
 *
 * Return value: %CAIRO_STATUS_SUCCESS if successful,
 * %CAIRO_INT_STATUS_UNSUPPORTED if the font is not TrueType/OpenType
 * or the name table is not present.  Possible errors include
 * %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_private cairo_int_status_t
_cairo_truetype_read_font_name (cairo_scaled_font_t   *scaled_font,
                char           **ps_name,
                char           **font_name);

#endif /* CAIRO_HAS_FONT_SUBSET */

#endif /* CAIRO_SCALED_FONT_SUBSETS_PRIVATE_H */
