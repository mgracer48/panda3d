// Filename: graphicsStateGuardian.h
// Created by:  drose (02Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef GRAPHICSSTATEGUARDIAN_H
#define GRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"

#include "frameBufferProperties.h"
#include "preparedGraphicsObjects.h"
#include "lens.h"
#include "graphicsStateGuardianBase.h"
#include "graphicsThreadingModel.h"
#include "graphicsPipe.h"
#include "sceneSetup.h"
#include "displayRegion.h"
#include "luse.h"
#include "coordinateSystem.h"
#include "factory.h"
#include "pStatCollector.h"
#include "transformState.h"
#include "renderState.h"
#include "light.h"
#include "planeNode.h"
#include "config_display.h"
#include "geomMunger.h"
#include "geomVertexData.h"
#include "pnotify.h"
#include "pvector.h"
#include "attribSlots.h"
#include "shaderContext.h"
#include "bitMask.h"
#include "texture.h"
#include "occlusionQueryContext.h"
#include "stencilRenderStates.h"

class DrawableRegion;
class GraphicsEngine;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsStateGuardian
// Description : Encapsulates all the communication with a particular
//               instance of a given rendering backend.  Tries to
//               guarantee that redundant state-change requests are
//               not issued (hence "state guardian").
//
//               There will be one of these objects for each different
//               graphics context active in the system.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY GraphicsStateGuardian : public GraphicsStateGuardianBase {
  //
  // Interfaces all GSGs should have
  //
public:
  GraphicsStateGuardian(CoordinateSystem internal_coordinate_system,
                        GraphicsPipe *pipe);
  virtual ~GraphicsStateGuardian();

PUBLISHED:

  enum ShaderModel
  {
    SM_00,
    SM_11,
    SM_20,
    SM_2X,
    SM_30,
    SM_40,
  };

  INLINE void release_all();
  INLINE int release_all_textures();
  INLINE int release_all_geoms();
  INLINE int release_all_vertex_buffers();
  INLINE int release_all_index_buffers();

  INLINE void set_active(bool active);
  INLINE bool is_active() const;
  INLINE bool is_valid() const;
  INLINE bool needs_reset() const;

  INLINE GraphicsPipe *get_pipe() const;
  INLINE GraphicsEngine *get_engine() const;
  INLINE const GraphicsThreadingModel &get_threading_model() const;

  virtual INLINE bool prefers_triangle_strips() const;
  virtual INLINE int get_max_vertices_per_array() const;
  virtual INLINE int get_max_vertices_per_primitive() const;

  INLINE int get_max_texture_stages() const;
  INLINE int get_max_texture_dimension() const;
  INLINE int get_max_3d_texture_dimension() const;
  INLINE int get_max_cube_map_dimension() const;

  INLINE bool get_supports_texture_combine() const;
  INLINE bool get_supports_texture_saved_result() const;
  INLINE bool get_supports_texture_dot3() const;

  INLINE bool get_supports_3d_texture() const;
  INLINE bool get_supports_cube_map() const;
  INLINE bool get_supports_tex_non_pow2() const;

  INLINE bool get_supports_compressed_texture() const;
  INLINE bool get_supports_compressed_texture_format(Texture::CompressionMode compression) const;

  INLINE int get_max_lights() const;
  INLINE int get_max_clip_planes() const;

  INLINE int get_max_vertex_transforms() const;
  INLINE int get_max_vertex_transform_indices() const;

  INLINE bool get_copy_texture_inverted() const;
  virtual bool get_supports_multisample() const;
  INLINE bool get_supports_generate_mipmap() const;
  INLINE bool get_supports_render_texture() const;
  INLINE bool get_supports_depth_texture() const;
  INLINE bool get_supports_depth_stencil() const;
  INLINE bool get_supports_shadow_filter() const;
  INLINE bool get_supports_basic_shaders() const;
  INLINE bool get_supports_two_sided_stencil() const;

  INLINE int get_shader_model() const;
  INLINE void set_shader_model(int shader_model);

  virtual int get_supported_geom_rendering() const;

  INLINE bool get_color_scale_via_lighting() const;
  INLINE bool get_alpha_scale_via_texture() const;
  INLINE bool get_alpha_scale_via_texture(const TextureAttrib *tex_attrib) const;

  INLINE static TextureStage *get_alpha_scale_texture_stage();

  void set_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_coordinate_system() const;
  virtual CoordinateSystem get_internal_coordinate_system() const;

  virtual PreparedGraphicsObjects *get_prepared_objects();

  virtual bool set_gamma(float gamma);
  float get_gamma(float gamma);
  
