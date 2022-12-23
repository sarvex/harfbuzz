/*
 * Copyright © 2022  Behdad Esfahbod
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef HB_FT_COLR_HH
#define HB_FT_COLR_HH

#include "hb.hh"


#ifndef HB_NO_PAINT

static hb_paint_composite_mode_t
_hb_ft_paint_composite_mode (FT_Composite_Mode mode)
{
  switch (mode)
  {
    case FT_COLR_COMPOSITE_CLEAR:          return HB_PAINT_COMPOSITE_MODE_CLEAR;
    case FT_COLR_COMPOSITE_SRC:            return HB_PAINT_COMPOSITE_MODE_SRC;
    case FT_COLR_COMPOSITE_DEST:           return HB_PAINT_COMPOSITE_MODE_DEST;
    case FT_COLR_COMPOSITE_SRC_OVER:       return HB_PAINT_COMPOSITE_MODE_SRC_OVER;
    case FT_COLR_COMPOSITE_DEST_OVER:      return HB_PAINT_COMPOSITE_MODE_DEST_OVER;
    case FT_COLR_COMPOSITE_SRC_IN:         return HB_PAINT_COMPOSITE_MODE_SRC_IN;
    case FT_COLR_COMPOSITE_DEST_IN:        return HB_PAINT_COMPOSITE_MODE_DEST_IN;
    case FT_COLR_COMPOSITE_SRC_OUT:        return HB_PAINT_COMPOSITE_MODE_SRC_OUT;
    case FT_COLR_COMPOSITE_DEST_OUT:       return HB_PAINT_COMPOSITE_MODE_DEST_OUT;
    case FT_COLR_COMPOSITE_SRC_ATOP:       return HB_PAINT_COMPOSITE_MODE_SRC_ATOP;
    case FT_COLR_COMPOSITE_DEST_ATOP:      return HB_PAINT_COMPOSITE_MODE_DEST_ATOP;
    case FT_COLR_COMPOSITE_XOR:            return HB_PAINT_COMPOSITE_MODE_XOR;
    case FT_COLR_COMPOSITE_PLUS:           return HB_PAINT_COMPOSITE_MODE_PLUS;
    case FT_COLR_COMPOSITE_SCREEN:         return HB_PAINT_COMPOSITE_MODE_SCREEN;
    case FT_COLR_COMPOSITE_OVERLAY:        return HB_PAINT_COMPOSITE_MODE_OVERLAY;
    case FT_COLR_COMPOSITE_DARKEN:         return HB_PAINT_COMPOSITE_MODE_DARKEN;
    case FT_COLR_COMPOSITE_LIGHTEN:        return HB_PAINT_COMPOSITE_MODE_LIGHTEN;
    case FT_COLR_COMPOSITE_COLOR_DODGE:    return HB_PAINT_COMPOSITE_MODE_COLOR_DODGE;
    case FT_COLR_COMPOSITE_COLOR_BURN:     return HB_PAINT_COMPOSITE_MODE_COLOR_BURN;
    case FT_COLR_COMPOSITE_HARD_LIGHT:     return HB_PAINT_COMPOSITE_MODE_HARD_LIGHT;
    case FT_COLR_COMPOSITE_SOFT_LIGHT:     return HB_PAINT_COMPOSITE_MODE_SOFT_LIGHT;
    case FT_COLR_COMPOSITE_DIFFERENCE:     return HB_PAINT_COMPOSITE_MODE_DIFFERENCE;
    case FT_COLR_COMPOSITE_EXCLUSION:      return HB_PAINT_COMPOSITE_MODE_EXCLUSION;
    case FT_COLR_COMPOSITE_MULTIPLY:       return HB_PAINT_COMPOSITE_MODE_MULTIPLY;
    case FT_COLR_COMPOSITE_HSL_HUE:        return HB_PAINT_COMPOSITE_MODE_HSL_HUE;
    case FT_COLR_COMPOSITE_HSL_SATURATION: return HB_PAINT_COMPOSITE_MODE_HSL_SATURATION;
    case FT_COLR_COMPOSITE_HSL_COLOR:      return HB_PAINT_COMPOSITE_MODE_HSL_COLOR;
    case FT_COLR_COMPOSITE_HSL_LUMINOSITY: return HB_PAINT_COMPOSITE_MODE_HSL_LUMINOSITY;

    case FT_COLR_COMPOSITE_MAX:            HB_FALLTHROUGH;
    default:                               return HB_PAINT_COMPOSITE_MODE_CLEAR;
  }
}

typedef struct
{
  FT_Face face;
  FT_Color *palette;
  hb_color_t foreground;
} _hb_ft_get_color_stops_data_t;

static unsigned
_hb_ft_color_line_get_color_stops (hb_color_line_t *color_line,
				   void *color_line_data,
				   unsigned int start,
				   unsigned int *count,
				   hb_color_stop_t *color_stops,
				   void *user_data)
{
  FT_ColorLine *c = (FT_ColorLine *) color_line_data;
  _hb_ft_get_color_stops_data_t *data = (_hb_ft_get_color_stops_data_t *) user_data;

  if (count)
  {
    FT_ColorStop stop;
    unsigned wrote = 0;

    c->color_stop_iterator.current_color_stop = start;

    while (count && *count &&
	   FT_Get_Colorline_Stops(data->face,
				  &stop,
				  &c->color_stop_iterator))
    {
      color_stops->offset = stop.stop_offset / 16384.f;
      color_stops->is_foreground = stop.color.palette_index == 0xFFFF;
      if (color_stops->is_foreground)
	color_stops->color = HB_COLOR (hb_color_get_blue (data->foreground),
				       hb_color_get_green (data->foreground),
				       hb_color_get_red (data->foreground),
				       (hb_color_get_alpha (data->foreground) * stop.color.alpha) >> 14);
      else
      {
	FT_Color ft_color = data->palette[stop.color.palette_index];
	color_stops->color = HB_COLOR (ft_color.blue,
				       ft_color.green,
				       ft_color.red,
				       (ft_color.alpha * stop.color.alpha) >> 14);
      }

      color_stops++;
      wrote++;
    }
    *count = wrote;
  }

  return c->color_stop_iterator.num_color_stops;
}

static hb_paint_extend_t
_hb_ft_color_line_get_extend (hb_color_line_t *color_line,
			      void *color_line_data,
			      void *user_data)
{
  FT_ColorLine *c = (FT_ColorLine *) color_line_data;
  switch (c->extend)
  {
    default:
    case FT_COLR_PAINT_EXTEND_PAD:     return HB_PAINT_EXTEND_PAD;
    case FT_COLR_PAINT_EXTEND_REPEAT:  return HB_PAINT_EXTEND_REPEAT;
    case FT_COLR_PAINT_EXTEND_REFLECT: return HB_PAINT_EXTEND_REFLECT;
  }
}

static void
_hb_ft_paint (FT_OpaquePaint opaque_paint,
	      const hb_ft_font_t *ft_font,
	      hb_font_t *font,
	      hb_paint_funcs_t *paint_funcs, void *paint_data,
	      FT_Color *palette,
	      hb_color_t foreground)
{
  FT_Face ft_face = ft_font->ft_face;
  FT_COLR_Paint paint;
  if (!FT_Get_Paint (ft_face, opaque_paint, &paint))
    return;

#define paint_recurse(other_paint) \
	_hb_ft_paint (other_paint, ft_font, font, paint_funcs, paint_data, palette, foreground)

  switch (paint.format)
  {
    case FT_COLR_PAINTFORMAT_COLR_LAYERS:
    {
      FT_OpaquePaint other_paint = {0};
      while (FT_Get_Paint_Layers (ft_face,
				  &paint.u.colr_layers.layer_iterator,
				  &other_paint))
      {
	paint_funcs->push_group (paint_data);
	paint_recurse (other_paint);
	paint_funcs->pop_group (paint_data, HB_PAINT_COMPOSITE_MODE_SRC_OVER);
      }
    }
    break;
    case FT_COLR_PAINTFORMAT_SOLID:
    {
      bool is_foreground = paint.u.solid.color.palette_index ==  0xFFFF;
      hb_color_t color;
      if (is_foreground)
	color = HB_COLOR (hb_color_get_blue (foreground),
			  hb_color_get_green (foreground),
			  hb_color_get_red (foreground),
			  (hb_color_get_alpha (foreground) * paint.u.solid.color.alpha) >> 14);
      else
      {
	FT_Color ft_color = palette[paint.u.solid.color.palette_index];
	color = HB_COLOR (ft_color.blue,
			  ft_color.green,
			  ft_color.red,
			  (ft_color.alpha * paint.u.solid.color.alpha) >> 14);
      }
      paint_funcs->color (paint_data, is_foreground, color);
    }
    break;
    case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT:
    {
      _hb_ft_get_color_stops_data_t data = {ft_face, palette, foreground};
      hb_color_line_t cl = {
	&paint.u.linear_gradient.colorline,
	_hb_ft_color_line_get_color_stops, &data,
	_hb_ft_color_line_get_extend, nullptr
      };

      paint_funcs->linear_gradient (paint_data, &cl,
				    paint.u.linear_gradient.p0.x / 65535.f,
				    paint.u.linear_gradient.p0.y / 65535.f,
				    paint.u.linear_gradient.p1.x / 65535.f,
				    paint.u.linear_gradient.p1.y / 65535.f,
				    paint.u.linear_gradient.p2.x / 65535.f,
				    paint.u.linear_gradient.p2.y / 65535.f);
    }
    break;
    case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT:
    {
      _hb_ft_get_color_stops_data_t data = {ft_face, palette, foreground};
      hb_color_line_t cl = {
	&paint.u.linear_gradient.colorline,
	_hb_ft_color_line_get_color_stops, &data,
	_hb_ft_color_line_get_extend, nullptr
      };

      paint_funcs->radial_gradient (paint_data, &cl,
				    paint.u.radial_gradient.c0.x / 65535.f,
				    paint.u.radial_gradient.c0.y / 65535.f,
				    (paint.u.radial_gradient.r0 / 65535.f),
				    paint.u.radial_gradient.c1.x / 65535.f,
				    paint.u.radial_gradient.c1.y / 65535.f,
				    (paint.u.radial_gradient.r1 / 65535.f));
    }
    break;
    case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT:
    {
      _hb_ft_get_color_stops_data_t data = {ft_face, palette, foreground};
      hb_color_line_t cl = {
	&paint.u.linear_gradient.colorline,
	_hb_ft_color_line_get_color_stops, &data,
	_hb_ft_color_line_get_extend, nullptr
      };

      paint_funcs->sweep_gradient (paint_data, &cl,
				   paint.u.sweep_gradient.center.x / 65535.f,
				   paint.u.sweep_gradient.center.y / 65535.f,
				   (paint.u.sweep_gradient.start_angle / 65536.f + 1) * (float) M_PI,
				   (paint.u.sweep_gradient.end_angle / 65536.f + 1) * (float) M_PI);
    }
    break;
    case FT_COLR_PAINTFORMAT_GLYPH:
    {
      //paint_funcs->push_inverse_root_transform (paint_data, font);
      ft_font->lock.unlock ();
      paint_funcs->push_clip_glyph (paint_data, paint.u.glyph.glyphID, font);
      ft_font->lock.lock ();
      paint_recurse (paint.u.glyph.paint);
      paint_funcs->pop_clip (paint_data);
      //paint_funcs->pop_inverse_root_transform (paint_data);
    }
    break;
    case FT_COLR_PAINTFORMAT_COLR_GLYPH:
    {
      /* TODO Depth counter. */
      FT_OpaquePaint other_paint = {0};
      if (FT_Get_Color_Glyph_Paint (ft_face, paint.u.colr_glyph.glyphID,
				    FT_COLOR_NO_ROOT_TRANSFORM,
				    &other_paint))
	paint_recurse (other_paint);
    }
    break;
    case FT_COLR_PAINTFORMAT_TRANSFORM:
    {
      paint_funcs->push_transform (paint_data,
				   paint.u.transform.affine.xx / 65536.f,
				   paint.u.transform.affine.yx / 65536.f,
				   paint.u.transform.affine.xy / 65536.f,
				   paint.u.transform.affine.yy / 65536.f,
				   paint.u.transform.affine.dx / 65536.f,
				   paint.u.transform.affine.dy / 65536.f);
      paint_recurse (paint.u.transform.paint);
      paint_funcs->pop_transform (paint_data);
    }
    break;
    case FT_COLR_PAINTFORMAT_TRANSLATE:
    {
      paint_funcs->push_transform (paint_data,
				   0.f, 0.f, 0.f, 0.f,
				   paint.u.translate.dx / 65536.f,
				   paint.u.translate.dy / 65536.f);
      paint_recurse (paint.u.translate.paint);
      paint_funcs->pop_transform (paint_data);
    }
    break;
    case FT_COLR_PAINTFORMAT_SCALE:
    {
      paint_funcs->push_transform (paint_data,
				   1.f, 0.f, 0.f, 1.f,
				   +paint.u.scale.center_x / 65536.f,
				   +paint.u.scale.center_y / 65536.f);
      paint_funcs->push_transform (paint_data,
				   paint.u.scale.scale_x / 65536.f,
				   0.f, 0.f,
				   paint.u.scale.scale_y / 65536.f,
				   0.f, 0.f);
      paint_funcs->push_transform (paint_data,
				   1.f, 0.f, 0.f, 1.f,
				   -paint.u.scale.center_x / 65536.f,
				   -paint.u.scale.center_y / 65536.f);
      paint_recurse (paint.u.scale.paint);
      paint_funcs->pop_transform (paint_data);
      paint_funcs->pop_transform (paint_data);
      paint_funcs->pop_transform (paint_data);
    }
    break;
    case FT_COLR_PAINTFORMAT_ROTATE:
    {
      float a = paint.u.rotate.angle / 65536.f;
      float cc = cosf (a * (float) M_PI);
      float ss = sinf (a * (float) M_PI);
      paint_funcs->push_transform (paint_data,
				   1.f, 0.f, 0.f, 1.f,
				   +paint.u.rotate.center_x / 65536.f,
				   +paint.u.rotate.center_y / 65536.f);
      paint_funcs->push_transform (paint_data, cc, ss, -ss, cc, 0., 0.);
      paint_funcs->push_transform (paint_data,
				   1.f, 0.f, 0.f, 1.f,
				   -paint.u.rotate.center_x / 65536.f,
				   -paint.u.rotate.center_y / 65536.f);
      paint_recurse (paint.u.rotate.paint);
      paint_funcs->pop_transform (paint_data);
      paint_funcs->pop_transform (paint_data);
      paint_funcs->pop_transform (paint_data);
    }
    break;
    case FT_COLR_PAINTFORMAT_SKEW:
    {
      float x = +tanf (paint.u.skew.x_skew_angle / 65536.f * (float) M_PI);
      float y = -tanf (paint.u.skew.y_skew_angle / 65536.f * (float) M_PI);
      paint_funcs->push_transform (paint_data,
				   1.f, 0.f, 0.f, 1.f,
				   +paint.u.skew.center_x / 65536.f,
				   +paint.u.skew.center_y / 65536.f);
      paint_funcs->push_transform (paint_data, 1., y, x, 1., 0., 0.);
      paint_funcs->push_transform (paint_data,
				   1.f, 0.f, 0.f, 1.f,
				   -paint.u.skew.center_x / 65536.f,
				   -paint.u.skew.center_y / 65536.f);
      paint_recurse (paint.u.skew.paint);
      paint_funcs->pop_transform (paint_data);
      paint_funcs->pop_transform (paint_data);
      paint_funcs->pop_transform (paint_data);
    }
    break;
    case FT_COLR_PAINTFORMAT_COMPOSITE:
    {
      paint_funcs->push_group (paint_data);
      paint_recurse (paint.u.composite.backdrop_paint);
      paint_funcs->push_group (paint_data);
      paint_recurse (paint.u.composite.source_paint);
      paint_funcs->pop_group (paint_data, _hb_ft_paint_composite_mode (paint.u.composite.composite_mode));
      paint_funcs->pop_group (paint_data, HB_PAINT_COMPOSITE_MODE_SRC_OVER);
    }
    break;

    case FT_COLR_PAINT_FORMAT_MAX: break;
    default: HB_FALLTHROUGH;
    case FT_COLR_PAINTFORMAT_UNSUPPORTED: break;
  }
#undef paint_recurse
}

static bool
hb_ft_paint_glyph_colr (hb_font_t *font,
			void *font_data,
			hb_codepoint_t gid,
			hb_paint_funcs_t *paint_funcs, void *paint_data,
			unsigned int palette_index,
			hb_color_t foreground,
			void *user_data)
{
  const hb_ft_font_t *ft_font = (const hb_ft_font_t *) font_data;
  FT_Face ft_face = ft_font->ft_face;

  /* Face is locked. */

  FT_Error error;
  FT_Color*         palette;
  FT_LayerIterator  iterator;

  FT_Bool  have_layers;
  FT_UInt  layer_glyph_index;
  FT_UInt  layer_color_index;

  error = FT_Palette_Select(ft_face, palette_index, &palette);
  if (error)
    palette = NULL;

  /* COLRv1 */
  FT_OpaquePaint paint = {0};
  if (FT_Get_Color_Glyph_Paint (ft_face, gid,
			        FT_COLOR_NO_ROOT_TRANSFORM,
			        &paint))
  {
    FT_ClipBox clip_box;
    bool pop_clip = false;
    if (FT_Get_Color_Glyph_ClipBox (ft_face, gid,
				    &clip_box))
    {
      /* TODO mult's like hb-ft. */
      paint_funcs->push_clip_rectangle (paint_data,
					clip_box.bottom_left.x,
					clip_box.bottom_left.y,
					clip_box.top_right.x,
					clip_box.top_right.y);
    }

    _hb_ft_paint (paint,
		  ft_font,
		  font,
		  paint_funcs, paint_data,
		  palette, foreground);

    if (pop_clip)
      paint_funcs->pop_clip (paint_data);

    return true;
  }

  /* COLRv0 */
  iterator.p  = NULL;
  have_layers = FT_Get_Color_Glyph_Layer(ft_face,
					 gid,
					 &layer_glyph_index,
					 &layer_color_index,
					 &iterator);

  if (palette && have_layers)
  {
    do
    {
      hb_bool_t is_foreground = true;
      hb_color_t color = foreground;

      if ( layer_color_index != 0xFFFF )
      {
	FT_Color layer_color = palette[layer_color_index];
	color = HB_COLOR (layer_color.blue,
			  layer_color.green,
			  layer_color.red,
			  layer_color.alpha);
	is_foreground = false;
      }

      ft_font->lock.unlock ();
      paint_funcs->push_clip_glyph (paint_data, layer_glyph_index, font);
      ft_font->lock.lock ();
      paint_funcs->color (paint_data, is_foreground, color);
      paint_funcs->pop_clip (paint_data);

    } while (FT_Get_Color_Glyph_Layer(ft_face,
				      gid,
				      &layer_glyph_index,
				      &layer_color_index,
				      &iterator));
    return true;
  }

  return false;
}
#endif


#endif /* HB_FT_COLR_HH */