public:
  bool set_scene(SceneSetup *scene_setup);
  virtual SceneSetup *get_scene() const;

  virtual TextureContext *prepare_texture(Texture *tex);
  virtual void release_texture(TextureContext *tc);
  virtual bool extract_texture_data(Texture *tex);

  virtual GeomContext *prepare_geom(Geom *geom);
  virtual void release_geom(GeomContext *gc);

  virtual ShaderContext *prepare_shader(ShaderExpansion *shader);
  virtual void release_shader(ShaderContext *sc);

  virtual VertexBufferContext *prepare_vertex_buffer(GeomVertexArrayData *data);
  virtual void release_vertex_buffer(VertexBufferContext *vbc);

  virtual IndexBufferContext *prepare_index_buffer(GeomPrimitive *data);
  virtual void release_index_buffer(IndexBufferContext *ibc);

  virtual bool get_supports_occlusion_query() const;
  virtual void begin_occlusion_query();
  virtual PT(OcclusionQueryContext) end_occlusion_query();

  virtual PT(GeomMunger) get_geom_munger(const RenderState *state,
                                         Thread *current_thread);
  virtual PT(GeomMunger) make_geom_munger(const RenderState *state,
                                          Thread *current_thread);

  virtual void set_state_and_transform(const RenderState *state,
                                       const TransformState *transform);

  virtual float compute_distance_to(const LPoint3f &point) const;

  virtual void set_color_clear_value(const Colorf &value);
  virtual void set_depth_clear_value(const float value);
  virtual void do_clear(const RenderBuffer &buffer)=0;

  void clear(DrawableRegion *clearable);

  const LMatrix4f *fetch_specified_value(ShaderExpansion::ShaderMatSpec &spec, bool altered);
  const LMatrix4f *fetch_specified_part(ShaderExpansion::ShaderMatInput input, InternalName *name, LMatrix4f &t);

  virtual void prepare_display_region(DisplayRegionPipelineReader *dr,
                                      Lens::StereoChannel stereo_channel);

  virtual CPT(TransformState) calc_projection_mat(const Lens *lens);
  virtual bool prepare_lens();

  virtual bool begin_frame(Thread *current_thread);
  virtual bool begin_scene();
  virtual void end_scene();
  virtual void end_frame(Thread *current_thread);

  void set_current_properties(const FrameBufferProperties *properties);

  virtual bool depth_offset_decals();
  virtual CPT(RenderState) begin_decal_base_first();
  virtual CPT(RenderState) begin_decal_nested();
  virtual CPT(RenderState) begin_decal_base_second();
  virtual void finish_decal();

  virtual bool begin_draw_primitives(const GeomPipelineReader *geom_reader,
                                     const GeomMunger *munger,
                                     const GeomVertexDataPipelineReader *data_reader,
                                     bool force);
  virtual bool draw_triangles(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_tristrips(const GeomPrimitivePipelineReader *reader,
                              bool force);
  virtual bool draw_trifans(const GeomPrimitivePipelineReader *reader,
                            bool force);
  virtual bool draw_lines(const GeomPrimitivePipelineReader *reader,
                          bool force);
  virtual bool draw_linestrips(const GeomPrimitivePipelineReader *reader,
                               bool force);
  virtual bool draw_points(const GeomPrimitivePipelineReader *reader,
                           bool force);
  virtual void end_draw_primitives();

  INLINE bool reset_if_new();
  INLINE void mark_new();
  virtual void reset();

  INLINE CPT(TransformState) get_external_transform() const;
  INLINE CPT(TransformState) get_internal_transform() const;

  RenderBuffer get_render_buffer(int buffer_type, const FrameBufferProperties &prop);

  INLINE const DisplayRegion *get_current_display_region() const;
  INLINE Lens::StereoChannel get_current_stereo_channel() const;
  INLINE const Lens *get_current_lens() const;

  virtual const TransformState *get_cs_transform() const;
  INLINE const TransformState *get_inv_cs_transform() const;

  void do_issue_clip_plane();
  void do_issue_color();
  void do_issue_color_scale();
  void do_issue_light();

  virtual void bind_light(PointLight *light_obj, const NodePath &light,
                          int light_id);
  virtual void bind_light(DirectionalLight *light_obj, const NodePath &light,
                          int light_id);
  virtual void bind_light(Spotlight *light_obj, const NodePath &light,
                          int light_id);

  INLINE void set_stencil_clear_value(unsigned int stencil_clear_value);
  INLINE unsigned int get_stencil_clear_value();

  static void create_gamma_table (float gamma, unsigned short *red_table, unsigned short *green_table, unsigned short *blue_table);

#ifdef DO_PSTATS
  static void init_frame_pstats();
#endif

protected:
  virtual void enable_lighting(bool enable);
  virtual void set_ambient_light(const Colorf &color);
  virtual void enable_light(int light_id, bool enable);
  virtual void begin_bind_lights();
  virtual void end_bind_lights();

  virtual void enable_clip_planes(bool enable);
  virtual void enable_clip_plane(int plane_id, bool enable);
  virtual void begin_bind_clip_planes();
  virtual void bind_clip_plane(const NodePath &plane, int plane_id);
  virtual void end_bind_clip_planes();

  void determine_effective_texture();

  virtual void free_pointers();
  virtual void close_gsg();
  void panic_deactivate();

  void determine_light_color_scale();

  static CPT(RenderState) get_unlit_state();
  static CPT(RenderState) get_unclipped_state();
  static CPT(RenderState) get_untextured_state();

protected:
  PT(SceneSetup) _scene_null;
  PT(SceneSetup) _scene_setup;

  AttribSlots _state;
  AttribSlots _target;
  CPT(RenderState) _state_rs;
  CPT(RenderState) _target_rs;
  CPT(TransformState) _internal_transform;

  // The current TextureAttrib is a special case; we may further
  // restrict it (according to graphics cards limits) or extend it
  // (according to ColorScaleAttribs in effect) beyond what is
  // specifically requested in the scene graph.
  CPT(TextureAttrib) _effective_texture;
  CPT(TexGenAttrib) _effective_tex_gen;

  // These are set by begin_draw_primitives(), and are only valid
  // between begin_draw_primitives() and end_draw_primitives().
  CPT(GeomMunger) _munger;
  const GeomVertexDataPipelineReader *_data_reader;

  unsigned int _color_write_mask;
  Colorf _color_clear_value;
  float _depth_clear_value;
  unsigned int _stencil_clear_value;
  Colorf _accum_clear_value;

  CPT(DisplayRegion) _current_display_region;
  Lens::StereoChannel _current_stereo_channel;
  CPT(Lens) _current_lens;
  CPT(TransformState) _projection_mat;
  CPT(TransformState) _projection_mat_inv;
  const FrameBufferProperties *_current_properties;

  CoordinateSystem _coordinate_system;
  CoordinateSystem _internal_coordinate_system;
  CPT(TransformState) _cs_transform;
  CPT(TransformState) _inv_cs_transform;

  Colorf _scene_graph_color;
  bool _has_scene_graph_color;
  bool _transform_stale;
  bool _color_blend_involves_color_scale;
  bool _texture_involves_color_scale;
  bool _vertex_colors_enabled;
  bool _lighting_enabled;
  bool _clip_planes_enabled;
  bool _color_scale_enabled;
  LVecBase4f _current_color_scale;

  bool _has_material_force_color;
  Colorf _material_force_color;
  LVecBase4f _light_color_scale;
  bool _has_texture_alpha_scale;

  bool _tex_gen_modifies_mat;
  bool _tex_gen_point_sprite;
  int _last_max_stage_index;

  bool _needs_reset;
  bool _is_valid;
  bool _closing_gsg;
  bool _active;

  PT(PreparedGraphicsObjects) _prepared_objects;

  bool _prefers_triangle_strips;
  int _max_vertices_per_array;
  int _max_vertices_per_primitive;

  int _max_texture_stages;
  int _max_texture_dimension;
  int _max_3d_texture_dimension;
  int _max_cube_map_dimension;

  bool _supports_texture_combine;
  bool _supports_texture_saved_result;
  bool _supports_texture_dot3;

  bool _supports_3d_texture;
  bool _supports_cube_map;
  bool _supports_tex_non_pow2;

  bool _supports_compressed_texture;
  BitMask32 _compressed_texture_formats;

  int _max_lights;
  int _max_clip_planes;

  int _max_vertex_transforms;
  int _max_vertex_transform_indices;

  bool _supports_occlusion_query;
  PT(OcclusionQueryContext) _current_occlusion_query;

  bool _copy_texture_inverted;
  bool _supports_multisample;
  bool _supports_generate_mipmap;
  bool _supports_render_texture;
  bool _supports_depth_texture;
  bool _supports_depth_stencil;
  bool _supports_shadow_filter;
  bool _supports_basic_shaders;

  bool _supports_stencil_wrap;
  bool _supports_two_sided_stencil;

  int _supported_geom_rendering;
  bool _color_scale_via_lighting;
  bool _alpha_scale_via_texture;

  int _stereo_buffer_mask;

  StencilRenderStates *_stencil_render_states;

  int _auto_detect_shader_model;
  int _shader_model;

  static PT(TextureStage) _alpha_scale_texture_stage;

  ShaderExpansion::ShaderCaps _shader_caps;

  float _gamma;
  
public:
  // Statistics
  static PStatCollector _vertex_buffer_switch_pcollector;
  static PStatCollector _index_buffer_switch_pcollector;
  static PStatCollector _load_vertex_buffer_pcollector;
  static PStatCollector _load_index_buffer_pcollector;
  static PStatCollector _create_vertex_buffer_pcollector;
  static PStatCollector _create_index_buffer_pcollector;
  static PStatCollector _load_texture_pcollector;
  static PStatCollector _data_transferred_pcollector;
  static PStatCollector _texmgrmem_total_pcollector;
  static PStatCollector _texmgrmem_resident_pcollector;
  static PStatCollector _primitive_batches_pcollector;
  static PStatCollector _primitive_batches_tristrip_pcollector;
  static PStatCollector _primitive_batches_trifan_pcollector;
  static PStatCollector _primitive_batches_tri_pcollector;
  static PStatCollector _primitive_batches_other_pcollector;
  static PStatCollector _vertices_tristrip_pcollector;
  static PStatCollector _vertices_trifan_pcollector;
  static PStatCollector _vertices_tri_pcollector;
  static PStatCollector _vertices_other_pcollector;
  static PStatCollector _vertices_indexed_tristrip_pcollector;
  static PStatCollector _state_pcollector;
  static PStatCollector _transform_state_pcollector;
  static PStatCollector _texture_state_pcollector;
  static PStatCollector _draw_primitive_pcollector;
  static PStatCollector _draw_set_state_pcollector;
  static PStatCollector _clear_pcollector;
  static PStatCollector _flush_pcollector;
  static PStatCollector _wait_occlusion_pcollector;

  // A whole slew of collectors to measure the cost of individual
  // state changes.  These are disabled by default.
  static PStatCollector _draw_set_state_transform_pcollector;
  static PStatCollector _draw_set_state_alpha_test_pcollector;
  static PStatCollector _draw_set_state_antialias_pcollector;
  static PStatCollector _draw_set_state_clip_plane_pcollector;
  static PStatCollector _draw_set_state_color_pcollector;
  static PStatCollector _draw_set_state_cull_face_pcollector;
  static PStatCollector _draw_set_state_depth_offset_pcollector;
  static PStatCollector _draw_set_state_depth_test_pcollector;
  static PStatCollector _draw_set_state_depth_write_pcollector;
  static PStatCollector _draw_set_state_render_mode_pcollector;
  static PStatCollector _draw_set_state_rescale_normal_pcollector;
  static PStatCollector _draw_set_state_shade_model_pcollector;
  static PStatCollector _draw_set_state_blending_pcollector;
  static PStatCollector _draw_set_state_shader_pcollector;
  static PStatCollector _draw_set_state_texture_pcollector;
  static PStatCollector _draw_set_state_tex_matrix_pcollector;
  static PStatCollector _draw_set_state_tex_gen_pcollector;
  static PStatCollector _draw_set_state_material_pcollector;
  static PStatCollector _draw_set_state_light_pcollector;
  static PStatCollector _draw_set_state_stencil_pcollector;
  static PStatCollector _draw_set_state_fog_pcollector;

private:
  int _num_lights_enabled;
  int _num_clip_planes_enabled;

  PT(GraphicsPipe) _pipe;
  GraphicsEngine *_engine;
  GraphicsThreadingModel _threading_model;

public:
  void traverse_prepared_textures(bool (*pertex_callbackfn)(TextureContext *,void *),void *callback_arg);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    GraphicsStateGuardianBase::init_type();
    register_type(_type_handle, "GraphicsStateGuardian",
                  GraphicsStateGuardianBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsPipe;
  friend class GraphicsWindow;
  friend class GraphicsEngine;
};

#include "graphicsStateGuardian.I"

#endif
